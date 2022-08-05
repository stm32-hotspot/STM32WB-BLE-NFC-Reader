/**
 ******************************************************************************
 * File Name          : motion_ext_server_app.c
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
/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "ble.h"
#include "dbg_trace.h"

#include "motenv_server_app.h"
#include "motion_ext_server_app.h"
#include "config_server_app.h"

#include "iks01a3_motion_sensors_ex.h"

/* Private defines -----------------------------------------------------------*/

#define EXT_CHECK_HW_FEATURE(Feature) ((HWExtFeaturesStatus&(Feature)) ? 1 : 0)
#define EXT_ON_HW_FEATURE(Feature)    (HWExtFeaturesStatus|=(Feature))
#define EXT_OFF_HW_FEATURE(Feature)   (HWExtFeaturesStatus&=(~Feature))

#define VALUE_LEN_SMALL (2+2)
#define VALUE_LEN_LARGE (2+3)

/* Private typedef -----------------------------------------------------------*/

/**
 * @brief  Enumerate the Acc event type
 */
typedef enum
{
  ACC_NOT_USED     = 0x00,
  ACC_6D_OR_TOP    = 0x01,
  ACC_6D_OR_LEFT   = 0x02,
  ACC_6D_OR_BOTTOM = 0x03,
  ACC_6D_OR_RIGTH  = 0x04,
  ACC_6D_OR_UP     = 0x05,
  ACC_6D_OR_DOWN   = 0x06,
  ACC_TILT         = 0x08,
  ACC_FREE_FALL    = 0x10,
  ACC_SINGLE_TAP   = 0x20,
  ACC_DOUBLE_TAP   = 0x40,
  ACC_WAKE_UP      = 0x80
} MOTION_EXT_Server_App_AccEvent_t;

/**
 * @brief  HW/Motion Extended (Acc Events) Service/Char Context structure definition
 */
typedef struct
{
  uint8_t NotificationStatus;

  uint8_t MultiEventEnabled;
  uint16_t PedometerStepCount;

  float defaultODR;
} MOTION_EXT_Server_App_Context_t;

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

PLACE_IN_SECTION("BLE_APP_CONTEXT") static MOTION_EXT_Server_App_Context_t MOTION_EXT_Server_App_Context;

static uint32_t HWExtFeaturesStatus = 0;

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void EnableExtFeatures(void);
static void DisableExtFeatures(void);
static void AccEvent_Notify(uint16_t Command, uint8_t dimByte);
static void Enable_Pedometer(void);
static void Disable_Pedometer(void);
static void Enable_FreeFall(void);
static void Disable_FreeFall(void);
static void Enable_DoubleTap(void);
static void Disable_DoubleTap(void);
static void Enable_SingleTap(void);
static void Disable_SingleTap(void);
static void Enable_WakeUp(void);
static void Disable_WakeUp(void);
static void Enable_Tilt(void);
static void Disable_Tilt(void);
static void Enable_6D_Orientation(void);
static void Disable_6D_Orientation(void);
static void Enable_MultiEvent(void);
static void Disable_MultiEvent(void);
static MOTION_EXT_Server_App_AccEvent_t GetHWOrientation6D(void);

/* Functions Definition ------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Init the HW/Motion Extended (Acc Events) Service/Char Context
 * @param  None
 * @retval None
 */
void MOTION_EXT_Context_Init(void)
{
  int32_t decPart, intPart;

  /* Save the initial Output Data Rate */
  IKS01A3_MOTION_SENSOR_GetOutputDataRate(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, &MOTION_EXT_Server_App_Context.defaultODR);
  MCR_BLUEMS_F2I_2D(MOTION_EXT_Server_App_Context.defaultODR, intPart, decPart);
  MOTION_EXT_Server_App_Context.defaultODR = intPart*100+decPart;

#if(CFG_DEBUG_APP_TRACE != 0)
  APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ODR=%d.%02d [Hz] \n ", (int)intPart, (int)decPart);
  APP_DBG_MSG(" \n\r");
#endif

  MOTION_EXT_Set_Notification_Status(0);

  MOTION_EXT_Server_App_Context.MultiEventEnabled = 0;
}

/**
 * @brief  Set the notification status (enabled/disabled)
 * @param  status The new notification status
 * @retval None
 */
void MOTION_EXT_Set_Notification_Status(uint8_t status)
{
  MOTION_EXT_Server_App_Context.NotificationStatus = status;
  if(status == 1)
  {
    Enable_MultiEvent();
    CONFIG_Send_Notification(FEATURE_MASK_ACC_EVENTS,'m',1);
  }
  else if(status == 0)
  {
    Disable_MultiEvent();
  }
}

/**
 * @brief  Enable a specific Extended feature according to the request GATT Client (ST BLE Sensor App)
 * @param  feature The feature to be enabled
 * @retval None
 */
void MOTION_EXT_Enable_Feature(uint8_t feature)
{
  if(MOTION_EXT_Server_App_Context.MultiEventEnabled == 1)
  {
    DisableExtFeatures();
  }

  switch(feature)
  {
  case EXT_HWF_PEDOMETER:
    Enable_Pedometer();
    break;

  case EXT_HWF_FREE_FALL:
    Enable_FreeFall();
    break;

  case EXT_HWF_DOUBLE_TAP:
    Enable_DoubleTap();
    break;

  case EXT_HWF_SINGLE_TAP:
    Enable_SingleTap();
    break;

  case EXT_HWF_WAKE_UP:
    Enable_WakeUp();
    break;

  case EXT_HWF_TILT:
    Enable_Tilt();
    break;

  case EXT_HWF_6D_ORIENTATION:
    Enable_6D_Orientation();
    break;

  case EXT_HWF_MULTIPLE_EVENTS:
    Enable_MultiEvent();
    break;
  }
  
}

/**
 * @brief  Disable a specific Extended feature according to the request GATT Client (ST BLE Sensor App)
 * @param  feature The feature to be disabled
 * @retval None
 */
void MOTION_EXT_Disable_Feature(uint8_t feature)
{
  switch(feature)
  {
  case EXT_HWF_PEDOMETER:
    Disable_Pedometer();
    break;

  case EXT_HWF_FREE_FALL:
    Disable_FreeFall();
    break;

  case EXT_HWF_DOUBLE_TAP:
    Disable_DoubleTap();
    break;

  case EXT_HWF_SINGLE_TAP:
    Disable_SingleTap();
    break;

  case EXT_HWF_WAKE_UP:
    Disable_WakeUp();
    break;

  case EXT_HWF_TILT:
    Disable_Tilt();
    break;

  case EXT_HWF_6D_ORIENTATION:
    Disable_6D_Orientation();
    break;

  case EXT_HWF_MULTIPLE_EVENTS:
    Disable_MultiEvent();
    break;
  }
}

/**
 * @brief  Send a notification for Step Count on read request from GATT Client (ST BLE Sensor App)
 * @param  None
 * @retval None
 */
void MOTION_EXT_ReadCB(void)
{
  uint16_t StepCount = 0;

  if(EXT_CHECK_HW_FEATURE(EXT_HWF_PEDOMETER))
  {
    IKS01A3_MOTION_SENSOR_Get_Step_Count(IKS01A3_LSM6DSO_0, &StepCount); 
  }
  AccEvent_Notify(StepCount, 2);
}

/**
  * @brief  Send a notification when there is an interrupt from MEMS
  * @param  None
  * @retval None
  */
void MOTION_EXT_Handle_IT(void)
{
  IKS01A3_MOTION_SENSOR_Event_Status_t status;

//  APP_DBG_MSG("*** IRQ ON PC10 Detected *** \n ");

  if (IKS01A3_MOTION_SENSOR_Get_Event_Status(IKS01A3_LSM6DSO_0, &status) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR GETTING EVENT STATUS\n ");
#endif
    return;
  }

  if((EXT_CHECK_HW_FEATURE(EXT_HWF_PEDOMETER)) ||
     (EXT_CHECK_HW_FEATURE(EXT_HWF_MULTIPLE_EVENTS)) )
  {
    /* Check if the interrupt is due to Pedometer */
    if(status.StepStatus != 0)
    {
      IKS01A3_MOTION_SENSOR_Get_Step_Count(IKS01A3_LSM6DSO_0, &MOTION_EXT_Server_App_Context.PedometerStepCount);
      if(EXT_CHECK_HW_FEATURE(EXT_HWF_PEDOMETER))
      {
        AccEvent_Notify(MOTION_EXT_Server_App_Context.PedometerStepCount, 2);
      }
    }
  }

  if((EXT_CHECK_HW_FEATURE(EXT_HWF_FREE_FALL)) ||
     (EXT_CHECK_HW_FEATURE(EXT_HWF_MULTIPLE_EVENTS)))
  {
    /* Check if the interrupt is due to Free Fall */
    if(status.FreeFallStatus != 0)
    {
      AccEvent_Notify(ACC_FREE_FALL, 2);
    }
  }

  if((EXT_CHECK_HW_FEATURE(EXT_HWF_SINGLE_TAP)) ||
     (EXT_CHECK_HW_FEATURE(EXT_HWF_MULTIPLE_EVENTS)))
  {
    /* Check if the interrupt is due to Single Tap */
    if(status.TapStatus != 0)
    {
      AccEvent_Notify(ACC_SINGLE_TAP, 2);
    }
  }

  if((EXT_CHECK_HW_FEATURE(EXT_HWF_DOUBLE_TAP)) ||
     (EXT_CHECK_HW_FEATURE(EXT_HWF_MULTIPLE_EVENTS)))
  {
    /* Check if the interrupt is due to Double Tap */
    if(status.DoubleTapStatus != 0)
    {
      AccEvent_Notify(ACC_DOUBLE_TAP, 2);
    }
  }

  if((EXT_CHECK_HW_FEATURE(EXT_HWF_TILT)) ||
     (EXT_CHECK_HW_FEATURE(EXT_HWF_MULTIPLE_EVENTS)))
  {
    /* Check if the interrupt is due to Tilt */
    if(status.TiltStatus != 0)
    {
      AccEvent_Notify(ACC_TILT, 2);
    }
  }

  if((EXT_CHECK_HW_FEATURE(EXT_HWF_6D_ORIENTATION)) ||
     (EXT_CHECK_HW_FEATURE(EXT_HWF_MULTIPLE_EVENTS)))
  {
    /* Check if the interrupt is due to 6D Orientation */
    if(status.D6DOrientationStatus != 0)
    {
      MOTION_EXT_Server_App_AccEvent_t Orientation = GetHWOrientation6D();
      AccEvent_Notify(Orientation, 2);
    }
  }

  if(EXT_CHECK_HW_FEATURE(EXT_HWF_WAKE_UP))
  {
    /* Check if the interrupt is due to Wake Up */
    if(status.WakeUpStatus != 0)
    {
      AccEvent_Notify(ACC_WAKE_UP, 2);
    }
  }

  if(EXT_CHECK_HW_FEATURE(EXT_HWF_MULTIPLE_EVENTS))
  {
    AccEvent_Notify(MOTION_EXT_Server_App_Context.PedometerStepCount, 3);
  }
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Enable Pedometer Extended Feature
 * @param  None
 * @retval None
 */
static void Enable_Pedometer(void)
{
  if (IKS01A3_MOTION_SENSOR_Enable_Pedometer(IKS01A3_LSM6DSO_0) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR ENABLING PEDOMETER\n ");
#endif
  }
  else
  {
    EXT_ON_HW_FEATURE(EXT_HWF_PEDOMETER);
  }
  IKS01A3_MOTION_SENSOR_Reset_Step_Counter(IKS01A3_LSM6DSO_0);
}

/**
 * @brief  Disable Pedometer Extended Feature
 * @param  None
 * @retval None
 */
static void Disable_Pedometer(void)
{
  if (IKS01A3_MOTION_SENSOR_Disable_Pedometer(IKS01A3_LSM6DSO_0) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR DISABLING PEDOMETER\n ");
#endif
  }
  else
  {
    EXT_OFF_HW_FEATURE(EXT_HWF_PEDOMETER);
  }
  if (IKS01A3_MOTION_SENSOR_SetOutputDataRate(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, MOTION_EXT_Server_App_Context.defaultODR) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR SETTING DEFAULT ODR\n ");
#endif
  }
}

/**
 * @brief  Enable Free Fall Extended Feature
 * @param  None
 * @retval None
 */
static void Enable_FreeFall(void)
{
  if (IKS01A3_MOTION_SENSOR_Enable_Free_Fall_Detection(IKS01A3_LSM6DSO_0, IKS01A3_MOTION_SENSOR_INT1_PIN) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR ENABLING FREE FALL\n ");
#endif
  }
  else
  {
    EXT_ON_HW_FEATURE(EXT_HWF_FREE_FALL);
  }
  if (IKS01A3_MOTION_SENSOR_Set_Free_Fall_Threshold(IKS01A3_LSM6DSO_0, LSM6DSO_FF_TSH_250mg) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR SETTING FF_TSH\n ");
#endif
  }
}

/**
 * @brief  Disable Free Fall Extended Feature
 * @param  None
 * @retval None
 */
static void Disable_FreeFall(void)
{
  if (IKS01A3_MOTION_SENSOR_Disable_Free_Fall_Detection(IKS01A3_LSM6DSO_0) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR DISABLING FREE FALL\n ");
#endif
  }
  else
  {
    EXT_OFF_HW_FEATURE(EXT_HWF_FREE_FALL);
  }
  if (IKS01A3_MOTION_SENSOR_SetOutputDataRate(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, MOTION_EXT_Server_App_Context.defaultODR) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR SETTING DEFAULT ODR\n ");
#endif
  }
}

/**
 * @brief  Enable DoubleTap Extended Feature
 * @param  None
 * @retval None
 */
static void Enable_DoubleTap(void)
{
  if (IKS01A3_MOTION_SENSOR_Enable_Double_Tap_Detection(IKS01A3_LSM6DSO_0, IKS01A3_MOTION_SENSOR_INT1_PIN) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR ENABLING DOUBLE TAP\n ");
#endif
  }
  else
  {
    EXT_ON_HW_FEATURE(EXT_HWF_DOUBLE_TAP);
  }
  if (IKS01A3_MOTION_SENSOR_Set_Tap_Threshold(IKS01A3_LSM6DSO_0, 0x10/*LSM6DSL_TAP_THRESHOLD_MID*/) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR SETTING TAP_TSH\n ");
#endif
  }
}

/**
 * @brief  Disable DoubleTap Extended Feature
 * @param  None
 * @retval None
 */
static void Disable_DoubleTap(void)
{
  if (IKS01A3_MOTION_SENSOR_Disable_Double_Tap_Detection(IKS01A3_LSM6DSO_0) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR DISABLING DOUBLE TAP\n ");
#endif
  }
  else
  {
    EXT_OFF_HW_FEATURE(EXT_HWF_DOUBLE_TAP);
  }
  if (IKS01A3_MOTION_SENSOR_SetOutputDataRate(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, MOTION_EXT_Server_App_Context.defaultODR) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR SETTING DEFAULT ODR\n ");
#endif
  }
}

/**
 * @brief  Enable SingleTap Extended Feature
 * @param  None
 * @retval None
 */
static void Enable_SingleTap(void)
{
  if (IKS01A3_MOTION_SENSOR_Enable_Single_Tap_Detection(IKS01A3_LSM6DSO_0, IKS01A3_MOTION_SENSOR_INT1_PIN) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR ENABLING SINGLE TAP\n ");
#endif
  }
  else
  {
    EXT_ON_HW_FEATURE(EXT_HWF_SINGLE_TAP);
  }
}

/**
 * @brief  Disable SingleTap Extended Feature
 * @param  None
 * @retval None
 */
static void Disable_SingleTap(void)
{
  if (IKS01A3_MOTION_SENSOR_Disable_Single_Tap_Detection(IKS01A3_LSM6DSO_0) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR DISABLING SINGLE TAP\n ");
#endif
  }
  else
  {
    EXT_OFF_HW_FEATURE(EXT_HWF_SINGLE_TAP);
  }
  if (IKS01A3_MOTION_SENSOR_SetOutputDataRate(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, MOTION_EXT_Server_App_Context.defaultODR) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR SETTING DEFAULT ODR\n ");
#endif
  }
}

/**
 * @brief  Enable WakeUp Extended Feature
 * @param  None
 * @retval None
 */
static void Enable_WakeUp(void)
{
  if (IKS01A3_MOTION_SENSOR_Enable_Wake_Up_Detection(IKS01A3_LSM6DSO_0, IKS01A3_MOTION_SENSOR_INT1_PIN) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR ENABLING WAKE UP\n ");
#endif
  }
  else
  {
    EXT_ON_HW_FEATURE(EXT_HWF_WAKE_UP);
  }
}

/**
 * @brief  Disable WakeUp Extended Feature
 * @param  None
 * @retval None
 */
static void Disable_WakeUp(void)
{
  if (IKS01A3_MOTION_SENSOR_Disable_Wake_Up_Detection(IKS01A3_LSM6DSO_0) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR DISABLING WAKE UP\n ");
#endif
  }
  else
  {
    EXT_OFF_HW_FEATURE(EXT_HWF_WAKE_UP);
  }
  if (IKS01A3_MOTION_SENSOR_SetOutputDataRate(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, MOTION_EXT_Server_App_Context.defaultODR) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR SETTING DEFAULT ODR\n ");
#endif
  }
}

/**
 * @brief  Enable Tilt Extended Feature
 * @param  None
 * @retval None
 */
static void Enable_Tilt(void)
{
  if (IKS01A3_MOTION_SENSOR_Enable_Tilt_Detection(IKS01A3_LSM6DSO_0, IKS01A3_MOTION_SENSOR_INT1_PIN) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR ENABLING TILT\n ");
#endif
  }
  else
  {
    EXT_ON_HW_FEATURE(EXT_HWF_TILT);
  }
}

/**
 * @brief  Disable Tilt Extended Feature
 * @param  None
 * @retval None
 */
static void Disable_Tilt(void)
{
  if (IKS01A3_MOTION_SENSOR_Disable_Tilt_Detection(IKS01A3_LSM6DSO_0) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR DISABLING TILT\n ");
#endif
  }
  else
  {
    EXT_OFF_HW_FEATURE(EXT_HWF_TILT);
  }
  if (IKS01A3_MOTION_SENSOR_SetOutputDataRate(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, MOTION_EXT_Server_App_Context.defaultODR) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR SETTING DEFAULT ODR\n ");
#endif
  }
}

/**
 * @brief  Enable 6D Orientation Extended Feature
 * @param  None
 * @retval None
 */
static void Enable_6D_Orientation(void)
{
  if (IKS01A3_MOTION_SENSOR_Enable_6D_Orientation(IKS01A3_LSM6DSO_0, IKS01A3_MOTION_SENSOR_INT1_PIN) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR ENABLING 6D ORIENTATION\n ");
#endif
  }
  else
  {
    EXT_ON_HW_FEATURE(EXT_HWF_6D_ORIENTATION);
  }
}

/**
 * @brief  Disable 6D Orientation Extended Feature
 * @param  None
 * @retval None
 */
static void Disable_6D_Orientation(void)
{
  if (IKS01A3_MOTION_SENSOR_Disable_6D_Orientation(IKS01A3_LSM6DSO_0) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR DISABLING 6D ORIENTATION\n ");
#endif
  }
  else
  {
    EXT_OFF_HW_FEATURE(EXT_HWF_6D_ORIENTATION);
  }
  if (IKS01A3_MOTION_SENSOR_SetOutputDataRate(IKS01A3_LSM6DSO_0, MOTION_ACCELERO, MOTION_EXT_Server_App_Context.defaultODR) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : ERROR SETTING DEFAULT ODR\n ");
#endif
  }
}

/**
 * @brief  Enable Multi Event Extended Feature
 * @param  None
 * @retval None
 */
static void Enable_MultiEvent(void)
{
  DisableExtFeatures();

  EnableExtFeatures();

  MOTION_EXT_Server_App_Context.PedometerStepCount = 0;
  AccEvent_Notify(MOTION_EXT_Server_App_Context.PedometerStepCount, 3);

  EXT_ON_HW_FEATURE(EXT_HWF_MULTIPLE_EVENTS);

  IKS01A3_MOTION_SENSOR_Reset_Step_Counter(IKS01A3_LSM6DSO_0);
}

/**
 * @brief  Disable Multi Event Extended Feature
 * @param  None
 * @retval None
 */
static void Disable_MultiEvent(void)
{
  DisableExtFeatures();
  EXT_OFF_HW_FEATURE(EXT_HWF_MULTIPLE_EVENTS);
}

/**
 * @brief  Enable all Extended Features
 * @param  None
 * @retval None
 */
static void EnableExtFeatures(void)
{
  /* Do not change the enable sequenze of the HW events */
  /* It depends on the ODR value (from minor value to max value) */
  Enable_Pedometer();
  Enable_Tilt();
  Enable_FreeFall();
  Enable_SingleTap();
  Enable_DoubleTap();
  Enable_6D_Orientation();

  MOTION_EXT_Server_App_Context.MultiEventEnabled = 1;
}


/**
 * @brief  Disable all Extended Features
 * @param  None
 * @retval None
 */
static void DisableExtFeatures(void)
{
  if(EXT_CHECK_HW_FEATURE(EXT_HWF_PEDOMETER))
  {
    Disable_Pedometer();
  }

  if(EXT_CHECK_HW_FEATURE(EXT_HWF_FREE_FALL))
  {
    Disable_FreeFall();
  }

  if(EXT_CHECK_HW_FEATURE(EXT_HWF_DOUBLE_TAP))
  {
    Disable_DoubleTap();
  }

  if(EXT_CHECK_HW_FEATURE(EXT_HWF_SINGLE_TAP))
  {
    Disable_SingleTap();
  }

  if(EXT_CHECK_HW_FEATURE(EXT_HWF_WAKE_UP))
  {
    Disable_WakeUp();
  }

  if(EXT_CHECK_HW_FEATURE(EXT_HWF_TILT))
  {
    Disable_Tilt();
  }

  if(EXT_CHECK_HW_FEATURE(EXT_HWF_6D_ORIENTATION))
  {
    Disable_6D_Orientation();
  }

  MOTION_EXT_Server_App_Context.MultiEventEnabled = 0;
}

/**
  * @brief  Return the HW's 6D Orientation result
  * @param  None
  * @retval AccEventType 6D Orientation Found
  */
static MOTION_EXT_Server_App_AccEvent_t GetHWOrientation6D(void)
{
  MOTION_EXT_Server_App_AccEvent_t OrientationResult = ACC_NOT_USED;

  uint8_t xl = 0;
  uint8_t xh = 0;
  uint8_t yl = 0;
  uint8_t yh = 0;
  uint8_t zl = 0;
  uint8_t zh = 0;

  if (IKS01A3_MOTION_SENSOR_Get_6D_Orientation_XL(IKS01A3_LSM6DSO_0, &xl) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : Error getting 6D orientation XL axis from LSM6DSL - accelerometer.\r\n ");
#endif
  }
  if (IKS01A3_MOTION_SENSOR_Get_6D_Orientation_XH(IKS01A3_LSM6DSO_0, &xh) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : Error getting 6D orientation XH axis from LSM6DSL - accelerometer.\r\n");
#endif
  }
  if (IKS01A3_MOTION_SENSOR_Get_6D_Orientation_YL(IKS01A3_LSM6DSO_0, &yl) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : Error getting 6D orientation YL axis from LSM6DSL - accelerometer.\r\n");
#endif
  }
  if (IKS01A3_MOTION_SENSOR_Get_6D_Orientation_YH(IKS01A3_LSM6DSO_0, &yh) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : Error getting 6D orientation YH axis from LSM6DSL - accelerometer.\r\n");
#endif
  }
  if (IKS01A3_MOTION_SENSOR_Get_6D_Orientation_ZL(IKS01A3_LSM6DSO_0, &zl) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : Error getting 6D orientation ZL axis from LSM6DSL - accelerometer.\r\n");
#endif
  }
  if (IKS01A3_MOTION_SENSOR_Get_6D_Orientation_ZH(IKS01A3_LSM6DSO_0, &zh) != BSP_ERROR_NONE)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : Error getting 6D orientation ZH axis from LSM6DSL - accelerometer.\r\n");
#endif
  }

  if (xl == 0U && yl == 0U && zl == 0U && xh == 0U && yh == 1U && zh == 0U)
  {
    OrientationResult = ACC_6D_OR_RIGTH;
  }

  else if (xl == 1U && yl == 0U && zl == 0U && xh == 0U && yh == 0U && zh == 0U)
  {
    OrientationResult = ACC_6D_OR_TOP;
  }

  else if (xl == 0U && yl == 0U && zl == 0U && xh == 1U && yh == 0U && zh == 0U)
  {
    OrientationResult = ACC_6D_OR_BOTTOM;
  }

  else if (xl == 0U && yl == 1U && zl == 0U && xh == 0U && yh == 0U && zh == 0U)
  {
    OrientationResult = ACC_6D_OR_LEFT;
  }

  else if (xl == 0U && yl == 0U && zl == 0U && xh == 0U && yh == 0U && zh == 1U)
  {
    OrientationResult = ACC_6D_OR_UP;
  }

  else if (xl == 0U && yl == 0U && zl == 1U && xh == 0U && yh == 0U && zh == 0U)
  {
    OrientationResult = ACC_6D_OR_DOWN;
  }

  else
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : None of the 6D orientation axes is set in LSM6DSL - accelerometer.\r\n");
#endif
  }
  return OrientationResult;
}

/**
 * @brief  Send a notification when the DS3 detects one Acceleration event
 * @param  Command to Send
 * @param  dimByte Num of Command bytes
 * @retval None
 */
static void AccEvent_Notify(uint16_t Command, uint8_t dimByte)
{
  uint8_t valueSmall[VALUE_LEN_SMALL];
  uint8_t valueLarge[VALUE_LEN_LARGE];
  
  switch(dimByte)
  {
  case 2:
    STORE_LE_16(valueSmall, (HAL_GetTick()>>3));
    STORE_LE_16(valueSmall+2, Command);
    break;
  case 3:
    STORE_LE_16(valueLarge, (HAL_GetTick()>>3));
    valueLarge[2] = 0;
    STORE_LE_16(valueLarge+3, Command);
    break;
  }

  if(MOTION_EXT_Server_App_Context.NotificationStatus)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : NOTIFY CLIENT WITH NEW MOTION PARAMETER VALUE \n ");
    APP_DBG_MSG(" \n\r");
#endif
    switch(dimByte)
    {
    case 2:
      MOTENV_STM_App_Update_Char(ACC_EVENT_CHAR_UUID, VALUE_LEN_SMALL, (uint8_t *)&valueSmall);
      break;
    case 3:
      MOTENV_STM_App_Update_Char(ACC_EVENT_CHAR_UUID, VALUE_LEN_LARGE, (uint8_t *)&valueLarge);
      break;
    }
  }
  else
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- MOTION EXT APPLICATION SERVER : CAN'T INFORM CLIENT - NOTIFICATION DISABLED\n ");
#endif
  }

  return;
}

 /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
