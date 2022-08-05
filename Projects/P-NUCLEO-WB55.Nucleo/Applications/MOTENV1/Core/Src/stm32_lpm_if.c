 /*******************************************************************************
  * @file    stm32_lpm_if.c
  * @author  MCD Application Team
  * @brief   Low layer function to enter/exit low power modes (stop, sleep)
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
#include "stm32_lpm_if.h"
#include "stm32_lpm.h"
#include "app_conf.h"


/* Exported variables --------------------------------------------------------*/
const struct UTIL_LPM_Driver_s UTIL_PowerDriver = 
{
  PWR_EnterSleepMode,
  PWR_ExitSleepMode,
  
  PWR_EnterStopMode,
  PWR_ExitStopMode, 
  
  PWR_EnterOffMode,
  PWR_ExitOffMode,
};

/* Private functions prototypes-----------------------------------------------*/
static void Switch_On_HSI( void );


/* Functions Definition ------------------------------------------------------*/
/**
  * @brief Enters Low Power Off Mode
  * @param none
  * @retval none
  */
void PWR_EnterOffMode( void )
{
  /************************************************************************************
   * ENTER OFF MODE
   ***********************************************************************************/
  /*
   * There is no risk to clear all the WUF here because in the current implementation, this API is called
   * in critical section. If an interrupt occurs while in that critical section before that point,
   * the flag is set and will be cleared here but the system will not enter Off Mode
   * because an interrupt is pending in the NVIC. The ISR will be executed when moving out
   * of this critical section
   */
  LL_PWR_ClearFlag_WU( );

  LL_PWR_SetPowerMode( LL_PWR_MODE_STANDBY );

  LL_LPM_EnableDeepSleep( ); /**< Set SLEEPDEEP bit of Cortex System Control Register */

  /**
   * This option is used to ensure that store operations are completed
   */
#if defined ( __CC_ARM)
  __force_stores( );
#endif

  __WFI( );
}

/**
  * @brief Exits Low Power Off Mode
  * @param none
  * @retval none
  */
void PWR_ExitOffMode( void )
{
}

/**
  * @brief Enters Low Power Stop Mode
  * @note ARM exists the function when waking up
  * @param none
  * @retval none
  */
void PWR_EnterStopMode( void )
{
  /**
   * This function is called from CRITICAL SECTION
   */
  while( LL_HSEM_1StepLock( HSEM, CFG_HW_RCC_SEMID ) );

  if ( ! LL_HSEM_1StepLock( HSEM, CFG_HW_ENTRY_STOP_MODE_SEMID ) )
  {
    if( LL_PWR_IsActiveFlag_C2DS( ) )
    {
      /* Release ENTRY_STOP_MODE semaphore */
      LL_HSEM_ReleaseLock( HSEM, CFG_HW_ENTRY_STOP_MODE_SEMID, 0 );

      /**
       * The switch on HSI before entering Stop Mode is required on Cut2.0
       * It is useless from Cut2.1
       */
      Switch_On_HSI( );
    }
  }
  else
  {
    /**
     * The switch on HSI before entering Stop Mode is required on Cut2.0
     * It is useless from Cut2.1
     */
    Switch_On_HSI( );
  }

  /* Release RCC semaphore */
  LL_HSEM_ReleaseLock( HSEM, CFG_HW_RCC_SEMID, 0 );

  /************************************************************************************
   * ENTER STOP MODE
   ***********************************************************************************/
  LL_PWR_SetPowerMode( LL_PWR_MODE_STOP2 );

  LL_LPM_EnableDeepSleep( ); /**< Set SLEEPDEEP bit of Cortex System Control Register */

  /**
   * This option is used to ensure that store operations are completed
   */
#if defined ( __CC_ARM)
  __force_stores( );
#endif

  __WFI();
}

/**
  * @brief Exits Low Power Stop Mode
  * @note Enable the pll at 32MHz
  * @param none
  * @retval none
  */
void PWR_ExitStopMode( void )
{
  /**
   * This function is called from CRITICAL SECTION
   */

  /* Release ENTRY_STOP_MODE semaphore */
  LL_HSEM_ReleaseLock( HSEM, CFG_HW_ENTRY_STOP_MODE_SEMID, 0 );

  while( LL_HSEM_1StepLock( HSEM, CFG_HW_RCC_SEMID ) );

  if(LL_RCC_GetSysClkSource( ) == LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {
    LL_RCC_HSE_Enable( );
    while(!LL_RCC_HSE_IsReady( ));
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSE);
    while (LL_RCC_GetSysClkSource( ) != LL_RCC_SYS_CLKSOURCE_STATUS_HSE);
  }
  else
  {
    /**
     * As long as the current application is fine with HSE as system clock source,
     * there is nothing to do here
     */
  }

  /* Release RCC semaphore */
  LL_HSEM_ReleaseLock( HSEM, CFG_HW_RCC_SEMID, 0 );
}

/**
  * @brief Enters Low Power Sleep Mode
  * @note ARM exits the function when waking up
  * @param none
  * @retval none
  */
void PWR_EnterSleepMode( void )
{
  /************************************************************************************
   * ENTER SLEEP MODE
   ***********************************************************************************/
  LL_LPM_EnableSleep( ); /**< Clear SLEEPDEEP bit of Cortex System Control Register */

  /**
   * This option is used to ensure that store operations are completed
   */
#if defined ( __CC_ARM)
  __force_stores();
#endif

  __WFI( );
}

/**
  * @brief Exits Low Power Sleep Mode
  * @note ARM exits the function when waking up
  * @param none
  * @retval none
  */
void PWR_ExitSleepMode( void )
{
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
/**
  * @brief Switch the system clock on HSI
  * @param none
  * @retval none
  */
static void Switch_On_HSI( void )
{
  LL_RCC_HSI_Enable( );
  while(!LL_RCC_HSI_IsReady( ));
  LL_RCC_SetSysClkSource( LL_RCC_SYS_CLKSOURCE_HSI );
  while (LL_RCC_GetSysClkSource( ) != LL_RCC_SYS_CLKSOURCE_STATUS_HSI);
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

