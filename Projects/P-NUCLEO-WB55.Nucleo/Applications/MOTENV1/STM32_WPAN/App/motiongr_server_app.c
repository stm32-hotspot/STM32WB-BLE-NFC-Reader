/**
 ******************************************************************************
 * File Name          : motiongr_server_app.c
 * Description        : Handle SW/Gesture Recognition Service/Char
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
#include "motiongr_server_app.h"
#include "iks01a3_motion_sensors.h"

#include "MotionGR_Manager.h"

/* Private defines -----------------------------------------------------------*/
#define VALUE_LEN_GR    (2+1)

/* Private typedef -----------------------------------------------------------*/
/**
 * @brief  SW/Gesture Recognition Service/Char Context structure definition
 */
typedef struct
{
  uint8_t  NotificationStatus;
  MGR_output_t GestureRecCode;

} MOTIONGR_Server_App_Context_t;

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

PLACE_IN_SECTION("BLE_APP_CONTEXT") static MOTIONGR_Server_App_Context_t MOTIONGR_Server_App_Context;

/* Global variables ----------------------------------------------------------*/
extern uint8_t manuf_data[14];

/* Private function prototypes -----------------------------------------------*/
static void ComputeMotionGR(void);
static void GestureRec_Update(MGR_output_t GestureRecCode);

/* Functions Definition ------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Init the SW/Gesture Recognition Service/Char Context
 *         and update the ADV data accordingly
 * @param  None
 * @retval None
 */
void MOTIONGR_Context_Init(void)
{
  /* CarryPosition API initialization function */
  MotionGR_manager_init();

  /* Update BLE ADV field (GestureRec) */
  manuf_data[7] |= 0x02; /* GestureRec */

  MOTIONGR_Set_Notification_Status(0);
  MOTIONGR_Server_App_Context.GestureRecCode = MGR_NOGESTURE;
  GestureRec_Update(MOTIONGR_Server_App_Context.GestureRecCode);
}

/**
 * @brief  Set the notification status (enabled/disabled) and full scale
 * @param  status The new notification status
 * @retval None
 */
void MOTIONGR_Set_Notification_Status(uint8_t status)
{
  MOTIONGR_Server_App_Context.NotificationStatus = status;
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
 * @brief  Send a notification for Gesture Recognition events
 * @param  None
 * @retval None
 */
void MOTIONGR_Send_Notification_Task(void)
{
  ComputeMotionGR();
}

/**
 * @brief  Update the Gesture Recognition char value
 * @param  None
 * @retval None
 */
void MOTIONGR_GestureRec_Update(void)
{
  GestureRec_Update(MOTIONGR_Server_App_Context.GestureRecCode);
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Run the GR Manager and update the Gesture Recognition char value
 * @param  None
 * @retval None
 */
static void ComputeMotionGR(void)
{
  IKS01A3_MOTION_SENSOR_Axes_t ACC_Value;
  MGR_input_t data_in = {.AccX = 0.0f, .AccY = 0.0f, .AccZ = 0.0f};
  static MGR_output_t GestureRecCodePrev = MGR_NOGESTURE;

  /* Read the Acc values */
  (void)IKS01A3_MOTION_SENSOR_GetAxes(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, &ACC_Value);

  /* Convert acceleration from [mg] to [g] */
  data_in.AccX = (float)ACC_Value.x * FROM_MG_TO_G;
  data_in.AccY = (float)ACC_Value.y * FROM_MG_TO_G;
  data_in.AccZ = (float)ACC_Value.z * FROM_MG_TO_G;

  MotionGR_manager_run(&data_in, &MOTIONGR_Server_App_Context.GestureRecCode);

  if(GestureRecCodePrev != MOTIONGR_Server_App_Context.GestureRecCode)
  {
    GestureRecCodePrev = MOTIONGR_Server_App_Context.GestureRecCode;
    if(MOTIONGR_Server_App_Context.NotificationStatus)
    {
      GestureRec_Update(MOTIONGR_Server_App_Context.GestureRecCode);
    }
    else
    {
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- MOTIONGR APPLICATION SERVER : CAN'T INFORM CLIENT - NOTIFICATION DISABLED\n ");
#endif
    }
  }
}

/**
 * @brief  Update the Gesture Recognition char value
 * @param  GestureRecCode Gesture Recognized
 * @retval None
 */
static void GestureRec_Update(MGR_output_t GestureRecCode)
{
  uint8_t value[VALUE_LEN_GR];

  /* Timestamp */
  STORE_LE_16(value, (HAL_GetTick()>>3));
  value[2] = GestureRecCode;

#if(CFG_DEBUG_APP_TRACE != 0)
  APP_DBG_MSG("-- MOTIONGR APPLICATION SERVER : NOTIFY CLIENT WITH NEW PARAMETER VALUE \n ");
  APP_DBG_MSG(" \n\r");
#endif
  MOTENV_STM_App_Update_Char(GESTURE_REC_CHAR_UUID, VALUE_LEN_GR, (uint8_t *)&value);

  return;
}

 /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
