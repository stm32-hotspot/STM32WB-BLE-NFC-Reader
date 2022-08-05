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
 *  \brief NDEF message system tests implementation
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
#include "ndef_smoke_tests.h"
#include "ndef_system_tests.h"


//void ndefCCDump(ndefContext *ctx); // from ndef_demo.c


/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

#define SYSTEM_TEST_BUFFER_LEN       64U

#define ASSERT_ERR_NONE(str, err)    do{ if (err != ERR_NONE) { platformLog(str, err); return err; } } while(0)

#define NDEF_T2T_BLOCK_SIZE            4U         /*!< block size                                        */

#define NDEF_T3T_BLOCKLEN                    16U /*!< T3T block len is always 16                         */
#define NDEF_T3T_ATTRIB_INFO_CHECKSUM_LEN   0xEU /*!< T3T checksum len for attribute info to compute     */


static rfalNfcDevice        nfcDevice;

static ndefContext          ndefCtx;


/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */


const uint8_t* ndefDeviceTypeStr[] =
{
    (uint8_t *)"NFC_DEVICE_NONE",
    (uint8_t *)"NFC_DEVICE_TYPE_NFCA_T1T",
    (uint8_t *)"NFC_DEVICE_TYPE_NFCA_T2T",
    (uint8_t *)"NFC_DEVICE_TYPE_NFCF_T3T",
    (uint8_t *)"NFC_DEVICE_TYPE_NFCA_T4T",
    (uint8_t *)"NFC_DEVICE_TYPE_NFCB_T4T",
    (uint8_t *)"NFC_DEVICE_TYPE_NFCV_T5T",
    (uint8_t *)"NFC_DEVICE_TYPE_ANY"
};


/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */

ReturnCode ndefDumpInfo(ndefInfo* info)
{
    const uint8_t* ndefState[] =
    {
        (uint8_t *)"NDEF_STATE_INVALID",
        (uint8_t *)"NDEF_STATE_INITIALIZED",
        (uint8_t *)"NDEF_STATE_READWRITE",
        (uint8_t *)"NDEF_STATE_READONLY",
    };

    if (info == NULL)
    {
        return ERR_PARAM;
    }

    if ((uint32_t)info->state > SIZEOF_ARRAY(ndefState))
    {
        platformLog("NDEF Info: Invalid state\r\n");
        return ERR_PARAM;
    }

    platformLog("NDEF Info:\r\n Version: %d.%d, Area: %d (available %d)\r\n Message length: %d, state %s\r\n",
        info->majorVersion, info->minorVersion,
        info->areaLen, info->areaAvalableSpaceLen,
        info->messageLen,
        ndefState[info->state]);

    return ERR_NONE;
}

/*
 ******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 ******************************************************************************
 */

static ReturnCode ndefSystemTest_PrepareWriteEmptyMessage(ndefContext* ndefCtx, nfcDeviceType deviceType)
{
    ReturnCode err = ERR_NONE;
    uint32_t   offset;

    platformLog("Write empty message to %s\r\n", ndefDeviceTypeStr[deviceType]);

    if (ndefCtx == NULL)
    {
        return ERR_PARAM;
    }

    offset = ndefCtx->messageOffset;

    switch (deviceType)
    {
    case NFC_DEVICE_TYPE_NFCA_T1T:
        err = ERR_NOT_IMPLEMENTED;
        break;
    case NFC_DEVICE_TYPE_NFCA_T2T:
        {
        offset = ndefCtx->messageOffset;
        platformLog("Offset %d\r\n", offset);
        //uint8_t buffer[]      = { 0xE1, 0x03, 0x00 };
        ////err = ndefPollerWriteBytes(ndefCtx, offset, buffer, sizeof(buffer));
        }
        break;
    case NFC_DEVICE_TYPE_NFCF_T3T:
        err = ERR_NOT_IMPLEMENTED;
        break;
    case NFC_DEVICE_TYPE_NFCA_T4T:
        {
        offset = ndefCtx->messageOffset;
        platformLog("Offset %d\r\n", offset);
        //uint8_t buffer[]      = { 0x00, 0x00, 0x20, 0x00, 0x0F, 0x00, 0x0D };
        ////err = ndefPollerWriteBytes(ndefCtx, offset/*16*/, buffer, sizeof(buffer));
        }
        break;
    case NFC_DEVICE_TYPE_NFCB_T4T:
        err = ERR_NOT_IMPLEMENTED;
        break;
    case NFC_DEVICE_TYPE_NFCV_T5T:
        err = ERR_NOT_IMPLEMENTED;
        break;
    default:
        platformLog("Device not supported\r\n");
        err = ERR_PARAM;
        break;
    }

    if (err != ERR_NONE)
    {
        platformLog("Error %d\r\n", err);
        //ndefCtx->state = NDEF_STATE_INVALID;
    }

    return err;
}


/*****************************************************************************/
static ReturnCode ndefSystemTest_PrepareWriteMessage17bytes()
{
    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
static ReturnCode ndefSystemTest_CreateRecord(ndefTypeId typeId, ndefRecord* record)
{
    ReturnCode  err;
    ndefType    type;
    ndefConstBuffer bufPayload;

    if (record == NULL)
    {
        return ERR_PARAM;
    }

    switch (typeId)
    {
    case NDEF_TYPE_EMPTY:
        type.id = NDEF_TYPE_EMPTY;
        bufPayload.buffer = NULL;
        bufPayload.length = 0;
        // TODO Add ndefTypeEmpty, to allow ndefTypeToRecord() to create empty record !!!
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
        err = ndefRtdDeviceInfo(&type, deviceInfoData, SIZEOF_ARRAY(deviceInfoData));
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
        err = ndefRtdText(&type, TEXT_ENCODING_UTF8, &bufLanguage, &bufPayload);
        ASSERT_ERR_NONE("ndefRtdText() returned %d\r\n", err);
        }
        break;
    case NDEF_TYPE_RTD_URI:
        {
        static uint8_t ndefURI[] = "st.com";
        bufPayload.buffer = ndefURI;
        bufPayload.length = sizeof(ndefURI) - 1U;
        err = ndefRtdUri(&type, NDEF_URI_PREFIX_HTTP_WWW, &bufPayload); /* Initialize URI type structure */
        ASSERT_ERR_NONE("ndefRtdUri() returned %d\r\n", err);
        }
        break;
    case NDEF_TYPE_RTD_AAR:
        {
        static const uint8_t app[] = "com.st.st25nfc";
        bufPayload.buffer = app;
        bufPayload.length = sizeof(app) - 1U;
        err = ndefRtdAar(&type, &bufPayload);
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
        err = ndefMedia(&type, &bufType, &bufPayload);
        ASSERT_ERR_NONE("ndefMedia() returned %d\r\n", err);
        }
        break;
    case NDEF_TYPE_MEDIA_VCARD:
        {
        ndefConstBuffer bufTypeN     = { (uint8_t*)"N",     strlen((char*)"N")     };
        ndefConstBuffer bufTypeFN    = { (uint8_t*)"FN",    strlen((char*)"FN")    };
        ndefConstBuffer bufTypeADR   = { (uint8_t*)"ADR",   strlen((char*)"ADR")   };
        ndefConstBuffer bufTypeTEL   = { (uint8_t*)"TEL",   strlen((char*)"TEL")   };
        ndefConstBuffer bufTypeEMAIL = { (uint8_t*)"EMAIL", strlen((char*)"EMAIL") };
        ndefConstBuffer bufTypeTITLE = { (uint8_t*)"TITLE", strlen((char*)"TITLE") };
        ndefConstBuffer bufTypeORG   = { (uint8_t*)"ORG",   strlen((char*)"ORG")   };
        ndefConstBuffer bufTypeURL   = { (uint8_t*)"URL",   strlen((char*)"URL")   };

        ndefConstBuffer bufSubTypeNone    = {  NULL           , 0 };
        ndefConstBuffer bufSubTypeTelCELL = { (uint8_t*)"CELL", strlen((char*)"CELL") };

        uint8_t N[]     = "Doe;john;;Dr";
        uint8_t FN[]    = "Dr. John";
        uint8_t ADR[]   = "Main St.";
        uint8_t TEL[]   = "+1 123 456 7890";
        uint8_t EMAIL[] = "john.doe@gmail.com";
        uint8_t TITLE[] = "Doctor";
        uint8_t ORG[]   = "Corporation";
        uint8_t URL[]   = "http://www.johnny.com";

        ndefConstBuffer bufValueN     = { N,     strlen((char*)N)     };
        ndefConstBuffer bufValueFN    = { FN,    strlen((char*)FN)    };
        ndefConstBuffer bufValueADR   = { ADR,   strlen((char*)ADR)   };
        ndefConstBuffer bufValueTEL   = { TEL,   strlen((char*)TEL)   };
        ndefConstBuffer bufValueEMAIL = { EMAIL, strlen((char*)EMAIL) };
        ndefConstBuffer bufValueTITLE = { TITLE, strlen((char*)TITLE) };
        ndefConstBuffer bufValueORG   = { ORG,   strlen((char*)ORG)   };
        ndefConstBuffer bufValueURL   = { URL,   strlen((char*)URL)   };

        ndefVCardInput bufVCard[] = {
            { &bufTypeN    , &bufSubTypeNone   , &bufValueN     },
            { &bufTypeFN   , &bufSubTypeNone   , &bufValueFN    },
            { &bufTypeADR  , &bufSubTypeNone   , &bufValueADR   },
            { &bufTypeTEL  , &bufSubTypeTelCELL, &bufValueTEL   },
            { &bufTypeEMAIL, &bufSubTypeNone   , &bufValueEMAIL },
            { &bufTypeTITLE, &bufSubTypeNone   , &bufValueTITLE },
            { &bufTypeORG  , &bufSubTypeNone   , &bufValueORG   },
            { &bufTypeURL  , &bufSubTypeNone   , &bufValueURL   },
        };
        err = ndefVCard(&type, bufVCard, SIZEOF_ARRAY(bufVCard));
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
        err = ndefWifi(&type, &wifiConfig);
        ASSERT_ERR_NONE("ndefWifi() returned %d\r\n", err);
        }
        break;
    default:
        platformLog("Type not supported");
        return ERR_NOT_IMPLEMENTED;
        //break;
    }

    err = ndefTypeToRecord(&type, record); /* Encode Type into record */
    ASSERT_ERR_NONE("ndefTypeToRecord() returned %d\r\n", err);

    return err;
}


/*****************************************************************************/
static ReturnCode ndefSystemTest_WrapMessage(rfalNfcDevice* nfcDevice, ndefContext* ndefCtx, nfcDeviceType deviceType, const ndefBuffer* bufPayload, ndefBuffer* bufMessage)
{
    ReturnCode err = ERR_NOT_IMPLEMENTED;

    if ( (bufPayload == NULL) || (bufMessage == NULL) )
    {
        return ERR_PARAM;
    }

    switch (deviceType)
    {
    case NFC_DEVICE_TYPE_NFCA_T1T:
        break;
    case NFC_DEVICE_TYPE_NFCA_T2T:
        if (bufMessage->length < bufPayload->length + 3)
        {
            return ERR_NOMEM;
        }
        bufMessage->buffer[0] = 0x03;
        bufMessage->buffer[1] = bufPayload->length;
        ST_MEMCPY(&bufMessage->buffer[2], bufPayload->buffer, bufPayload->length); /* message */
        bufMessage->buffer[2 + bufPayload->length] = 0xFE;

        bufMessage->length = 1 + 1 + bufPayload->length + 1;
        err = ERR_NONE;
        break;
    case NFC_DEVICE_TYPE_NFCF_T3T:
#if 0
        {
        uint8_t AIB[16];
        uint32_t len;
        uint32_t offset = 0;

        err = ndefPollerContextInitialization(ndefCtx, nfcDevice);
        ASSERT_ERR_NONE("ndefPollerContextInitialization() returned %d\r\n", err);

        //err = ndefPollerNdefDetect(ndefCtx, NULL);
        ASSERT_ERR_NONE("ndefPollerNdefDetect() returned %d\r\n", err);

        /* Read Attribute Information Block */
        err = ndefPollerReadBytes(ndefCtx, offset, sizeof(AIB), AIB, &len);
        ASSERT_ERR_NONE("ndefPollerWriteBytes() returned %d\r\n", err);

        AIB[13] = bufPayload->length;
        uint16_t checksum = 0;
        for (uint32_t i = 0U; i < NDEF_T3T_ATTRIB_INFO_CHECKSUM_LEN; i++)
        {
            checksum += /*(uint16_t)*/AIB[i];
        }

        AIB[14] = checksum >>   8U;
        AIB[15] = checksum  & 0xFF;
        }
#endif
        err = ERR_NONE;
        break;
    case NFC_DEVICE_TYPE_NFCA_T4T: /* Fall through */
    case NFC_DEVICE_TYPE_NFCB_T4T:
        //ST_MEMCPY(&bufMessage->buffer[0], bufPayload->buffer, bufPayload->length); /* message */
        //bufMessage->length = bufPayload->length;
        //err = ERR_NONE;
        break;
    case NFC_DEVICE_TYPE_NFCV_T5T:
        if (bufMessage->length < bufPayload->length + 3)
        {
            return ERR_NOMEM;
        }
        bufMessage->buffer[0] = 0x03;
        bufMessage->buffer[1] = bufPayload->length;
        ST_MEMCPY(&bufMessage->buffer[2], bufPayload->buffer, bufPayload->length); /* message */
        bufMessage->buffer[2 + bufPayload->length] = 0xFE;

        bufMessage->length = 1 + 1 + bufPayload->length + 1;
        err = ERR_NONE;
        break;
    default:
        break;
    }

    return err;
}


/*****************************************************************************/
static ReturnCode ndefSystemTest_WriteMessage(rfalNfcDevice* nfcDevice, ndefContext* ndefCtx, nfcDeviceType deviceType, ndefBuffer* bufMessage)
{
    ReturnCode err;
    uint32_t   offset = 0;

    if ( (nfcDevice == NULL) || (ndefCtx == NULL) || (bufMessage == NULL) )
    {
        return ERR_PARAM;
    }

    switch (deviceType)
    {
    case NFC_DEVICE_TYPE_NFCA_T1T:
        break;
    case NFC_DEVICE_TYPE_NFCA_T2T:
        offset = 4 * NDEF_T2T_BLOCK_SIZE; // 4 x 4-byte block. Skip S/N and OTP
        break;
    case NFC_DEVICE_TYPE_NFCF_T3T:
        {
        uint8_t AIB[NDEF_T3T_BLOCKLEN];
        uint32_t len;

        err = ndefPollerContextInitialization(ndefCtx, nfcDevice);
        ASSERT_ERR_NONE("ndefPollerContextInitialization() returned %d\r\n", err);

        //err = ndefPollerNdefDetect(ndefCtx, NULL);
        ASSERT_ERR_NONE("ndefPollerNdefDetect() returned %d\r\n", err);

        /* Read Attribute Information Block */
        err = ndefPollerReadBytes(ndefCtx, offset, sizeof(AIB), AIB, &len);
        ASSERT_ERR_NONE("ndefPollerWriteBytes() returned %d\r\n", err);

        if (len != NDEF_T3T_BLOCKLEN)
        {
            platformLog("Read Attribute Information Block failed");
        }

        AIB[13] = bufMessage->length;
        uint16_t checksum = 0;
        for (uint32_t i = 0U; i < NDEF_T3T_ATTRIB_INFO_CHECKSUM_LEN; i++)
        {
            checksum += /*(uint16_t)*/AIB[i];
        }

        AIB[14] = checksum >>   8U;
        AIB[15] = checksum  & 0xFF;

        /* Write updated Attribute Information Block */
        err = ndefPollerWriteBytes(ndefCtx, offset, AIB, len);
        ASSERT_ERR_NONE("ndefPollerWriteBytes() returned %d\r\n", err);

        /* Write NDEF message in the following block */
        offset = NDEF_T3T_BLOCKLEN;
        }
        break;
    case NFC_DEVICE_TYPE_NFCA_T4T:  // Fall through
    case NFC_DEVICE_TYPE_NFCB_T4T:
        /* Select File Id */
        offset = 0;
        break;
    case NFC_DEVICE_TYPE_NFCV_T5T:
        {
        uint8_t CC[4];
        uint32_t len;

        err = ndefPollerContextInitialization(ndefCtx, nfcDevice);
        ASSERT_ERR_NONE("ndefPollerContextInitialization() returned %d\r\n", err);

        //err = ndefPollerNdefDetect(ndefCtx, NULL);
        ASSERT_ERR_NONE("ndefPollerNdefDetect() returned %d\r\n", err);

        err = ndefPollerReadBytes(ndefCtx, offset, sizeof(CC), CC, &len);
        ASSERT_ERR_NONE("ndefPollerWriteBytes() returned %d\r\n", err);
        //platformLog("Read %d bytes: 0x%x 0x%x 0x%x 0x%x\r\n", len, CC[0], CC[1], CC[2], CC[3]);
        if (CC[2] != 0)
        {
            offset = 4; // 4-byte CC
        }
        else
        {
            offset = 8; // 8-byte CC
        }
        }
        break;
    default:
        break;
    }

    //bool done = false;
    //while (!done)
    //{
    //    if (rfalNfcIsDevActivated(rfalNfcGetState()))
    //    {
    /* Write message */
    err = ndefPollerContextInitialization(ndefCtx, nfcDevice);
    ASSERT_ERR_NONE("ndefPollerContextInitialization() returned %d\r\n", err);

    //err = ndefPollerNdefDetect(ndefCtx, NULL);
    ASSERT_ERR_NONE("ndefPollerNdefDetect() returned %d\r\n", err);

    err = ndefPollerWriteBytes(ndefCtx, offset, bufMessage->buffer, bufMessage->length);
    ASSERT_ERR_NONE("ndefPollerWriteBytes() returned %d\r\n", err);
    //        done = true;
    //    }
    //}

    return err;
}


/*****************************************************************************/
static ReturnCode ndefSystemTest_ReadMessage(rfalNfcDevice* nfcDevice, ndefContext* ndefCtx, ndefBuffer* bufMessage)
{
    ReturnCode err;

    if ( (nfcDevice == NULL) || (ndefCtx == NULL) || (bufMessage == NULL) || (bufMessage->buffer == NULL) )
    {
        return ERR_PARAM;
    }

    //bool done = false;
    //while (!done)
    //{
    //    rfalNfcWorker();
    //    if ( rfalNfcIsDevActivated(rfalNfcGetState()))
    //    {
    err = ndefPollerContextInitialization(ndefCtx, nfcDevice);
    ASSERT_ERR_NONE("ndefPollerContextInitialization() returned %d\r\n", err);

    //err = ndefPollerNdefDetect(ndefCtx, NULL);
    ASSERT_ERR_NONE("ndefPollerNdefDetect() returned %d\r\n", err);

    err = ndefPollerReadRawMessage(ndefCtx, bufMessage->buffer, bufMessage->length, &bufMessage->length);
    ASSERT_ERR_NONE("ndefPollerReadMessage() returned %d\r\n", err);
    //        done = true;
    //    }
    //}

    return err;
}


/*****************************************************************************/
/* Poll until a given type of tag is detected */
/*****************************************************************************/
#if 0
ReturnCode ndefSystemTest_DetectDeviceType(rfalNfcDevice* nfcDevice, nfcDeviceType* deviceType)
{
    ReturnCode    err = ERR_NONE;
    nfcDeviceType deviceDetected;

    if ( (nfcDevice == NULL) || (deviceType == NULL) )
    {
        return ERR_PARAM;
    }

    platformLog("Present a tag %s: ", ndefDeviceTypeStr[*deviceType]);

    bool found = false;
    while(!found)
    {
        rfalNfcWorker(); /* Run RFAL worker periodically */

        if (rfalNfcIsDevActivated(rfalNfcGetState()))
        {
            switch (nfcDevice->type)
            {
            case RFAL_NFC_LISTEN_TYPE_NFCA:
                switch (nfcDevice->dev.nfca.type)
                {
                case RFAL_NFCA_T1T:
                    //platformLog("ISO14443A/Topaz (NFC-A T1T) TAG found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
                    deviceDetected = NFC_DEVICE_TYPE_NFCA_T1T;
                    break;
                case RFAL_NFCA_T2T:
                    //platformLog("T2T device found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
                    deviceDetected = NFC_DEVICE_TYPE_NFCA_T2T;
                    break;
                case RFAL_NFCA_T4T:
                    //platformLog("NFCA Passive ISO-DEP device found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
                    deviceDetected = NFC_DEVICE_TYPE_NFCA_T4T;
                    break;
                default:
                    platformLog("Unknown NFCA device %d\r\n", nfcDevice->dev.nfca.type);
                    break;
                }
                break;
            case RFAL_NFC_LISTEN_TYPE_NFCB:
                //platformLog("NFCB T4B device found\r\n");
                deviceDetected = NFC_DEVICE_TYPE_NFCB_T4T;
                break;
            case RFAL_NFC_LISTEN_TYPE_NFCF:
                //platformLog("NFCF T3T device found\r\n");
                deviceDetected = NFC_DEVICE_TYPE_NFCF_T3T;
                break;
            case RFAL_NFC_LISTEN_TYPE_NFCV:
                //platformLog("NFCV T5T device found\r\n");
                deviceDetected = NFC_DEVICE_TYPE_NFCV_T5T;
                break;
            default:
                platformLog("Unknown device type found %d\r\n", nfcDevice->type);
                break;
            }

            //rfalNfcDeactivate(false);
            if ( (*deviceType == NFC_DEVICE_TYPE_ANY) || (*deviceType == deviceDetected) )
            {
                found       = true;
                *deviceType = deviceDetected; /* Update output param with actual detected type */
            }
        }
    }

    platformLog("Detected %s\r\n", ndefDeviceTypeStr[*deviceType]);
    return err;
}
#endif


/*****************************************************************************/
/*************************** Read Raw Message ********************************/
/*****************************************************************************/


/*****************************************************************************/
/*
 * ReadRawMessage with 0-length message
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_ReadRawMessageEmpty(nfcDeviceType deviceType)
{
    ReturnCode    err = ERR_NONE;
    //rfalNfcDevice nfcDevice;
    ndefContext   ndefCtx;
    ndefInfo      info;

    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Detect a given tag */
    //err = ndefSmokeTest_DetectDeviceType(&nfcDevice, &deviceType);
    ASSERT_ERR_NONE("DetectTag() returned %d\r\n", err);

    platformLog("Found device %s\r\n", ndefDeviceTypeStr[deviceType]);

    /* NDEF Context Initialization */
    err = ndefPollerContextInitialization(&ndefCtx, &nfcDevice);
    ASSERT_ERR_NONE("ndefPollerContextInitialization() returned %d\r\n", err);

    /* NDEF Detect */
    err = ndefPollerNdefDetect(&ndefCtx, &info);
    ASSERT_ERR_NONE("ndefPollerNdefDetect() returned %d\r\n", err);

    ndefDumpInfo(&info);

    //ndefCCDump(&ndefCtx);

    platformLog("NDEF Len: %d, Offset=%d\r\n", ndefCtx.messageLen, ndefCtx.messageOffset);

    /* Prepare the tag */
    err = ndefSystemTest_PrepareWriteEmptyMessage(&ndefCtx, deviceType);
    ASSERT_ERR_NONE("WriteEmptyMessage() returned %d)\r\n", err);

    // Write

    /* NDEF Detect */
    err = ndefPollerNdefDetect(&ndefCtx, &info);
    ASSERT_ERR_NONE("ndefPollerNdefDetect() returned %d\r\n", err);

    ndefDumpInfo(&info);

    //ndefCCDump(&ndefCtx);

    /* Check status is INITIALIZED */
    if (info.state != NDEF_STATE_INITIALIZED)
    {
        platformLog("Incorrect state, expecting NDEF_STATE_INITIALIZED\r\n");
        /*return*/err = ERR_INTERNAL;
    }

    /* Check info.length = 0 */
    if (info.areaLen != 0U)
    {
        platformLog("Incorrect length, expecting info.length=0\r\n");
        /*return*/err = ERR_INTERNAL;
    }

    /* ReadRawMessage with 0-length message */
    uint8_t  rawMessageBuf[SYSTEM_TEST_BUFFER_LEN];
    uint32_t rawMessageLen;
    err = ndefPollerReadRawMessage(&ndefCtx, rawMessageBuf, sizeof(rawMessageBuf), &rawMessageLen);
    ASSERT_ERR_NONE("ndefPollerReadRawMessage() returned %d\r\n", err);

    if (rawMessageLen != 0U)
    {
        platformLog("rawMessageLen != 0\r\n");
    }

    ndefConstBuffer bufRawMessage = { rawMessageBuf, rawMessageLen };
    ndefBufferDump("NDEF Content", (ndefConstBuffer*)&bufRawMessage, true);

    return err;
}


/*****************************************************************************/
/* ReadRawMessage with 17 bytes message and write access granted
 * buffer size 17
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_ReadRawMessageWriteGranted(nfcDeviceType deviceType)
{
    ReturnCode    err = ERR_NONE;
    //rfalNfcDevice nfcDevice;
    ndefContext   ndefCtx;
    ndefInfo      info;

    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Detect a given tag */
    //err = ndefSmokeTest_DetectDeviceType(&nfcDevice, &deviceType);
    ASSERT_ERR_NONE("DetectTag() returned %d\r\n", err);

    err = ndefSystemTest_PrepareWriteMessage17bytes();
    ASSERT_ERR_NONE("Preparation returned %d\r\n", err);

    ndefDumpInfo(&info);

    //ndefCCDump(&ndefCtx);

    /* Check status is READWRITE */
    if (info.state != NDEF_STATE_READWRITE)
    {
        platformLog("Incorrect state, expecting NDEF_STATE_READWRITE\r\n");
        /*return*/err = ERR_INTERNAL;
    }

    /* Check info.length = 17 */
    if (info.areaLen != 17U)
    {
        platformLog("Incorrect length, expecting info.length=17\r\n");
        /*return*/err = ERR_INTERNAL;
    }

    uint8_t rawMessageBuf[17];
    uint32_t rawMessageLen;
    err = ndefPollerReadRawMessage(&ndefCtx, rawMessageBuf, sizeof(rawMessageBuf), &rawMessageLen);
    ASSERT_ERR_NONE("ndefPollerReadRawMessage() returned %d\r\n", err);

    if (rawMessageLen != 17U)
    {
        platformLog("rawMessageLen != 17\r\n");
    }

    ndefConstBuffer bufRawMessage = { rawMessageBuf, rawMessageLen };
    ndefBufferDump("NDEF Content", (ndefConstBuffer*)&bufRawMessage, true);

    return err;
}


/*****************************************************************************/
/* ReadRawMessage with 16 bytes message and write access denied
 * buffer size 17
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_ReadRawMessageWriteDenied(nfcDeviceType deviceType)
{
    ReturnCode    err = ERR_NONE;
    //rfalNfcDevice nfcDevice;
    ndefContext   ndefCtx;
    ndefInfo      info;

    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Detect a given tag */
    //err = ndefSmokeTest_DetectDeviceType(&nfcDevice, &deviceType);
    ASSERT_ERR_NONE("DetectTag() returned %d\r\n", err);

    err = ndefSystemTest_PrepareWriteMessage17bytes();
    ASSERT_ERR_NONE("Preparation returned %d\r\n", err);

    ndefDumpInfo(&info);

    //ndefCCDump(&ndefCtx);

    /* Check status is READONLY */
    if (info.state != NDEF_STATE_READONLY)
    {
        platformLog("Incorrect state, expecting NDEF_STATE_READONLY\r\n");
        /*return*/err = ERR_INTERNAL;
    }

    /* Check info.length = 17 */
    if (info.areaLen != 17U)
    {
        platformLog("Incorrect length, expecting info.length=17\r\n");
        /*return*/err = ERR_INTERNAL;
    }

    uint8_t rawMessageBuf[17];
    uint32_t rawMessageLen;
    err = ndefPollerReadRawMessage(&ndefCtx, rawMessageBuf, sizeof(rawMessageBuf), &rawMessageLen);
    ASSERT_ERR_NONE("ndefPollerReadRawMessage() returned %d\r\n", err);

    if (rawMessageLen != 17U)
    {
        platformLog("rawMessageLen != 17\r\n");
    }

    ndefConstBuffer bufRawMessage = { rawMessageBuf, rawMessageLen };
    ndefBufferDump("NDEF Content", (ndefConstBuffer*)&bufRawMessage, true);

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* ReadRawMessage with full area size message and write access granted
 * Test variation: run with different tag sizes
 */
/*****************************************************************************/
ReturnCode ndefsystemTest_ReadRawMessageFull(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Check status is READWRITE */
    /* Check info.length = correct length */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* ReadRawMessage with 16 bytes message and write access granted
 * buffer size 15
 * The reader tries to read 1 block beyond the memory and expects an error
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_ReadRawMessageFullPlusOne(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Check status is ERR_NOMEM */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* Pre: Write a predefined NDEF message with multiple records to the tag
 * Check all records are read
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_ReadMultipleRecords(nfcDeviceType deviceType)
{
    ReturnCode    err = ERR_NONE;
    //rfalNfcDevice nfcDevice;
    //ndefContext   ndefCtx;
    ndefMessage   messageIn;
    ndefMessage   messageOut;
    ndefRecord    recordIn;
    ndefRecord*   record;
    ndefMessageInfo info;

    uint8_t  rawPayloadBuf[SYSTEM_TEST_BUFFER_LEN];
    //uint32_t rawPayloadLen = sizeof(rawPayloadBuf);
    ndefBuffer bufPayload = { rawPayloadBuf, sizeof(rawPayloadBuf) };

    uint8_t  rawMessageBuf[SYSTEM_TEST_BUFFER_LEN];
    //uint32_t rawMessageLen = sizeof(rawMessageBuf);
    ndefBuffer bufMessage = { rawMessageBuf, sizeof(rawMessageBuf) };

    uint8_t  rawMessageOutBuf[SYSTEM_TEST_BUFFER_LEN];
    ndefBuffer bufMessageOut = { rawMessageOutBuf, sizeof(rawMessageOutBuf) };

    platformLog("Running %s...\r\n", __FUNCTION__);

    ndefTypeId typeId;
    ndefTypeId typeIdRead;
    //typeId = NDEF_TYPE_EMPTY;
    //typeId = NDEF_TYPE_RTD_DEVICE_INFO;
    typeId = NDEF_TYPE_RTD_TEXT;
    //typeId = NDEF_TYPE_RTD_URI;
    //typeId = NDEF_TYPE_RTD_AAR;
    //typeId = NDEF_TYPE_MEDIA;
    //typeId = NDEF_TYPE_MEDIA_VCARD;
    //typeId = NDEF_TYPE_MEDIA_WIFI;

    /* Detect a given tag */
    //err = ndefSmokeTest_DetectDeviceType(&nfcDevice, &deviceType);
    ASSERT_ERR_NONE("DetectTag() returned %d\r\n", err);

    err = ndefMessageInit(&messageIn); /* Initialize message */
    ASSERT_ERR_NONE("ndefMessageInit() returned %d\r\n", err);

    err = ndefSystemTest_CreateRecord(typeId, &recordIn);
    ASSERT_ERR_NONE("ndefSystemTest_CreateRecord() returned %d\r\n", err);

    err = ndefMessageAppend(&messageIn, &recordIn); /* Append record to message */
    ASSERT_ERR_NONE("ndefMessageAppend() returned %d\r\n", err);

    // TODO Message with several records

    err = ndefMessageEncode(&messageIn, &bufPayload);
    ASSERT_ERR_NONE("ndefMessageEncode() returned %d\r\n", err);

    //err = ndefSystemTest_PrepareNdefBuffer(typeId, &messageIn);
    //ASSERT_ERR_NONE("ndefSystemTest_PrepareMessage() returned %d\r\n", err);

    // Compare buffers
    /*if ( (bufPayload.length != ...) ||
       (ST_BYTECMP(bufPayload.buffer, ..., bufPayload.length) != 0)
    {
        platformLog("Message mismatch");
        return ERR_INTERNAL;
    }*/

    err = ndefSystemTest_WrapMessage(&nfcDevice, &ndefCtx, deviceType, &bufPayload, &bufMessage);
    ASSERT_ERR_NONE("ndefSystemTest_EncapsulateMessage() returned %d\r\n", err);

    err = ndefSystemTest_WriteMessage(&nfcDevice, &ndefCtx, deviceType, &bufMessage);
    ASSERT_ERR_NONE("ndefSystemTest_WriteMessage() returned %d\r\n", err);

    /* Check all records are decoded */
    err = ndefSystemTest_ReadMessage(&nfcDevice, &ndefCtx, &bufMessageOut);
    ASSERT_ERR_NONE("ndefSystemTest_ReadMessage() returned %d\r\n", err);

    err = ndefMessageDecode((const ndefConstBuffer*)&bufMessageOut, &messageOut);
    ASSERT_ERR_NONE("ndefMessageDecode() returned %d\r\n", err);

    // Check type
    err = ndefMessageGetInfo(&messageOut, &info);
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

    // Get record type
    record = ndefMessageGetFirstRecord(&messageOut);
    if (record == NULL)
    {
        platformLog("Record not found!");
        return ERR_INTERNAL;
    }
    // Rename to ndefGetRecordTypeId ? (ndefRecordGetType already exists, return ndefTypeStruct->id ?)
    err = ndefRecordTypeStringToTypeId(record, &typeIdRead);
    ASSERT_ERR_NONE("ndefRecordTypeStringToTypeId() returned %d\r\n", err);

    if (typeIdRead != typeId)
    {
        platformLog("Invalid record type found!");
        return ERR_INTERNAL;
    }

    err = ndefMessageDump(&messageOut, true);
    ASSERT_ERR_NONE("ndefMessageDump() returned %d\r\n", err);

    /* Manual test: check the same with a phone */
    platformLog("Check the message is decoded on a phone\r\n");

    /* Wait for tag removed from the field */

    platformLog("Decoded or not ? [y/n]\r\n");

    return err;
}


/*****************************************************************************/
/******************** Write Raw Message / Write Message **********************/
/*****************************************************************************/


/*****************************************************************************/
/* WriteRawMessage with 0-length message
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_WriteRawMessageEmpty(nfcDeviceType deviceType)
{
    ReturnCode    err = ERR_NONE;
    //rfalNfcDevice nfcDevice;
    ndefContext   ndefCtx;
    ndefInfo      info;
//    uint32_t      offset = 0;

    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Detect a given tag */
    //err = ndefSmokeTest_DetectDeviceType(&nfcDevice, &deviceType);
    ASSERT_ERR_NONE("DetectTag() returned %d\r\n", err);

    /* WriteRawMessage with 0-length message */
//    offset = 0;
    //uint8_t* buffer = NULL;
    uint8_t buffer[1] = {1};
    //ndefBuffer bufPayload;
    //bufPayload.buffer = buffer;
    //bufPayload.length = 0;
    err = ndefPollerContextInitialization(&ndefCtx, &nfcDevice);

    //ndefCCDump(&ndefCtx);

    err = ndefPollerNdefDetect(&ndefCtx, &info);

    ndefDumpInfo(&info);

    //ndefMessageGetInfo(message, &info);
    //err = ndefPollerBeginWriteMessage(&ndefCtx, /*info.length*/0);
    //ASSERT_ERR_NONE("ndefPollerBeginWriteMessage() returned %d\r\n", err);
    //
    //err = ndefPollerWriteBytes(&ndefCtx, offset, buffer, sizeof(buffer));
    //ASSERT_ERR_NONE("ndefPollerWriteBytes() returned %d\r\n", err);
    //
    //err = ndefPollerEndWriteMessage(&ndefCtx, /*info.length*/0);
    //ASSERT_ERR_NONE("ndefPollerEndWriteMessage() returned %d\r\n", err);
    err = ndefPollerWriteRawMessage(&ndefCtx, buffer, sizeof(buffer));
    ASSERT_ERR_NONE("ndefPollerWriteRawMessage() returned %d\r\n", err);

    /* NDEF Detect */
    err = ndefPollerNdefDetect(&ndefCtx, &info);
    ASSERT_ERR_NONE("ndefPollerNdefDetect() returned %d\r\n", err);

    ndefDumpInfo(&info);

    //ndefCCDump(&ndefCtx);

    /* Check L-field/NLEN=0 */
    if (info.areaLen != 0U)
    {
        platformLog("Incorrect length, expecting info.length=0\r\n");
        /*return*/err = ERR_INTERNAL;
    }

    /* Check Terminator TLV (except T4T) */
    if ( (deviceType != NFC_DEVICE_TYPE_NFCA_T4T) &&
         (deviceType != NFC_DEVICE_TYPE_NFCB_T4T) )
    {
        // TODO
    }

    /* Manual test: Check a phone can recognize the empty NDEF tag */

    return err;
}


/*****************************************************************************/
/* WriteMessage with 0-length message
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_WriteMessageEmpty(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* WriteMessage with 0-length message */

    /* Check L-field/NLEN=0 */
    /* Check Terminator TLV (except T4T) */

    /* Manual test: Check a phone can recognize the empty NDEF tag */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* WriteRawMessage and write access denied
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_WriteRawMessageWriteDenied(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* WriteRawMessage */

    /* Check WriteRawMessage returns an error */
    /* Check previous tag content is not modified */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* WriteMessage and write access denied
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_WriteMessageWriteDenied(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* WriteMessage */

    /* Check WriteMessage returns an error */
    /* Check previous tag content is not modified */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* WriteRawMessage full area size message - 1
 * (to check the terminator byte is handled properly)
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_WriteRawMessageFullMinusOne(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Test applies to all tags except T4A and T4B */
    if ( (deviceType == NFC_DEVICE_TYPE_NFCA_T4T) ||
         (deviceType == NFC_DEVICE_TYPE_NFCB_T4T) )
    {
        return ERR_NONE;
    }

    /* WriteRawMessage full area size message - 1 */
    /* (to check the terminator byte is handled properly) */

    /* Check L-field/NLEN value */
    /* Check tag memory content */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* WriteMessage full area size message - 1
 * (to check the terminator byte is handled properly )
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_WriteMessageFullMinusOne(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Test applies to all tags except T4A and T4B */
    if ( (deviceType == NFC_DEVICE_TYPE_NFCA_T4T) ||
         (deviceType == NFC_DEVICE_TYPE_NFCB_T4T) )
    {
        return ERR_NONE;
    }

    /* WriteMessage full area size message - 1 */
    /* (to check the terminator byte is handled properly ) */

    /* Check L-field/NLEN value */
    /* Check tag memory content */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* WriteRawMessage full area size message
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_WriteRawMessageFull(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* WriteRawMessage full area size message */

    /* Check L-field/NLEN value */
    /* Check tag memory content */

    /* Manual test: Check a phone can read the full NDEF message */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* WriteMessage full area size message
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_WriteMessageFull(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* WriteMessage full area size message */

    /* Check L-field/NLEN value */
    /* Check tag memory content */

    /* Manual test: Check a phone can read the full NDEF message */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* WriteRawMessage with message size greater than area size
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_WriteRawMessageFullPlusOne(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* WriteRawMessage with message size greater than area size*/

    /* Check WriteRawMessage returns an error */
    /* Check previous tag content is not modified */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* WriteMessage with message size greater than area size
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_WriteMessageFullPlusOne(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* WriteMessage with message size greater than area size */

    /* Check WriteMessage returns an error */
    /* Check previous tag content is not modified */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* Write an NDEF message with multiple records to the tag
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_WriteMultipleRecords(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Write an NDEF message with multiple records to the tag */

    /* Check all records are decoded */

    /* Manual test: Check a phone can read the NDEF message and decode all the records */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/*********************** Capability Container Check **************************/
/*****************************************************************************/


/*****************************************************************************/
/* Format Tag
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_FormatTag(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Format Tag */

    /* Check CC values */
    /* Check L-field/NLEN=0 */
    /* Check terminator TLV (except T4T) */

    /* Manual test: Check a phone can recognize the empty NDEF tag */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* NDEF Check Presence
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_CheckPresence(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* NDEF CheckPresence */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* Read tag with invalid access rights (invalid value or access denied)
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_ReadInvalidAccess(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Read tag with invalid access rights (invalid value or access denied) */

    /* Check NDEF Detect returns an error */
    /* Check no read except CC */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* Read tag with erroneous magic
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_ReadCCBadMagicNumber(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Test applies to all tags except T3, T4A and T4B */
    if ( (deviceType == NFC_DEVICE_TYPE_NFCF_T3T) ||
         (deviceType == NFC_DEVICE_TYPE_NFCA_T4T) ||
         (deviceType == NFC_DEVICE_TYPE_NFCB_T4T) )
    {
        return ERR_NONE;
    }

    /* Read tag with erroneous magic */

    /* Check NDEF Detect returns an error */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* Read tag with erroneous checksum in AIB
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_ReadCCBadAibChecksum(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Test applies to T3T tags only */
    if (deviceType != NFC_DEVICE_TYPE_NFCF_T3T)
    {
        return ERR_NONE;
    }

    /* Read tag with erroneous checksum in AIB */

    /* Check NDEF Detect returns an error */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* Read tag with erroneous version
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_ReadCCBadVersion(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Read tag with erroneous version */

    /* Check NDEF Detect returns an error*/

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* Read tag with the magic number E2
 */
/*****************************************************************************/
ReturnCode ndefsystemTest_ReadCCMagicNumberE2(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Test applies to T5T tags only */
    if (deviceType != NFC_DEVICE_TYPE_NFCV_T5T)
    {
        return ERR_NONE;
    }

    /* Read tag with the magic number E2 */

    /* Check read OK */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* Read tag with CC Len = 8
 */
/*****************************************************************************/
ReturnCode ndefsystemTest_ReadCCLength8(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Test applies to T5T tags only */
    if (deviceType != NFC_DEVICE_TYPE_NFCV_T5T)
    {
        return ERR_NONE;
    }

    /* Read tag with CC Len = 8 */

    /* Check read OK */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* Read tag with Android CC
 */
/*****************************************************************************/
ReturnCode ndefsystemTest_ReadCCAndroid(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Test applies to T5T tags only */
    if (deviceType != NFC_DEVICE_TYPE_NFCV_T5T)
    {
        return ERR_NONE;
    }

    /* Read tag with Android CC */

    /* Check read OK */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* Read T4 tag with no CC FILE
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_ReadNoCCFile(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Test applies to T4TA and T4TB tags only */
    if ( (deviceType != NFC_DEVICE_TYPE_NFCA_T4T) &&
         (deviceType != NFC_DEVICE_TYPE_NFCB_T4T) )
    {
        return ERR_NONE;
    }

    /* Read T4 tag with no CC FILE */

    /* Check NDEF Detect returns an error */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* Read T4 tag with no NDEF application
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_ReadNoNdefApp(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Test applies to T4TA and T4TB tags only */
    if ( (deviceType != NFC_DEVICE_TYPE_NFCA_T4T) &&
         (deviceType != NFC_DEVICE_TYPE_NFCB_T4T) )
    {
        return ERR_NONE;
    }

    /* Read T4 tag with no NDEF application */

    /* Check NDEF Detect returns an error */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* Read T4 tag with no ISO DEP
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_ReadNoIsoDep(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Test applies to T4TA and T4TB tags only */
    if ( (deviceType != NFC_DEVICE_TYPE_NFCA_T4T) &&
         (deviceType != NFC_DEVICE_TYPE_NFCB_T4T) )
    {
        return ERR_NONE;
    }

    /* Read T4 tag with no ISO DEP */

    /* Check NDEF Detect returns an error */

    return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
/* WriteRawMessage, tag removed from the field during write
 */
/*****************************************************************************/
ReturnCode ndefSystemTest_WriteRawMessageRobustness(nfcDeviceType deviceType)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* WriteRawMessage, tag removed from the field during write */

    /* Check either previous content or length=0 */

    return ERR_NOT_IMPLEMENTED;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
ReturnCode ndefSystemTests(void)
{
    static uint8_t NFCID3[] = {0x01, 0xFE, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    static uint8_t GB[] = {0x46, 0x66, 0x6d, 0x01, 0x01, 0x11, 0x02, 0x02, 0x07, 0x80, 0x03, 0x02, 0x00, 0x03, 0x04, 0x01, 0x32, 0x07, 0x01, 0x03};

    ReturnCode err = ERR_NONE;
    nfcDeviceType tag = NFC_DEVICE_TYPE_ANY;

    rfalNfcDiscoverParam discParam;

    platformLog("Running NDEF System tests...\r\n");

    err = rfalNfcInitialize();
    ASSERT_ERR_NONE("rfalNfcInitialize() returned %d\r\n", err);
    if (err == ERR_NONE)
    {
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
        discParam.techs2Find           = RFAL_NFC_POLL_TECH_AP2P;
    }

    //rfalNfcDeactivate(false);
    rfalNfcDiscover(&discParam);
    //ASSERT_ERR_NONE("rfalNfcDiscover() returned %d\r\n", err);

    /*************************** Read Raw Message ****************************/

    //for (nfcDeviceType tag = NFC_DEVICE_TYPE_NFCA_T1T; tag < NFC_DEVICE_TYPE_NFCV_T5T; tag++)
    //{
/*
        err |= ndefSystemTest_ReadRawMessageEmpty(NFC_DEVICE_TYPE_ANY);

        err |= ndefSystemTest_ReadRawMessageWriteGranted(tag);
        err |= ndefSystemTest_ReadRawMessageWriteDenied(tag);

        err |= ndefsystemTest_ReadRawMessageFull(tag);
        err |= ndefSystemTest_ReadRawMessageFullPlusOne(tag);
*/
        err |= ndefSystemTest_ReadMultipleRecords(tag);

        /******************** Write Raw Message / Write Message ******************/
/*
        err |= ndefSystemTest_WriteRawMessageEmpty(tag);
        err |= ndefSystemTest_WriteMessageEmpty(tag);

        err |= ndefSystemTest_WriteRawMessageWriteDenied(tag);
        err |= ndefSystemTest_WriteMessageWriteDenied(tag);

        err |= ndefSystemTest_WriteRawMessageFullMinusOne(tag);
        err |= ndefSystemTest_WriteMessageFullMinusOne(tag);

        err |= ndefSystemTest_WriteRawMessageFull(tag);
        err |= ndefSystemTest_WriteMessageFull(tag);

        err |= ndefSystemTest_WriteRawMessageFullPlusOne(tag);
        err |= ndefSystemTest_WriteMessageFullPlusOne(tag);

        err |= ndefSystemTest_WriteMultipleRecords(tag);
*/
        /*********************** Capability Container Check **********************/
/*
        err |= ndefSystemTest_FormatTag(tag);

        err |= ndefSystemTest_CheckPresence(tag);

        err |= ndefSystemTest_ReadInvalidAccess(tag);

        err |= ndefSystemTest_ReadCCBadMagicNumber(tag);
        err |= ndefSystemTest_ReadCCBadAibChecksum(tag);
        err |= ndefSystemTest_ReadCCBadVersion(tag);

        err |= ndefsystemTest_ReadCCMagicNumberE2(tag);
        err |= ndefsystemTest_ReadCCLength8(tag);
        err |= ndefsystemTest_ReadCCAndroid(tag);

        err |= ndefSystemTest_ReadNoCCFile(tag);
        err |= ndefSystemTest_ReadNoNdefApp(tag);
        err |= ndefSystemTest_ReadNoIsoDep(tag);

        err |= ndefSystemTest_WriteRawMessageRobustness(tag);
*/
    //}

    if (err == ERR_NONE)
    {
        platformLog("System tests passed!\r\n");
    }
    else
    {
        platformLog("System tests failed...\r\n");
    }

    return err;
}
