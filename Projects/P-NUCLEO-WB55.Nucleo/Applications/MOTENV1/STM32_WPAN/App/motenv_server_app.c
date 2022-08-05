/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : motenv_server_app.c
 * Description        : MOTENV Server Application
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
/* USER CODE END Header */
/* USER CODE BEGIN UserCode */
/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "dbg_trace.h"
#include "ble.h"
#include "MOTENV_server_app.h"
#include "stm32_seq.h"

#include "env_server_app.h"
#include "motion_server_app.h"
#include "motion_ext_server_app.h"
#include "motionfx_server_app.h"
#include "motionar_server_app.h"
//#include "motionaw_server_app.h"
#include "motioncp_server_app.h"
#include "motiongr_server_app.h"
#include "motionpm_server_app.h"
#include "motionid_server_app.h"
#include "config_server_app.h"
#include "console_server_app.h"

/* Private defines -----------------------------------------------------------*/

//#define ENVIRONMENT_UPDATE_PERIOD       (uint32_t)(1000*1000/CFG_TS_TICK_VAL) /*1s*/
#define ENVIRONMENT_UPDATE_PERIOD       (uint32_t)(0.5*1000*1000/CFG_TS_TICK_VAL) /*500ms*/
#define ACC_GYRO_MAG_UPDATE_PERIOD      (uint32_t)(0.05*1000*1000/CFG_TS_TICK_VAL) /*50ms (20Hz)*/
#define MOTIONFX_UPDATE_PERIOD          (uint32_t)(0.01*1000*1000/CFG_TS_TICK_VAL) /*10ms (100Hz)*/
#define ECOMPASS_UPDATE_PERIOD          (uint32_t)(0.01*1000*1000/CFG_TS_TICK_VAL) /*10ms (100Hz)*/
#define ACTIVITY_REC_UPDATE_PERIOD      (uint32_t)(0.0625*1000*1000/CFG_TS_TICK_VAL) /*62.5ms (16Hz)*/
#define CARRY_POSITION_UPDATE_PERIOD    (uint32_t)(0.02*1000*1000/CFG_TS_TICK_VAL) /*20ms (50Hz)*/
#define GESTURE_REC_UPDATE_PERIOD       (uint32_t)(0.02*1000*1000/CFG_TS_TICK_VAL) /*20ms (50Hz)*/
#define PEDOMETER_UPDATE_PERIOD         (uint32_t)(0.02*1000*1000/CFG_TS_TICK_VAL) /*20ms (50Hz)*/
#define INTENSITY_DET_UPDATE_PERIOD     (uint32_t)(0.0625*1000*1000/CFG_TS_TICK_VAL) /*62.5ms (16Hz)*/

/* Private typedef -----------------------------------------------------------*/

/**
 * @brief  MOTENV Server Context structure definition
 *         Include just the Timer Ids for the Notifications
 */
typedef struct
{
  uint8_t AccGyroMag_Update_Timer_Id;
  uint8_t Env_Update_Timer_Id;
  uint8_t MotionFx_Update_Timer_Id;
  uint8_t ECompass_Update_Timer_Id;
  uint8_t ActivityRec_Update_Timer_Id;
  uint8_t CarryPosition_Update_Timer_Id;
  uint8_t GestureRec_Update_Timer_Id;
  uint8_t Pedometer_Update_Timer_Id;
  uint8_t IntensityDet_Update_Timer_Id;
} MOTENV_Server_App_Context_t;

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
 * START of Section BLE_APP_CONTEXT
 */
PLACE_IN_SECTION("BLE_APP_CONTEXT") static MOTENV_Server_App_Context_t MOTENV_Server_App_Context;

/**
 * END of Section BLE_APP_CONTEXT
 */
/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void MOTENV_AccGyroMagUpdate_Timer_Callback(void);
static void MOTENV_EnvUpdate_Timer_Callback(void);
static void MOTENV_MotionFxUpdate_Timer_Callback(void);
static void MOTENV_ECompassUpdate_Timer_Callback(void);
static void MOTENV_ActivityRecUpdate_Timer_Callback(void);
static void MOTENV_CarryPositionUpdate_Timer_Callback(void);
static void MOTENV_GestureRecUpdate_Timer_Callback(void);
static void MOTENV_PedometerUpdate_Timer_Callback(void);
static void MOTENV_IntensityDetUpdate_Timer_Callback(void);

static void MOTENV_APP_context_Init(void);

/* Functions Definition ------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Handle the request from the GATT Client
 *         (e.g., on notification enabling/disabling request, start/stop the timer)
 * @param  pNotification: Request data coming from the GATT Client
 * @retval None
 */
void MOTENV_STM_App_Notification(MOTENV_STM_App_Notification_evt_t *pNotification)
{
  switch(pNotification->Motenv_Evt_Opcode)
  {
    /*
     * Env char notification enabled
     */
    case HW_ENV_NOTIFY_ENABLED_EVT:
      ENV_Set_Notification_Status(1);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : ENV NOTIFICATION ENABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Start the timer used to update the Env characteristic */
      HW_TS_Start(MOTENV_Server_App_Context.Env_Update_Timer_Id, ENVIRONMENT_UPDATE_PERIOD);
      break; /* HW_ENV_NOTIFY_ENABLED_EVT */

    /*
     * Motion char notification enabled
     */
    case HW_MOTION_NOTIFY_ENABLED_EVT:
      MOTION_Set_Notification_Status(1);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : MOTION NOTIFICATION ENABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Start the timer used to update the AccGyroMag characteristic */
      HW_TS_Start(MOTENV_Server_App_Context.AccGyroMag_Update_Timer_Id, ACC_GYRO_MAG_UPDATE_PERIOD);
      break; /* HW_MOTION_NOTIFY_ENABLED_EVT */

    /*
     * Motion Ext char notification enabled
     */
    case HW_ACC_EVENT_NOTIFY_ENABLED_EVT:
      MOTION_EXT_Set_Notification_Status(1);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : MOTION EXT NOTIFICATION ENABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      break; /* HW_ACC_EVENT_NOTIFY_ENABLED_EVT */

    /*
     * MotionFx char notification enabled
     */
    case SW_MOTIONFX_NOTIFY_ENABLED_EVT:
      MOTIONFX_Set_Quat_Notification_Status(1);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : MOTIONFX NOTIFICATION ENABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Start the timer used to update the MotionFx characteristic */
      HW_TS_Start(MOTENV_Server_App_Context.MotionFx_Update_Timer_Id, MOTIONFX_UPDATE_PERIOD);
      break; /* SW_MOTIONFX_NOTIFY_ENABLED_EVT */

    /*
     * ECompass char notification enabled
     */
    case SW_ECOMPASS_NOTIFY_ENABLED_EVT:
      MOTIONFX_Set_ECompass_Notification_Status(1);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : ECOMPASS NOTIFICATION ENABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Start the timer used to update the ECompass characteristic */
      HW_TS_Start(MOTENV_Server_App_Context.ECompass_Update_Timer_Id, ECOMPASS_UPDATE_PERIOD);
      break; /* SW_ECOMPASS_NOTIFY_ENABLED_EVT */

    /*
     * ActivityRec char notification enabled
     */
    case SW_ACTIVITY_REC_NOTIFY_ENABLED_EVT:
      MOTIONAR_Set_Notification_Status(1);
//      MOTIONAW_Set_Notification_Status(1);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : ACITIVITY REC NOTIFICATION ENABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Start the timer used to update the Activity Rec characteristic */
      HW_TS_Start(MOTENV_Server_App_Context.ActivityRec_Update_Timer_Id, ACTIVITY_REC_UPDATE_PERIOD);
      break; /* SW_ACTIVITY_REC_NOTIFY_ENABLED_EVT */

    /*
     * CarryPosition char notification enabled
     */
    case SW_CARRY_POSITION_NOTIFY_ENABLED_EVT:
      MOTIONCP_Set_Notification_Status(1);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : CARRY POSITION NOTIFICATION ENABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Start the timer used to update the Carry Position characteristic */
      HW_TS_Start(MOTENV_Server_App_Context.CarryPosition_Update_Timer_Id, CARRY_POSITION_UPDATE_PERIOD);
      break; /* SW_CARRY_POSITION_NOTIFY_ENABLED_EVT */

    /*
     * GestureRec char notification enabled
     */
    case SW_GESTURE_REC_NOTIFY_ENABLED_EVT:
      MOTIONGR_Set_Notification_Status(1);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : GESTURE REC NOTIFICATION ENABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Start the timer used to update the GestureRec characteristic */
      HW_TS_Start(MOTENV_Server_App_Context.GestureRec_Update_Timer_Id, GESTURE_REC_UPDATE_PERIOD);
      break; /* SW_GESTURE_REC_NOTIFY_ENABLED_EVT */

    /*
     * Pedometer char notification enabled
     */
    case SW_PEDOMETER_NOTIFY_ENABLED_EVT:
      MOTIONPM_Set_Notification_Status(1);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : PEDOMETER NOTIFICATION ENABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Start the timer used to update the Pedometer characteristic */
      HW_TS_Start(MOTENV_Server_App_Context.Pedometer_Update_Timer_Id, PEDOMETER_UPDATE_PERIOD);
      break; /* SW_PEDOMETER_NOTIFY_ENABLED_EVT */

    /*
     * IntensityDet char notification enabled
     */
    case SW_INTENSITY_DET_NOTIFY_ENABLED_EVT:
      MOTIONID_Set_Notification_Status(1);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : INTENSITY DET NOTIFICATION ENABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Start the timer used to update the IntensityDet characteristic */
      HW_TS_Start(MOTENV_Server_App_Context.IntensityDet_Update_Timer_Id, INTENSITY_DET_UPDATE_PERIOD);
      break; /* SW_INTENSITY_DET_NOTIFY_ENABLED_EVT */

    /*
     * Config char notification enabled
     */
    case CONFIG_NOTIFY_ENABLED_EVT:
      CONFIG_Set_Notification_Status(1);
      CONFIG_Set_FirstConnection_Config(1);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : CONFIG NOTIFICATION ENABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      break; /* CONFIG_NOTIFY_ENABLED_EVT */

    /*
     * Console Term char notification enabled
     */
    case CONSOLE_TERM_NOTIFY_ENABLED_EVT:
      CONSOLE_Set_Term_Notification_Status(1);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : CONSOLE TERM NOTIFICATION ENABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      break; /* CONSOLE_TERM_NOTIFY_ENABLED_EVT */

    /*
     * Console Stderr char notification enabled
     */
    case CONSOLE_STDERR_NOTIFY_ENABLED_EVT:
      CONSOLE_Set_Stderr_Notification_Status(1);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : CONSOLE STDERR NOTIFICATION ENABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      break; /* CONSOLE_STDERR_NOTIFY_ENABLED_EVT */

    /*
     * Env char notification disabled
     */
    case HW_ENV_NOTIFY_DISABLED_EVT:
      ENV_Set_Notification_Status(0);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : ENV NOTIFICATION DISABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Stop the timer used to update the Env characteristic */
      HW_TS_Stop(MOTENV_Server_App_Context.Env_Update_Timer_Id);
      break; /* HW_ENV_NOTIFY_DISABLED_EVT */

    /*
     * Motion char notification disabled
     */
    case HW_MOTION_NOTIFY_DISABLED_EVT:
      MOTION_Set_Notification_Status(0);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : MOTION NOTIFICATION DISABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Stop the timer used to update the Motion characteristic */
      HW_TS_Stop(MOTENV_Server_App_Context.AccGyroMag_Update_Timer_Id);
      break; /* HW_ENV_NOTIFY_DISABLED_EVT */

    /*
     * Motion Ext (Acc Event) char notification disabled
     */
    case HW_ACC_EVENT_NOTIFY_DISABLED_EVT:
      MOTION_EXT_Set_Notification_Status(0);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : MOTION EXT NOTIFICATION DISABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      break; /* HW_ACC_EVENT_NOTIFY_DISABLED_EVT */

    /*
     * MotionFx char notification disabled
     */
    case SW_MOTIONFX_NOTIFY_DISABLED_EVT:
      MOTIONFX_Set_Quat_Notification_Status(0);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : MOTIONFX NOTIFICATION DISABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Stop the timer used to update the MotionFx characteristic */
      HW_TS_Stop(MOTENV_Server_App_Context.MotionFx_Update_Timer_Id);
      break; /* SW_MOTIONFX_NOTIFY_DISABLED_EVT */

    /*
     * ECompass char notification disabled
     */
    case SW_ECOMPASS_NOTIFY_DISABLED_EVT:
      MOTIONFX_Set_ECompass_Notification_Status(0);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : ECOMPASS NOTIFICATION DISABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Stop the timer used to update the ECopmass characteristic */
      HW_TS_Stop(MOTENV_Server_App_Context.ECompass_Update_Timer_Id);
      break; /* SW_ECOMPASS_NOTIFY_DISABLED_EVT */

    /*
     * ActivityRec char notification disabled
     */
    case SW_ACTIVITY_REC_NOTIFY_DISABLED_EVT:
      MOTIONAR_Set_Notification_Status(0);
//      MOTIONAW_Set_Notification_Status(0);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : ACTIVITY REC NOTIFICATION DISABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Stop the timer used to update the ActivityRec characteristic */
      HW_TS_Stop(MOTENV_Server_App_Context.ActivityRec_Update_Timer_Id);
      break; /* SW_ACTIVITY_REC_NOTIFY_DISABLED_EVT */

    /*
     * CarryPosition char notification disabled
     */
    case SW_CARRY_POSITION_NOTIFY_DISABLED_EVT:
      MOTIONCP_Set_Notification_Status(0);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : CARRY POSITION NOTIFICATION DISABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Stop the timer used to update the Carry Position characteristic */
      HW_TS_Stop(MOTENV_Server_App_Context.CarryPosition_Update_Timer_Id);
      break; /* SW_CARRY_POSITION_NOTIFY_DISABLED_EVT */

    /*
     * GestureRec char notification disabled
     */
    case SW_GESTURE_REC_NOTIFY_DISABLED_EVT:
      MOTIONGR_Set_Notification_Status(0);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : GESTURE REC NOTIFICATION DISABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Stop the timer used to update the Gesture Rec characteristic */
      HW_TS_Stop(MOTENV_Server_App_Context.GestureRec_Update_Timer_Id);
      break; /* SW_GESTURE_REC_NOTIFY_DISABLED_EVT */

    /*
     * Pedometer char notification disabled
     */
    case SW_PEDOMETER_NOTIFY_DISABLED_EVT:
      MOTIONPM_Set_Notification_Status(0);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : PEDOMETER NOTIFICATION DISABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Stop the timer used to update the Pedometer characteristic */
      HW_TS_Stop(MOTENV_Server_App_Context.Pedometer_Update_Timer_Id);
      break; /* SW_PEDOMETER_NOTIFY_DISABLED_EVT */

    /*
     * IntensityDet char notification disabled
     */
    case SW_INTENSITY_DET_NOTIFY_DISABLED_EVT:
      MOTIONID_Set_Notification_Status(0);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : INTENSITY DET NOTIFICATION DISABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      /* Stop the timer used to update the IntensityDet characteristic */
      HW_TS_Stop(MOTENV_Server_App_Context.IntensityDet_Update_Timer_Id);
      break; /* SW_INTENSITY_DET_NOTIFY_DISABLED_EVT */

    /*
     * Config char notification disabled
     */
    case CONFIG_NOTIFY_DISABLED_EVT:
      CONFIG_Set_Notification_Status(0);
      CONFIG_Set_FirstConnection_Config(0);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : CONFIG NOTIFICATION DISABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      break; /* CONFIG_NOTIFY_DISABLED_EVT */

    /*
     * Console Term char notification disabled
     */
    case CONSOLE_TERM_NOTIFY_DISABLED_EVT:
      CONSOLE_Set_Term_Notification_Status(0);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : CONSOLE TERM NOTIFICATION DISABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      break; /* CONSOLE_TERM_NOTIFY_DISABLED_EVT */

    /*
     * Console Stderr char notification disabled
     */
    case CONSOLE_STDERR_NOTIFY_DISABLED_EVT:
      CONSOLE_Set_Stderr_Notification_Status(0);
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : CONSOLE STDERR NOTIFICATION DISABLED\n");
      APP_DBG_MSG(" \n\r");
#endif
      break; /* CONSOLE_STDERR_NOTIFY_DISABLED_EVT */

    /*
     * Env char read request
     */
    case HW_ENV_READ_EVT:
      ENV_Update();
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : ENV READ\n");
      APP_DBG_MSG(" \n\r");
#endif
      break; /* HW_ENV_READ_EVT */

    /*
     * Motion Ext (Acc Event) char read request
     */
    case HW_ACC_EVENT_READ_EVT:
      MOTION_EXT_ReadCB();
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : MOTION EXT READ\n");
      APP_DBG_MSG(" \n\r");
#endif
      break; /* HW_ACC_EVENT_READ_EVT */

    /*
     * ActivityRec char read request
     */
    case SW_ACTIVITY_REC_READ_EVT:
      MOTIONAR_ActivityRec_Update();
//      MOTIONAW_ActivityRec_Update();
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : ACITIVITY REC READ\n");
      APP_DBG_MSG(" \n\r");
#endif
      break; /* SW_ACTIVITY_REC_READ_EVT */

    /*
     * CarryPosition char read request
     */
    case SW_CARRY_POSITION_READ_EVT:
      MOTIONCP_CarryPosition_Update();
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : CARRY POSITION READ\n");
      APP_DBG_MSG(" \n\r");
#endif
      break; /* SW_CARRY_POSITION_READ_EVT */

    /*
     * Gesture Rec char read request
     */
    case SW_GESTURE_REC_READ_EVT:
      MOTIONGR_GestureRec_Update();
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : GESTURE REC READ\n");
      APP_DBG_MSG(" \n\r");
#endif
      break; /* SW_GESTURE_REC_READ_EVT */

    /*
     * Pedometer char read request
     */
    case SW_PEDOMETER_READ_EVT:
      MOTIONPM_Pedometer_Update();
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : PEDOMETER READ\n");
      APP_DBG_MSG(" \n\r");
#endif
      break; /* SW_PEDOMETER_READ_EVT */

    /*
     * Console Term char read request
     */
    case CONSOLE_TERM_READ_EVT:
      CONSOLE_Term_Update_AfterRead();
      break; /* CONSOLE_TERM_READ_EVT */

    /*
     * Console Stderr char read request
     */
    case CONSOLE_STDERR_READ_EVT:
      CONSOLE_Stderr_Update_AfterRead();
      break; /* CONSOLE_STDERR_READ_EVT */

    /*
     * Configuration char write request
     */
    case CONFIG_WRITE_EVT:
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- TEMPLATE APPLICATION SERVER : CONFIG WRITE EVENT RECEIVED\n");
      APP_DBG_MSG(" \n\r");
#endif
      CONFIG_Parse_Command(pNotification->DataTransfered.pPayload, pNotification->DataTransfered.Length);
      break; /* CONFIG_WRITE_EVT */
      
    default:
      break; /* DEFAULT */
  }

  return;
}

/**
 * @brief  Handle disconnection (Stop all timers)
 * @param  None
 * @retval None
 */
void MOTENV_APP_HandleDisconnection( void )
{
  ENV_Set_Notification_Status(0);
  /* Stop the timer used to update the Env characteristic */
  HW_TS_Stop(MOTENV_Server_App_Context.Env_Update_Timer_Id);

  MOTION_Set_Notification_Status(0);
  /* Stop the timer used to update the Motion characteristic */
  HW_TS_Stop(MOTENV_Server_App_Context.AccGyroMag_Update_Timer_Id);
  
  MOTIONFX_Set_Quat_Notification_Status(0);
  /* Stop the timer used to update the MotionFx characteristic */
  HW_TS_Stop(MOTENV_Server_App_Context.MotionFx_Update_Timer_Id);

  MOTIONFX_Set_ECompass_Notification_Status(0);
  /* Stop the timer used to update the ECopmass characteristic */
  HW_TS_Stop(MOTENV_Server_App_Context.ECompass_Update_Timer_Id);
 
  MOTIONAR_Set_Notification_Status(0);
  /* Stop the timer used to update the ActivityRec characteristic */
  HW_TS_Stop(MOTENV_Server_App_Context.ActivityRec_Update_Timer_Id);

  MOTIONCP_Set_Notification_Status(0);
  /* Stop the timer used to update the Carry Position characteristic */
  HW_TS_Stop(MOTENV_Server_App_Context.CarryPosition_Update_Timer_Id);

  MOTIONGR_Set_Notification_Status(0);
  /* Stop the timer used to update the Gesture Rec characteristic */
  HW_TS_Stop(MOTENV_Server_App_Context.GestureRec_Update_Timer_Id);

  MOTIONPM_Set_Notification_Status(0);
  /* Stop the timer used to update the Pedometer characteristic */
  HW_TS_Stop(MOTENV_Server_App_Context.Pedometer_Update_Timer_Id);

  MOTIONID_Set_Notification_Status(0);
  /* Stop the timer used to update the IntensityDet characteristic */
  HW_TS_Stop(MOTENV_Server_App_Context.IntensityDet_Update_Timer_Id);
}

/**
 * @brief  Init the MOTENV APP (Register Tasks, Create Notification timers)
 * @param  None
 * @retval None
 */
void MOTENV_APP_Init(void)
{
  UTIL_SEQ_RegTask( 1<<CFG_TASK_NOTIFY_ENVIRONMENT_ID, UTIL_SEQ_RFU, ENV_Send_Notification_Task);
  /* Create timer to change the Environment params and update charecteristic */
  HW_TS_Create(CFG_TIM_PROC_ID_ISR,
		&(MOTENV_Server_App_Context.Env_Update_Timer_Id),
		hw_ts_Repeated,
		MOTENV_EnvUpdate_Timer_Callback);

#ifndef NFC_READER_ONLY_DEMO	   // Disable other sensors, when not using an X-NUCLEO-ISK01A3 expansion board
  UTIL_SEQ_RegTask( 1<<CFG_TASK_NOTIFY_ACC_GYRO_MAG_ID, UTIL_SEQ_RFU, MOTION_Send_Notification_Task);
  /* Create timer to get the AccGyroMag params and update charecteristic */
  HW_TS_Create(CFG_TIM_PROC_ID_ISR,
        &(MOTENV_Server_App_Context.AccGyroMag_Update_Timer_Id),
        hw_ts_Repeated,
        MOTENV_AccGyroMagUpdate_Timer_Callback);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_NOTIFY_MOTIONFX_ID, UTIL_SEQ_RFU, MOTIONFX_Send_Quat_Notification_Task);
  /* Create timer to change the MotionFx params and update charecteristic */
  HW_TS_Create(CFG_TIM_PROC_ID_ISR,
        &(MOTENV_Server_App_Context.MotionFx_Update_Timer_Id),
        hw_ts_Repeated,
        MOTENV_MotionFxUpdate_Timer_Callback);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_NOTIFY_ECOMPASS_ID, UTIL_SEQ_RFU, MOTIONFX_Send_ECompass_Notification_Task);
  /* Create timer to change the ECompass params and update charecteristic */
  HW_TS_Create(CFG_TIM_PROC_ID_ISR,
        &(MOTENV_Server_App_Context.ECompass_Update_Timer_Id),
        hw_ts_Repeated,
        MOTENV_ECompassUpdate_Timer_Callback);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_NOTIFY_ACTIVITY_REC_ID, UTIL_SEQ_RFU, MOTIONAR_Send_Notification_Task);
//  UTIL_SEQ_RegTask( 1<<CFG_TASK_NOTIFY_ACTIVITY_REC_ID, UTIL_SEQ_RFU, MOTIONAW_Send_Notification_Task);
  /* Create timer to change the Activity Rec params and update charecteristic */
  HW_TS_Create(CFG_TIM_PROC_ID_ISR,
        &(MOTENV_Server_App_Context.ActivityRec_Update_Timer_Id),
        hw_ts_Repeated,
        MOTENV_ActivityRecUpdate_Timer_Callback);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_NOTIFY_CARRY_POSITION_ID, UTIL_SEQ_RFU, MOTIONCP_Send_Notification_Task);
  /* Create timer to change the Carry Position params and update charecteristic */
  HW_TS_Create(CFG_TIM_PROC_ID_ISR,
        &(MOTENV_Server_App_Context.CarryPosition_Update_Timer_Id),
        hw_ts_Repeated,
        MOTENV_CarryPositionUpdate_Timer_Callback);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_NOTIFY_GESTURE_REC_ID, UTIL_SEQ_RFU, MOTIONGR_Send_Notification_Task);
  /* Create timer to change the Gesture Rec params and update charecteristic */
  HW_TS_Create(CFG_TIM_PROC_ID_ISR,
        &(MOTENV_Server_App_Context.GestureRec_Update_Timer_Id),
        hw_ts_Repeated,
        MOTENV_GestureRecUpdate_Timer_Callback);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_NOTIFY_PEDOMETER_ID, UTIL_SEQ_RFU, MOTIONPM_Send_Notification_Task);
  /* Create timer to change the Pedometer params and update charecteristic */
  HW_TS_Create(CFG_TIM_PROC_ID_ISR,
        &(MOTENV_Server_App_Context.Pedometer_Update_Timer_Id),
        hw_ts_Repeated,
        MOTENV_PedometerUpdate_Timer_Callback);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_NOTIFY_INTENSITY_DET_ID, UTIL_SEQ_RFU, MOTIONID_Send_Notification_Task);
  /* Create timer to change the IntensityDet params and update charecteristic */
  HW_TS_Create(CFG_TIM_PROC_ID_ISR,
        &(MOTENV_Server_App_Context.IntensityDet_Update_Timer_Id),
        hw_ts_Repeated,
        MOTENV_IntensityDetUpdate_Timer_Callback);

  /* Register the task handling Interrupt events from MEMS */
  UTIL_SEQ_RegTask( 1<<CFG_TASK_HANDLE_MEMS_IT_ID, UTIL_SEQ_RFU, MOTION_EXT_Handle_IT);

#endif


  /**
   * Initialize MOTENV application context
   */
  MOTENV_APP_context_Init();

  return;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  On timeout, trigger the task
 *         for Motion Char (Acc-Gyro-Mag) notification
 * @param  None
 * @retval None
 */
static void MOTENV_AccGyroMagUpdate_Timer_Callback(void)
{
  UTIL_SEQ_SetTask(1<<CFG_TASK_NOTIFY_ACC_GYRO_MAG_ID, CFG_SCH_PRIO_0);   
}

/**
 * @brief  On timeout, trigger the task
 *         for Environmental Char notification
 * @param  None
 * @retval None
 */
static void MOTENV_EnvUpdate_Timer_Callback(void)
{
  UTIL_SEQ_SetTask(1<<CFG_TASK_NOTIFY_ENVIRONMENT_ID, CFG_SCH_PRIO_0);
}

/**
 * @brief  On timeout, trigger the task
 *         for Sensor Fusion Char notification
 * @param  None
 * @retval None
 */
static void MOTENV_MotionFxUpdate_Timer_Callback(void)
{
  UTIL_SEQ_SetTask(1<<CFG_TASK_NOTIFY_MOTIONFX_ID, CFG_SCH_PRIO_0);
}

/**
 * @brief  On timeout, trigger the task
 *         for ECompass Char notification
 * @param  None
 * @retval None
 */
static void MOTENV_ECompassUpdate_Timer_Callback(void)
{
  UTIL_SEQ_SetTask(1<<CFG_TASK_NOTIFY_ECOMPASS_ID, CFG_SCH_PRIO_0);
}

/**
 * @brief  On timeout, trigger the task
 *         for Activity Recognition Char notification
 * @param  None
 * @retval None
 */
static void MOTENV_ActivityRecUpdate_Timer_Callback(void)
{
  UTIL_SEQ_SetTask(1<<CFG_TASK_NOTIFY_ACTIVITY_REC_ID, CFG_SCH_PRIO_0);
}

/**
 * @brief  On timeout, trigger the task
 *         for Carry Position Char notification
 * @param  None
 * @retval None
 */
static void MOTENV_CarryPositionUpdate_Timer_Callback(void)
{
  UTIL_SEQ_SetTask(1<<CFG_TASK_NOTIFY_CARRY_POSITION_ID, CFG_SCH_PRIO_0);
}

/**
 * @brief  On timeout, trigger the task
 *         for Gesture Recognition Char notification
 * @param  None
 * @retval None
 */
static void MOTENV_GestureRecUpdate_Timer_Callback(void)
{
  UTIL_SEQ_SetTask(1<<CFG_TASK_NOTIFY_GESTURE_REC_ID, CFG_SCH_PRIO_0);
}

/**
 * @brief  On timeout, trigger the task
 *         for Pedometer Char notification
 * @param  None
 * @retval None
 */
static void MOTENV_PedometerUpdate_Timer_Callback(void)
{
  UTIL_SEQ_SetTask(1<<CFG_TASK_NOTIFY_PEDOMETER_ID, CFG_SCH_PRIO_0);
}

/**
 * @brief  On timeout, trigger the task
 *         for Intensity Detection Char notification
 * @param  None
 * @retval None
 */
static void MOTENV_IntensityDetUpdate_Timer_Callback(void)
{
  UTIL_SEQ_SetTask(1<<CFG_TASK_NOTIFY_INTENSITY_DET_ID, CFG_SCH_PRIO_0);
}

/**
 * @brief  Init Context for each Service exposed by MOTENV Server App
 * @param  None
 * @retval None
 */
static void MOTENV_APP_context_Init(void)
{
  /* Init ENV context */
  ENV_Context_Init();

#ifndef NFC_READER_ONLY_DEMO     // Disable other sensors, when not using an X-NUCLEO-ISK01A3 expansion board
  /* Init MOTION Context */
  MOTION_Context_Init();

  /* Init MOTION Context */
  MOTION_EXT_Context_Init();

  /* Init MOTIONFX Context */
  MOTIONFX_Context_Init();

  /* Init MOTIONAR Context */
  MOTIONAR_Context_Init();

//  /* Init MOTIONAW Context */
//  MOTIONAW_Context_Init();

  /* Init MOTIONCP Context */
  MOTIONCP_Context_Init();

  /* Init MOTIONGR Context */
  MOTIONGR_Context_Init();

  /* Init MOTIONPM Context */
  MOTIONPM_Context_Init();

  /* Init MOTIONID Context */
  MOTIONID_Context_Init();
#endif

  /* Init CONFIG Context */
  CONFIG_Context_Init();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
/* USER CODE END UserCode */
