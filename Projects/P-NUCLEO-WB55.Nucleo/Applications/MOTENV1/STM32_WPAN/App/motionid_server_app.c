/**
 ******************************************************************************
 * File Name          : motionid_server_app.c
 * Description        : Handle SW/Motion Intensity Service/Char
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
#include "motionid_server_app.h"
#include "iks01a3_motion_sensors.h"

#include "MotionID_Manager.h"

/* Private defines -----------------------------------------------------------*/
#define VALUE_LEN_ID    (2+1)

/* Private typedef -----------------------------------------------------------*/
/**
 * @brief  SW/Motion Intensity Service/Char Context structure definition
 */
typedef struct
{
  uint8_t  NotificationStatus;
  MID_output_t MIDCode;

} MOTIONID_Server_App_Context_t;

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

PLACE_IN_SECTION("BLE_APP_CONTEXT") static MOTIONID_Server_App_Context_t MOTIONID_Server_App_Context;

/* Global variables ----------------------------------------------------------*/
extern uint8_t manuf_data[14];

/* Private function prototypes -----------------------------------------------*/
static void ComputeMotionID(void);
static void IntensityDet_Update(MID_output_t MIDCode);

/* Functions Definition ------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Init the SW/Motion Intensity Service/Char Context
 *         and update the ADV data accordingly
 * @param  None
 * @retval None
 */
void MOTIONID_Context_Init(void)
{
  /* CarryPosition API initialization function */
  MotionID_manager_init();

  /* Update BLE ADV field (IntensityDet) */
  manuf_data[7] |= 0x20; /* IntensityDet */

  MOTIONID_Set_Notification_Status(0);
  MOTIONID_Server_App_Context.MIDCode = MID_ON_DESK;
  IntensityDet_Update(MOTIONID_Server_App_Context.MIDCode);
}

/**
 * @brief  Set the notification status (enabled/disabled) and full scale
 * @param  status The new notification status
 * @retval None
 */
void MOTIONID_Set_Notification_Status(uint8_t status)
{
  MOTIONID_Server_App_Context.NotificationStatus = status;
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

/**
 * @brief  Send a notification for Motion Intensity events
 * @param  None
 * @retval None
 */
void MOTIONID_Send_Notification_Task(void)
{
  ComputeMotionID();
}

/**
 * @brief  Update the Motion Intensity char value
 * @param  None
 * @retval None
 */
void MOTIONID_IntensityDet_Update(void)
{
  IntensityDet_Update(MOTIONID_Server_App_Context.MIDCode);
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Run the MID Manager and update the Motion Intensity char value
 * @param  None
 * @retval None
 */
static void ComputeMotionID(void)
{
  IKS01A3_MOTION_SENSOR_Axes_t ACC_Value;
  MID_input_t data_in = {.AccX = 0.0f, .AccY = 0.0f, .AccZ = 0.0f};
  static MID_output_t MIDCodePrev = MID_ON_DESK;

  /* Read the Acc values */
  (void)IKS01A3_MOTION_SENSOR_GetAxes(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, &ACC_Value);

  /* Convert acceleration from [mg] to [g] */
  data_in.AccX = (float)ACC_Value.x * FROM_MG_TO_G;
  data_in.AccY = (float)ACC_Value.y * FROM_MG_TO_G;
  data_in.AccZ = (float)ACC_Value.z * FROM_MG_TO_G;

  MotionID_manager_run(&data_in, &MOTIONID_Server_App_Context.MIDCode);

  if(MIDCodePrev != MOTIONID_Server_App_Context.MIDCode)
  {
    MIDCodePrev = MOTIONID_Server_App_Context.MIDCode;
    if(MOTIONID_Server_App_Context.NotificationStatus)
    {
      IntensityDet_Update(MOTIONID_Server_App_Context.MIDCode);
    }
    else
    {
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- MOTIONID APPLICATION SERVER : CAN'T INFORM CLIENT - NOTIFICATION DISABLED\n ");
#endif
    }
  }
}

/**
 * @brief  Update the Motion Intensity char value
 * @param  MIDCode Motion Intensity Detected
 * @retval None
 */
static void IntensityDet_Update(MID_output_t MIDCode)
{
  uint8_t value[VALUE_LEN_ID];

  /* Timestamp */
  STORE_LE_16(value, (HAL_GetTick()>>3));
  value[2] = MIDCode;

#if(CFG_DEBUG_APP_TRACE != 0)
  APP_DBG_MSG("-- MOTIONID APPLICATION SERVER : NOTIFY CLIENT WITH NEW PARAMETER VALUE \n ");
  APP_DBG_MSG(" \n\r");
#endif
  MOTENV_STM_App_Update_Char(INTENSITY_DET_CHAR_UUID, VALUE_LEN_ID, (uint8_t *)&value);

  return;
}

 /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
