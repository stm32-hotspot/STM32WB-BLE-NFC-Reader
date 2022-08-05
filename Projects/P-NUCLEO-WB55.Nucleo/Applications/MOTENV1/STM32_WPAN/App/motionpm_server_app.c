/**
 ******************************************************************************
 * File Name          : motionpm_server_app.c
 * Description        : Handle SW/Motion Pedometer Service/Char
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
#include "motionpm_server_app.h"
#include "iks01a3_motion_sensors.h"

#include "MotionPM_Manager.h"

/* Private defines -----------------------------------------------------------*/
#define VALUE_LEN_PM    (2+4+2)

/* Private typedef -----------------------------------------------------------*/
/**
 * @brief  SW/Motion Pedometer Service/Char Context structure definition
 */
typedef struct
{
  uint8_t  NotificationStatus;
  MPM_output_t PMData;

} MOTIONPM_Server_App_Context_t;

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

PLACE_IN_SECTION("BLE_APP_CONTEXT") static MOTIONPM_Server_App_Context_t MOTIONPM_Server_App_Context;

/* Global variables ----------------------------------------------------------*/
extern uint8_t manuf_data[14];

/* Private function prototypes -----------------------------------------------*/
static void ComputeMotionPM(void);
static void Pedometer_Update(MPM_output_t *PM_Data);

/* Functions Definition ------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Init the SW/Motion Pedometer Service/Char Context
 *         and update the ADV data accordingly
 * @param  None
 * @retval None
 */
void MOTIONPM_Context_Init(void)
{
  /* Pedometer API initialization function */
  MotionPM_manager_init();

  /* Update BLE ADV field (Pedometer) */
  manuf_data[7] |= 0x01; /* Pedometer */

  MOTIONPM_Set_Notification_Status(0);
  MOTIONPM_Server_App_Context.PMData.Cadence = 0;
  MOTIONPM_Server_App_Context.PMData.Nsteps = 0;
  Pedometer_Update(&MOTIONPM_Server_App_Context.PMData);
}

/**
 * @brief  Set the notification status (enabled/disabled) and full scale
 * @param  status The new notification status
 * @retval None
 */
void MOTIONPM_Set_Notification_Status(uint8_t status)
{
  MOTIONPM_Server_App_Context.NotificationStatus = status;
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
 * @brief  Send a notification for Motion Pedometer events
 * @param  None
 * @retval None
 */
void MOTIONPM_Send_Notification_Task(void)
{
  ComputeMotionPM();
}

/**
 * @brief  Update the Motion Pedometer char value
 * @param  None
 * @retval None
 */
void MOTIONPM_Pedometer_Update(void)
{
  Pedometer_Update(&MOTIONPM_Server_App_Context.PMData);
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Run the MPM Manager and update the Motion Pedometer char value
 * @param  None
 * @retval None
 */
static void ComputeMotionPM(void)
{
  IKS01A3_MOTION_SENSOR_Axes_t ACC_Value;
  MPM_input_t data_in = {.AccX = 0.0f, .AccY = 0.0f, .AccZ = 0.0f};
  static MPM_output_t PMDataPrev = {.Cadence = 0, .Nsteps = 0};

  /* Read the Acc values */
  (void)IKS01A3_MOTION_SENSOR_GetAxes(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, &ACC_Value);

  /* Convert acceleration from [mg] to [g] */
  data_in.AccX = (float)ACC_Value.x * FROM_MG_TO_G;
  data_in.AccY = (float)ACC_Value.y * FROM_MG_TO_G;
  data_in.AccZ = (float)ACC_Value.z * FROM_MG_TO_G;

  MotionPM_manager_run(&data_in, &MOTIONPM_Server_App_Context.PMData);

  if((PMDataPrev.Cadence != MOTIONPM_Server_App_Context.PMData.Cadence) ||
     (PMDataPrev.Nsteps != MOTIONPM_Server_App_Context.PMData.Nsteps))
  {
    PMDataPrev = MOTIONPM_Server_App_Context.PMData;
    if(MOTIONPM_Server_App_Context.NotificationStatus)
    {
      Pedometer_Update(&MOTIONPM_Server_App_Context.PMData);
    }
    else
    {
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- MOTIONPM APPLICATION SERVER : CAN'T INFORM CLIENT - NOTIFICATION DISABLED\n ");
#endif
    }
  }
}

/**
 * @brief  Update the Motion Pedometer char value
 * @param  PMData Motion Pedometer Data
 * @retval None
 */
static void Pedometer_Update(MPM_output_t *PMData)
{
  uint8_t value[VALUE_LEN_PM];
  uint16_t Cadence = (uint16_t) PMData->Cadence;

  /* Timestamp */
  STORE_LE_16(value, (HAL_GetTick()>>3));
  STORE_LE_32(value+2,PMData->Nsteps);
  STORE_LE_16(value+6,Cadence);

#if(CFG_DEBUG_APP_TRACE != 0)
  APP_DBG_MSG("-- MOTIONPM APPLICATION SERVER : NOTIFY CLIENT WITH NEW PARAMETER VALUE \n ");
  APP_DBG_MSG(" \n\r");
#endif
  MOTENV_STM_App_Update_Char(PEDOMETER_CHAR_UUID, VALUE_LEN_PM, (uint8_t *)&value);

  return;
}

 /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
