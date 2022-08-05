/**
 ******************************************************************************
 * File Name          : motionaw_server_app.c
 * Description        : MOTIONAW Server Application
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
#include "motionaw_server_app.h"
#include "iks01a3_motion_sensors.h"

#include "MotionAW_Manager.h"
#include "MotionAR_Manager.h"

/* Private defines -----------------------------------------------------------*/
#define VALUE_LEN_AW    (2+1)

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint8_t  NotificationStatus;
  volatile uint32_t TimeStamp;
  MAR_output_t ActivityCode;

} MOTIONAW_Server_App_Context_t;

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

PLACE_IN_SECTION("BLE_APP_CONTEXT") static MOTIONAW_Server_App_Context_t MOTIONAW_Server_App_Context;

/* Global variables ----------------------------------------------------------*/
extern uint8_t manuf_data[14];

/* Private function prototypes -----------------------------------------------*/
static void ComputeMotionAW(void);

static void Accelero_Sensor_Handler(IKS01A3_MOTION_SENSOR_Axes_t *ACC_Value);
static void ActivityRec_Update(MAR_output_t ActivityCode);

/* Functions Definition ------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/
void MOTIONAW_Context_Init(void)
{
  /* Activity Rec API initialization function */
  MotionAW_manager_init();

  /* Update BLE ADV field (ActivityRec) */
  manuf_data[7] |= 0x10; /* ActivityRec */

  MOTIONAW_Server_App_Context.ActivityCode = MAR_STATIONARY;
  ActivityRec_Update(MOTIONAW_Server_App_Context.ActivityCode);
  MOTIONAW_Set_Notification_Status(0);
}

void MOTIONAW_Set_Notification_Status(uint8_t status)
{
  MOTIONAW_Server_App_Context.NotificationStatus = status;
  if(status == 1)
  {
    /* Set accelerometer:
     *   - FS   = <-4g, 4g>
     */
    (void)IKS01A3_MOTION_SENSOR_SetFullScale(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, 4);
  }
  else
  {
    /* Set accelerometer:
     *   - FS   = <-2g, 2g>
     */
    (void)IKS01A3_MOTION_SENSOR_SetFullScale(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, 2);
  }
}

void MOTIONAW_Send_Notification_Task(void)
{
  ComputeMotionAW();
}

void MOTIONAW_ActivityRec_Update(void)
{
  ActivityRec_Update(MOTIONAW_Server_App_Context.ActivityCode);
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
static void ComputeMotionAW(void)
{
  IKS01A3_MOTION_SENSOR_Axes_t ACC_Value;
  MAW_input_t data_in = {.AccX = 0.0f, .AccY = 0.0f, .AccZ = 0.0f};
  static MAR_output_t ActivityCodePrev = MAR_NOACTIVITY;

  MAW_activity_t CurrentActivityCode;
  /* Read the Acc values */
  Accelero_Sensor_Handler(&ACC_Value);

  /* Convert acceleration from [mg] to [g] */
  data_in.AccX = (float)ACC_Value.x * FROM_MG_TO_G;
  data_in.AccY = (float)ACC_Value.y * FROM_MG_TO_G;
  data_in.AccZ = (float)ACC_Value.z * FROM_MG_TO_G;

  MotionAW_manager_run(&data_in, &CurrentActivityCode);
  if(CurrentActivityCode >= MAW_STATIONARY && 
     CurrentActivityCode <= MAW_LYING)
  {
    MOTIONAW_Server_App_Context.ActivityCode = MAR_STATIONARY;
  }
  else if(CurrentActivityCode == MAW_WALKING)
  {
    MOTIONAW_Server_App_Context.ActivityCode = MAR_WALKING;
  }
  else if(CurrentActivityCode == MAW_FASTWALKING)
  {
    MOTIONAW_Server_App_Context.ActivityCode = MAR_FASTWALKING;
  }
  else if(CurrentActivityCode == MAW_JOGGING)
  {
    MOTIONAW_Server_App_Context.ActivityCode = MAR_JOGGING;
  }
  else if(CurrentActivityCode == MAW_BIKING)
  {
    MOTIONAW_Server_App_Context.ActivityCode = MAR_BIKING;
  }
  else
  {
    /* do nothing */
  }

  if(ActivityCodePrev != MOTIONAW_Server_App_Context.ActivityCode)
  {
    ActivityCodePrev = MOTIONAW_Server_App_Context.ActivityCode;
    if(MOTIONAW_Server_App_Context.NotificationStatus)
    {
      ActivityRec_Update(MOTIONAW_Server_App_Context.ActivityCode);
    }
    else
    {
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- MOTIONAW APPLICATION SERVER : CAN'T INFORM CLIENT - NOTIFICATION DISABLED\n ");
#endif
    }
  }
}

/**
 * @brief  Handles the ACC axes data getting
 * @param  ACC_Value Accelerometer value to be read
 * @retval None
 */
static void Accelero_Sensor_Handler(IKS01A3_MOTION_SENSOR_Axes_t *ACC_Value)
{
  (void)IKS01A3_MOTION_SENSOR_GetAxes(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, ACC_Value);
}

static void ActivityRec_Update(MAR_output_t ActivityCode)
{
  uint8_t value[VALUE_LEN_AW];

  /* Timestamp */
  STORE_LE_16(value, (HAL_GetTick()>>3));
  value[2] = ActivityCode;

#if(CFG_DEBUG_APP_TRACE != 0)
  APP_DBG_MSG("-- MOTIONAW APPLICATION SERVER : NOTIFY CLIENT WITH NEW PARAMETER VALUE \n ");
  APP_DBG_MSG(" \n\r");
#endif
  MOTENV_STM_App_Update_Char(ACTIVITY_REC_CHAR_UUID, VALUE_LEN_AW, (uint8_t *)&value);

  return;
}

 /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
