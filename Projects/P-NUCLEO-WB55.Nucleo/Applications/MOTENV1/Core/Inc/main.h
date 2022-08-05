/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wbxx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_GREEN_Pin GPIO_PIN_0     //Free LED_GREEN_Pin
#define LED_GREEN_GPIO_Port GPIOB
#define LED_RED_Pin GPIO_PIN_1       //Free LED_RED_Pin
#define LED_RED_GPIO_Port GPIOB
#define BUTTON_SW2_Pin GPIO_PIN_0
#define BUTTON_SW2_GPIO_Port GPIOD
#define BUTTON_SW2_EXTI_IRQn EXTI0_IRQn


/* USER CODE BEGIN Private defines */
#define LED_FIELD_Pin GPIO_PIN_13
#define LED_FIELD_GPIO_Port GPIOC
#define IRQ_3916_Pin GPIO_PIN_0
#define IRQ_3916_GPIO_Port GPIOC
#define IRQ_3916_EXTI_IRQn EXTI0_IRQn
#define LED_F_Pin GPIO_PIN_1
#define LED_F_GPIO_Port GPIOC
#define LED_AP2P_Pin GPIO_PIN_2
#define LED_AP2P_GPIO_Port GPIOC
#define LED_V_Pin GPIO_PIN_3
#define LED_V_GPIO_Port GPIOC
#define LED_A_Pin GPIO_PIN_0
#define LED_A_GPIO_Port GPIOA
#define LED_B_Pin GPIO_PIN_1
#define LED_B_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_4
#define SPI1_CS_GPIO_Port GPIOA
#define B1_Pin GPIO_PIN_4
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI4_IRQn
#define LD2_Pin GPIO_PIN_0
#define LD2_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_1
#define LD3_GPIO_Port GPIOB
#define JTMS_Pin GPIO_PIN_13
#define JTMS_GPIO_Port GPIOA
#define JTCK_Pin GPIO_PIN_14
#define JTCK_GPIO_Port GPIOA
//#define B2_Pin GPIO_PIN_0
//#define B2_GPIO_Port GPIOD
//#define B3_Pin GPIO_PIN_1
//#define B3_GPIO_Port GPIOD
#define JTDO_Pin GPIO_PIN_3
#define JTDO_GPIO_Port GPIOB
#define LD1_Pin GPIO_PIN_5
#define LD1_GPIO_Port GPIOB
//#define STLINK_RX_Pin GPIO_PIN_6
//#define STLINK_RX_GPIO_Port GPIOB
//#define STLINK_TX_Pin GPIO_PIN_7
//#define STLINK_TX_GPIO_Port GPIOB


/* USER CODE BEGIN Private defines */

#define LSM6DSL_INT1_O_GPIO_PIN         GPIO_PIN_10
#define LSM6DSL_INT1_O_GPIO_PORT        GPIOC

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
