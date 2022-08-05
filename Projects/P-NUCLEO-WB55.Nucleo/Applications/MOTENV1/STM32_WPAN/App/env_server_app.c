/**
 ******************************************************************************
 * File Name          : env_server_app.c
 * Description        : Handle HW/Environmental Service/Char
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
#include "env_server_app.h"
#include "motionfx_server_app.h"
#include "config_server_app.h"

#include "iks01a3_env_sensors.h"

/* Private defines -----------------------------------------------------------*/
#define PRESSURE_BYTES          (4)
#define HUMIDITY_BYTES          (2)
#define TEMPERATURE_BYTES       (2)

#define VALUE_LEN_ENV           (2+PRESSURE_BYTES+HUMIDITY_BYTES+TEMPERATURE_BYTES/*Temp2*/+TEMPERATURE_BYTES/*Temp1*/)

/* Private typedef -----------------------------------------------------------*/

/**
 * @brief  HW/Environmental Service/Char Context structure definition
 */
typedef struct
{
  uint8_t  NotificationStatus;

  int32_t PressureValue;
  uint16_t HumidityValue;
  int16_t TemperatureValue[2];
  uint8_t hasPressure;
  uint8_t hasHumidity;
  uint8_t hasTemperature;
} ENV_Server_App_Context_t;

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/**
 * @brief  Environmental Capabilities
 */
static IKS01A3_ENV_SENSOR_Capabilities_t EnvCapabilities[IKS01A3_ENV_INSTANCES_NBR];

PLACE_IN_SECTION("BLE_APP_CONTEXT") static ENV_Server_App_Context_t ENV_Server_App_Context;

/* Global variables ----------------------------------------------------------*/
extern uint8_t manuf_data[14];

/* Private function prototypes -----------------------------------------------*/
static void ENV_Handle_Sensor(void);
static void EnvSensor_GetCaps(void);

/* Functions Definition ------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Init the HW/Environmental Service/Char Context
 * @param  None
 * @retval None
 */
void ENV_Context_Init(void)
{
  /* Env Sensors */
  (void)IKS01A3_ENV_SENSOR_Init(IKS01A3_HTS221_0, ENV_TEMPERATURE | ENV_HUMIDITY);
  (void)IKS01A3_ENV_SENSOR_Init(IKS01A3_LPS22HH_0, ENV_TEMPERATURE | ENV_PRESSURE);

  ENV_Server_App_Context.hasPressure = 0;
  ENV_Server_App_Context.hasHumidity = 0;
  ENV_Server_App_Context.hasTemperature = 0;

  ENV_Set_Notification_Status(0);

  /* Check Env caps */
  EnvSensor_GetCaps();
}

/**
 * @brief  Set the notification status (enabled/disabled)
 * @param  status The new notification status
 * @retval None
 */
void ENV_Set_Notification_Status(uint8_t status)
{
  ENV_Server_App_Context.NotificationStatus = status;
}

/**
 * @brief  Send a notification for Environmental char
 * @param  None
 * @retval None
 */
void ENV_Send_Notification_Task(void)
{
  /* Notifications of Compass Calibration status */
  if(CONFIG_Get_FirstConnection_Config() == 1)
  {
    CONFIG_Send_Notification(FEATURE_MASK_SENSORFUSION_SHORT, W2ST_COMMAND_CAL_STATUS, MOTIONFX_Get_MagCalStatus() ? 100: 0);
    CONFIG_Send_Notification(FEATURE_MASK_ECOMPASS, W2ST_COMMAND_CAL_STATUS, MOTIONFX_Get_MagCalStatus() ? 100: 0);
    CONFIG_Set_FirstConnection_Config(0);
  }

  if(ENV_Server_App_Context.NotificationStatus)
  {
#if(CFG_DEBUG_APP_TRACE != 0)
//    APP_DBG_MSG("-- ENV APPLICATION SERVER : NOTIFY CLIENT WITH NEW ENV PARAMETER VALUE \n ");
//    APP_DBG_MSG(" \n\r");
#endif
    ENV_Update();
  }
  else
  {
#if(CFG_DEBUG_APP_TRACE != 0)
    APP_DBG_MSG("-- ENV APPLICATION SERVER : CAN'T INFORM CLIENT - NOTIFICATION DISABLED\n ");
#endif
  }

  return;
}

/**
 * @brief  Update the Environmental char value
 * @param  None
 * @retval None
 */
void ENV_Update(void)
{
  uint8_t tempIndex = 0;
  uint8_t value[VALUE_LEN_ENV];
  uint8_t BuffPos = 2;

  /* Read ENV values */
  ENV_Handle_Sensor();

  /* Timestamp */
  STORE_LE_16(value, (HAL_GetTick()>>3));

  if(ENV_Server_App_Context.hasPressure == 1)
  {
    STORE_LE_32(&value[BuffPos], ENV_Server_App_Context.PressureValue);
    BuffPos += PRESSURE_BYTES;
  }

  if(ENV_Server_App_Context.hasHumidity == 1)
  {
    STORE_LE_16(&value[BuffPos], ENV_Server_App_Context.HumidityValue);
    BuffPos += HUMIDITY_BYTES;
  }

  for(tempIndex = 0; tempIndex < ENV_Server_App_Context.hasTemperature; tempIndex++)
  {
    STORE_LE_16(&value[BuffPos], ENV_Server_App_Context.TemperatureValue[tempIndex]);
    BuffPos += TEMPERATURE_BYTES;
  }

  MOTENV_STM_App_Update_Char(ENV_CHAR_UUID, VALUE_LEN_ENV, (uint8_t *)&value);

  return;
}

/* Private functions ---------------------------------------------------------*/
#ifdef NFC_ENABLE     
extern uint8_t string_buff[];   
#endif

/**
 * @brief  Parse the values read by Environmental sensors
 * @param  None
 * @retval None
 */
static void ENV_Handle_Sensor(void)
{
  uint8_t i;
  uint8_t tempIndex = 0;
  float pressure, humidity, temperature;
  int32_t decPart, intPart;
  
  for(i = 0; i < IKS01A3_ENV_INSTANCES_NBR; i++)
  {
    if(ENV_Server_App_Context.hasPressure == 1)
    {
#ifdef NFC_ENABLE      
      /* Stuff Pressure Sensor data from NFC reader here */
      if (string_buff[11] == '.')   /* Check decimal point position on mBar value to determine value length */
      {
        /* string_buff [8:10] = intPart, [12:13] = decPart */        
        string_buff[13] = NULL;  /* End the presssure sensor value with NULL after two decimals */
        intPart = atoi(&string_buff[8]);                
        decPart = atoi(&string_buff[12]);        
      }
      else if (string_buff[12] == '.')
      {
        /* string_buff [8:11] = intPart, [13:14] = decPart */
        string_buff[14] = NULL;  /* End the presssure sensor value with NULL after two decimals */
        intPart = atoi(&string_buff[8]);                
        decPart = atoi(&string_buff[13]);          
      }

      
      ENV_Server_App_Context.PressureValue = intPart*100+decPart;
#else
      if (IKS01A3_ENV_SENSOR_GetValue(i, ENV_PRESSURE, &pressure) == 0)
      {        
        MCR_BLUEMS_F2I_2D(pressure, intPart, decPart);      
        ENV_Server_App_Context.PressureValue = intPart*100+decPart;
      }
#endif      
    }

    if(ENV_Server_App_Context.hasHumidity == 1)
    {
      if (IKS01A3_ENV_SENSOR_GetValue(i, ENV_HUMIDITY, &humidity) == 0)
      {
        MCR_BLUEMS_F2I_1D(humidity, intPart, decPart);
        ENV_Server_App_Context.HumidityValue = intPart*10+decPart;
      }
    }

    if(ENV_Server_App_Context.hasTemperature >= 1)
    {
      if (IKS01A3_ENV_SENSOR_GetValue(i, ENV_TEMPERATURE, &temperature) == 0)
      {
        MCR_BLUEMS_F2I_1D(temperature, intPart, decPart);
        ENV_Server_App_Context.TemperatureValue[tempIndex] = intPart*10+decPart;
        tempIndex++;
      }
    }
  }
}

/**
 * @brief  Check the Environmental active capabilities and set the ADV data accordingly
 * @param  None
 * @retval None
 */
static void EnvSensor_GetCaps(void)
{
  uint8_t i;

#ifdef NFC_READER_ONLY_DEMO
  // Hardcode enabled pressure sensor for NFC Reader demo, when not using an X-NUCLEO-ISK01A3 expandion board
  ENV_Server_App_Context.hasPressure = 1;
  manuf_data[5] |= 0x10; /* Pressure value*/
#else
  APP_DBG_MSG("-- ENV APPLICATION SERVER : IKS01A3_ENV_INSTANCES_NBR=%d\n ", IKS01A3_ENV_INSTANCES_NBR);
  for(i = 0; i < IKS01A3_ENV_INSTANCES_NBR; i++)
  {
    IKS01A3_ENV_SENSOR_GetCapabilities(i, &EnvCapabilities[i]);
    if(EnvCapabilities[i].Pressure)
    {
      ENV_Server_App_Context.hasPressure = 1;
    }
    if(EnvCapabilities[i].Humidity)
    {
      ENV_Server_App_Context.hasHumidity = 1;
    }
    if(EnvCapabilities[i].Temperature)
    {
      ENV_Server_App_Context.hasTemperature++;
    }
  }

  /* Update BLE ADV field (Env) */
  if(ENV_Server_App_Context.hasTemperature > 1)
  {
    manuf_data[5] |= 0x05; /* Two Temperature values*/
  }
  else if(ENV_Server_App_Context.hasTemperature == 1)
  {
    manuf_data[5] |= 0x04; /* One Temperature value*/
  }

  if(ENV_Server_App_Context.hasHumidity)
  {
    manuf_data[5] |= 0x08; /* Humidity value */
  }

  if(ENV_Server_App_Context.hasPressure)
  {
    manuf_data[5] |= 0x10; /* Pressure value*/
  }
#endif

}

 /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
