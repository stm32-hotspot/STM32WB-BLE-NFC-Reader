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
 *  \brief NDEF message test implementation
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include <stdlib.h>
#include "platform.h"
#include "utils.h"
#include "ndef_record.h"
#include "ndef_message.h"
#include "ndef_types.h"
#include "ndef_types_rtd.h"
#include "ndef_types_mime.h"
#include "ndef_type_wifi.h"
#include "ndef_dump.h"
#include "ndef_unitary_tests.h"

/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

#define NDEF_MIN_RECORD_LEN       3U    /*!< Minimum record length is: sizeof(uint8_t) + sizeof(typeLength) + sizeof(payloadLength) */


/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */


/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */


/*****************************************************************************/
ReturnCode ndefRecordCopy(const ndefRecord* recordSrc, ndefRecord* recordDest)
{
    if ( (recordSrc == NULL) || (recordDest == NULL) )
    {
        return ERR_NOMEM;
    }

    recordDest->header = recordSrc->header;

    recordDest->typeLength    = recordSrc->typeLength;
    recordDest->idLength      = recordSrc->idLength;

    recordDest->type    = recordSrc->type;
    recordDest->id      = recordSrc->id;
    recordDest->bufPayload = recordSrc->bufPayload;

    recordDest->ndeftype = recordSrc->ndeftype;

    return ERR_NONE;
}


/*****************************************************************************/
bool ndefRecordMatch(const ndefRecord* record1, const ndefRecord* record2)
{
    if ( (record1 == NULL) || (record2 == NULL) )
    {
        return false;
    }

    if ( (record1->header            == record2->header)            &&
         (record1->typeLength        == record2->typeLength)        &&
         (record1->idLength          == record2->idLength)          &&

         (ST_BYTECMP(record1->type,    record2->type,    record1->typeLength)    == 0) &&
         (ST_BYTECMP(record1->id,      record2->id,      record1->idLength)      == 0) &&
         (
             (record1->bufPayload.length == record2->bufPayload.length)
          ||
             ( /* Not a well-known type */
             (record1->bufPayload.length != 0) &&
             (record1->bufPayload.length == record2->bufPayload.length) &&
             (ST_BYTECMP(record1->bufPayload.buffer, record2->bufPayload.buffer, record1->bufPayload.length) == 0) )
          ||
             ( /* Well-known type */
             ( (record1->ndeftype != NULL) || (record2->ndeftype != NULL) ) &&
             ndefRecordGetPayloadLength(record1) == ndefRecordGetPayloadLength(record2) )
         )
        )
    {
        return true;
    }

    return false;
}


/*****************************************************************************/
bool ndefMessageMatch(const ndefMessage* message1, const ndefMessage* message2)
{
    uint32_t recordCount1 = ndefMessageGetRecordCount(message1);
    uint32_t recordCount2 = ndefMessageGetRecordCount(message2);

    if (recordCount1 != recordCount2)
    {
        return false;
    }

    ndefRecord* record1 = ndefMessageGetFirstRecord(message1);
    ndefRecord* record2 = ndefMessageGetFirstRecord(message2);

    while ( (record1 != NULL) && (record2 != NULL) )
    {
        if (ndefRecordMatch(record1, record2) == false)
        {
            return false;
        }

        record1 = ndefMessageGetNextRecord(record1);
        record2 = ndefMessageGetNextRecord(record2);
    }

    /* Check one message shorter than the other */
    if ( (record1 == NULL) ^ (record2 == NULL) )
    {
        return false;
    }

    return true;
}


/*
 ******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 ******************************************************************************
 */


/*****************************************************************************/
ReturnCode ndefMessageConformance(const ndefMessage* message)
{
    ndefRecord* record = NULL;

    if (message == NULL)
    {
        return ERR_NOMSG;
    }

    /*
       2.5 NDEF Mechanisms Test Requirements
       ===================================== */

    /* 
       Message requirements
       -------------------- */
    /*    M1. Each NDEF message MUST be exchanged in its entirety. */
    /*    M2. The first record in a message is marked with the MB (Message Begin) flag set. */
    record = message->record;
    if (record == NULL)
    {
        return ERR_NOMSG;
    }

    /* First record */
    MY_ASSERT(ndefHeaderMB(record) == 1U, ERR_SYNTAX);

    /*    M3. The last record in the message is marked with the ME (Message End) flag set. */
    while (record->next != NULL)
    {
        record = record->next;
    }
    if (message->record != record)
    {
        platformLog("Reached the last record\r\n");
        MY_ASSERT(ndefHeaderME(record) == 1U, ERR_SYNTAX);
    }

    /* Start again from first record */
    record = message->record;
    while (record != NULL)
    {
        /* Check field range (Needed for single-bit fields ?) */
        CHECK_RANGE(ndefHeaderMB(record),  0, 1);
        CHECK_RANGE(ndefHeaderME(record),  0, 1);
        CHECK_RANGE(ndefHeaderCF(record),  0, 1);
        CHECK_RANGE(ndefHeaderSR(record),  0, 1);
        CHECK_RANGE(ndefHeaderIL(record),  0, 1);
        CHECK_RANGE(ndefHeaderTNF(record), NDEF_TNF_EMPTY, NDEF_TNF_RESERVED);

        /* Check other fields */
        /* MY_ASSERT(record->payloadLength != NULL, ERR_SYNTAX); */

        /* NDEF messages MUST NOT overlap; that is, the MB and the ME flags MUST NOT be used to nest NDEF messages. */

        record = record->next;
    }


    /*
       Record chunk requirements 
       ------------------------- */

    /*    R1. Each chunked payload is encoded as an initial record chunk followed by 0 or more middle
          record chunks and finally by a terminating record chunk. */
    /* Start again from first record */
    record = message->record;

    MY_ASSERT(ndefHeaderCF(record) == 1U, ERR_SYNTAX);

    record = record->next;
    while (record != NULL)
    {
        platformLog("Going through the list of records until the last one\r\n");

        /* Check middle and last is cleared */
        MY_ASSERT(ndefHeaderCF(record) == 0U, ERR_SYNTAX);

        record = record->next;
    }

    /*    R2. The initial record chunk is an NDEF record with the CF (Chunk Flag) flag set. */
    /* Same as R1 ? */

    /*    R3. The type of the entire chunked payload MUST be indicated in the TYPE field of the initial record chunk. */
    record = message->record;
    uint8_t typeLength = record->typeLength;
    const uint8_t* type = record->type;
    while (record != NULL)
    {
        platformLog("Going through the list of records until the last one\r\n");

        MY_ASSERT(typeLength == record->typeLength, ERR_SYNTAX);
        int diff = ST_BYTECMP(type, record->type, typeLength);
        if (diff)
        {
            return ERR_SYNTAX;
        }

        record = record->next;

        /* Need to check type length also ? */
    }

    /*    R4. The payloadLength field of the initial record indicates the size of the data carried 
              in the PAYLOAD field of the initial record only, not the entire payload size. */

    /*    R5. Each middle record chunk is an NDEF record with the CF flag set. */
    /* same as R1, R2 ? */

    /*    R6. For each middle record chunk the value of the typeLength and the IL fields MUST be 0. */
    record = message->record;
    record = record->next;
    while (record != NULL)
    {
        platformLog("Going through the list of records until the last one\r\n");
        
        MY_ASSERT(record->typeLength == 0U, ERR_SYNTAX);
        MY_ASSERT(ndefHeaderIL(record) == 0U, ERR_SYNTAX);
        
        record = record->next;
    }

    /*    R7. For each middle record chunk the TNF (Type Name Format) field value MUST be 0x06 (Unchanged). */
    record = message->record;
    record = record->next;
    while (record != NULL)
    {
        platformLog("Going through the list of records until the last one\r\n");
        MY_ASSERT(ndefHeaderTNF(record) == NDEF_TNF_UNCHANGED, ERR_SYNTAX);

        record = record->next;
    }

    /*    R8. For each middle record chunk, the payloadLength field indicates the size of the data carried in the PAYLOAD field of this single record only. */

    /*    R9. The terminating record chunk is an NDEF record with the CF flag cleared. */
    /* Same as R1 ? */

    /*    R10. For the terminating record chunk, the value of the typeLength and the IL fields MUST be 0. */
    /* Same as R6 ? */

    /*    R11. For the terminating record chunk, the TNF (Type Name Format) field value MUST be 0x06 (Unchanged). */
    /* Same as R7 ? */

    /*    R12. For the terminating record chunk, the payloadLength field indicates the size of the data carried in the PAYLOAD field of this record only. */

    /*    R13. A chunked payload MUST be entirely encapsulated within a single NDEF message. */

    /*    R14. An initial record chunk MUST NOT have the ME (Message End) flag set. */
    record = message->record;
    MY_ASSERT(ndefHeaderME(record) == 0U, ERR_SYNTAX);

    /*     R15. A middle record chunk MUST NOT have the ME (Message End) flag set. */
    record = message->record;
    record = record->next;
    while (record != NULL)
    {
        platformLog("Going through the list of records until the last one\r\n");
        record = record->next;
    }
    if (record != message->record)
    {
        MY_ASSERT(ndefHeaderME(record) == 0U, ERR_SYNTAX);
    }

    /*
    NDEF payload requirements
    -------------------------

    P1. The payloadLength field is four octets for normal records.
    P2. The payloadLength field is one octet for records with an SR (Short Record) bit flag value of 1.
    P3. The payloadLength field of a short record MUST have a value between 0 and 255.
    P4. The payloadLength field of a normal record MUST have a value between 0 and 2^32-1. */

    return ERR_NONE;
}

/*****************************************************************************/
ReturnCode ndefMessageTestRequirements(const ndefMessage* message)
{
    ndefRecord* record = NULL;

    if (message == NULL)
    {
        return ERR_NOMSG;
    }

    /*
      3.3 The NDEF Specification Test Requirements
      ============================================

    Data transmission order requirements
    ------------------------------------
    Quantities are transmitted in a big-endian manner with the most significant octet transmitted first.

    Record layout requirements
    -------------------------- */
    record = message->record;
    while (record != NULL)
    {
        /*    R1. NDEF parsers MUST accept both normal and short record layouts. */
        CHECK_RANGE(ndefHeaderSR(record), 0, 1);

        /*    R2. NDEF parsers MUST accept single NDEF messages composed of both normal and short records. */

        /*    R3. If the IL flag is 1, the idLength field MUST be present. */
        if (ndefHeaderIL(record) == 1U)
        {
            /* MY_ASSERT(record->idLength != NULL, ERR_SYNTAX);*/
        }

        /*    R4. If the IL flag is 0, the idLength field MUST NOT be present. */
        /*    R5. If the IL flag is 0, the ID field MUST NOT be present. */
        /*if (record->IL == 0U)
        {
            MY_ASSERT(record->idLength == NULL, ERR_SYNTAX);
            MY_ASSERT(record->id        == 0U,    ERR_SYNTAX);
        }*/

        /*     R6. The TNF field MUST have a value between 0x00 and 0x06. */
        CHECK_RANGE(ndefHeaderTNF(record), NDEF_TNF_EMPTY, NDEF_TNF_RESERVED);

        /*    R7. If the TNF value is 0x00, the typeLength, idLength, and payloadLength fields MUST be zero
                  and the TYPE, ID, and PAYLOAD fields MUST be omitted from the record. */
        if (ndefHeaderTNF(record) == 0U)
        {
            MY_ASSERT(record->typeLength    == 0U,    ERR_SYNTAX);

            /*MY_ASSERT(record->idLength      != NULL, ERR_SYNTAX);*/
            /*MY_ASSERT(*record->idLength     == 0U,    ERR_SYNTAX);*/

            /*MY_ASSERT(record->payloadLength   != NULL, ERR_SYNTAX);*/
            MY_ASSERT(record->bufPayload.length == 0U,    ERR_SYNTAX);

            MY_ASSERT(record->bufPayload.buffer == NULL, ERR_SYNTAX);
        }

        /*    R8. If the TNF value is 0x05 (Unknown), the typeLength field MUST be 0 and the TYPE field MUST be omitted from the NDEF record-> */
        if (ndefHeaderTNF(record) == 0x05)
        {
            MY_ASSERT(record->typeLength == 0U, ERR_SYNTAX);
            /*MY_ASSERT(record->type        == 0U, ERR_SYNTAX);*/
        }

        /*    R9. If the TNF value is 0x06 (Unchanged), the typeLength field MUST be 0 and the TYPE field MUST be omitted from the NDEF record-> */
        if (ndefHeaderTNF(record) == 0x06)
        {
            MY_ASSERT(record->typeLength == 0U, ERR_SYNTAX);
            /*MY_ASSERT(record->type        == 0U, ERR_SYNTAX);*/
        }

        /*    R10. The TNF value MUST NOT be 0x07. */
        MY_ASSERT(ndefHeaderTNF(record) != 0x07U, ERR_SYNTAX);

        /*    R11. If the idLength field has a value 0, the ID field MUST NOT be present. */
        /*if ( (record->idLength != NULL) && (*record->idLength == 0U) )*/
        /*if (record->idLength == 0U)
        {
            MY_ASSERT(record->id == 0U, ERR_SYNTAX);
        }*/

        /*    R12. If the SR flag is 0, the payloadLength field is four octets, representing a 32-bit unsigned integer, and the transmission order of the octets is MSB-first. */
    
        /*    R13. If the SR flag is 1, the payloadLength field is a single octet representing an 8-bit unsigned integer. */
    
        /*    R14. If the payloadLength field value is 0, the PAYLOAD field MUST NOT be present. */
        if (ndefRecordGetPayloadLength(record) == 0U)
        {
            MY_ASSERT(record->bufPayload.buffer == NULL, ERR_SYNTAX);
        }
    
        /*    R15. The value of the TYPE field MUST follow the structure, encoding, and format implied by the value of the TNF field. */
        switch (ndefHeaderTNF(record))
        {
        case NDEF_TNF_EMPTY:
            break;
        case NDEF_TNF_RTD_WELL_KNOWN_TYPE:
            break;
        case NDEF_TNF_MEDIA_TYPE:
            break;
        case NDEF_TNF_URI:
            break;
        case NDEF_TNF_RTD_EXTERNAL_TYPE:
            break;
        case NDEF_TNF_UNKNOWN:
            break;
        case NDEF_TNF_UNCHANGED:
            break;
        case NDEF_TNF_RESERVED: /* fall through */
        default:
            MY_ASSERT(ndefHeaderTNF(record) < NDEF_TNF_RESERVED, ERR_SYNTAX);
            break;
        }

        record = record->next;
    }

    record = message->record;
    record = record->next;
    while (record != NULL)
    {
        /*    R16. Middle and terminating record chunks MUST NOT have an ID field. */
        platformLog("Going through the list of records until the last one\r\n");
        /*MY_ASSERT(record->id == 0U, ERR_SYNTAX);*/

        record = record->next;
    }

    return ERR_NONE;
}


/*****************************************************************************/
/*
 * Test different empty messages (no message, no record)
 */
ReturnCode ndefTest_RecordEmpty_0(void)
{
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* No message */
    ndefMessage* message = NULL;
    ndefMessageDump(message, true);

    /* Empty message */
    message = malloc(sizeof(ndefMessage));
    ndefRecord* record = NULL;
    message->record = record;
    ndefMessageDump(message, true);

    free(message);

    return ERR_NONE;
}


/*****************************************************************************/
/*
 * Test empty message, empty records
 */
ReturnCode ndefTest_RecordEmpty_1(void)
{
    ReturnCode err = ERR_NONE;
    ndefRecord  record;
    ndefMessage message;

    platformLog("Running %s...\r\n", __FUNCTION__);

    err = ndefRecordReset(&record);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record, true);

    ndefRecord record0 = {
        .header         = ndefHeader(0U, 0U, 0U, 1U, 0U, NDEF_TNF_EMPTY), /* Set SR bit */
        .typeLength     = 0,
        .idLength       = 0,
        .type           = NULL,
        .id             = NULL,
        .bufPayload     = { NULL, 0 },
        .next = NULL
    };

    ndefRecordDump(&record0, true);

    if (ndefRecordMatch(&record, &record0) == false)
    {
        return ERR_SYNTAX;
    }

    ndefRecord recordCopy;
    err = ndefRecordCopy(&record, &recordCopy);
    if (err != ERR_NONE)
    {
        return err;
    }

    if (ndefRecordMatch(&record, &recordCopy) == false)
    {
        return ERR_SYNTAX;
    }

    err = ndefMessageInit(&message);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefMessageAppend(&message, &record);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefMessageDump(&message, true);

    err = ndefMessageAppend(&message, &record0);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefMessageDump(&message, true);

    return err;
}


/*****************************************************************************/
/*
 * Test message with empty records and record count
 */
ReturnCode ndefTest_RecordEmpty_2(void)
{
    ReturnCode err = ERR_NONE;
    ndefRecord  record;
    ndefMessage message;
    ndefMessageInfo info;
    uint32_t recordCount;

    platformLog("Running %s...\r\n", __FUNCTION__);

    err = ndefRecordReset(&record);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecord record0 = {
        .header         = ndefHeader(0U, 0U, 0U, 1U, 0U, NDEF_TNF_EMPTY), /* Set SR bit */
        .typeLength     = 0,
        .idLength       = 0,
        .type           = NULL,
        .id             = NULL,
        .bufPayload     = { NULL, 0 },
        .next = NULL
    };

    if (ndefRecordMatch(&record, &record0) == false)
    {
        return ERR_SYNTAX;
    }

    ndefRecord recordCopy;
    err = ndefRecordCopy(&record, &recordCopy);
    if (err != ERR_NONE)
    {
        return err;
    }

    if (ndefRecordMatch(&record, &recordCopy) == false)
    {
        return ERR_INTERNAL;
    }

    err = ndefMessageInit(&message);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* testing 0 record */
    err = ndefMessageGetInfo(&message, &info);
    if (err != ERR_NONE)
    {
        return err;
    }
    recordCount = info.recordCount;
    if (recordCount != 0U)
    {
        return ERR_INTERNAL;
    }

    recordCount = ndefMessageGetRecordCount(&message);
    if (recordCount != 0U)
    {
        return ERR_INTERNAL;
    }

    /* testing 1 record */
    err = ndefMessageAppend(&message, &record);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefMessageGetInfo(&message, &info);
    if (err != ERR_NONE)
    {
        return err;
    }
    recordCount = info.recordCount;
    if (recordCount != 1)
    {
        return ERR_INTERNAL;
    }

    recordCount = ndefMessageGetRecordCount(&message);
    if (recordCount != 1)
    {
        return ERR_INTERNAL;
    }

    /* testing 2 record2 */
    err = ndefMessageAppend(&message, &record0);
    if (err != ERR_NONE)
    {
        return err;
    }
    err = ndefMessageGetInfo(&message, &info);
    if (err != ERR_NONE)
    {
        return err;
    }
    recordCount = info.recordCount;
    if (recordCount != 2)
    {
        return ERR_INTERNAL;
    }

    recordCount = ndefMessageGetRecordCount(&message);
    if (recordCount != 2)
    {
        return ERR_INTERNAL;
    }

    return err;
}


/*****************************************************************************/
/*
 * Test message with empty records and utils
 */
ReturnCode ndefTest_RecordEmpty_3(void)
{
    ReturnCode err = ERR_NONE;
    ndefRecord  record;

    uint8_t tnf;
    uint8_t* payload;
    const uint8_t* id;
    const uint8_t* type;
    uint32_t payloadLength;
    uint8_t length;

    platformLog("Running %s...\r\n", __FUNCTION__);

    err = ndefRecordReset(&record);
    if (err != ERR_NONE)
    {
        return err;
    }

    length = ndefRecordGetLength(&record);
    if (length < NDEF_MIN_RECORD_LEN)
    {
        return ERR_INTERNAL;
    }

    /* Test payload */
    length = ndefRecordGetPayloadLength(&record);
    if (length != 0U)
    {
        return ERR_INTERNAL;
    }

    payloadLength = sizeof(payload);
    ndefConstBuffer bufPayload = { payload, payloadLength };
    err = ndefRecordGetPayload(&record, &bufPayload);
    if (err != ERR_NONE)
    {
        return err;
    }
    if (bufPayload.length != 0U)
    {
        return ERR_INTERNAL;
    }

    /* Test Id */
    length = sizeof(id);
    ndefConstBuffer8 bufId = { id, length };
    err = ndefRecordGetId(&record, &bufId);
    if (err != ERR_NONE)
    {
        return err;
    }
    if (bufId.length != 0U)
    {
        return ERR_INTERNAL;
    }

    /* Test type */
    length = sizeof(type);
    ndefConstBuffer8 bufType = { type, length };
    err = ndefRecordGetType(&record, &tnf, &bufType);
    if (err != ERR_NONE)
    {
        return err;
    }
    if (tnf != NDEF_TNF_EMPTY)
    {
        return ERR_INTERNAL;
    }
    if (bufType.length != 0U)
    {
        return ERR_INTERNAL;
    }

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_RecordEmpty_4(void)
{
    ReturnCode err = ERR_NONE;
    ndefRecord record;
    ndefRecord record2;
    ndefType   empty;
    uint8_t buffer[32];
    ndefBuffer bufPayload = { buffer, sizeof(buffer) };

    platformLog("Running %s...\r\n", __FUNCTION__);

    err = ndefRecordReset(&record);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefRecordDump(&record, true);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefRecordToType(&record, &empty);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefEmptyTypeDump(&empty);
    if (err != ERR_NONE)
    {
        return err;
    }


    err = ndefEmptyType(&empty);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefTypeToRecord(&empty, &record2);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefRecordDump(&record2, true);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefRecordEncode(&record, &bufPayload);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefBufferDump("Encoded", (ndefConstBuffer*)&bufPayload, true);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Reset buffer length */
    bufPayload.length = sizeof(buffer);
    err = ndefRecordEncode(&record2, &bufPayload);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefBufferDump("Encoded2", (ndefConstBuffer*)&bufPayload, true);
    if (err != ERR_NONE)
    {
        return err;
    }

    return err;
}


/*****************************************************************************/
/*
 * Test message with empty records and utils
 */
ReturnCode ndefTest_RecordEncodeDecode_1(void)
{
    ReturnCode err = ERR_NONE;
    ndefRecord record;
    ndefRecord record1;
    uint8_t buffer[50];
    uint32_t buffer_length;
    uint32_t length;

    platformLog("Running %s...\r\n", __FUNCTION__);

    err = ndefRecordReset(&record);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Encode */
    /* Retrieve the length */
    length = 0;
    ndefBuffer bufPayload = { buffer, length };
    err = ndefRecordEncode(&record, &bufPayload);
    if (err != ERR_NOMEM)
    {
        return err;
    }
    if ( (bufPayload.length == 0U) || (bufPayload.length < NDEF_MIN_RECORD_LEN) )
    {
        return ERR_INTERNAL;
    }
    if (bufPayload.length != ndefRecordGetLength(&record))
    {
        return ERR_INTERNAL;
    }

    buffer_length = sizeof(buffer);
    length        = buffer_length;
    bufPayload.buffer = buffer;
    bufPayload.length = length;
    err = ndefRecordEncode(&record, &bufPayload);
    if (err != ERR_NONE)
    {
        return err;
    }
    if (bufPayload.length == buffer_length)
    {
        return ERR_INTERNAL;
    }
    if (bufPayload.length != ndefRecordGetLength(&record))
    {
        return ERR_INTERNAL;
    }

    /* Decode */
    err = ndefRecordDecode((ndefConstBuffer*)&bufPayload, &record1);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Compare */
    if (ndefRecordMatch(&record, &record1) == false)
    {
        return ERR_INTERNAL;
    }

    return err;
}


/*****************************************************************************/
/* 
 * Create a message with a single record
 *  convert it back and forth to raw buffer
 *  compare messages and raw buffers
 */
ReturnCode ndefTest_Message1record(void)
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    uint8_t payload0[] = { 0x01, 0x02, 0x03, 0x04 };
    ndefRecord  record0;

    ndefMessage message;

    err = ndefMessageInit(&message);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefConstBuffer8 type = { NULL, 0 };
    ndefConstBuffer8 id   = { NULL, 0 };
    ndefConstBuffer payload = { payload0, sizeof(payload0) };
    err = ndefRecordInit(&record0, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &type, &id, &payload);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefMessageAppend(&message, &record0);
    if (err != ERR_NONE)
    {
        return err;
    }
    ndefMessageDump(&message, true);

    /* Convert message to raw buffer */
    uint8_t  buffer[50];
    uint32_t bufferLength = sizeof(buffer);
    ndefBuffer bufPayload = { buffer, bufferLength };
    err = ndefMessageEncode(&message, &bufPayload);
    if (err != ERR_NONE)
    {
        return err;
    }
    ndefBufferDump("Converted to:\r\n", (ndefConstBuffer*)&bufPayload, true);

    /* Convert back to message */
    ndefMessage message2;

    err = ndefMessageDecode((ndefConstBuffer*)&bufPayload, &message2);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefMessageDump(&message2, true);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Compare NDEF messages */
    if (ndefMessageMatch(&message, &message2) == false)
    {
        return ERR_INTERNAL;
    }

    /* Compare raw buffers */
    uint8_t  buffer2[50];
    uint32_t buffer2Length = sizeof(buffer2);
    ndefBuffer bufPayload2 = { buffer2, buffer2Length };
    err = ndefMessageEncode(&message2, &bufPayload2);
    if (err != ERR_NONE)
    {
        return err;
    }

    int diff = ST_BYTECMP(bufPayload.buffer, bufPayload2.buffer, bufPayload.length);
    if (diff != 0U)
    {
        return ERR_INTERNAL;
    }

    return err;
}


/*****************************************************************************/
/*
 * Message with 2 records
 */
ReturnCode ndefTest_Message2records_1(void)
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    uint8_t id0 = 3;
    uint8_t payload0[] = { 0x00 };
    ndefRecord  record0 = {
        .header         = ndefHeader(1U, 0U, 1U, 1U, 1U, NDEF_TNF_RTD_WELL_KNOWN_TYPE),
        .typeLength     = 1,
        .idLength       = 1,
        .type           = NULL,
        .id             = &id0,
        .bufPayload     = { payload0, 1 },
        .next = NULL
    };

    uint8_t id1 = 4;
    uint8_t payload1[] = { 0x02, 0x03, 0x04, 0x05 };
    ndefRecord record1 = {
        .header         = ndefHeader(1U, 1U, 0U, 1U, 1U, NDEF_TNF_RTD_WELL_KNOWN_TYPE),
        .typeLength     = 0,
        .idLength       = 1,
        .type           = NULL,
        .id             = &id1,
        .bufPayload     = { payload1, 4 },
        .next = NULL
    };

    ndefMessage message;

    /* Create a message with 2 records */
    err = ndefMessageInit(&message);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefMessageAppend(&message, &record0);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefMessageAppend(&message, &record1);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefMessageDump(&message, true);

    /* Convert to buffer */
    uint8_t  buffer[50];
    uint32_t bufferLength = sizeof(buffer);
    ndefBuffer bufPayload = { buffer, bufferLength };
    err = ndefMessageEncode(&message, &bufPayload);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefBufferDump("Converted to:\r\n", (ndefConstBuffer*)&bufPayload, true);

    err = ndefMessageDump(&message, true);
    if (err != ERR_NONE)
    {
        return err;
    }

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_Message2records_2(void)
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    ndefMessage message;

    uint8_t id0 = 3;
    uint8_t payload0[] = { 0x00 };
    ndefRecord  record0 = {
        .header         = ndefHeader(1U, 0U, 1U, 1U, 1U, NDEF_TNF_RTD_WELL_KNOWN_TYPE),
        .typeLength     = 1,
        .idLength       = 1,
        .type           = NULL,
        .id             = &id0,
        .bufPayload     = { payload0, 1 },
        .next = NULL
    };

    uint8_t id1 = 4;
    uint8_t payload1[] = { 0x02, 0x03, 0x04, 0x05 };
    ndefRecord record1 = {
        .header         = ndefHeader(1U, 1U, 0U, 1U, 1U, NDEF_TNF_MEDIA_TYPE),
        .typeLength     = 2,
        .idLength       = 1,
        .type           = NULL,
        .id             = &id1,
        .bufPayload     = { payload1, 4 },
        .next = NULL
    };

    ndefRecord  recordMatch;
    //ndefRecord* recordOut;

    /* Create a message with 2 records */
    err  = ndefMessageInit(&message);
    err |= ndefMessageAppend(&message, &record0);
    if (err != ERR_NONE)
    {
        return err;
    }
    err = ndefMessageAppend(&message, &record1);
    if (err != ERR_NONE)
    {
        return err;
    }
    ndefMessageDump(&message, true);

    /* Create a record with specific parameters to seek */
    ndefRecordReset(&recordMatch);
    ndefHeaderSetTNF(&recordMatch, NDEF_TNF_MEDIA_TYPE);
    recordMatch.typeLength = 2;

    //*recordOut = 0;
    //err = ndefMessageSeekRecord(&message, &recordMatch, &recordOut);
    //if (err != ERR_NONE)
    //{
    //    return err;
    //}
    //
    //ndefRecordDump(recordOut, true);

    ///* Scan the list of records */
    //platformLog("Scan the list of records\r\n");
    //ndefRecord* recordScan;
    //
    //recordScan = NULL;
    //do
    //{
    //    err = ndefMessageBrowse(&message, &recordScan);
    //    if (err != ERR_NONE)
    //    {
    //        return err;
    //    }
    //    ndefRecordDump(recordScan, true);
    //} while (recordScan != NULL);

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_Message3records()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    ndefMessage message;
    ndefRecord record1, record2, record3;

    err  = ndefMessageInit(&message);
    err |= ndefRecordReset(&record1);
    err |= ndefRecordReset(&record2);
    err |= ndefRecordReset(&record3);
    err |= ndefMessageAppend(&message, &record1);
    if (err != ERR_NONE)
    {
        return err;
    }

    if ( (ndefHeaderMB(&record1) != 1) || (ndefHeaderME(&record1) != 1) )
    {
        return ERR_INTERNAL;
    }

    err = ndefMessageAppend(&message, &record2);
    if (err != ERR_NONE)
    {
        return err;
    }

    if ( (ndefHeaderMB(&record1) != 1) || (ndefHeaderME(&record1) != 0) ||
         (ndefHeaderMB(&record2) != 0) || (ndefHeaderME(&record2) != 1) )
    {
        return ERR_INTERNAL;
    }

    err = ndefMessageAppend(&message, &record3);
    if (err != ERR_NONE)
    {
        return err;
    }

    if ( (ndefHeaderMB(&record1) != 1) || (ndefHeaderME(&record1) != 0) ||
         (ndefHeaderMB(&record2) != 0) || (ndefHeaderME(&record2) != 0) ||
         (ndefHeaderMB(&record3) != 0) || (ndefHeaderME(&record3) != 1) )
    {
        return ERR_INTERNAL;
    }

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_PlainText(void)
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    ndefMessage message;

    /* Plain text: */
    /* Payload: 0x 00 0f 20 00 3b 00 34 04 06 e1 04 10 00 00 00 90 00 */
    uint8_t payload[] = { ndefHeader(0U, 0U, 0U, 1U, 0U, NDEF_TNF_RTD_WELL_KNOWN_TYPE), /* Set SR bit */
                          1,  /* Type length */
                          8,  /* Payload length */
                              /* No Id length */
                         'T', /* Type */
                              /* No Id */
                          0x02, 'e', 'n', 'H', 'e', 'l', 'l', 'o'  /* Payload : Status + language code + sentence */
    };

    ndefConstBuffer bufPayload = { payload, sizeof(payload) };
    ndefBufferDump("Got\r\n", (ndefConstBuffer*)&bufPayload, true);
    err = ndefMessageDecode(&bufPayload, &message);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefMessageDump(&message, true);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefMessageConformance(&message);

    ndefMessageTestRequirements(&message);

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_rtd_type_deviceinfo_1()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    const uint8_t buffer[] = {
                         0xD1, /* MB=1 ME=1 SR=1 IL=0 TNF=1 */
                         0x02, 0x28, /* Type Length, Payload Length */
                         0x44, 0x69, /* Type "Di" */
                         0x01, /* Device Info TLV #1 Type: Model Name */
                         0x0F, /* Device Info TLV #1 Length: 15 octets */
                         0x45, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65, 0x4A,
                         0x65, 0x74, 0x20, 0x38, 0x38, 0x38, 0x38, /* Device Info TLV #1 Value: "ExampleJet 888" */
                         0x02, /* Device Info TLV #2 Type: Device Unique Name */
                         0x0F, /* Device Info TLV #2 Length: 15 octets */
                         0x6D, 0x6F, 0x20, 0x67, 0x6C, 0xC3, 0xA9, 0x61,
                         0x73, 0x20, 0x6E, 0x69, 0x66, 0x74, 0x79, /* Device Info TLV #2 Value: "mo gléas nifty" ("my nifty device" in Irish)*/
                         0x00, /* Device Info TLV #3 Type: Manufacturer Name */
                         0x04, /* Device Info TLV #3 Length: 4 octets */
                         0x41, 0x63, 0x6D, 0x65 /* Device Info TLV #3 Value: "Acme" */
                         };

    ndefRecord record;
    ndefConstBuffer bufPayload = { buffer, sizeof(buffer) };
    err = ndefRecordDecode(&bufPayload, &record);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefRecordDump(&record, true);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Convert record to Device Information */
    ndefType devInfo;
    err = ndefRecordToType(&record, &devInfo);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefRecordDump(&record, false);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record, true);

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_rtd_type_deviceinfo_2()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    const uint8_t buffer[] = {
                         0x11, /* MB=0 ME=0 SR=1 IL=0 TNF=1 */
                         0x02, 0x28, /* Type Length, Payload Length */
                         0x44, 0x69, /* Type "Di" */
                         0x01, /* Device Info TLV #1 Type: Model Name */
                         0x0F, /* Device Info TLV #1 Length: 15 octets */
                         0x45, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65, 0x4A,
                         0x65, 0x74, 0x20, 0x38, 0x38, 0x38, 0x38, /* Device Info TLV #1 Value: "ExampleJet 888" */
                         0x02, /* Device Info TLV #2 Type: Device Unique Name */
                         0x0F, /* Device Info TLV #2 Length: 15 octets */
                         0x6D, 0x6F, 0x20, 0x67, 0x6C, 0xC3, 0xA9, 0x61,
                         0x73, 0x20, 0x6E, 0x69, 0x66, 0x74, 0x79, /* Device Info TLV #2 Value: "mo gléas nifty" ("my nifty device" in Irish)*/
                         0x00, /* Device Info TLV #3 Type: Manufacturer Name */
                         0x04, /* Device Info TLV #3 Length: 4 octets */
                         0x41, 0x63, 0x6D, 0x65 /* Device Info TLV #3 Value: "Acme" */
                         };

    ndefRecord record1;
    ndefConstBuffer bufPayload = { buffer, sizeof(buffer) };
    err = ndefRecordDecode(&bufPayload, &record1);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record1, true);

    static const uint8_t manufacturerName[] = "Acme";
    static const uint8_t modelName[]        = "ExampleJet 8888";
    static const uint8_t deviceUniqueName[] = "mo glé_as nifty";
    ndefDeviceInfoEntry deviceInfoData[] = {
        NDEF_DEVICE_INFO_MANUFACTURER_NAME,  strlen((char*)manufacturerName), manufacturerName,
        NDEF_DEVICE_INFO_MODEL_NAME,         strlen((char*)modelName)       , modelName       ,
        NDEF_DEVICE_INFO_DEVICE_UNIQUE_NAME, strlen((char*)deviceUniqueName), deviceUniqueName
    };

    ndefType deviceInfo;
    err = ndefRtdDeviceInfo(&deviceInfo, deviceInfoData, SIZEOF_ARRAY(deviceInfoData));
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Convert Device Information to record */
    ndefRecord record2;
    err = ndefTypeToRecord(&deviceInfo, &record2);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefRtdDeviceInfoToRecord(&deviceInfo, &record2);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record2, true);

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_rtd_type_deviceinfo_3()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    /* Invalid UUID length */
    static const uint8_t manufacturerName[] = "Acme";
    static const uint8_t modelName[]        = "ExampleJet 8888";
    static const uint8_t deviceUniqueName[] = "mo glé_as nifty";
    static const uint8_t UUID[]             = "0102030405060708090A0B0C0D0E0F00";
    static const uint8_t firmwareVersion[]  = "Version alpha 0.1";
    ndefDeviceInfoEntry deviceInfoData[] = {
        NDEF_DEVICE_INFO_MANUFACTURER_NAME,  strlen((char*)manufacturerName), manufacturerName,
        NDEF_DEVICE_INFO_MODEL_NAME,         strlen((char*)modelName)       , modelName       ,
        NDEF_DEVICE_INFO_DEVICE_UNIQUE_NAME, strlen((char*)deviceUniqueName), deviceUniqueName,
        NDEF_DEVICE_INFO_UUID,               5 /* Invalid UUID length*/     , UUID            ,
        NDEF_DEVICE_INFO_FIRMWARE_VERSION,   strlen((char*)firmwareVersion) , firmwareVersion
    };

    ndefType deviceInfo;
    err = ndefRtdDeviceInfo(&deviceInfo, deviceInfoData, SIZEOF_ARRAY(deviceInfoData));
    if (err == ERR_NONE)
    {
        return ERR_INTERNAL;
    }

    if ( (((ndefTypeRtdDeviceInfo*)&deviceInfo.data)->devInfo[NDEF_DEVICE_INFO_UUID].buffer != NULL) || (((ndefTypeRtdDeviceInfo*)&deviceInfo.data)->devInfo[NDEF_DEVICE_INFO_UUID].length != 0) )
    {
        return ERR_INTERNAL;
    }

    /* Convert Device Information to record */
    ndefRecord record;
    err = ndefRtdDeviceInfoToRecord(&deviceInfo, &record);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record, true);

    /* Valid UUID */
    ndefDeviceInfoEntry deviceInfoData2[] = {
        NDEF_DEVICE_INFO_MANUFACTURER_NAME, sizeof(manufacturerName) - 1U, manufacturerName,
        NDEF_DEVICE_INFO_MODEL_NAME       , sizeof(modelName) - 1U       , modelName       ,
        NDEF_DEVICE_INFO_DEVICE_UNIQUE_NAME, sizeof(deviceUniqueName) - 1U, deviceUniqueName,
        NDEF_DEVICE_INFO_UUID            , NDEF_UUID_LENGTH             , UUID            ,
        NDEF_DEVICE_INFO_FIRMWARE_VERSION , sizeof(firmwareVersion) - 1U , firmwareVersion
    };

    err = ndefRtdDeviceInfo(&deviceInfo, deviceInfoData2, SIZEOF_ARRAY(deviceInfoData2));
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Convert Device Information to record */
    ndefRecord record2;
    err = ndefRtdDeviceInfoToRecord(&deviceInfo, &record2);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record2, false);

    return err;
}

/*****************************************************************************/
ReturnCode ndefTest_rtd_type_deviceinfo_4()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    const uint8_t buffer[] = {
                         0x11, /* MB=0 ME=0 SR=1 IL=0 TNF=1 */
                         0x02, 0x28, /* Type Length, Payload Length */
                         0x44, 0x69, /* Type "Di" */
                         0x01, /* Device Info TLV #1 Type: Model Name */
                         0x0F, /* Device Info TLV #1 Length: 15 octets */
                         0x45, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65, 0x4A,
                         0x65, 0x74, 0x20, 0x38, 0x38, 0x38, 0x38, /* Device Info TLV #1 Value: "ExampleJet 888" */
                         0x02, /* Device Info TLV #2 Type: Device Unique Name */
                         0x0F, /* Device Info TLV #2 Length: 15 octets */
                         0x6D, 0x6F, 0x20, 0x67, 0x6C, 0xC3, 0xA9, 0x61,
                         0x73, 0x20, 0x6E, 0x69, 0x66, 0x74, 0x79, /* Device Info TLV #2 Value: "mo gléas nifty" ("my nifty device" in Irish)*/
                         0x00, /* Device Info TLV #3 Type: Manufacturer Name */
                         0x04, /* Device Info TLV #3 Length: 4 octets */
                         0x41, 0x63, 0x6D, 0x65 /* Device Info TLV #3 Value: "Acme" */
                         };

    ndefRecord record1;
    ndefConstBuffer bufPayload = { buffer, sizeof(buffer) };
    err = ndefRecordDecode(&bufPayload, &record1);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record1, true);

    uint8_t workingBuffer[48];
    ndefBuffer bufPayload2 = { workingBuffer, sizeof(workingBuffer) };
    err = ndefRecordEncode(&record1, &bufPayload2);
    if (err != ERR_NONE)
    {
        return err;
    }

    if (ST_BYTECMP(buffer, bufPayload2.buffer, bufPayload2.length) != 0)
    {
        return ERR_INTERNAL;
    }

    /* */
    static const uint8_t manufacturerName[] = "Acme";
    static const uint8_t modelName[]        = "ExampleJet 8888";
    static const uint8_t deviceUniqueName[] = "mo glé_as nifty";
    ndefDeviceInfoEntry deviceInfoData[] = {
        NDEF_DEVICE_INFO_MANUFACTURER_NAME, sizeof(manufacturerName) - 1U, manufacturerName,
        NDEF_DEVICE_INFO_MODEL_NAME       , sizeof(modelName) - 1U       , modelName       ,
        NDEF_DEVICE_INFO_DEVICE_UNIQUE_NAME, sizeof(deviceUniqueName) - 1U, deviceUniqueName,
    };

    ndefType deviceInfo;
    err = ndefRtdDeviceInfo(&deviceInfo, deviceInfoData, SIZEOF_ARRAY(deviceInfoData));
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Convert Device Information to record */
    ndefRecord record2;
    err = ndefRtdDeviceInfoToRecord(&deviceInfo, &record2);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record2, true);

    uint8_t workingBuffer2[48];
    ndefBuffer bufRecord2 = { workingBuffer2, sizeof(workingBuffer2) };
    err = ndefRecordEncode(&record2, &bufRecord2);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Not in the same order
    if (ST_BYTECMP(buffer, bufRecord2.buffer, bufRecord2.length) != 0)
    {
        return ERR_INTERNAL;
    }*/

    return err;
}


// miss  ndefTypeToRecord then ndefRecordToType()


/*****************************************************************************/
ReturnCode ndefTest_rtd_type_text_1()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    const uint8_t buffer[] = { 0x91 /* SR=1 IL=0 TNF=Text */, 0x01, 0x10, 0x54, 0x02, 0x65, 0x6E,
                         0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x2C, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64, 0x21 }; /* "Hello, world!" */

    ndefRecord record;
    ndefConstBuffer bufPayload = { buffer, sizeof(buffer) };
    err = ndefRecordDecode(&bufPayload, &record);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefRecordDump(&record, true);

    ndefMessage message;
    err  = ndefMessageInit(&message);
    err |= ndefMessageDecode(&bufPayload, &message);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefMessageInfo info;
    err  = ndefMessageGetInfo(&message, &info);
    err |= ndefMessageAppend(&message, &record);
    err |= ndefMessageGetInfo(&message, &info);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Convert record to text */
    ndefType text;
    err = ndefRecordToRtdText(&record, &text);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefMessageDump(&message, true);

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_rtd_type_text_2()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    const uint8_t buffer[] = { 0x11 /* SR=1 IL=0 TNF=Text */, 0x01, 0x10, 0x54, 0x02, 0x65, 0x6E,
                         0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x2C, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64, 0x21 }; /* "Hello, world!" */

    ndefRecord record1;
    ndefConstBuffer bufPayload = { buffer, sizeof(buffer) };
    err = ndefRecordDecode(&bufPayload, &record1);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record1, true);

    /* 1. Convert record to text using direct function */
    ndefType text1;
    err = ndefRecordToRtdText(&record1, &text1);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* 2. Convert record to text using record type detection and pointer of function */
    ndefType text2;
    err = ndefRecordToType(&record1, &text2);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* 3. compare the 2 texts */
    /*if (ST_BYTECMP(&text1, &text2, sizeof(text1)) != 0)
    {
        return ERR_INTERNAL;
    }*/

    /* Convert text to record */
    ndefRecord record2;
    err = ndefTypeToRecord(&text2, &record2);
    if (err != ERR_NONE)
    {
        return err;
    }

    uint8_t workingBuffer[32];
    ndefBuffer bufRecord = { workingBuffer, sizeof(workingBuffer) };
    err = ndefRecordEncode(&record2, &bufRecord);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record2, false);

    /* Payload buffer doesn't match, since no payload for record2
    if (! ndefRecordMatch(&record1, &record2))
    {
        return ERR_INTERNAL;
    }*/

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_rtd_type_text_3()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    const uint8_t buffer[] = { 0x11 /* SR=1 IL=0 TNF=Text */, 0x01, 0x10, 0x54, 0x02, 0x65, 0x6E,
                         0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x2C, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64, 0x21 }; /* "Hello, world!" */

    ndefRecord record1;
    ndefConstBuffer bufPayload = { buffer, sizeof(buffer) };
    err = ndefRecordDecode(&bufPayload, &record1);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record1, true);

    ndefType text2;
    const uint8_t language[] = "en";
    const uint8_t message[]  = "Hello, world!";
    ndefConstBuffer8 bufLanguage = { language, sizeof(language) - 1U };
    ndefConstBuffer  bufMessage  = { message,  sizeof(message) - 1U  };
    err = ndefRtdText(&text2, TEXT_ENCODING_UTF8, &bufLanguage, &bufMessage);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Convert text to record */
    ndefRecord record2;
    //err = ndefTypeToRecord(NDEF_RTD_TEXT, &text2, &record2);
    err = ndefRtdTextToRecord(&text2, &record2);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record2, false);

    /*if (! ndefRecordMatch(&record1, &record2))
    {
        return ERR_INTERNAL;
    }*/

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_rtd_type_uri_1()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    // Simple URL with No Substitution
    const uint8_t buffer[] = { 0x11 /* SR=1 TNF=Text */, 0x01, 0x08, 0x55, 0x01,
                               0x6E, 0x66, 0x63, 0x2E, 0x63, 0x6F, 0x6D }; /* "nfc.com" */

    ndefRecord record1;
    ndefConstBuffer bufPayload = { buffer, sizeof(buffer) };
    err = ndefRecordDecode(&bufPayload, &record1);
    if (err != ERR_NONE)
    {
        return err;
    }
    ndefRecordDump(&record1, true);

    ndefType uri1;
    err = ndefRecordToType(&record1, &uri1);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Convert URI to record */
    ndefRecord record2;
    err = ndefTypeToRecord(&uri1, &record2);
    if (err != ERR_NONE)
    {
        return err;
    }
    ndefRecordDump(&record2, false);

    /* Payload buffer doesn't match, since no payload for record2
    if (! ndefRecordMatch(&record1, &record2))
    {
        return ERR_INTERNAL;
    }*/

    return err;
}

// add a URI test calling ndefRecordEncode()

/*****************************************************************************/
ReturnCode ndefTest_rtd_type_uri_2()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    // Simple URL with No Substitution
    const uint8_t buffer[] = { 0x11 /* MB=0, SR=1 TNF=Text, ME=0 */, 0x01, 0x08, 0x55, 0x01,
                               0x6E, 0x66, 0x63, 0x2E, 0x63, 0x6F, 0x6D }; /* "nfc.com" */

    ndefRecord record1;
    ndefConstBuffer bufPayload = { buffer, sizeof(buffer) };
    err = ndefRecordDecode(&bufPayload, &record1);
    if (err != ERR_NONE)
    {
        return err;
    }
    ndefRecordDump(&record1, true);

    ndefType uri;
    ndefConstBuffer bufUri = { (uint8_t*)"nfc.com", strlen("nfc.com") };
    err = ndefRtdUri(&uri, NDEF_URI_PREFIX_HTTP_WWW, &bufUri);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecord record2;
    err = ndefRtdUriToRecord(&uri, &record2);
    if (err != ERR_NONE)
    {
        return err;
    }
    ndefRecordDump(&record2, false);

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_rtd_type_uri_3()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);
    ndefRecord record;
    ndefType uri;
    ndefConstBuffer bufProtocol;

    uint8_t protocol;

    /* 1 */
    protocol = NDEF_URI_PREFIX_NONE;
    ndefConstBuffer bufUri = { (uint8_t*)"hello", strlen("hello") };

    ndefRtdUri(&uri, protocol, &bufUri);

    err |= ndefRtdUriToRecord(&uri, &record);
    ndefRecordDump(&record, false);

    /* 2 */
    protocol = NDEF_URI_PREFIX_AUTODETECT;
    ndefRtdUri(&uri, protocol, &bufUri);

    err |= ndefRtdUriToRecord(&uri, &record);
    ndefRecordDump(&record, false);

    /* 3 */
    protocol      = NDEF_URI_PREFIX_AUTODETECT;
    bufUri.buffer = (uint8_t*)"mailto:john@gmail.com";
    bufUri.length = strlen("mailto:john@gmail.com");
    ndefRtdUri(&uri, protocol, &bufUri);

    ndefGetRtdUri(&uri, &bufProtocol, &bufUri);

    if ( (bufUri.length != strlen("john@gmail.com")) ||
         (ST_BYTECMP(bufUri.buffer, "john@gmail.com", bufUri.length) != 0) )
    {
        return ERR_INTERNAL;
    }

    err |= ndefRtdUriToRecord(&uri, &record);
    ndefRecordDump(&record, false);

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_rtd_aar_1()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    // Android Application Record
    const uint8_t buffer[] = { 0x14 /* SR=1 TNF=Ext */,
                               0x0F /* Type length */,
                               0x1F /* Payload length */,
                               0x61, 0x6E, 0x64, 0x72, 0x6F, 0x69, 0x64, 0x2E,
                               0x63, 0x6F, 0x6D, 0x3A, 0x70, 0x6B, 0x67 /* android.com:pkg */,

                               0x73, 0x74, 0x2E, 0x63, 0x6F, 0x6D, 0x3A, 0x6D, 0x32, 0x34,
                               0x73, 0x72, 0x5F, 0x64, 0x69, 0x73, 0x63, 0x6F, 0x76, 0x65,
                               0x72, 0x79, 0x5F, 0x64, 0x65, 0x6D, 0x6F, 0x63, 0x74, 0x72, 0x6C /* st.com:m24sr_discovery_democtrl */
                               };

    ndefRecord record1;
    ndefConstBuffer bufPayload = { buffer, sizeof(buffer) };
    err = ndefRecordDecode(&bufPayload, &record1);
    if (err != ERR_NONE)
    {
        return err;
    }
    ndefRecordDump(&record1, true);

    ndefType ext;
    err = ndefRecordToRtdAar(&record1, &ext);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Convert URI to record */
    ndefRecord record2;
    err = ndefRtdAarToRecord(&ext, &record2);
    if (err != ERR_NONE)
    {
        return err;
    }
    ndefRecordDump(&record2, false);

    if (! ndefRecordMatch(&record1, &record2))
    {
        return ERR_INTERNAL;
    }

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_rtd_aar_2()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    // Android Application Record
    const uint8_t buffer[] = { 0x14 /* SR=1 TNF=Ext */,
                               0x0F /* Type length */,
                               0x1F /* Payload length */,
                               0x61, 0x6E, 0x64, 0x72, 0x6F, 0x69, 0x64, 0x2E,
                               0x63, 0x6F, 0x6D, 0x3A, 0x70, 0x6B, 0x67 /* android.com:pkg */,

                               0x73, 0x74, 0x2E, 0x63, 0x6F, 0x6D, 0x3A, 0x6D, 0x32, 0x34,
                               0x73, 0x72, 0x5F, 0x64, 0x69, 0x73, 0x63, 0x6F, 0x76, 0x65,
                               0x72, 0x79, 0x5F, 0x64, 0x65, 0x6D, 0x6F, 0x63, 0x74, 0x72, 0x6C /* st.com:m24sr_discovery_democtrl */
                               };

    ndefRecord record1;
    ndefConstBuffer bufPayload = { buffer, sizeof(buffer) };
    err = ndefRecordDecode(&bufPayload, &record1);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record1, true);

    const uint8_t app[] = "com.android.calculator2";
    ndefConstBuffer bufApp = { app, sizeof(app) - 1U };
    ndefType aar;
    err = ndefRtdAar(&aar, &bufApp);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Convert External Type to record */
    ndefRecord record2;
    err = ndefRtdAarToRecord(&aar, &record2);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record2, false);

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_rtd_aar_3()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    const uint8_t app[] = "com.st.st25nfc";
    ndefConstBuffer bufApp = { app, sizeof(app) - 1U };
    ndefType ext;
    err = ndefRtdAar(&ext, &bufApp);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Convert External Type to record */
    ndefRecord record;
    err = ndefRtdAarToRecord(&ext, &record);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record, true);

    /* Convert External Type from record */
    ndefType ext2;
    err = ndefRecordToRtdAar(&record, &ext2);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record, false);

    err = ndefRecordToType(&record, &ext);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDumpType(&record);

    err = ndefTypeToRecord(&ext, &record);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record, false);

    return err;
}

// add an External Type test calling ndefRecordEncode()

/*****************************************************************************/
ReturnCode ndefTest_Media_1()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    const uint8_t typeVCard[] = "text/x-vCard";
    const uint8_t payload[] = "BEGIN:VCARD\r\nEND:VCARD";

    ndefType media;
    ndefConstBuffer8 bufType  = { typeVCard,  sizeof(typeVCard) - 1U};
    ndefConstBuffer  bufVCard = { payload,    sizeof(payload) - 1U };
    err = ndefMedia(&media, &bufType, &bufVCard);
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Convert Media type to record */
    ndefRecord record;
    err = ndefMediaToRecord(&media, &record);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record, false);

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_vCard_1()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

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

    const ndefVCardInput bufVCard[] = {
        { &bufTypeN    , &bufSubTypeNone   , &bufValueN     },
        { &bufTypeFN   , &bufSubTypeNone   , &bufValueFN    },
        { &bufTypeADR  , &bufSubTypeNone   , &bufValueADR   },
        { &bufTypeTEL  , &bufSubTypeTelCELL, &bufValueTEL   },
        { &bufTypeEMAIL, &bufSubTypeNone   , &bufValueEMAIL },
        { &bufTypeTITLE, &bufSubTypeNone   , &bufValueTITLE },
        { &bufTypeORG  , &bufSubTypeNone   , &bufValueORG   },
        { &bufTypeURL  , &bufSubTypeNone   , &bufValueURL   },
    };

    /* Create the vCard type */
    ndefType typeVCard;
    err = ndefVCard(&typeVCard, bufVCard, SIZEOF_ARRAY(bufVCard));
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Convert vCard type to record */
    ndefRecord record;
    err = ndefVCardToRecord(&typeVCard, &record);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record, true);

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_vCard_2()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

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

    const ndefVCardInput bufVCard[] = {
        { &bufTypeN    , &bufSubTypeNone   , &bufValueN     },
        { &bufTypeFN   , &bufSubTypeNone   , &bufValueFN    },
        { &bufTypeADR  , &bufSubTypeNone   , &bufValueADR   },
        { &bufTypeTEL  , &bufSubTypeTelCELL, &bufValueTEL   },
        { &bufTypeEMAIL, &bufSubTypeNone   , &bufValueEMAIL },
        { &bufTypeTITLE, &bufSubTypeNone   , &bufValueTITLE },
        { &bufTypeORG  , &bufSubTypeNone   , &bufValueORG   },
        { &bufTypeURL  , &bufSubTypeNone   , &bufValueURL   },
    };

    /* Create the vCard type */
    ndefType vCard;
    err = ndefVCard(&vCard, bufVCard, SIZEOF_ARRAY(bufVCard));
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Convert vCard type to record */
    ndefRecord record;
    err = ndefVCardToRecord(&vCard, &record);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record, true);

    /* Convert */
    ndefRecord record2;
    err = ndefTypeToRecord(&vCard, &record2);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record2, true);

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_vCard_3()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

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

    const ndefVCardInput bufVCard[] = {
        { &bufTypeN    , &bufSubTypeNone   , &bufValueN     },
        { &bufTypeFN   , &bufSubTypeNone   , &bufValueFN    },
        { &bufTypeADR  , &bufSubTypeNone   , &bufValueADR   },
        { &bufTypeTEL  , &bufSubTypeTelCELL, &bufValueTEL   },
        { &bufTypeEMAIL, &bufSubTypeNone   , &bufValueEMAIL },
        { &bufTypeTITLE, &bufSubTypeNone   , &bufValueTITLE },
        { &bufTypeORG  , &bufSubTypeNone   , &bufValueORG   },
        { &bufTypeURL  , &bufSubTypeNone   , &bufValueURL   },
    };

    /* Create the vCard type */
    ndefType vCard;
    err = ndefVCard(&vCard, bufVCard, SIZEOF_ARRAY(bufVCard));
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Convert vCard type to record */
    ndefRecord record;
    err = ndefVCardToRecord(&vCard, &record);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record, true);

    /* Convert back record to vCard */
    ndefType vCard2;
    err = ndefRecordToVCard(&record, &vCard2);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefTypeDump(&vCard2);

    ndefType vCard3;
    err = ndefRecordToType(&record, &vCard3);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefTypeDump(&vCard3);


    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_vCard_4()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

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

    const ndefVCardInput bufVCard[] = {
        { &bufTypeN    , &bufSubTypeNone   , &bufValueN     },
        { &bufTypeFN   , &bufSubTypeNone   , &bufValueFN    },
        { &bufTypeADR  , &bufSubTypeNone   , &bufValueADR   },
        { &bufTypeTEL  , &bufSubTypeTelCELL, &bufValueTEL   },
        { &bufTypeEMAIL, &bufSubTypeNone   , &bufValueEMAIL },
        { &bufTypeTITLE, &bufSubTypeNone   , &bufValueTITLE },
        { &bufTypeORG  , &bufSubTypeNone   , &bufValueORG   },
        { &bufTypeURL  , &bufSubTypeNone   , &bufValueURL   },
    };

    /* Create the vCard type */
    ndefType vCard;
    err = ndefVCard(&vCard, bufVCard, SIZEOF_ARRAY(bufVCard));
    if (err != ERR_NONE)
    {
        return err;
    }

    /* Convert vCard type to record */
    ndefRecord record;
    err = ndefVCardToRecord(&vCard, &record);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record, true);

    uint8_t buffer[255];
    ndefBuffer bufRecord = { buffer, sizeof(buffer) };

    err = ndefRecordEncode(&record, &bufRecord);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefBufferDump("Encoded:", (ndefConstBuffer*)&bufRecord, true);

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_vCard_5()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    #define vCardBEGIN          'B', 'E', 'G', 'I', 'N'
    #define vCardVERSION        'V', 'E', 'R', 'S', 'I', 'O', 'N'
    #define vCardEND            'E', 'N', 'D'
    #define vCardN              'N'
    #define vCardFN             'F', 'N'
    #define vCardADR            'A', 'D', 'R'
    #define vCardTEL            'T', 'E', 'L'
    #define vCardEMAIL          'E', 'M', 'A', 'I', 'L'
    #define vCardTITLE          'T', 'I', 'T', 'L', 'E'
    #define vCardORG            'O', 'R', 'G'
    #define vCardURL            'U', 'R', 'L'

    #define vCardVARD          'V', 'C', 'A', 'R', 'D'
    #define vCardV2_1          '2', '.', '1'
    #define vCardEOL           '\r', '\n'

    #define vCardNValue       'D', 'o', 'e', ';', 'J', 'o', 'h', 'n', ';', ';', 'D', 'r', ';'
    #define vCardFNValue      'D', 'r', '.', ' ', 'J', 'o', 'h', 'n', ' ', 'D', 'o', 'e'
    #define vCardADRValue     'M', 'a', 'i', 'n', ' ', 'S', 't', '.'
    #define vCardTELValue     '+', '1', ' ', '1', '2', '3', ' ', '4', '5', '6', ' ', '7', '8', '9', '0'
    #define vCardEMAILValue   'j', 'o', 'h', 'n', '.', 'd', 'o', 'e', '@', 'g', 'm', 'a', 'i', 'l', '.', 'c', 'o', 'm'
    #define vCardTITLEValue   'D', 'o', 'c', 't', 'o', 'r'
    #define vCardORGValue     'C', 'o', 'r', 'p', 'o', 'r', 'a', 't', 'i', 'o', 'n'
    #define vCardURLValue     'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'j', 'o', 'h', 'n', 'n', 'y', '.', 'c', 'o', 'm'

    const uint8_t vCardData[] =
    {
        0x12 /* SR=1 TNF=2 */,
        0x0C /* Type length */,
        0xAB /* Payload length */,
        0x74, 0x65, 0x78, 0x74, 0x2F, 0x78, 0x2D, 0x76,
        0x43, 0x61, 0x72, 0x64 /* text/x-vCard */,

        vCardBEGIN   , (uint8_t)':', vCardVARD       , vCardEOL,
        vCardVERSION , (uint8_t)':', vCardV2_1       , vCardEOL,
        vCardN       , (uint8_t)':', vCardNValue     , vCardEOL,
        vCardADR     , (uint8_t)':', vCardADRValue   , vCardEOL,
        vCardTEL     , (uint8_t)':', vCardTELValue   , vCardEOL,
        vCardEMAIL   , (uint8_t)':', vCardEMAILValue , vCardEOL,
        vCardTITLE   , (uint8_t)':', vCardTITLEValue , vCardEOL,
        vCardORG     , (uint8_t)':', vCardORGValue   , vCardEOL,
        vCardURL     , (uint8_t)':', vCardURLValue   , vCardEOL,
        vCardEND     , (uint8_t)':', vCardVARD//       , vCardEOL,
    };

    ndefConstBuffer vCard = { vCardData, sizeof(vCardData) };

    ndefRecord record;
    err = ndefRecordDecode(&vCard, &record);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record, true);

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_wifi_1()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    const uint8_t buffer[] = {
#if 0
        0xDA, /* MB=1 ME=1 SR=1 IL=1 TNF=2 */
        0x17, /* Type Length */
        0x32, /* Payload Length */
        0x01, /* ID length */
        0x61, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74,
        0x69, 0x6F, 0x6E, 0x2F, 0x76, 0x6E, 0x64, 0x2E,
        0x77, 0x66, 0x61, 0x2E, 0x77, 0x73, 0x63,       /* application/vnd.wfa.wsc */
                          0x30, /* ID */
        0x10, 0x0E, 0x00, 0x24,
        0x10, 0x45, 0x00, 0x06, /* Payload */
        0x6D, 0x60, 0x69, 0x73, 0x6F, 0x6D,
        0x10, 0x20, 0x00, 0x06,
        0xFF, 0x72, 0x6F, 0x69, 0x64, 0x2E,
        0x63, 0x6F, 0x6D, 0x3A, 0x70, 0x6B, 0x67, 0x63,
        0x6F, 0x6D, 0x2E, 0x73, 0x74, 0x2E, 0x73, 0x74,
        0x32, 0x35, 0x6E, 0x66, 0x63, 0x00, 0x00, 0x00,
        0x00, 0x00
#else
        0xD2, /* MB=1 ME=1 SR=1 TNF=2 */
        0x17, /* Type Length */
        0x48, /* Payload Length */
        0x61, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74,
        0x69, 0x6F, 0x6E, 0x2F, 0x76, 0x6E, 0x64, 0x2E,
        0x77, 0x66, 0x61, 0x2E, 0x77, 0x73, 0x63,       /* 0x17 */

                                                  0x10, /* Payload 70 = 0x48 */
        0x4A, 0x00, 0x01, 0x10, 0x10, 0x0E, 0x00, 0x3F,
        0x10, 0x26, 0x00, 0x01, 0x01,

        0x10, 0x45, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04, /* Payload */ /* data length 4, data 0x01, 0x02, 0x03, 0x04 */
        0x10, 0x03, 0x00, 0x02, 0x00, 0x02, 0x10, 0x0F,
        0x00, 0x02, 0x00, 0x03, 0x10, 0x27, 0x00, 0x04, /* data length 4, data 0x05, 0x06, 0x07, 0x08 */
        0x05, 0x06, 0x07, 0x08, 0x10, 0x20, 0x00, 0x06,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x49,
        0x00, 0x06, 0x00, 0x37, 0x2A, 0x02, 0x01, 0x01,
        0x10, 0x49, 0x00, 0x06, 0x00, 0x37, 0x2A, 0x00, 0, 0
#endif
};

    ndefRecord record;
    ndefConstBuffer bufPayload = { buffer, sizeof(buffer) };
    err = ndefRecordDecode(&bufPayload, &record);
    if (err != ERR_NONE)
    {
        return err;
    }

    err = ndefRecordDump(&record, true);
    if (err != ERR_NONE)
    {
        return err;
    }

    // Convert record to wifi */
    ndefType wifi;
    err = ndefRecordToWifi(&record, &wifi);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecordDump(&record, true);

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_wifi_2()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    uint8_t buffer[255];
    ndefBuffer bufEncode = { buffer, sizeof(buffer) };

    ndefType wifi;
    uint8_t ssid[] = { 0x01, 0x02, 0x03, 0x04 };
    ndefConstBuffer bufNetworkSSID = { ssid, sizeof(ssid) };
    uint8_t  key[] = { 0x05, 0x06, 0x07, 0x08 };
    ndefConstBuffer bufNetworkKey  = { key, sizeof(key) };

    ndefTypeWifi wifiConfig = {
        .bufNetworkSSID = bufNetworkSSID,
        .bufNetworkKey  = bufNetworkKey,
        .authentication = NDEF_WIFI_AUTHENTICATION_WPAPSK,
        .encryption     = NDEF_WIFI_ENCRYPTION_TKIP
    };

    err = ndefWifi(&wifi, &wifiConfig);
    if (err != ERR_NONE)
    {
        return err;
    }

    ndefRecord record;
    err = ndefWifiToRecord(&wifi, &record);
    if (err != ERR_NONE)
    {
        return err;
    }
    ndefRecordDump(&record, true);

    err = ndefRecordEncode(&record, &bufEncode);

    return err;
}


/*****************************************************************************/
ReturnCode ndefTest_types_memory_size()
{
    ReturnCode err = ERR_NONE;
    platformLog("Running %s...\r\n", __FUNCTION__);

    platformLog("sizeof(ndefRecord) = %d bytes\r\n",             sizeof(ndefRecord));
    platformLog("sizeof(ndefMessage) = %d bytes\r\n",            sizeof(ndefMessage));
    platformLog("sizeof(ndefTypeRtdDeviceInfo) = %d bytes\r\n",  sizeof(ndefTypeRtdDeviceInfo));
    platformLog("sizeof(ndefTypeRtdText) = %d bytes\r\n",        sizeof(ndefTypeRtdText));
    platformLog("sizeof(ndefTypeRtdUri) = %d bytes\r\n",         sizeof(ndefTypeRtdUri));
    platformLog("sizeof(ndefTypeRtdAar) = %d bytes\r\n",         sizeof(ndefTypeRtdAar));
    platformLog("sizeof(ndefTypeMedia) = %d bytes\r\n",          sizeof(ndefTypeMedia));
    platformLog("sizeof(ndefTypeVCard) = %d bytes\r\n",          sizeof(ndefTypeVCard));
    platformLog("sizeof(ndefTypeWifi) = %d bytes\r\n",           sizeof(ndefTypeWifi));
    platformLog("=======================\r\n");
    platformLog("sizeof(ndefType) = %d bytes\r\n",               sizeof(ndefType));

    return err;
}


/*****************************************************************************/
ReturnCode ndefUnitaryTests(void)
{
    ReturnCode err = ERR_NONE;
    platformLog("Running NDEF tests...\r\n");

    /* 4.5 Special Consideration Test Requirements 
       -------------------------------------------
        An NDEF parser MUST NOT reject an NDEF message based solely on the value of the SR flag.
        An NDEF parser MAY reject messages that include records with TYPE, ID, or PAYLOAD fields larger than its design limits. */


    err |= ndefTest_RecordEmpty_0();
    err |= ndefTest_RecordEmpty_1();
    err |= ndefTest_RecordEmpty_2();
    err |= ndefTest_RecordEmpty_3();
    err |= ndefTest_RecordEmpty_4();
    err |= ndefTest_RecordEncodeDecode_1();

    err |= ndefTest_Message1record();

    err |= ndefTest_Message2records_1();
    err |= ndefTest_Message2records_2();

    err |= ndefTest_Message3records();

    err |= ndefTest_PlainText();

    // Device Info
    err |= ndefTest_rtd_type_deviceinfo_1();
    err |= ndefTest_rtd_type_deviceinfo_2();
    err |= ndefTest_rtd_type_deviceinfo_3();
    err |= ndefTest_rtd_type_deviceinfo_4();

    // Text
    err |= ndefTest_rtd_type_text_1();
    err |= ndefTest_rtd_type_text_2();
    err |= ndefTest_rtd_type_text_3();

    // URI
    err |= ndefTest_rtd_type_uri_1();
    err |= ndefTest_rtd_type_uri_2();
    err |= ndefTest_rtd_type_uri_3();
    /* TODO URI with non US-ASCII characters */

    // NFC External type
    err |= ndefTest_rtd_aar_1();
    err |= ndefTest_rtd_aar_2();
    err |= ndefTest_rtd_aar_3();

    // Media type
    err |= ndefTest_Media_1();

    // VCard
    err |= ndefTest_vCard_1();
    err |= ndefTest_vCard_2();
    err |= ndefTest_vCard_3();
    err |= ndefTest_vCard_4();
    err |= ndefTest_vCard_5();

    // Wifi
    err |= ndefTest_wifi_1();
    err |= ndefTest_wifi_2();

    // Bluetooth

    // Smart Poster
    // Record chunks

    err |= ndefTest_types_memory_size();

    if (err == ERR_NONE)
    {
        platformLog("Tests passed!\r\n");
    }
    else
    {
        platformLog("Tests failed...\r\n");
    }

    return err;
}
