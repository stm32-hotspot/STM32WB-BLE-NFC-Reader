/**
 ******************************************************************************
  * File Name          : utilities_conf.h
  * Description        : Utilities configuration file for BLE 
  *                      middleWare.
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
#ifndef UTILITIES_CONF_H
#define UTILITIES_CONF_H

#ifdef __cplusplus
extern "C" {
#endif    
    
#include "cmsis_compiler.h"
#include "string.h"    
#include "app_conf.h"
  
/******************************************************************************
 * common
 ******************************************************************************/
#define UTILS_ENTER_CRITICAL_SECTION( )   uint32_t primask_bit = __get_PRIMASK( );\
                                          __disable_irq( )

#define UTILS_EXIT_CRITICAL_SECTION( )          __set_PRIMASK( primask_bit )

#define UTILS_MEMSET8( dest, value, size )      memset( dest, value, size);

/******************************************************************************
 * tiny low power manager
 * (any macro that does not need to be modified can be removed)
 ******************************************************************************/
#define UTIL_LPM_INIT_CRITICAL_SECTION( )
#define UTIL_LPM_ENTER_CRITICAL_SECTION( )      UTILS_ENTER_CRITICAL_SECTION( )
#define UTIL_LPM_EXIT_CRITICAL_SECTION( )       UTILS_EXIT_CRITICAL_SECTION( )


/******************************************************************************
 * sequencer
 * (any macro that does not need to be modified can be removed)
 ******************************************************************************/
#define UTIL_SEQ_INIT_CRITICAL_SECTION( )
#define UTIL_SEQ_ENTER_CRITICAL_SECTION( )      UTILS_ENTER_CRITICAL_SECTION( )
#define UTIL_SEQ_EXIT_CRITICAL_SECTION( )       UTILS_EXIT_CRITICAL_SECTION( )
#define UTIL_SEQ_CONF_TASK_NBR                  CFG_TASK_NBR
#define UTIL_SEQ_CONF_PRIO_NBR                  CFG_PRIO_NBR
#define UTIL_SEQ_MEMSET8( dest, value, size )   UTILS_MEMSET8( dest, value, size )

/******************************************************************************
 * Debug Trace
 ******************************************************************************/
/**
 * When DBG_TRACE_FULL is set to 1, the trace are output with the API name, the file name and the line number
 * When DBG_TRACE_LIGTH is set to 1, only the debug message is output
 *
 * When both are set to 0, no trace are output
 * When both are set to 1,  DBG_TRACE_FULL is selected
 */
#define DBG_TRACE_LIGTH     1
#define DBG_TRACE_FULL      0

#if (( CFG_DEBUG_TRACE != 0 ) && ( DBG_TRACE_LIGTH == 0 ) && (DBG_TRACE_FULL == 0))
#undef DBG_TRACE_FULL
#undef DBG_TRACE_LIGTH
#define DBG_TRACE_FULL      0
#define DBG_TRACE_LIGTH     1
#endif

#if ( CFG_DEBUG_TRACE == 0 )
#undef DBG_TRACE_FULL
#undef DBG_TRACE_LIGTH
#define DBG_TRACE_FULL      0
#define DBG_TRACE_LIGTH     0
#endif

/**
 * When not set, the traces is looping on sending the trace over UART
 */
#define DBG_TRACE_USE_CIRCULAR_QUEUE 1

/**
 * max buffer Size to queue data traces and max data trace allowed.
 * Only Used if DBG_TRACE_USE_CIRCULAR_QUEUE is defined
 */
#define DBG_TRACE_MSG_QUEUE_SIZE 4096
#define MAX_DBG_TRACE_MSG_SIZE 1024
                                            
#ifdef __cplusplus
}
#endif

#endif /*UTILITIES_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
