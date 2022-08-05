/**
 ******************************************************************************
 * File Name          : motionfx_server_app.c
 * Description        : Handle SW/Sensor Data Fusion and ECompass Service/Char
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "ble.h"
#include "dbg_trace.h"

#include "motenv_server_app.h"
#include "motionfx_server_app.h"
#include "config_server_app.h"

#include "MotionFX_Manager.h"

/* Private defines -----------------------------------------------------------*/
/**
 * @brief Define The transmission interval in Multiple of 10ms for quaternions
 */
#define QUAT_UPDATE_MUL_10MS (3)
/**
 * @brief Define How Many quaterions you want to trasmit (from 1 to 3)
 */
#define SEND_N_QUATERNIONS (3)

#define MOTIONFX_ENGINE_DELTATIME       0.01f
/**
 * @brief Algorithm period [ms]
 */
#define MOTIONFX_ALGO_PERIOD            (10)

#define VALUE_LEN_QUAT          (2+6*SEND_N_QUATERNIONS)
#define VALUE_LEN_ECOMPASS      (2+2)

/* Private typedef -----------------------------------------------------------*/
/**
 * @brief  SW/Sensor Data Fusion Service/Char Context structure definition
 */
typedef struct
{
  uint8_t  QuatNotificationStatus;
  uint8_t  ECompassNotificationStatus;

  IKS01A3_MOTION_SENSOR_Axes_t MAG_Offset;
  uint32_t MagTimeStamp;
  uint8_t MagCalStatus;

  IKS01A3_MOTION_SENSOR_Axes_t quat_axes[SEND_N_QUATERNIONS];
  uint16_t Angle; /* ECompass */

} MOTIONFX_Server_App_Context_t;

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

PLACE_IN_SECTION("BLE_APP_CONTEXT") static MOTIONFX_Server_App_Context_t MOTIONFX_Server_App_Context;

/* Global variables ----------------------------------------------------------*/
extern uint8_t manuf_data[14];

/* Private function prototypes -----------------------------------------------*/
static void MagCalibTest(void);
static void ComputeQuaternions(void);
static void Quat_Update(IKS01A3_MOTION_SENSOR_Axes_t *data);
static void ECompass_Update(uint16_t Angle);

static void Accelero_Sensor_Handler(IKS01A3_MOTION_SENSOR_Axes_t *ACC_Value);
static void Gyro_Sensor_Handler(IKS01A3_MOTION_SENSOR_Axes_t *GYR_Value);
static void Magneto_Sensor_Handler(IKS01A3_MOTION_SENSOR_Axes_t *MAG_Value);

/* Functions Definition ------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Init the SW/Sensor Data Fusion Service/Char Context
 *         and update the ADV data accordingly
 * @param  None
 * @retval None
 */
void MOTIONFX_Context_Init(void)
{
  MOTIONFX_Server_App_Context.MagTimeStamp = 0;
  MOTIONFX_Server_App_Context.MagCalStatus = 0;

  /* Sensor Fusion API initialization function */
  MotionFX_manager_init();
  MotionFX_manager_start_9X();

  /* Enable magnetometer calibration */
  MagCalibTest();

  /* Update BLE ADV field (MotionFx) */
  manuf_data[6] |= 0x01; /* Sensor fusion*/
  manuf_data[7] |= 0x40; /* ECompass*/

  MOTIONFX_Set_Quat_Notification_Status(0);
  MOTIONFX_Set_ECompass_Notification_Status(0);
}

/**
 * @brief  Set the notification status (enabled/disabled) for Sensor Data Fusion
 * @param  status The new notification status
 * @retval None
 */
void MOTIONFX_Set_Quat_Notification_Status(uint8_t status)
{
  MOTIONFX_Server_App_Context.QuatNotificationStatus = status;
}

/**
 * @brief  Set the notification status (enabled/disabled) for ECompass
 * @param  status The new notification status
 * @retval None
 */
void MOTIONFX_Set_ECompass_Notification_Status(uint8_t status)
{
  MOTIONFX_Server_App_Context.ECompassNotificationStatus = status;
}

/**
 * @brief  Send a notification for Quaternions (Sensor Data Fusion case)
 * @param  None
 * @retval None
 */
void MOTIONFX_Send_Quat_Notification_Task(void)
{
  ComputeQuaternions();
}

/**
 * @brief  Send a notification for Quaternions (ECompass case)
 * @param  None
 * @retval None
 */
void MOTIONFX_Send_ECompass_Notification_Task(void)
{
  ComputeQuaternions();
}

/**
 * @brief  Return the Magneto Calibration status
 * @param  None
 * @retval Magneto Calibration status
 */
uint8_t MOTIONFX_Get_MagCalStatus(void)
{
  return MOTIONFX_Server_App_Context.MagCalStatus;
}

/**
 * @brief  Return the Magneto Calibration offset
 * @param  None
 * @retval Magneto Calibration offset
 */
IKS01A3_MOTION_SENSOR_Axes_t *MOTIONFX_Get_MAG_Offset(void)
{
  return &(MOTIONFX_Server_App_Context.MAG_Offset);
}

/**
 * @brief  Force Magneto Calibration
 * @param  None
 * @retval None
 */
void MOTIONFX_ReCalibration(void)
{
  /* Reset Magneto Calibration */
  MOTIONFX_Server_App_Context.MagCalStatus = 0;

  CONFIG_Send_Notification(FEATURE_MASK_SENSORFUSION_SHORT, W2ST_COMMAND_CAL_STATUS, 0);
  CONFIG_Send_Notification(FEATURE_MASK_ECOMPASS, W2ST_COMMAND_CAL_STATUS, 0);

  /* Enable Magneto calibration */
  MotionFX_manager_MagCal_start(MOTIONFX_ALGO_PERIOD);
}

/* Private functions ---------------------------------------------------------*/

/** 
 * @brief  MotionFX Working function
 * @param  None
 * @retval None
 */
static void ComputeQuaternions(void)
{
  MFX_input_t data_in;
  MFX_input_t *pdata_in = &data_in;
  MFX_output_t data_out;
  MFX_output_t *pdata_out = &data_out;

  static int32_t CounterFX = 0;
  static int32_t CounterEC = 0;
  IKS01A3_MOTION_SENSOR_Axes_t ACC_Value;
  IKS01A3_MOTION_SENSOR_Axes_t GYR_Value;
  IKS01A3_MOTION_SENSOR_Axes_t MAG_Value;

   /* Increment the Counter */
  if(MOTIONFX_Server_App_Context.QuatNotificationStatus)
  {
    CounterFX++;
  }
  else if(MOTIONFX_Server_App_Context.ECompassNotificationStatus)
  {
    CounterEC++;
  }

  /* Read the Acc values */
  Accelero_Sensor_Handler(&ACC_Value);
  /* Read the Gyro values */
  Gyro_Sensor_Handler(&GYR_Value);
  /* Read the Magneto values */
  Magneto_Sensor_Handler(&MAG_Value);

  data_in.gyro[0] = (float)GYR_Value.x * FROM_MDPS_TO_DPS;
  data_in.gyro[1] = (float)GYR_Value.y * FROM_MDPS_TO_DPS;
  data_in.gyro[2] = (float)GYR_Value.z * FROM_MDPS_TO_DPS;

  data_in.acc[0] = (float)ACC_Value.x * FROM_MG_TO_G;
  data_in.acc[1] = (float)ACC_Value.y * FROM_MG_TO_G;
  data_in.acc[2] = (float)ACC_Value.z * FROM_MG_TO_G;

  data_in.mag[0] = (float)MAG_Value.x * FROM_MGAUSS_TO_UT50;
  data_in.mag[1] = (float)MAG_Value.y * FROM_MGAUSS_TO_UT50;
  data_in.mag[2] = (float)MAG_Value.z * FROM_MGAUSS_TO_UT50;

  /* Run Sensor Fusion algorithm */
  MotionFX_manager_run(pdata_in, pdata_out, MOTIONFX_ENGINE_DELTATIME);

  if(MOTIONFX_Server_App_Context.QuatNotificationStatus)
  {
    int32_t QuaternionNumber = (CounterFX>SEND_N_QUATERNIONS) ? (SEND_N_QUATERNIONS-1) : (CounterFX-1);

    /* Scaling quaternions data by a factor of 10000
      (Scale factor to handle float during data transfer BT) */

    /* Save the quaternions values */
    if(pdata_out->quaternion_9X[3] < 0)
    {
      MOTIONFX_Server_App_Context.quat_axes[QuaternionNumber].x = (int32_t)(pdata_out->quaternion_9X[0] * (-10000));
      MOTIONFX_Server_App_Context.quat_axes[QuaternionNumber].y = (int32_t)(pdata_out->quaternion_9X[1] * (-10000));
      MOTIONFX_Server_App_Context.quat_axes[QuaternionNumber].z = (int32_t)(pdata_out->quaternion_9X[2] * (-10000));
    }
    else
    {
      MOTIONFX_Server_App_Context.quat_axes[QuaternionNumber].x = (int32_t)(pdata_out->quaternion_9X[0] * 10000);
      MOTIONFX_Server_App_Context.quat_axes[QuaternionNumber].y = (int32_t)(pdata_out->quaternion_9X[1] * 10000);
      MOTIONFX_Server_App_Context.quat_axes[QuaternionNumber].z = (int32_t)(pdata_out->quaternion_9X[2] * 10000);
    }

    /* Every QUAT_UPDATE_MUL_10MS*10 mSeconds Send Quaternions informations via bluetooth */
    if(CounterFX == QUAT_UPDATE_MUL_10MS)
    {
      Quat_Update(MOTIONFX_Server_App_Context.quat_axes);
      CounterFX = 0;
    }
  }
  else if(MOTIONFX_Server_App_Context.ECompassNotificationStatus)
  {
    /* E-Compass Updated every 0.1 Seconds*/
    if(CounterEC == 10)
    {
      MOTIONFX_Server_App_Context.Angle = (uint16_t)trunc(100*pdata_out->heading_9X);
      ECompass_Update(MOTIONFX_Server_App_Context.Angle);
      CounterEC = 0;
    }
  }
}

/**
 * @brief  Handle the ACC axes data getting
 * @param  ACC_Value Accelerometer value to be read
 * @retval None
 */
static void Accelero_Sensor_Handler(IKS01A3_MOTION_SENSOR_Axes_t *ACC_Value)
{
  (void)IKS01A3_MOTION_SENSOR_GetAxes(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, ACC_Value);
}

/**
 * @brief  Handle the GYRO axes data getting
 * @param  GYR_Value Gyro value to be read
 * @retval None
 */
static void Gyro_Sensor_Handler(IKS01A3_MOTION_SENSOR_Axes_t *GYR_Value)
{
  (void)IKS01A3_MOTION_SENSOR_GetAxes(IKS01A3_LSM6DSO_0, MOTION_GYRO, GYR_Value);
}

/**
 * @brief  Handle the MAGNETO axes data getting
 * @param  MAG_Value Magneto value to be read
 * @retval None
 */
static void Magneto_Sensor_Handler(IKS01A3_MOTION_SENSOR_Axes_t *MAG_Value)
{
  float ans_float;
  MFX_MagCal_input_t mag_data_in;
  MFX_MagCal_output_t mag_data_out;
  static int32_t calibIndex = 0;

  IKS01A3_MOTION_SENSOR_GetAxes(IKS01A3_LIS2MDL_0, MOTION_MAGNETO, MAG_Value);

  if (MOTIONFX_Server_App_Context.MagCalStatus == 0U)
  {
    /* Run Compass Calibration @ 25Hz */
    calibIndex++;
    if (calibIndex == 4)
    {
      calibIndex = 0;
      mag_data_in.mag[0] = (float)MAG_Value->x * FROM_MGAUSS_TO_UT50;
      mag_data_in.mag[1] = (float)MAG_Value->y * FROM_MGAUSS_TO_UT50;
      mag_data_in.mag[2] = (float)MAG_Value->z * FROM_MGAUSS_TO_UT50;

      mag_data_in.time_stamp = (int)MOTIONFX_Server_App_Context.MagTimeStamp;
      MOTIONFX_Server_App_Context.MagTimeStamp += (uint32_t)MOTIONFX_ALGO_PERIOD;

      MotionFX_manager_MagCal_run(&mag_data_in, &mag_data_out);

      if (mag_data_out.cal_quality == MFX_MAGCALGOOD)
      {
        MOTIONFX_Server_App_Context.MagCalStatus = 1;

        ans_float = (mag_data_out.hi_bias[0] * FROM_UT50_TO_MGAUSS);
        MOTIONFX_Server_App_Context.MAG_Offset.x = (int32_t)ans_float;
        ans_float = (mag_data_out.hi_bias[1] * FROM_UT50_TO_MGAUSS);
        MOTIONFX_Server_App_Context.MAG_Offset.y = (int32_t)ans_float;
        ans_float = (mag_data_out.hi_bias[2] * FROM_UT50_TO_MGAUSS);
        MOTIONFX_Server_App_Context.MAG_Offset.z = (int32_t)ans_float;

        /* Disable magnetometer calibration */
        MotionFX_manager_MagCal_stop(MOTIONFX_ALGO_PERIOD);
      }

      if(MOTIONFX_Server_App_Context.MagCalStatus == 1)
      {
        /* Notifications of Compass Calibration */
        CONFIG_Send_Notification(FEATURE_MASK_SENSORFUSION_SHORT, W2ST_COMMAND_CAL_STATUS, 100);
        CONFIG_Send_Notification(FEATURE_MASK_ECOMPASS, W2ST_COMMAND_CAL_STATUS, 100);
      }
      else
      {
#if(CFG_DEBUG_APP_TRACE != 0)
    //APP_DBG_MSG("-- MOTIONFX APPLICATION SERVER : Magneto Calibration quality is not good\r\n");
#endif
      }
    }
  }
  else
  {
    calibIndex = 0;
  }

  MAG_Value->x = (int32_t)(MAG_Value->x - MOTIONFX_Server_App_Context.MAG_Offset.x);
  MAG_Value->y = (int32_t)(MAG_Value->y - MOTIONFX_Server_App_Context.MAG_Offset.y);
  MAG_Value->z = (int32_t)(MAG_Value->z - MOTIONFX_Server_App_Context.MAG_Offset.z);
}

/**
 * @brief  Update quaternions characteristic value
 * @param  data Structure containing the quaterions
 * @retval None
 */
static void Quat_Update(IKS01A3_MOTION_SENSOR_Axes_t *data)
{
  uint8_t value[VALUE_LEN_QUAT];

  /* Timestamp */
  STORE_LE_16(value, (HAL_GetTick()>>3));

  STORE_LE_16(value+2,data[0].x);
  STORE_LE_16(value+4,data[0].y);
  STORE_LE_16(value+6,data[0].z);

  STORE_LE_16(value+8 ,data[1].x);
  STORE_LE_16(value+10,data[1].y);
  STORE_LE_16(value+12,data[1].z);

  STORE_LE_16(value+14,data[2].x);
  STORE_LE_16(value+16,data[2].y);
  STORE_LE_16(value+18,data[2].z);

  if(MOTIONFX_Server_App_Context.QuatNotificationStatus)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    //APP_DBG_MSG("-- MOTIONFX APPLICATION SERVER : NOTIFY CLIENT WITH NEW QUAT PARAMETER VALUE \n ");
    //APP_DBG_MSG(" \n\r");
#endif
    MOTENV_STM_App_Update_Char(MOTION_FX_CHAR_UUID, VALUE_LEN_QUAT, (uint8_t *)&value);
  }
  else
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTIONFX APPLICATION SERVER : CAN'T INFORM CLIENT - NOTIFICATION DISABLED\n ");
#endif
  }

  return;
}

/**
 * @brief  Update E-Compass characteristic value
 * @param  Angle To Magnetic North in cents of degree [0.00 -> 259,99]
 * @retval None
 */
static void ECompass_Update(uint16_t Angle)
{
  uint8_t value[VALUE_LEN_ECOMPASS];

  /* Timestamp */
  STORE_LE_16(value, (HAL_GetTick()>>3));

  STORE_LE_16(value+2, Angle);
  
  if(MOTIONFX_Server_App_Context.ECompassNotificationStatus)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    //APP_DBG_MSG("-- MOTIONFX APPLICATION SERVER : NOTIFY CLIENT WITH NEW ECOMPASS PARAMETER VALUE \n ");
    //APP_DBG_MSG(" \n\r");
#endif
    MOTENV_STM_App_Update_Char(ECOMPASS_CHAR_UUID, VALUE_LEN_ECOMPASS, (uint8_t *)&value);
  }
  else
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTIONFX APPLICATION SERVER : CAN'T INFORM CLIENT - NOTIFICATION DISABLED\n ");
#endif
  }
}

/**
 * @brief  Test if calibration data are available
 * @param  None
 * @retval None
 */
static void MagCalibTest(void)
{
  MFX_MagCal_output_t mag_cal_test;
  
  /* Recall the calibration Credential saved */
  MotionFX_manager_MagCal_start(MOTIONFX_ALGO_PERIOD);
  MotionFX_MagCal_getParams(&mag_cal_test);
    
  if(mag_cal_test.cal_quality == MFX_MAGCALGOOD)
  {
    MOTIONFX_Server_App_Context.MAG_Offset.x = (int32_t) (mag_cal_test.hi_bias[0] * FROM_UT50_TO_MGAUSS);
    MOTIONFX_Server_App_Context.MAG_Offset.y = (int32_t) (mag_cal_test.hi_bias[1] * FROM_UT50_TO_MGAUSS);
    MOTIONFX_Server_App_Context.MAG_Offset.z = (int32_t) (mag_cal_test.hi_bias[2] * FROM_UT50_TO_MGAUSS);
    
    MOTIONFX_Server_App_Context.MagCalStatus = 1;
    
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTIONFX APPLICATION SERVER : Magneto Calibration Read\r\n");
#endif
  }
  else
  {
    MOTIONFX_Server_App_Context.MagCalStatus = 0;
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTIONFX APPLICATION SERVER : Magneto Calibration quality is not good\r\n");
#endif
  }
  
  if(!MOTIONFX_Server_App_Context.MagCalStatus)
  {
    MOTIONFX_Server_App_Context.MAG_Offset.x = 0;
    MOTIONFX_Server_App_Context.MAG_Offset.y = 0;
    MOTIONFX_Server_App_Context.MAG_Offset.z = 0;
  }
}

 /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
