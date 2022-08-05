/******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/myliberty
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/
/*
 *      PROJECT:   
 *      $Revision: $
 *      LANGUAGE:  ANSI C
 */

/*! \file
 *
 *  \author 
 *
 *  \brief Implementation of common STM32 LPUART components
 *
 */
/*!
 * 
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __lpuart_H
#define __lpuart_H

/* Includes ------------------------------------------------------------------*/
#include "platform.h"

/*!
 *****************************************************************************
 *  \brief  Init function
 *
 *  \param[in] *hlpuart : pointer to a already initialized lpuart handle
 *
 *****************************************************************************
 */
void lpuartInit(UART_HandleTypeDef *hlpuart);

/*!
 *****************************************************************************
 *  \brief  Transmit data 
 * 
 * 
 * 
 *  \param[in] data : pointer to buffer to be transmitted.
 *
 *  \param[in] dataLen : buffer length
 *
 *  \return : HAL error code
 *
 *****************************************************************************
 */
uint8_t lpuartTx(uint8_t *data, uint16_t dataLen);

/*!
 *****************************************************************************
 *  \brief  Receive data
 *
 *  \param[out] data : pointer to buffer where data should be stored
 *
 *  \param[in] dataLen : buffer length
 *
 *  \return : HAL error code
 *
 *****************************************************************************
 */
uint8_t lpuartRx(uint8_t *data, uint16_t *dataLen);

#endif /*__lpuart_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
