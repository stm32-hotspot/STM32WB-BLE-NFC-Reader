/**
 ******************************************************************************
 * File Name          : config_server_app.c
 * Description        : Handle Configuration Service/Char
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
#include "config_server_app.h"
#include "motionfx_server_app.h"
#include "motion_ext_server_app.h"

/* Private defines -----------------------------------------------------------*/
#define VALUE_LEN_CONFIG        (2+4+1+1)

/* Private typedef -----------------------------------------------------------*/

/**
 * @brief  Configuration Service/Char Context structure definition
 */
typedef struct
{
  uint8_t NotificationStatus;

  uint8_t FirstConnectionConfig;

} CONFIG_Server_App_Context_t;

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

PLACE_IN_SECTION("BLE_APP_CONTEXT") static CONFIG_Server_App_Context_t CONFIG_Server_App_Context;

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Init the Configuration Service/Char Context
 * @param  None
 * @retval None
 */
void CONFIG_Context_Init(void)
{
  CONFIG_Set_Notification_Status(0);
  CONFIG_Set_FirstConnection_Config(0);
}

/**
 * @brief  Set the notification status (enabled/disabled)
 * @param  status The new notification status
 * @retval None
 */
void CONFIG_Set_Notification_Status(uint8_t status)
{
  CONFIG_Server_App_Context.NotificationStatus = status;
}

/**
 * @brief  Read the current notification status (enabled/disabled)
 * @param  None
 * @retval The current notification
 */
uint8_t CONFIG_Get_Notification_Status(void)
{
  return CONFIG_Server_App_Context.NotificationStatus;
}

/**
 * @brief  Set the flag for notifying the calibration info
 * @param  status The new flag status
 * @retval None
 */
void CONFIG_Set_FirstConnection_Config(uint8_t status)
{
  CONFIG_Server_App_Context.FirstConnectionConfig = status;
}

/**
 * @brief  Get the flag for notifying the calibration info
 * @param  None
 * @retval The current flag status
 */
uint8_t CONFIG_Get_FirstConnection_Config(void)
{
  return CONFIG_Server_App_Context.FirstConnectionConfig;
}

/**
 * @brief  Send a notification for answering to a configuration command for Accelerometer events
 * @param  Feature Feature calibrated
 * @param  Command Reply to this Command
 * @param  data Result to send back
 * @retval None
 */
void CONFIG_Send_Notification(uint32_t Feature, uint8_t Command, uint8_t data)
{
  uint8_t value[VALUE_LEN_CONFIG];

  /* Timestamp */
  STORE_LE_16(value  ,(HAL_GetTick()>>3));
  STORE_BE_32(value+2,Feature);
  value[6] = Command;
  value[7] = data;

  if(CONFIG_Server_App_Context.NotificationStatus)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- CONFIG APPLICATION SERVER : NOTIFY CLIENT WITH NEW CONFIG PARAMETER VALUE \n ");
    APP_DBG_MSG(" \n\r");
#endif
    MOTENV_STM_App_Update_Char(CONFIG_CHAR_UUID, VALUE_LEN_CONFIG, (uint8_t *)&value);
  }
  else
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- CONFIG APPLICATION SERVER : CAN'T INFORM CLIENT - NOTIFICATION DISABLED\n ");
#endif
  }

  return;
}

/**
 * @brief Make the parsing of the Configuration Commands
 * @param att_data Attribute data
 * @param data_length Length of the data
 * @retval SendItBack true/false
 */
uint8_t CONFIG_Parse_Command(uint8_t *att_data, uint8_t data_length)
{
  uint32_t FeatureMask = (att_data[3]) | (att_data[2]<<8) | (att_data[1]<<16) | (att_data[0]<<24);
  uint8_t Command = att_data[4];
  uint8_t Data    = att_data[5];
  uint8_t SendItBack = 1;

  switch (FeatureMask)
  {
  case FEATURE_MASK_SENSORFUSION_SHORT:
  case FEATURE_MASK_ECOMPASS:
    /* Sensor Fusion */
    switch (Command)
    {
    case W2ST_COMMAND_CAL_STATUS:
      /* Reply with the calibration status for the feature */
      /* Control the calibration status */
      {
        CONFIG_Send_Notification(FeatureMask, Command, MOTIONFX_Get_MagCalStatus() ? 100: 0);
      }
      break;
    case W2ST_COMMAND_CAL_RESET:
      /* Reset the calibration */
      MOTIONFX_ReCalibration();
      break;
    case W2ST_COMMAND_CAL_STOP:
      /* Do nothing in this case */
      break;
    default:
      break;
    }
    break;
    
  case FEATURE_MASK_ACC_EVENTS:
    /* Acc events */
    switch (Command)
    {
    case 'm':
      /* Multiple Events */
      switch(Data) {
      case 1:
        MOTION_EXT_Enable_Feature(EXT_HWF_MULTIPLE_EVENTS);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      case 0:
        MOTION_EXT_Disable_Feature(EXT_HWF_MULTIPLE_EVENTS);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      }
      break;
    case 'f':
      /* FreeFall */
      switch(Data) {
      case 1:
        MOTION_EXT_Enable_Feature(EXT_HWF_FREE_FALL);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      case 0:
        MOTION_EXT_Disable_Feature(EXT_HWF_FREE_FALL);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      }
      break;
    case 'd':
      /* Double Tap */
      switch(Data) {
      case 1:
        MOTION_EXT_Enable_Feature(EXT_HWF_DOUBLE_TAP);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      case 0:
        MOTION_EXT_Disable_Feature(EXT_HWF_DOUBLE_TAP);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      }
      break;
    case 's':
      /* Single Tap */
      switch(Data) {
      case 1:
        MOTION_EXT_Enable_Feature(EXT_HWF_SINGLE_TAP);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      case 0:
        MOTION_EXT_Disable_Feature(EXT_HWF_SINGLE_TAP);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      }
      break;
    case 'p':
      /* Pedometer */
      switch(Data) {
      case 1:
        MOTION_EXT_Enable_Feature(EXT_HWF_PEDOMETER);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      case 0:
        MOTION_EXT_Disable_Feature(EXT_HWF_PEDOMETER);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      }
      break;
    case 'w':
      /* Wake UP */
      switch(Data) {
      case 1:
        MOTION_EXT_Enable_Feature(EXT_HWF_WAKE_UP);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      case 0:
        MOTION_EXT_Disable_Feature(EXT_HWF_WAKE_UP);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      }
      break;
    case 't':
      /* Tilt */
      switch(Data) {
      case 1:
        MOTION_EXT_Enable_Feature(EXT_HWF_TILT);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      case 0:
        MOTION_EXT_Disable_Feature(EXT_HWF_TILT);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      }
      break;
    case 'o' :
      /* 6DOrientation */
      switch(Data) {
      case 1:
        MOTION_EXT_Enable_Feature(EXT_HWF_6D_ORIENTATION);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      case 0:
        MOTION_EXT_Disable_Feature(EXT_HWF_6D_ORIENTATION);
        CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,Command,Data);
        break;
      }
      break;
    }
    break;
  }

  return SendItBack;
}

/* Private functions ---------------------------------------------------------*/

 /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
