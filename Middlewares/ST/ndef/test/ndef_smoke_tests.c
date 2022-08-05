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
 *  \brief NDEF message smoke tests implementation
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */

#include "platform.h"
#include "utils.h"
#include "rfal_nfc.h"
#include "ndef_poller.h"
#include "ndef_message.h"
#include "ndef_dump.h"
#include "ndef_record.h"
#include "ndef_types.h"
#include "ndef_types_rtd.h"
#include "ndef_types_mime.h"
#include "ndef_type_wifi.h"
#include "ndef_unitary_tests.h"
#include "ndef_smoke_tests.h"


/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */


// Define used for the Write/Read well-known types tests
// Define it to allow the user to check tag content, e.g. with a phone
// Don't define it to reduce user interaction
#define NDEF_TEST_WAIT_FOR_TAG_REMOVED


#define SMOKE_TEST_BUFFER_LEN       220U    //128U


#define ASSERT_ERR_NONE(str, err)    do{ if (err!=ERR_NONE) { platformLog(str, err); return err; } } while(0)


static rfalNfcDiscoverParam discParam; // Must be global because RFAL HL layer keeps only a pointer to it (cannot be on the stack)


/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */


static const uint8_t* ndefDeviceTypeStr[] =
{
    (uint8_t *)"NFC_DEVICE_TYPE_NONE",
    (uint8_t *)"NFC_DEVICE_TYPE_NFCA_T1T",
    (uint8_t *)"NFC_DEVICE_TYPE_NFCA_T2T",
    (uint8_t *)"NFC_DEVICE_TYPE_NFCF_T3T",
    (uint8_t *)"NFC_DEVICE_TYPE_NFCA_T4T",
    (uint8_t *)"NFC_DEVICE_TYPE_NFCB_T4T",
    (uint8_t *)"NFC_DEVICE_TYPE_NFCV_T5T",
    (uint8_t *)"NFC_DEVICE_TYPE_ANY"
};


static const uint8_t* ndefTypeIdStr[] =
{
    (uint8_t *)"NDEF_TYPE_EMPTY",
    (uint8_t *)"NDEF_TYPE_RTD_DEVICE_INFO",
    (uint8_t *)"NDEF_TYPE_RTD_TEXT",
    (uint8_t *)"NDEF_TYPE_RTD_URI",
    (uint8_t *)"NDEF_TYPE_RTD_AAR",
    (uint8_t *)"NDEF_TYPE_MEDIA",
    (uint8_t *)"NDEF_TYPE_MEDIA_VCARD",
    (uint8_t *)"NDEF_TYPE_MEDIA_WIFI",
};


/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */


/*
 ******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
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
ReturnCode initTests(void)
{
    ReturnCode err;

    err = rfalNfcInitialize();

    return err;
}


/*****************************************************************************/
/* Poll until a given type of tag is detected */
/*****************************************************************************/
ReturnCode DetectDeviceType(rfalNfcDevice** nfcDevice, nfcDeviceType expectedDevice)
{
    nfcDeviceType device     = NFC_DEVICE_TYPE_NONE;
    nfcDeviceType lastDevice = NFC_DEVICE_TYPE_NONE;
    bool          found;

    if (nfcDevice == NULL)
    {
        return ERR_PARAM;
    }

    if (expectedDevice != NFC_DEVICE_TYPE_NONE)
    {
        platformLog("Present a tag %s: ", ndefDeviceTypeStr[expectedDevice]);

        rfalNfcDeactivate(false);
        rfalNfcDiscover(&discParam);

        found = false;
        while(!found)
        {
            rfalNfcWorker(); /* Run RFAL NFC worker */

            if (rfalNfcIsDevActivated(rfalNfcGetState()))
            {
                rfalNfcGetActiveDevice(nfcDevice);

                switch ((*nfcDevice)->type)
                {
                case RFAL_NFC_LISTEN_TYPE_NFCA:
                    switch ((*nfcDevice)->dev.nfca.type)
                    {
                    case RFAL_NFCA_T1T:
                        //platformLog("ISO14443A/Topaz (NFC-A T1T) TAG found. UID: %s\r\n", hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                        device = NFC_DEVICE_TYPE_NFCA_T1T;
                        break;
                    case RFAL_NFCA_T2T:
                        //platformLog("T2T device found. UID: %s\r\n", hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                        device = NFC_DEVICE_TYPE_NFCA_T2T;
                        break;
                    case RFAL_NFCA_T4T:
                        //platformLog("NFCA Passive ISO-DEP device found. UID: %s\r\n", hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                        device = NFC_DEVICE_TYPE_NFCA_T4T;
                        break;
                    default:
                        platformLog("Unknown NFCA device %d\r\n", (*nfcDevice)->dev.nfca.type);
                        break;
                    }
                    break;
                case RFAL_NFC_LISTEN_TYPE_NFCB:
                    //platformLog("NFCB T4B device found\r\n");
                    device = NFC_DEVICE_TYPE_NFCB_T4T;
                    break;
                case RFAL_NFC_LISTEN_TYPE_NFCF:
                    //platformLog("NFCF T3T device found\r\n");
                    device = NFC_DEVICE_TYPE_NFCF_T3T;
                    break;
                case RFAL_NFC_LISTEN_TYPE_NFCV:
                    //platformLog("NFCV T5T device found\r\n");
                    device = NFC_DEVICE_TYPE_NFCV_T5T;
                    break;
                default:
                    platformLog("Unknown device type found %d\r\n", (*nfcDevice)->type);
                    break;
                }

                if ( (device == expectedDevice) ||
                     ((expectedDevice == NFC_DEVICE_TYPE_ANY) && (device != NFC_DEVICE_TYPE_NONE)))
                {
                    found = true;
                }
                else
                {
                    /* It was not the expected type, look for another one */
                    if (device != lastDevice)
                    {
                        platformLog("Detected %s instead of %s, try again\r\n", ndefDeviceTypeStr[device], ndefDeviceTypeStr[expectedDevice]);
                        lastDevice = device;
                    }
                    rfalNfcDeactivate(false);
                    rfalNfcDiscover(&discParam);
                }
            }
        }

        platformLog("Detected tag %s\r\n", ndefDeviceTypeStr[device]);
    }
    else
    {
        /* expectedDevice == NFC_DEVICE_TYPE_NONE */
        platformLog("Remove tag... ");
        do
        {
            rfalNfcDeactivate(false);
            rfalNfcDiscover(&discParam);

            found = false;

            uint32_t timer = platformTimerCreate(500U);

            while (!platformTimerIsExpired(timer))
            {
                rfalNfcWorker();
                
                if (rfalNfcIsDevActivated(rfalNfcGetState()))
                {
                    found = true;
                }
            }
        } while (found);
        platformLog("Tag removed\r\n");
    }

    return ERR_NONE;

}


/*****************************************************************************/
ReturnCode ndefSmokeTest_DetectDeviceType(rfalNfcDevice* nfcDevice)
{
    ReturnCode err = ERR_NONE;

    platformLog("Running %s...\r\n", __FUNCTION__);

    err |= DetectDeviceType(&nfcDevice, NFC_DEVICE_TYPE_NFCA_T2T);
    err |= DetectDeviceType(&nfcDevice, NFC_DEVICE_TYPE_NONE);
    err |= DetectDeviceType(&nfcDevice, NFC_DEVICE_TYPE_NFCF_T3T);
    err |= DetectDeviceType(&nfcDevice, NFC_DEVICE_TYPE_NONE);
    err |= DetectDeviceType(&nfcDevice, NFC_DEVICE_TYPE_NFCA_T4T);
    err |= DetectDeviceType(&nfcDevice, NFC_DEVICE_TYPE_NONE);
    err |= DetectDeviceType(&nfcDevice, NFC_DEVICE_TYPE_NFCB_T4T);
    err |= DetectDeviceType(&nfcDevice, NFC_DEVICE_TYPE_NONE);
    err |= DetectDeviceType(&nfcDevice, NFC_DEVICE_TYPE_NFCV_T5T);
    err |= DetectDeviceType(&nfcDevice, NFC_DEVICE_TYPE_NONE);

    if (err != ERR_NONE)
    {
        platformLog("%s() failed!\r\n", __FUNCTION__);
    }

    return err;
}

/*****************************************************************************/
ReturnCode ndefSmokeTest_CheckPresence(rfalNfcDevice* nfcDevice)
{
    ReturnCode  err = ERR_NONE;
    ndefContext ndefCtx;

    platformLog("Running %s...\r\n", __FUNCTION__);

    for (nfcDeviceType tag = NFC_DEVICE_TYPE_NFCA_T2T; tag <= NFC_DEVICE_TYPE_NFCV_T5T; tag++)
    //for (nfcDeviceType tag = NFC_DEVICE_TYPE_NFCB_T4T; tag <= NFC_DEVICE_TYPE_NFCV_T5T; tag++)
    //nfcDeviceType tag = NFC_DEVICE_TYPE_NFCV_T5T;
    {
        err |= DetectDeviceType(&nfcDevice, tag);

        err |= ndefPollerContextInitialization(&ndefCtx, nfcDevice);
        ASSERT_ERR_NONE("ndefPollerContextInitialization() returned %d\r\n", err);
        err |= ndefPollerNdefDetect(&ndefCtx, NULL);
        ASSERT_ERR_NONE("ndefPollerNdefDetect() returned %d\r\n", err);

        platformLog("Remove tag... ");
        while (ndefPollerCheckPresence(&ndefCtx) == ERR_NONE);
        //{
        //    platformDelay(130);
        //}
        platformLog("Tag removed\r\n");
    }

    if (err != ERR_NONE)
    {
        platformLog("%s() failed!\r\n", __FUNCTION__);
    }

    return err;
}


/*****************************************************************************/
ReturnCode ndefSmokeTest_FormatTag(rfalNfcDevice* nfcDevice)
{
    ReturnCode  err = ERR_NONE;
    ndefContext ndefCtx;
    ndefInfo    info;

    platformLog("Running %s...\r\n", __FUNCTION__);

    /***********************************************************/
    for (nfcDeviceType tag = NFC_DEVICE_TYPE_NFCA_T2T; tag <= NFC_DEVICE_TYPE_NFCV_T5T; tag++)
    //nfcDeviceType tag = NFC_DEVICE_TYPE_NFCA_T4T;
    {
        err = ERR_NONE;

        err = DetectDeviceType(&nfcDevice, tag);
        ASSERT_ERR_NONE("DetectDeviceType() returned %d\r\n", err);

        err = ndefPollerContextInitialization(&ndefCtx, nfcDevice);
        ASSERT_ERR_NONE("ndefPollerContextInitialization() returned %d\r\n", err);
        err = ndefPollerNdefDetect(&ndefCtx, NULL);
        ASSERT_ERR_NONE("ndefPollerNdefDetect() returned %d\r\n", err);

        err |= ndefPollerTagFormat(&ndefCtx, NULL, 0); // cc=NULL, options=0
        ASSERT_ERR_NONE("Format failed %d\r\n", err);

        err = ndefPollerNdefDetect(&ndefCtx, &info);
        if ( (info.state != NDEF_STATE_INITIALIZED) || (info.messageLen != 0) )
        {
            platformLog("Format failed, NDEF state is not INITIALIZED or messageLen!=0\r\n");
            return err;
        }

        platformLog("Check tag is formatted properly\r\n");

        /* Wait until tag is removed */
        err |= DetectDeviceType(&nfcDevice, NFC_DEVICE_TYPE_NONE);
    }

    if (err != ERR_NONE)
    {
        platformLog("%s() failed!\r\n", __FUNCTION__);
    }

    return err;
}


/*****************************************************************************/
static ReturnCode ndefTest_CreateType(ndefTypeId typeId, ndefType* type)
{
    ReturnCode err;
    ndefConstBuffer bufPayload;

    if (type == NULL)
    {
        return ERR_PARAM;
    }

    switch (typeId)
    {
    case NDEF_TYPE_EMPTY:
        err = ndefEmptyType(type);
        ASSERT_ERR_NONE("ndefEmptyType() returned %d\r\n", err);
        break;
    case NDEF_TYPE_RTD_DEVICE_INFO:
        {
        static const uint8_t manufacturerName[] = "STMicroelectronics";
        static const uint8_t modelName[]        = "ST25R";
        static const uint8_t deviceUniqueName[] = "ST25R391x";
        ndefDeviceInfoEntry deviceInfoData[] = {
            NDEF_DEVICE_INFO_MANUFACTURER_NAME,  sizeof(manufacturerName) - 1U, manufacturerName,
            NDEF_DEVICE_INFO_MODEL_NAME,         sizeof(modelName)        - 1U, modelName       ,
            NDEF_DEVICE_INFO_DEVICE_UNIQUE_NAME, sizeof(deviceUniqueName) - 1U, deviceUniqueName
        };
        err = ndefRtdDeviceInfo(type, deviceInfoData, SIZEOF_ARRAY(deviceInfoData));
        ASSERT_ERR_NONE("ndefRtdDeviceInfo() returned %d\r\n", err);
        }
        break;
    case NDEF_TYPE_RTD_TEXT:
        {
        static const uint8_t language[] = "en";
        static const uint8_t message[]  = "ST NDEF library demo";
        ndefConstBuffer8 bufLanguage = { language, sizeof(language) - 1U };
        bufPayload.buffer = message;
        bufPayload.length = sizeof(message)  - 1U;
        err = ndefRtdText(type, TEXT_ENCODING_UTF8, &bufLanguage, &bufPayload);
        ASSERT_ERR_NONE("ndefRtdText() returned %d\r\n", err);
        }
        break;
    case NDEF_TYPE_RTD_URI:
        {
        static uint8_t ndefURI[] = "st.com";
        bufPayload.buffer = ndefURI;
        bufPayload.length = sizeof(ndefURI) - 1U;
        err = ndefRtdUri(type, NDEF_URI_PREFIX_HTTP_WWW, &bufPayload); /* Initialize URI type structure */
        ASSERT_ERR_NONE("ndefRtdUri() returned %d\r\n", err);
        }
        break;
    case NDEF_TYPE_RTD_AAR:
        {
        static const uint8_t app[] = "com.st.st25nfc";
        bufPayload.buffer = app;
        bufPayload.length = sizeof(app) - 1U;
        err = ndefRtdAar(type, &bufPayload);
        ASSERT_ERR_NONE("ndefRtdAar() returned %d\r\n", err);
        }
        break;
    case NDEF_TYPE_MEDIA:
        {
        static const uint8_t typeVCard[] = "text/x-vCard";
        static const uint8_t content[] = "BEGIN:VCARD\r\nEND:VCARD";
        ndefConstBuffer8 bufType = { typeVCard,  sizeof(typeVCard) - 1U};
        bufPayload.buffer = content;
        bufPayload.length = sizeof(content) - 1U;
        err = ndefMedia(type, &bufType, &bufPayload);
        ASSERT_ERR_NONE("ndefMedia() returned %d\r\n", err);
        }
        break;
    case NDEF_TYPE_MEDIA_VCARD:
        {
        //static ndefConstBuffer bufTypeN     = { (uint8_t*)"N",     sizeof("N") - 1U     };
        static ndefConstBuffer bufTypeFN    = { (uint8_t*)"FN",    sizeof("FN") - 1U    };
        //static ndefConstBuffer bufTypeADR   = { (uint8_t*)"ADR",   sizeof("ADR") - 1U   };
        //static ndefConstBuffer bufTypeTEL   = { (uint8_t*)"TEL",   sizeof("TEL") - 1U   };
        //static ndefConstBuffer bufTypeEMAIL = { (uint8_t*)"EMAIL", sizeof("EMAIL") - 1U };
        //static ndefConstBuffer bufTypeTITLE = { (uint8_t*)"TITLE", sizeof("TITLE") - 1U };
        //static ndefConstBuffer bufTypeORG   = { (uint8_t*)"ORG",   sizeof("ORG") - 1U   };
        //static ndefConstBuffer bufTypeURL   = { (uint8_t*)"URL",   sizeof("URL") - 1U   };

        static ndefConstBuffer bufSubTypeNone    = {  NULL           , 0 };
        //static ndefConstBuffer bufSubTypeTelCELL = { (uint8_t*)"CELL", sizeof("CELL") - 1U };

        //static uint8_t N[]     = "Doe;john;;Dr";
        static uint8_t FN[]    = "Dr. John";
        //static uint8_t ADR[]   = "Main St.";
        //static uint8_t TEL[]   = "+1 123 456 7890";
        //static uint8_t EMAIL[] = "john.doe@gmail.com";
        //static uint8_t TITLE[] = "Doctor";
        //static uint8_t ORG[]   = "Corporation";
        //static uint8_t URL[]   = "http://www.johnny.com";

        //static ndefConstBuffer bufValueN     = { N,     sizeof(N) - 1U     };
        static ndefConstBuffer bufValueFN    = { FN,    sizeof(FN) - 1U    };
        //static ndefConstBuffer bufValueADR   = { ADR,   sizeof(ADR) - 1U   };
        //static ndefConstBuffer bufValueTEL   = { TEL,   sizeof(TEL) - 1U   };
        //static ndefConstBuffer bufValueEMAIL = { EMAIL, sizeof(EMAIL) - 1U };
        //static ndefConstBuffer bufValueTITLE = { TITLE, sizeof(TITLE) - 1U };
        //static ndefConstBuffer bufValueORG   = { ORG,   sizeof(ORG) - 1U   };
        //static ndefConstBuffer bufValueURL   = { URL,   sizeof(URL) - 1U   };

        ndefVCardInput bufVCard[] = {
            //{ &bufTypeN    , &bufSubTypeNone   , &bufValueN     },
            { &bufTypeFN   , &bufSubTypeNone   , &bufValueFN    },
            //{ &bufTypeADR  , &bufSubTypeNone   , &bufValueADR   },
            //{ &bufTypeTEL  , &bufSubTypeTelCELL, &bufValueTEL   },
            //{ &bufTypeEMAIL, &bufSubTypeNone   , &bufValueEMAIL },
            //{ &bufTypeTITLE, &bufSubTypeNone   , &bufValueTITLE },
            //{ &bufTypeORG  , &bufSubTypeNone   , &bufValueORG   },
            //{ &bufTypeURL  , &bufSubTypeNone   , &bufValueURL   },
        };
        err = ndefVCard(type, bufVCard, SIZEOF_ARRAY(bufVCard));
        ASSERT_ERR_NONE("ndefVCard() returned %d\r\n", err);
        }
        break;
    case NDEF_TYPE_MEDIA_WIFI:
        {
        static const uint8_t ssid[] = { 0x01, 0x02, 0x03, 0x04 };
        static ndefConstBuffer bufNetworkSSID = { ssid, sizeof(ssid) };
        static const uint8_t  key[] = { 0x05, 0x06, 0x07, 0x08 };
        static ndefConstBuffer bufNetworkKey  = { key, sizeof(key) };

        ndefTypeWifi wifiConfig = {
            .bufNetworkSSID = bufNetworkSSID,
            .bufNetworkKey  = bufNetworkKey,
            .authentication = NDEF_WIFI_AUTHENTICATION_WPAPSK,
            .encryption     = NDEF_WIFI_ENCRYPTION_TKIP
        };
        err = ndefWifi(type, &wifiConfig);
        ASSERT_ERR_NONE("ndefWifi() returned %d\r\n", err);
        }
        break;
    default:
        platformLog("Type not supported");
        return ERR_NOT_IMPLEMENTED;
        //break;
    }

    return err;
}


/*****************************************************************************/
static ReturnCode ndefSmokeTest_CreateMessage(ndefMessage* message, ndefRecord* record, ndefType* type, ndefTypeId typeId)
{
    ReturnCode err;

    err = ndefMessageInit(message); /* Initialize message */
    ASSERT_ERR_NONE("ndefMessageInit() returned %d\r\n", err);

    err = ndefTest_CreateType(typeId, type);
    ASSERT_ERR_NONE("ndefSystemTest_CreateRecord() returned %d\r\n", err);

    err = ndefTypeToRecord(type, record); /* Convert Type to Record */
    ASSERT_ERR_NONE("ndefTypeToRecord() returned %d\r\n", err);

    err = ndefMessageAppend(message, record); /* Append record to message */
    ASSERT_ERR_NONE("ndefMessageAppend() returned %d\r\n", err);

    return err;
}


/*****************************************************************************/
static ReturnCode ndefSmokeTest_WriteMessage(rfalNfcDevice* nfcDevice, ndefContext* ndefCtx, ndefMessage* message)
{
    ReturnCode err;

    if ( (nfcDevice == NULL) || (ndefCtx == NULL) || (message == NULL) )
    {
        return ERR_PARAM;
    }

    /* Write message */
    err = ndefPollerContextInitialization(ndefCtx, nfcDevice);
    ASSERT_ERR_NONE("ndefPollerContextInitialization() returned %d\r\n", err);

    err = ndefPollerNdefDetect(ndefCtx, NULL);
    ASSERT_ERR_NONE("ndefPollerNdefDetect() returned %d\r\n", err);

    err = ndefPollerWriteMessage(ndefCtx, message);
    ASSERT_ERR_NONE("ndefPollerWriteMessage() returned %d\r\n", err);

    return err;
}


/*****************************************************************************/
static ReturnCode ndefSmokeTest_ReadMessage(rfalNfcDevice* nfcDevice, ndefContext* ndefCtx, ndefBuffer* bufMessage)
{
    ReturnCode err;

    if ( (nfcDevice == NULL) || (ndefCtx == NULL) || (bufMessage == NULL) || (bufMessage->buffer == NULL) )
    {
        return ERR_PARAM;
    }

    /* Read raw message */
    err = ndefPollerContextInitialization(ndefCtx, nfcDevice);
    ASSERT_ERR_NONE("ndefPollerContextInitialization() returned %d\r\n", err);

    err = ndefPollerNdefDetect(ndefCtx, NULL);
    ASSERT_ERR_NONE("ndefPollerNdefDetect() returned %d\r\n", err);

    err = ndefPollerReadRawMessage(ndefCtx, bufMessage->buffer, bufMessage->length, &bufMessage->length);
    ASSERT_ERR_NONE("ndefPollerReadMessage() returned %d\r\n", err);

    return err;
}


/*****************************************************************************/
/*************************** XXXXXXXXXXXXXXXX ********************************/
/*****************************************************************************/


/*****************************************************************************/
/* Write a single record message to the tag */
/*****************************************************************************/
ReturnCode ndefSmokeTest_WriteSingleRecord(rfalNfcDevice* nfcDevice, nfcDeviceType deviceType, ndefTypeId typeId)
{
    ReturnCode      err;
    //rfalNfcDevice nfcDevice;
    ndefContext     ndefCtx;
    ndefMessage     message;
    ndefRecord      record;
    ndefType        type;
    ndefMessageInfo info;

    platformLog("Running %s: %s...\r\n", __FUNCTION__, ndefTypeIdStr[typeId]);

    /* Detect a given tag */
    err = DetectDeviceType(&nfcDevice, deviceType);
    ASSERT_ERR_NONE("DetectDeviceType() returned %d\r\n", err);

    err = ndefSmokeTest_CreateMessage(&message, &record, &type, typeId);
    ASSERT_ERR_NONE("ndefSmokeTest_CreateMessage() returned %d\r\n", err);

    err = ndefMessageDump(&message, true);
    ASSERT_ERR_NONE("ndefMessageDump() returned %d\r\n", err);

    /* Check message */
    err = ndefMessageGetInfo(&message, &info);
    ASSERT_ERR_NONE("ndefMessageGetInfo() returned %d\r\n", err);

    if (info.length == 0U)
    {
        platformLog("Invalid message length");
        return ERR_INTERNAL;
    }
    if (info.recordCount != 1U)
    {
        platformLog("Read invalid number of record");
        return ERR_INTERNAL;
    }

    /* Write message */
    err = ndefSmokeTest_WriteMessage(nfcDevice, &ndefCtx, &message);
    ASSERT_ERR_NONE("ndefSystemTest_WriteMessage() returned %d\r\n", err);

    platformLog("Check tag contains a %s\r\n", ndefTypeIdStr[typeId]);

#ifdef NDEF_TEST_WAIT_FOR_TAG_REMOVED
    /* Wait until tag is removed */
    err = DetectDeviceType(&nfcDevice, NFC_DEVICE_TYPE_NONE);
#endif

    return err;
}


/*****************************************************************************/
/******************** XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX **********************/
/*****************************************************************************/


/*****************************************************************************/
/* Read a single record message from a tag */
/*****************************************************************************/
ReturnCode ndefSmokeTest_ReadSingleRecord(rfalNfcDevice* nfcDevice, nfcDeviceType deviceType, ndefTypeId typeId)
{
    ReturnCode      err;
    ndefContext     ndefCtx;
    ndefMessage     message;
    ndefRecord      *record;
    ndefType        type;
    ndefMessageInfo info;

    ndefMessage     referenceMessage;
    ndefRecord      referenceRecord;
    ndefType        referenceType;
    bool            match;

    platformLog("Running %s: Expecting %s...\r\n", __FUNCTION__, ndefDeviceTypeStr[deviceType]);

    uint8_t rawMessageBuf[SMOKE_TEST_BUFFER_LEN];
    ndefBuffer bufMessage = { rawMessageBuf, sizeof(rawMessageBuf) };

    /* Detect a given tag */
    err = DetectDeviceType(&nfcDevice, deviceType);
    ASSERT_ERR_NONE("DetectTag() returned %d\r\n", err);

    /* Read message */
    err = ndefSmokeTest_ReadMessage(nfcDevice, &ndefCtx, &bufMessage);
    ASSERT_ERR_NONE("ndefSystemTest_ReadMessage() returned %d\r\n", err);

    err = ndefMessageDecode((const ndefConstBuffer*)&bufMessage, &message);
    ASSERT_ERR_NONE("ndefMessageDecode() returned %d\r\n", err);

    err = ndefMessageDump(&message, true);
    ASSERT_ERR_NONE("ndefMessageDump() returned %d\r\n", err);

    /* Check message */
    err = ndefMessageGetInfo(&message, &info);
    ASSERT_ERR_NONE("ndefMessageGetInfo() returned %d\r\n", err);

    if (info.length == 0U)
    {
        platformLog("Invalid message length");
        return ERR_INTERNAL;
    }
    if (info.recordCount != 1U)
    {
        platformLog("Read invalid number of record");
        return ERR_INTERNAL;
    }

    /* Check record type */
    record = ndefMessageGetFirstRecord(&message);
    if (record == NULL)
    {
        platformLog("Record not found!");
        return ERR_INTERNAL;
    }

    err = ndefRecordToType(record, &type); /* Convert Record to Type */
    ASSERT_ERR_NONE("ndefRecordToType() returned %d\r\n", err);

    if (typeId != type.id)
    {
        platformLog("Record Type MISMATCH: Expected %s but decoded %s!\r\n\n", ndefTypeIdStr[typeId], ndefTypeIdStr[type.id]);
        err = ERR_INTERNAL;
    }

    /* Compare with reference */
    err = ndefSmokeTest_CreateMessage(&referenceMessage, &referenceRecord, &referenceType, typeId);
    ASSERT_ERR_NONE("ndefSmokeTest_CreateMessage() returned %d\r\n", err);

    match = ndefMessageMatch(&referenceMessage, &message);
    if (match)
    {
        platformLog("=> Expected and Decoded message match <=\r\n");
    }
    else
    {
        platformLog("Message MISMATCH!\r\n");
        err = ERR_INTERNAL;
    }

#ifdef NDEF_TEST_WAIT_FOR_TAG_REMOVED
    /* Wait until tag is removed */
    err = DetectDeviceType(&nfcDevice, NFC_DEVICE_TYPE_NONE);
#endif

    return err;
}


/*****************************************************************************/
ReturnCode ndefSmokeTest_Write_Read_WellKnownTypes(rfalNfcDevice* nfcDevice)
{
    ReturnCode  err = ERR_NONE;

    for (nfcDeviceType tag = NFC_DEVICE_TYPE_NFCA_T2T; tag <= NFC_DEVICE_TYPE_NFCV_T5T; tag++)
    //nfcDeviceType tag = NFC_DEVICE_TYPE_ANY;
    {
        err = ERR_NONE;

        /* Write single record message, and read it back */
        err |= ndefSmokeTest_WriteSingleRecord(nfcDevice, tag, NDEF_TYPE_EMPTY);
        err |= ndefSmokeTest_ReadSingleRecord(nfcDevice, tag, NDEF_TYPE_EMPTY);

        err |= ndefSmokeTest_WriteSingleRecord(nfcDevice, tag, NDEF_TYPE_RTD_DEVICE_INFO);
        err |= ndefSmokeTest_ReadSingleRecord(nfcDevice, tag, NDEF_TYPE_RTD_DEVICE_INFO);

        err |= ndefSmokeTest_WriteSingleRecord(nfcDevice, tag, NDEF_TYPE_RTD_TEXT);
        err |= ndefSmokeTest_ReadSingleRecord(nfcDevice, tag, NDEF_TYPE_RTD_TEXT);

        err |= ndefSmokeTest_WriteSingleRecord(nfcDevice, tag, NDEF_TYPE_RTD_URI);
        err |= ndefSmokeTest_ReadSingleRecord(nfcDevice, tag, NDEF_TYPE_RTD_URI);

        err |= ndefSmokeTest_WriteSingleRecord(nfcDevice, tag, NDEF_TYPE_RTD_AAR);
        err |= ndefSmokeTest_ReadSingleRecord(nfcDevice, tag, NDEF_TYPE_RTD_AAR);

        err |= ndefSmokeTest_WriteSingleRecord(nfcDevice, tag, NDEF_TYPE_MEDIA_VCARD);
        err |= ndefSmokeTest_ReadSingleRecord(nfcDevice, tag, NDEF_TYPE_MEDIA_VCARD);

        err |= ndefSmokeTest_WriteSingleRecord(nfcDevice, tag, NDEF_TYPE_MEDIA_WIFI);
        err |= ndefSmokeTest_ReadSingleRecord(nfcDevice, tag, NDEF_TYPE_MEDIA_WIFI);

        if (err == ERR_NONE)
        {
            platformLog("Write/Read (%s) passed!\r\n", ndefDeviceTypeStr[tag]);
        }
        else
        {
            platformLog("Write/Read (%s) failed...\r\n", ndefDeviceTypeStr[tag]);
        }
    }

    if (err != ERR_NONE)
    {
        platformLog("%s() failed!\r\n", __FUNCTION__);
    }

    return err;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
ReturnCode ndefSmokeTests(void)
{
    static uint8_t NFCID3[] = {0x01, 0xFE, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    static uint8_t GB[] = {0x46, 0x66, 0x6d, 0x01, 0x01, 0x11, 0x02, 0x02, 0x07, 0x80, 0x03, 0x02, 0x00, 0x03, 0x04, 0x01, 0x32, 0x07, 0x01, 0x03};

    rfalNfcDevice* nfcDevice;
    ReturnCode err;

    platformLog("Running NDEF Smoke tests...\r\n");

    err = rfalNfcInitialize();
    ASSERT_ERR_NONE("rfalNfcInitialize() returned %d\r\n", err);

    discParam.compMode      = RFAL_COMPLIANCE_MODE_NFC;
    discParam.devLimit      = 1U;
    discParam.nfcfBR        = RFAL_BR_212;
    discParam.ap2pBR        = RFAL_BR_424;

    ST_MEMCPY( &discParam.nfcid3, NFCID3, sizeof(NFCID3) );
    ST_MEMCPY( &discParam.GB, GB, sizeof(GB) );
    discParam.GBLen         = sizeof(GB);

    discParam.notifyCb             = NULL;
    discParam.totalDuration        = 1000U;
    discParam.wakeupEnabled        = false;
    discParam.wakeupConfigDefault  = true;
    discParam.techs2Find           = ( RFAL_NFC_POLL_TECH_A | RFAL_NFC_POLL_TECH_B | RFAL_NFC_POLL_TECH_F | RFAL_NFC_POLL_TECH_V | RFAL_NFC_POLL_TECH_ST25TB );
#if defined(ST25R3911) || defined(ST25R3916)
    discParam.techs2Find          |= RFAL_NFC_POLL_TECH_AP2P;
#endif /* ST25R3911 || ST25R3916 */

    //err = rfalNfcInitialize();
    //ASSERT_ERR_NONE("rfalNfcInitialize() returned %d\r\n", err);

    rfalNfcDiscover(&discParam);

    // Detect different types of tags
    err |= ndefSmokeTest_DetectDeviceType(nfcDevice);

    // Test CheckPresence()
    err |= ndefSmokeTest_CheckPresence(nfcDevice);

    // Test FormatTag
    err |= ndefSmokeTest_FormatTag(nfcDevice);

    // Test Write/Read Well-known types
    err |= ndefSmokeTest_Write_Read_WellKnownTypes(nfcDevice);

    if (err == ERR_NONE)
    {
        platformLog("Smoke tests passed!\r\n");
    }
    else
    {
        platformLog("Smoke tests failed...\r\n");
    }

    return err;
}
