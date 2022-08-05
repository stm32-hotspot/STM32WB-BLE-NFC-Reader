/**
 ******************************************************************************
 * File Name          : console_server_app.c
 * Description        : Handle Console Service/Chars
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
#include "console_server_app.h"

/* Private defines -----------------------------------------------------------*/
#define CONSOLE_MAX_CHAR_LEN (20)

/* Private typedef -----------------------------------------------------------*/

/**
 * @brief  Console Service Context structure definition
 */
typedef struct
{
  uint8_t TermNotificationStatus;
  uint8_t StderrNotificationStatus;
  uint8_t ConsoleIsInitalized;
  uint8_t LastTermBuffer[CONSOLE_MAX_CHAR_LEN];
  uint8_t LastTermLen;

} CONSOLE_Server_App_Context_t;

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

PLACE_IN_SECTION("BLE_APP_CONTEXT") static CONSOLE_Server_App_Context_t CONSOLE_Server_App_Context;

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Init the Console Service/Chars Context
 * @param  None
 * @retval None
 */
void CONSOLE_Context_Init(void)
{
  CONSOLE_Set_Term_Notification_Status(0);
  CONSOLE_Set_Stderr_Notification_Status(0);
}

/**
 * @brief  Set the Term notification status (enabled/disabled)
 * @param  status The new notification status
 * @retval None
 */
void CONSOLE_Set_Term_Notification_Status(uint8_t status)
{
  CONSOLE_Server_App_Context.TermNotificationStatus = status;
}

/**
 * @brief  Set the Stderr notification status (enabled/disabled)
 * @param  status The new notification status
 * @retval None
 */
void CONSOLE_Set_Stderr_Notification_Status(uint8_t status)
{
  CONSOLE_Server_App_Context.StderrNotificationStatus = status;
}

/**
 * @brief  Send a notification for Terminal char
 * @param  data String to write
 * @param  lenght Lengt of string to write
 * @retval None
 */
void CONSOLE_Term_Update(uint8_t *data, uint8_t length)
{
  uint8_t Offset;
  uint8_t DataToSend;

  if(CONSOLE_Server_App_Context.TermNotificationStatus)
  {
    /* Split the code in packages */
    for(Offset =0; Offset<length; Offset += CONSOLE_MAX_CHAR_LEN)
    {
      DataToSend = (length-Offset);
      DataToSend = (DataToSend>CONSOLE_MAX_CHAR_LEN) ?  CONSOLE_MAX_CHAR_LEN : DataToSend;
      
      /* keep a copy */
      memcpy(CONSOLE_Server_App_Context.LastTermBuffer,data+Offset,DataToSend);
      CONSOLE_Server_App_Context.LastTermLen = DataToSend;
      
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- CONSOLE APPLICATION SERVER : NOTIFY CLIENT WITH NEW TERM PARAMETER VALUE \n ");
      APP_DBG_MSG(" \n\r");
#endif
      MOTENV_STM_App_Update_Char(CONSOLE_TERM_CHAR_UUID, DataToSend, data+Offset);
      
      HAL_Delay(20);
    }
  }

  return;
}

/**
 * @brief Send a notification for Terminal char value after a read request
 * @param None
 * @retval None
 */
void CONSOLE_Term_Update_AfterRead(void)
{
  if(CONSOLE_Server_App_Context.TermNotificationStatus)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- CONSOLE APPLICATION SERVER : NOTIFY CLIENT WITH NEW TERM PARAMETER VALUE \n ");
      APP_DBG_MSG(" \n\r");
#endif
      MOTENV_STM_App_Update_Char(CONSOLE_TERM_CHAR_UUID, CONSOLE_Server_App_Context.LastTermLen, CONSOLE_Server_App_Context.LastTermBuffer);
  }

  return;
}

/**
 * @brief  Send a notification for Stderr char
 * @param  data String to write
 * @param  lenght Lengt of string to write
 * @retval None
 */
void CONSOLE_Stderr_Update(uint8_t *data, uint8_t length)
{
  uint8_t Offset;
  uint8_t DataToSend;

  if(CONSOLE_Server_App_Context.StderrNotificationStatus)
  {
    /* Split the code in packages */
    for(Offset =0; Offset<length; Offset += CONSOLE_MAX_CHAR_LEN)
    {
      DataToSend = (length-Offset);
      DataToSend = (DataToSend>CONSOLE_MAX_CHAR_LEN) ?  CONSOLE_MAX_CHAR_LEN : DataToSend;
      
      /* keep a copy */
      memcpy(CONSOLE_Server_App_Context.LastTermBuffer,data+Offset,DataToSend);
      CONSOLE_Server_App_Context.LastTermLen = DataToSend;
      
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- CONSOLE APPLICATION SERVER : NOTIFY CLIENT WITH NEW STDERR PARAMETER VALUE \n ");
      APP_DBG_MSG(" \n\r");
#endif
      MOTENV_STM_App_Update_Char(CONSOLE_STDERR_CHAR_UUID, DataToSend, data+Offset);
      
      HAL_Delay(10);
    }
  }

  return;
}

/**
 * @brief Send a notification for Stderr char value after a read request
 * @param None
 * @retval None
 */
void CONSOLE_Stderr_Update_AfterRead(void)
{
  if(CONSOLE_Server_App_Context.StderrNotificationStatus)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("-- CONSOLE APPLICATION SERVER : NOTIFY CLIENT WITH NEW STDERR PARAMETER VALUE \n ");
      APP_DBG_MSG(" \n\r");
#endif
      MOTENV_STM_App_Update_Char(CONSOLE_TERM_CHAR_UUID, CONSOLE_Server_App_Context.LastTermLen, CONSOLE_Server_App_Context.LastTermBuffer);
  }

  return;
}

/* Private functions ---------------------------------------------------------*/

 /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
