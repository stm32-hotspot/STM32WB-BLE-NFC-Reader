/**
 ******************************************************************************
 * File Name          : motionfx_server_app.h
 * Description        : Handle SW/Sensor Data Fusion and ECompass Service/Char
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
#ifndef MOTIONFX_SERVER_APP_H
#define MOTIONFX_SERVER_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "iks01a3_motion_sensors.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void MOTIONFX_Context_Init(void);
void MOTIONFX_Set_Quat_Notification_Status(uint8_t status);
void MOTIONFX_Set_ECompass_Notification_Status(uint8_t status);
void MOTIONFX_Send_Quat_Notification_Task(void);
void MOTIONFX_Send_ECompass_Notification_Task(void);

uint8_t MOTIONFX_Get_MagCalStatus(void);
IKS01A3_MOTION_SENSOR_Axes_t *MOTIONFX_Get_MAG_Offset(void);

void MOTIONFX_ReCalibration(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTIONFX_SERVER_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
