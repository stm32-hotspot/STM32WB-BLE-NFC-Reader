/******************************************************************************
  * \attention
  *
  * <h2><center>&copy; COPYRIGHT 2019 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        www.st.com/myliberty
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
 *      PROJECT:   NDEF firmware
 *      Revision:
 *      LANGUAGE:  ISO C99
 */

/*! \file
 *
 *  \author
 *
 *  \brief NDEF message smoke tests header file
 *
 */
 
#ifndef NDEF_SMOKE_TESTS_H
#define NDEF_SMOKE_TESTS_H


/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */


#include "st_errno.h"
#include "rfal_nfc.h"

/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */


typedef enum
{
    NFC_DEVICE_TYPE_NONE,
    NFC_DEVICE_TYPE_NFCA_T1T,     /*!< NFC-A Listener device type T1T */
    NFC_DEVICE_TYPE_NFCA_T2T,     /*!< NFC-A Listener device type T2T */
    NFC_DEVICE_TYPE_NFCF_T3T,     /*!< NFC-F Listener device type T3T */
    NFC_DEVICE_TYPE_NFCA_T4T,     /*!< NFC-A Listener device type T4T */
    NFC_DEVICE_TYPE_NFCB_T4T,     /*!< NFC-B Listener device type T4T */
    NFC_DEVICE_TYPE_NFCV_T5T,     /*!< NFC-V Listener device type T5T */
    NFC_DEVICE_TYPE_ANY
} nfcDeviceType;


/*
 ******************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 ******************************************************************************
 */


/*!
 *****************************************************************************
 * \brief Initialize tests
 *
 *  This function Initializes the required layers for the tests
 *
 * \return true  : Initialization OK
 * \return false : Initialization failed
 *****************************************************************************
 */
ReturnCode initTests(void);


/*!
 *****************************************************************************
 * Wait for a given tag type
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode ndefSmokeTest_DetectDeviceType(rfalNfcDevice* nfcDevice);


/*!
 *****************************************************************************
 * Perform NDEF Smoke Tests
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode ndefSmokeTests(void);


#endif /* NDEF_SMOKE_TESTS_H */
