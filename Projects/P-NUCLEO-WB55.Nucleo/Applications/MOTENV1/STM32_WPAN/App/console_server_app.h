/**
 ******************************************************************************
 * File Name          : console_server_app.h
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CONSOLE_SERVER_APP_H
#define CONSOLE_SERVER_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void CONSOLE_Context_Init(void);
void CONSOLE_Set_Term_Notification_Status(uint8_t status);
void CONSOLE_Term_Update(uint8_t *data, uint8_t length);
void CONSOLE_Term_Update_AfterRead(void);
void CONSOLE_Set_Stderr_Notification_Status(uint8_t status);
void CONSOLE_Stderr_Update(uint8_t *data, uint8_t length);
void CONSOLE_Stderr_Update_AfterRead(void);

#ifdef __cplusplus
}
#endif

#endif /* CONSOLE_SERVER_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
