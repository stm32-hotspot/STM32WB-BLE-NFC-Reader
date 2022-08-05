/**
 ******************************************************************************
 * File Name          : motion_server_app.c
 * Description        : Handle HW/Motion (Acc/Gyro/Mag) Service/Char
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
#include "motion_server_app.h"
#include "motionfx_server_app.h"

#include "iks01a3_motion_sensors.h"

/* Private defines -----------------------------------------------------------*/
#define ACC_BYTES               (2)
#define GYRO_BYTES              (2)
#define MAG_BYTES               (2)

#define VALUE_LEN_MOTION        (2+3*ACC_BYTES+3*GYRO_BYTES+3*MAG_BYTES)

/* Private typedef -----------------------------------------------------------*/

/**
 * @brief  Motion Service/Char Context structure definition
 */
typedef struct
{
  uint8_t  NotificationStatus;

  IKS01A3_MOTION_SENSOR_Axes_t acceleration;
  IKS01A3_MOTION_SENSOR_Axes_t angular_velocity;
  IKS01A3_MOTION_SENSOR_Axes_t magnetic_field;
  uint8_t hasAcc;
  uint8_t hasGyro;
  uint8_t hasMag;
//  float sensitivity_Mul;
} MOTION_Server_App_Context_t;

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static IKS01A3_MOTION_SENSOR_Capabilities_t MotionCapabilities[IKS01A3_MOTION_INSTANCES_NBR];

PLACE_IN_SECTION("BLE_APP_CONTEXT") static MOTION_Server_App_Context_t MOTION_Server_App_Context;

/* Global variables ----------------------------------------------------------*/
extern uint8_t manuf_data[14];

/* Private function prototypes -----------------------------------------------*/
static void MOTION_Handle_Sensor(void);
static void MOTION_GetCaps(void);

/* Functions Definition ------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Init the HW/Motion Service/Char Context
 * @param  None
 * @retval None
 */
void MOTION_Context_Init(void)
{
  /* Motion Sensors */
  (void)IKS01A3_MOTION_SENSOR_Init(IKS01A3_LSM6DSO_0, MOTION_ACCELERO | MOTION_GYRO);
  (void)IKS01A3_MOTION_SENSOR_Init(IKS01A3_LIS2DW12_0, MOTION_ACCELERO);
  (void)IKS01A3_MOTION_SENSOR_Init(IKS01A3_LIS2MDL_0, MOTION_MAGNETO);

  MOTION_Server_App_Context.hasAcc = 0;
  MOTION_Server_App_Context.hasGyro = 0;
  MOTION_Server_App_Context.hasMag = 0;

  MOTION_Set2G_Accelerometer_FullScale();
  MOTION_Set_Notification_Status(0);

  /* Check Motion caps */
  MOTION_GetCaps();
}

/**
  * @brief  Set the ACC FS to 2g
  * @param  None
  * @retval None
  */
void MOTION_Set2G_Accelerometer_FullScale(void)
{
  /* Set Full Scale to +/-2g */
  IKS01A3_MOTION_SENSOR_SetFullScale(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, 2);
}

/**
 * @brief  Set the notification status (enabled/disabled)
 * @param  status The new notification status
 * @retval None
 */
void MOTION_Set_Notification_Status(uint8_t status)
{
  MOTION_Server_App_Context.NotificationStatus = status;
}

/**
 * @brief  Send a notification for Motion (Acc/Gyro/Mag) char
 * @param  None
 * @retval None
 */
void MOTION_Send_Notification_Task(void)
{
  uint8_t value[VALUE_LEN_MOTION];

  IKS01A3_MOTION_SENSOR_Axes_t AXIS;

  /* Read Motion values */
  MOTION_Handle_Sensor();

  /* Timestamp */
  STORE_LE_16(value, (HAL_GetTick()>>3));

  if(MOTION_Server_App_Context.hasAcc == 1)
  {
    STORE_LE_16(value+2, MOTION_Server_App_Context.acceleration.x);
    STORE_LE_16(value+4, MOTION_Server_App_Context.acceleration.y);
    STORE_LE_16(value+6, MOTION_Server_App_Context.acceleration.z);
  }

  if(MOTION_Server_App_Context.hasGyro == 1)
  {
    MOTION_Server_App_Context.angular_velocity.x/=100;
    MOTION_Server_App_Context.angular_velocity.y/=100;
    MOTION_Server_App_Context.angular_velocity.z/=100;

    STORE_LE_16(value+8, MOTION_Server_App_Context.angular_velocity.x);
    STORE_LE_16(value+10, MOTION_Server_App_Context.angular_velocity.y);
    STORE_LE_16(value+12, MOTION_Server_App_Context.angular_velocity.z);
  }

  if(MOTION_Server_App_Context.hasMag == 1)
  {
    AXIS.x = MOTION_Server_App_Context.magnetic_field.x - MOTIONFX_Get_MAG_Offset()->x;
    AXIS.y = MOTION_Server_App_Context.magnetic_field.y - MOTIONFX_Get_MAG_Offset()->y;
    AXIS.z = MOTION_Server_App_Context.magnetic_field.z - MOTIONFX_Get_MAG_Offset()->z;

    STORE_LE_16(value+14, AXIS.x);
    STORE_LE_16(value+16, AXIS.y);
    STORE_LE_16(value+18, AXIS.z);
  }

  if(MOTION_Server_App_Context.NotificationStatus)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION APPLICATION SERVER : NOTIFY CLIENT WITH NEW MOTION PARAMETER VALUE \n ");
    APP_DBG_MSG(" \n\r");
#endif
    MOTENV_STM_App_Update_Char(MOTION_CHAR_UUID, VALUE_LEN_MOTION, (uint8_t *)&value);
  }
  else
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION APPLICATION SERVER : CAN'T INFORM CLIENT - NOTIFICATION DISABLED\n ");
#endif
  }

  return;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Parse the values read by Motion sensors
 * @param  None
 * @retval None
 */
static void MOTION_Handle_Sensor(void)
{
  uint8_t i;
  IKS01A3_MOTION_SENSOR_Axes_t acceleration;
  IKS01A3_MOTION_SENSOR_Axes_t angular_velocity;
  IKS01A3_MOTION_SENSOR_Axes_t magnetic_field;

  for(i = 0; i < IKS01A3_MOTION_INSTANCES_NBR; i++)
  {
    if(MOTION_Server_App_Context.hasAcc == 1)
    {
      if (IKS01A3_MOTION_SENSOR_GetAxes(i, MOTION_ACCELERO, &acceleration) == 0)
      {
        MOTION_Server_App_Context.acceleration = acceleration;
      }
    }

    if(MOTION_Server_App_Context.hasGyro == 1)
    {
      if (IKS01A3_MOTION_SENSOR_GetAxes(i, MOTION_GYRO, &angular_velocity) == 0)
      {
        MOTION_Server_App_Context.angular_velocity = angular_velocity;
      }
    }

    if(MOTION_Server_App_Context.hasMag == 1)
    {
      if (IKS01A3_MOTION_SENSOR_GetAxes(i, MOTION_MAGNETO, &magnetic_field) == 0)
      {
        MOTION_Server_App_Context.magnetic_field = magnetic_field;
      }
    }
  }
}

/**
 * @brief  Check the Motion active capabilities and set the ADV data accordingly
 * @param  None
 * @retval None
 */
static void MOTION_GetCaps(void)
{
  uint8_t i;

  for(i = 0; i < IKS01A3_MOTION_INSTANCES_NBR; i++)
  {
    IKS01A3_MOTION_SENSOR_GetCapabilities(i, &MotionCapabilities[i]);
    if(MotionCapabilities[i].Acc)
    {
      MOTION_Server_App_Context.hasAcc = 1;
    }
    if(MotionCapabilities[i].Gyro)
    {
      MOTION_Server_App_Context.hasGyro = 1;
    }
    if(MotionCapabilities[i].Magneto)
    {
      MOTION_Server_App_Context.hasMag = 1;
    }
  }

  /* Update BLE ADV field (Motion) */
  if(MOTION_Server_App_Context.hasAcc)
  {
    manuf_data[5] |= 0x80; /* Acc value */
    manuf_data[6] |= 0x04; /* Extended Acc events */
  }
  if(MOTION_Server_App_Context.hasGyro)
  {
    manuf_data[5] |= 0x40; /* Gyro value */
  }
  if(MOTION_Server_App_Context.hasMag)
  {
    manuf_data[5] |= 0x20; /* Mag value */
  }
}

 /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
