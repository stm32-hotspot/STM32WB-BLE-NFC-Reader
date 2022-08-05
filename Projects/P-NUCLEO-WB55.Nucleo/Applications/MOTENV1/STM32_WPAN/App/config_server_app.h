/**
 ******************************************************************************
 * File Name          : config_server_app.h
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CONFIG_SERVER_APP_H
#define CONFIG_SERVER_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void CONFIG_Context_Init(void);
void CONFIG_Set_Notification_Status(uint8_t status);
void CONFIG_Set_FirstConnection_Config(uint8_t status);
uint8_t CONFIG_Get_Notification_Status(void);
uint8_t CONFIG_Get_FirstConnection_Config(void);
void CONFIG_Send_Notification(uint32_t Feature, uint8_t Command, uint8_t data);
uint8_t CONFIG_Parse_Command(uint8_t *att_data, uint8_t data_length);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_SERVER_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
