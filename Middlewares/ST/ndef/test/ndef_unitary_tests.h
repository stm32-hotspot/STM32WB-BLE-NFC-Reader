/**
  ******************************************************************************
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
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

/*! \file
 *
 *  \author
 *
 *  \brief NDEF message unitary tests header file
 *
 */
 
#ifndef NDEF_UNITARY_TESTS_H
#define NDEF_UNITARY_TESTS_H


/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */


#include <stdbool.h>
#include "st_errno.h"
#include "ndef_message.h"


/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */


#define MY_ASSERT(cond, err) do{ if (cond==false) { platformLog("Assert failed %s:%d\r\n", __MODULE__, __LINE__); return err; } } while(0)
#define CHECK_RANGE(value, min, max) do{ if ( ((min != 0) && (value < min)) || (value > max)) { platformLog("Check range failed line %d\r\n", __LINE__); return ERR_PARAM; } } while(0)


/*
 ******************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 ******************************************************************************
 */


/*!
 *****************************************************************************
 * NDEF message conformance
 *
 * This function checks the requirements provided in NFC Forum's NDEF Technical 
 * Specification, section 2.5 "NDEF Mechanisms Test Requirements".
 *
 * \param[in] message
 
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode ndefMessageConformance(const ndefMessage* message);

/*!
 *****************************************************************************
 * NDEF message test requirements
 *
 * This function checks the requirements provided in NFC Forum's NDEF Technical
 * Specification, section 3.3 "The NDEF Specification Test Requirements".
 *
 * \param[in] message
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode ndefMessageTestRequirements(const ndefMessage* message);

/*!
 *****************************************************************************
 * Copy the content of an NDEF record to another
 *
 * \param[in]  recordSrc:  Source record
 * \param[out] recordDest: Destination record
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode ndefRecordCopy(const ndefRecord* recordSrc, ndefRecord* recordDest);

/*!
 *****************************************************************************
 * Compare two NDEF records
 *
 * \param[in] record1: Record to compare
 * \param[in] record2: Record to compare
 *
 * \return true if both records match, false otherwise
 *****************************************************************************
 */
bool ndefRecordMatch(const ndefRecord* record1, const ndefRecord* record2);

/*!
 *****************************************************************************
 * Compare two NDEF messages
 *
 * \param[in] message1: Message to compare
 * \param[in] message2: Message to compare
 *
 * \return true if both records match, false otherwise
 *****************************************************************************
 */
bool ndefMessageMatch(const ndefMessage* message1, const ndefMessage* message2);

/*!
 *****************************************************************************
 * NDEF test function
 *
 * This function runs all the tests in a row.
 *
 * \return ERR_NONE : All tests passed
 * \return standard error code if a single test failed
 *****************************************************************************
 */
ReturnCode ndefUnitaryTests(void);


#endif /* NDEF_UNITARY_TESTS_H */
