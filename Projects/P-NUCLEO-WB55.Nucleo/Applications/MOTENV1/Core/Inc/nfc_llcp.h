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
 *      PROJECT:   LLCP
 *      Revision:
 *      LANGUAGE:  ISO C99
 */

/*! \file
 *
 *  \author
 *
 *  \brief LLCP header file
 *
 *
 * LLCP provides functionalities required to Logical Link Control Protocol.
 *
 *
 * \addtogroup LLCP
 * @{
 *
 */

#ifndef NFC_LLCP_H
#define NFC_LLCP_H


/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */

#include "rfal_nfcDep.h"
#include "rfal_nfc.h"


/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */


/* LLCP Configuration */

#define LLCP_CONNECTION_MAX         7U /*!< Number of simultaneous services bound */

#define PDU_HEADER_LENGTH           2U /*!< PDU Header length  */

#define LLCP_BUFFER_LENGTH          sizeof(rfalNfcDepBufFormat) /*!< LLCP Buffer length */

#define LLCP_DM_REASON_DISC_RECEIVED        0x00U
#define LLCP_DM_REASON_NO_ACTIVE_CONNECTION 0x01U
#define LLCP_DM_REASON_NO_BOUND_SERVICE     0x02U
#define LLCP_DM_REASON_REJECTED             0x03U

/*
 ******************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */


/*! LLCP PDU Type */
typedef enum
{
    LLCP_PDU_SYMM      = 0x0,  /*!< SYMM       Symmetry              */
    LLCP_PDU_PAX       = 0x1,  /*!< PAX        Parameter Exchange    */
    LLCP_PDU_AGF       = 0x2,  /*!< AGF        Aggregated Frame      */
    LLCP_PDU_UI        = 0x3,  /*!< UI         Unumbered Information */
    LLCP_PDU_CONNECT   = 0x4,  /*!< CONNECT    Connect               */
    LLCP_PDU_DISC      = 0x5,  /*!< DISCONNECT Disconnect            */
    LLCP_PDU_CC        = 0x6,  /*!< CC         Connection Complete   */
    LLCP_PDU_DM        = 0x7,  /*!< DM         Disconnected Mode     */
    LLCP_PDU_FRMR      = 0x8,  /*!< FRMR       Frame Reject          */
    LLCP_PDU_SNL       = 0x9,  /*!< SNL        Service Name Lookup   */
    LLCP_PDU_DPS       = 0xA,  /*!< DPS        Data Protection Setup */
    LLCP_PDU_Reserved1 = 0xB,  /*!< Reserved                         */
    LLCP_PDU_I         = 0xC,  /*!< I          Information           */
    LLCP_PDU_RR        = 0xD,  /*!< RR         Receive Ready         */
    LLCP_PDU_RNR       = 0xE,  /*!< RNR        Receive Not Ready     */
    LLCP_PDU_Reserved2 = 0xF,  /*!< Reserved                         */
    LLCP_PDU_COUNT             /*<! Keep this one last               */
} llcpPdu;


/*! LLCP PDU Header type*/
typedef struct
{
    uint8_t DSAP;   /*<! DSAP  Destination Service Access Point */
    uint8_t PTYPE;  /*<! PTYPE PDU Type                         */
    uint8_t SSAP;   /*<! SSAP  Source Service Access Point      */
} llcpPduHeader;


/*! LLCP Service Class */
typedef enum
{
    LLCP_CLASS_UNKNOWN             = 0, /*! No class set                               */
    LLCP_CLASS_CONNECTIONLESS      = 1, /*! Connectionless transport service class     */
    LLCP_CLASS_CONNECTION_ORIENTED = 2, /*! Connection-orented transport service class */
    LLCP_CLASS_BOTH = LLCP_CLASS_CONNECTIONLESS | LLCP_CLASS_CONNECTION_ORIENTED /*! Both */
} llcpServiceClass;


/*! Formward declaration */
typedef struct llcpServiceStruct llcpService;


/*! LLCP Service structure */
typedef struct
{
    const uint8_t*   uri;       /*<! Local URI         */
    uint8_t          SAP;       /*<! SAP               */
    llcpServiceClass Class;     /*<! Class             */
                                /*!< Callback the client whenever an event is received (UI, I buffer, CC) */
    ReturnCode       (*notifyCb)(llcpService* service, llcpPdu pdu, const uint8_t* rxBuffer, uint16_t rxBufferLength);
} llcpServiceConfig;


/*! LLCP State variables for I PDU */
typedef struct
{
    uint8_t  V_S;  /*!< V(S)  Send State Variable                    */
    uint8_t  V_SA; /*!< V(SA) Send Acknowledgement State Variable    */
    uint8_t  V_R;  /*!< V(R)  Receive State Variable                 */
    uint8_t  V_RA; /*!< V(RA) Receive Acknowledgement State Variable */
} llcpStateVariables;

/*! LLCP Service Configuration */
typedef struct llcpServiceStruct
{
    llcpServiceConfig SSAP; /*!< SSAP Source Service Access Point       */
    uint8_t  DSAP;       /*!< DSAP Destination Service Access Point  */

    uint8_t  TID;        /*!< TID (Transport ID)                     */

    uint8_t RW;          /*!< RW (Receive Window Size, for I PDU)    */

    bool connected;      /*!< True when service is connected         */

    llcpStateVariables state; /*!< State variables for I PDU         */

    /* buffer */
    uint8_t  txBuffer[LLCP_BUFFER_LENGTH];    /*!< txBuffer             */
    uint16_t txBufferLength;                  /*!< txBufferLength       */

    /* UI buffer */
    uint8_t  txUiBuffer[LLCP_BUFFER_LENGTH];  /*!< txUiBuffer           */
    uint16_t txUiBufferLength;                /*!< txUiBufferLength     */

    uint8_t  rxUiBuffer[LLCP_BUFFER_LENGTH];  /*!< rxUiBuffer           */
    uint16_t rxUiBufferLength;                /*!< rxUiBufferLength     */
    bool     rxUiBufferAtUser;                /*!< true while the user hasn't returned it */

    /* I buffer */
    uint8_t  txIBuffer[LLCP_BUFFER_LENGTH];  /*!< txIBuffer             */
    uint16_t txIBufferLength;                /*!< txIBufferLength       */

    uint8_t  rxIBuffer[LLCP_BUFFER_LENGTH];  /*!< rxIBuffer             */
    uint16_t rxIBufferLength;                /*!< rxIBufferLength       */
    bool     rxIBufferAtUser;                /*!< true while the user hasn't returned it  */

    uint8_t* txBuf;       /*!< Pointer to the buffer to send, either UI or I txBuffer        */
    uint16_t txBufLength; /*!< txBufLength                                                   */
    llcpPdu  nextPdu;     /*!< Next PDU that will be sent by the llcpWorker for this service */
} llcpService;


/*! LLCP server Configuration */
typedef struct
{
    uint8_t  VERSION;   /*!< Version */
    uint16_t MIUX;      /*!< Maximum Information Unit Extension */
    uint16_t WKS;       /*!< Well-known Service List */
    uint8_t  LTO;       /*!< Link Timeout, in multiples of 10 milliseconds */
    uint8_t  OPT;       /*!< Option */
} llcpServerConfig;


/*! LLCP roles */
typedef enum
{
    LLCP_TARGET,      /*!< LLCP server acts as a Target     */
    LLCP_INITIATOR    /*!< LLCP server acts as an Initiator */
} llcpServerRole;


/*! LLCP worker state */ 
typedef enum
{
    LLCP_TRANSMIT,    /*!< Accept only 1 command at a time                       */
    LLCP_RECEIVE,     /*!< Stuck in Receive state while transfer is not complete */
    LLCP_ERROR,       /*!< Error state */
} llcpWorkerState;


/*! LLCP server Configuration */
typedef struct
{
    llcpServerRole role;     /*!< Server role */
    bool activated;          /*!< Server activation state */

    llcpServerConfig config; /*!< Server Configuration */
    uint8_t agreedVersion;

    llcpService service[LLCP_CONNECTION_MAX]; /*!< Connection-oriented */

    llcpService* serviceSAP0; /*!< Link Management Service     */
    llcpService* serviceSAP1; /*!< Service Name Lookup Service */

    uint8_t  TID;       /*!< TID (Transport ID) */

    uint16_t MIU;       /*!< MIU Maximum Information Unit */

    llcpWorkerState state;

    /* Buffer to send */
    uint8_t* txBuf;       /*!< Pointer to the buffer to send */
    uint16_t txBufLength; /*!< Length of the buffer to send  */

    uint8_t*  rxRfalBuffer;       /*<! Pointer to RFAL Rx buffer        */
    uint16_t* rxRfalBufferLength; /*<! Pointer to RFAL Rx buffer length */

    /* Internal Buffer */
    uint8_t PDU_SYMM[PDU_HEADER_LENGTH];
} llcpServer;


/*
 ******************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 ******************************************************************************
 */


/*!
 *****************************************************************************
 * Initialize the LLCP
 *
 * This function initializes all configurations
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode llcpInit(void);


/*!
 *****************************************************************************
 * Activate the LLCP
 *
 * This function activates the LLCP
 *
 * \param[in] nfcDevice
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode llcpActivate(rfalNfcDevice* nfcDevice);


/*!
 *****************************************************************************
 * Deactivate the LLCP
 *
 * This function deactivates the LLCP
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode llcpDeactivate(void);


/*!
 *****************************************************************************
 * Deinitialize the LLCP
 *
 * This function deinitializes the configurations
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode llcpDeinit(void);


/*!
 *****************************************************************************
 * Bind a service to the LLCP server
 *
 * This function binds a new LLCP service for connection-oriented transmission.
 *
 * \param[in]  config to bind
 * \param[out] service to use for this service
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode llcpBindService(const llcpServiceConfig* config, llcpService** service);


/*!
 *****************************************************************************
 * Unbind the service associated to the LLCP server
 *
 * This function unbinds the LLCP service for connection-oriented transmission.
 *
 * \param[in] service to remove
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode llcpUnbindService(llcpService* service);


/*!
 *****************************************************************************
 * LLCP Service Name Lookup Request
 *
 * This function sends a Service Name Lookup Request.
 * The DSAP is retrieved by in the service callback().
 *
 * \param[in] uri to find SAP
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode llcpServiceNameLookup(llcpService* service, const uint8_t* uri);


/*!
 *****************************************************************************
 * LLCP Connect
 *
 * This function eithers connects to the given DSAP or the given URI.
 * DSAP is ignored if URI is given.
 *
 * \param[in] service previoulsy bound to connect
 * \param[in] dsap to connect to, if known
 * \param[in] uri  to connect to, if known
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode llcpConnect(llcpService* service, uint8_t dsap, const uint8_t* uri);


/*!
 *****************************************************************************
 * LLCP Disconnect
 *
 * This function disconnects the given LLCP SSAP service.
 *
 * \param[in] service to disconnect
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode llcpDisconnect(llcpService* service);


/*!
 *****************************************************************************
 * LLCP Send Unnumbered Information
 *
 * This function sends an unnumbered buffer.
 *
 * \param[in] service to use to send unnumbered information
 * \param[in] DSAP Destination SAP
 * \param[in] txBuffer
 * \param[in] txBufferLength
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode llcpSendUI(llcpService* service, uint8_t DSAP, const uint8_t* txBuffer, uint16_t txLength);


/*!
 *****************************************************************************
 * Check available RW
 *
 * This function checks whether RW is enough to send an I PDU.
 *
 * \param[in] service to use to send information
 *
 * \return true if the user can send and I PDU
 *****************************************************************************
 */
bool llcpCheckAvailableRW(llcpService* service);


/*!
 *****************************************************************************
 * LLCP Send Information
 *
 * This function sends a numbered buffer. LLCP must be previously connected.
 *
 * \param[in] service to use to send information
 * \param[in] txBuffer
 * \param[in] txBufferLength
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode llcpSendI(llcpService* service, const uint8_t* txBuffer, uint16_t txLength);


/*!
 *****************************************************************************
 * Return UI Buffer to LLCP server
 *
 * Allows the client to return the Unnumbered Information buffer
 * to the LLCP server.
 *
 * \param[in] service to notify the buffer has been processed by the user
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode llcpUiBufferProcessed(llcpService* service);


/*!
 *****************************************************************************
 * Return I Buffer to LLCP server
 *
 * Allows the client to return the Information buffer 
 * to the LLCP server.
 *
 * \param[in] service to notify the buffer has been processed by the user
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode llcpIBufferProcessed(llcpService* service);


/*!
 *****************************************************************************
 * LLCP Worker
 *
 * Must call the worker in a loop, to keep the LLCP connection active.
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
void llcpWorker(void);


/*!
 *****************************************************************************
 * Get LLCP Worker status
 *
 * Call the llcpWorker until it turns ready. This ensures the underliying RFAL 
 * processing completed.
 *
 * \return ERR_NONE if successful or a standard error code
 *****************************************************************************
 */
ReturnCode llcpWorkerGetStatus(void);


#endif

/**
  * @}
  *
  */
