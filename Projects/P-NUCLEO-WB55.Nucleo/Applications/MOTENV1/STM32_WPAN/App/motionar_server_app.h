/**
 ******************************************************************************
 * File Name          : motionar_server_app.h
 * Description        : Handle SW/Activity Recognition Service/Char
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MOTIONAR_SERVER_APP_H
#define MOTIONAR_SERVER_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void MOTIONAR_Context_Init(void);
void MOTIONAR_Set_Notification_Status(uint8_t status);
void MOTIONAR_Send_Notification_Task(void);
void MOTIONAR_ActivityRec_Update(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTIONAR_SERVER_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
