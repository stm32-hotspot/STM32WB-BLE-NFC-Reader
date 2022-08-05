/**
  ******************************************************************************
  * @file           :
  * @version        :
  * @brief          :
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  * 1. Redistributions of source code must retain the above copyright notice,
  * this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  * this list of conditions and the following disclaimer in the documentation
  * and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of its contributors
  * may be used to endorse or promote products derived from this software
  * without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/

#include "lpuart.h"
#include "st_errno.h"

#define LPUART_TIMEOUT          1000

UART_HandleTypeDef *pLpuart = 0;

/* init function */
void lpuartInit(UART_HandleTypeDef *hlpuart)
{
  pLpuart = hlpuart;
}

uint8_t lpuartTx(uint8_t *data, uint16_t dataLen)
{
  if(pLpuart == 0)
    return ERR_INVALID_HANDLE;

  HAL_UART_Transmit(pLpuart, data, dataLen, LPUART_TIMEOUT);
  return ERR_NONE;
}

uint8_t lpuartRx(uint8_t *data, uint16_t *dataLen)
{
  if(pLpuart == 0)
    return ERR_INVALID_HANDLE;

  for(uint8_t i = 0; i < *dataLen; i++) {
    HAL_UART_Receive(pLpuart, &data[i], 1, LPUART_TIMEOUT);
    if(data[i] == 0) {
      *dataLen = i;
    }
  }
  return ERR_NONE;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
