/**
 ******************************************************************************
 * File Name          : motion_ext_server_app.h
 * Description        : Handle HW/Motion Extended (Acc Events) Service/Char
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
#ifndef MOTION_EXT_SERVER_APP_H
#define MOTION_EXT_SERVER_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/

/**
 * @brief  Pedometer event
 */
#define EXT_HWF_PEDOMETER        (1   )
/**
 * @brief  Free Fall event
 */
#define EXT_HWF_FREE_FALL        (1<<1)
/**
 * @brief  Double tap event
 */
#define EXT_HWF_DOUBLE_TAP       (1<<2)
/**
 * @brief  Single tap event
 */
#define EXT_HWF_SINGLE_TAP       (1<<3)
/**
 * @brief  WakeUp event
 */
#define EXT_HWF_WAKE_UP          (1<<4)
/**
 * @brief  Tilt event
 */
#define EXT_HWF_TILT             (1<<5)
/**
 * @brief  6D Orientation event
 */
#define EXT_HWF_6D_ORIENTATION   (1<<6)
/**
 * @brief  Multiple events
 */
#define EXT_HWF_MULTIPLE_EVENTS  (1<<7)

/* Exported functions ------------------------------------------------------- */
void MOTION_EXT_Context_Init(void);
void MOTION_EXT_Set_Notification_Status(uint8_t status);
void MOTION_EXT_Enable_Feature(uint8_t feature);
void MOTION_EXT_Disable_Feature(uint8_t feature);
void MOTION_EXT_ReadCB(void);
void MOTION_EXT_Handle_IT(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTION_EXT_SERVER_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
