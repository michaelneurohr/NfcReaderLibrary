/*
*         Copyright (c), NXP Semiconductors Gratkorn / Austria
*
*                     (C)NXP Semiconductors
*       All rights are reserved. Reproduction in whole or in part is
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
*particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
*/

/** \file
* Example Source for Raspberry Pi and EXPLORE-NFC.
* $Author: Michael Neurohr $
* $Revision: 1.00 $
* $Date: 2015-11-18 $
*
*
* Description of the example:
* This example demonstrates reading NDEF message from a NFC Forum Type 4 Tag.
* It follows the NDEF detection procedure described in the NFC Forum Type 4 Tag operation
* specification.
* This example is tested successfully reading NDEF messages from an Android device in HCE mode.
* It does not support reading messages from MIFARE Desfire.
* The detection procedure is as follows:
*
* 1) Select the NDEF Tag Application.
*
* 2) If the NDEF Tag Application is successfully selected, then go to item 3. Otherwise, the Type
* 4 Tag Platform is not in a valid state. If the Type 4 Tag Platform is not in a valid state and the
* NFC Forum Device implements both Mapping Version 1.0 and Mapping Version 2.0, the
* NFC Forum Device SHALL select the NDEF Tag Application.
*
* 3) Select the Capability Container (CC) file
*
* 4) If the CC file is successfully selected, then go to item 5. Otherwise, the Type 4 Tag Platform
* is not in a valid state.
*
* 5) Read the CC file and select the NDEF file.
*
* 6) If the CC file is successfully read, and the NDEF file has read access without any security
* and is successfully selected, then go to item 7. Otherwise, the Type 4 Tag Platform is not in a
* valid state.
*
* 7) Read NLEN (NDEF length) from NDEF file.
*
* 8) Read the NDEF message.
*
* History:
* 2015-11-18 - initial version
*/

#include <stdio.h>

/**
* Reader Library Headers
*/
/* Status code definitions */
#include <ph_Status.h>
/* Generic ISO14443-3A Component of
 * Reader Library Framework */
#include <phpalI14443p3a.h>
/* Generic ISO14443-4 Component of
 * Reader Library Framework */
#include <phpalI14443p4.h>
/* Generic ISO14443-4 Component of
 * Reader Library Framework */
#include <phpalI14443p4a.h>
/* Generic BAL Component of
 * Reader Library Framework */
#include <phbalReg.h>

#include <phbalReg_RpiSpi.h>

/* prints if error is detected */
#define CHECK_STATUS(x)                                      \
    if ((x) != PH_ERR_SUCCESS)                               \
{                                                            \
    	DEBUG_PRINTF("Line: %d   Error - (0x%04X) has occurred : 0xCCEE CC-Component ID, EE-Error code. Refer-ph_Status.h\n", __LINE__, (x));    \
}

/* Returns if error is detected */
#define CHECK_SUCCESS(x)              \
    if ((x) != PH_ERR_SUCCESS)        \
{                                     \
    	DEBUG_PRINTF("\nLine: %d   Error - (0x%04X) has occurred : 0xCCEE CC-Component ID, EE-Error code. Refer-ph_Status.h\n ", __LINE__, (x)); \
    return x;                         \
}

#include <stdio.h>
#define  DEBUG_PRINTF(...) printf(__VA_ARGS__)
#define	 DEBUG_FLUSH(x)		{char buff[1]; buff[0] = x; __sys_write(0, buff, 1);}

phbalReg_RpiSpi_DataParams_t       balReader;
uint8_t                            bHalBufferTx[256];         /* HAL TX buffer. Size 256 - Based on maximum FSL */
uint8_t                            bHalBufferRx[256];         /* HAL RX buffer. Size 256 - Based on maximum FSL */
uint8_t                            bAtr_Res[30];              /* ATR response holder */
uint8_t                            bAtr_ResLength;            /* ATR response length */
phhalHw_Rc523_DataParams_t         hal;                       /* HAL component holder */
phpalI14443p3a_Sw_DataParams_t     palI14443p3a;              /* PAL I14443-A component */
phpalI14443p4a_Sw_DataParams_t     palI14443p4a;              /* PAL I14443-4A component */
phpalI14443p4_Sw_DataParams_t      palI14443p4;               /* PAL I14443-4 component */
void                              *pHal;                      /* HAL pointer */
uint8_t bBufferReader[0x60];
uint8_t **ppRxBuffer = (void *)&bBufferReader[0];

extern void phlnLlcp_Fri_OvrHal_Init(void);

static void PRINT_BUFF(uint8_t *hex, uint8_t num)
{
    uint32_t	i;

    for(i = 0; i < num; i++)
	{
    	DEBUG_PRINTF(" %02X",hex[i]);
	}
}


/*******************************************************************************
**   Main Function
*******************************************************************************/
int main (void)
{
    phStatus_t  status;
    uint8_t bUid[10];
    uint8_t bLength;
    uint16_t bRxLength;
    uint8_t bSak[1];
    uint8_t bMoreCardsAvailable;
    uint8_t pAtqa[2];
    uint8_t count_loops = 0;

    /* APDU commands used */
    uint8_t SELECT_BY_NAME[] = { 0x00, 0xA4, 0x04, 0x00 };
    uint8_t SELECT_FIRST_OCCURENCE[] = { 0x00, 0xA4, 0x00, 0x0C };
    uint8_t READ_BINARY[] = { 0x00, 0xB0 };
	uint8_t AID_NDEF[] = { 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01 };
	uint8_t FID[] = { 0xE1, 0x03 };
	uint8_t OFFSET[] = { 0x00, 0x00 };
	uint8_t LENGTH[] = { 0x00 };


    DEBUG_PRINTF("\nStart Example: Read NDEF from Type 4 Tag");

    /* Initialize the Reader BAL (Bus Abstraction Layer) component */
    status = phbalReg_RpiSpi_Init(&balReader, sizeof(phbalReg_RpiSpi_DataParams_t));

    /* Initialize the Stub timers component */
    CHECK_SUCCESS(status);

    /* Initialize the Reader HAL (Hardware Abstraction Layer) component */
    status = phhalHw_Rc523_Init(
        &hal,
        sizeof(phhalHw_Rc523_DataParams_t),
        &balReader,
        0,
        bHalBufferTx,
        sizeof(bHalBufferTx),
        bHalBufferRx,
        sizeof(bHalBufferRx)
        );
        
    /* Set the parameter to use the SPI interface */
    hal.bBalConnectionType = PHHAL_HW_BAL_CONNECTION_SPI;

    /* Set the generic pointer */
    pHal = &hal;

    status = phbalReg_RpiSpi_OpenPort(&balReader);
    CHECK_SUCCESS(status);

    phhalHw_ReadRegister(pHal, PHHAL_HW_RC523_REG_VERSION, bHalBufferTx);
    DEBUG_PRINTF("\nReader chip PN512: 0x%02x\n", bHalBufferTx[0]);

    /* Initialize the I14443-A PAL layer */
    status = phpalI14443p3a_Sw_Init(&palI14443p3a, sizeof(phpalI14443p3a_Sw_DataParams_t), &hal);
    CHECK_SUCCESS(status);

    /* Initialize the I14443-A PAL component */
    status = phpalI14443p4a_Sw_Init(&palI14443p4a, sizeof(phpalI14443p4a_Sw_DataParams_t), &hal);
    CHECK_SUCCESS(status);

    /* Initialize the I14443-4 PAL component */
    status = phpalI14443p4_Sw_Init(&palI14443p4, sizeof(phpalI14443p4_Sw_DataParams_t), &hal);
    CHECK_SUCCESS(status);

    /* Stay in a while loop to detect one after another Type 4 Tag. */
    while(1)
    {
    	if (!(count_loops % 30))
    		DEBUG_PRINTF("\nReady to detect\n");

		/* reset the IC  */
		status = phhalHw_FieldReset(pHal);
		CHECK_SUCCESS(status);

		status = phhalHw_ApplyProtocolSettings(pHal, PHHAL_HW_CARDTYPE_ISO14443A);


		/* Empty the pAtqa */
		memset(pAtqa, '\0', 2);
		status = phpalI14443p3a_RequestA(&palI14443p3a, pAtqa);

		/* Reset the RF field */
		status = phhalHw_FieldReset(pHal);

		/* Empty the bSak */
		memset(bSak, '\0', 1);

		/* Activate the communication layer part 3
		 * of the ISO 14443A standard. */
		status = phpalI14443p3a_ActivateCard(&palI14443p3a, NULL, 0x00, bUid, &bLength, bSak,
											 &bMoreCardsAvailable);

		if (PH_ERR_SUCCESS == status && !bMoreCardsAvailable)
		{
			DEBUG_PRINTF("Target activated\n");

			/* Check if there is an ISO-4 compliant card in the RF field */
			if (0x20 == (*bSak & 0x20))
			{
				/* Iso-4 card, send RATS */
				status = phpalI14443p4a_Rats(&palI14443p4a, palI14443p4a.bFsdi, palI14443p4a.bCid,
											 bBufferReader);

				/* Iso-4 card, set protocol */
				status = phpalI14443p4_SetProtocol(&palI14443p4,
									 PH_OFF, palI14443p4a.bCid, PH_OFF, PH_OFF,
									 palI14443p4a.bFwi, palI14443p4a.bFsdi, palI14443p4a.bFsci);

				DEBUG_PRINTF("\n********SEND SELECT AID COMMAND********\n");

				uint8_t message[sizeof(SELECT_BY_NAME) + sizeof(AID_NDEF) + 1];

				memcpy(&message[0], SELECT_BY_NAME, sizeof(SELECT_BY_NAME));
				message[4] = sizeof(AID_NDEF);
				memcpy(&message[5], AID_NDEF, sizeof(AID_NDEF));
				memcpy(&bBufferReader[3], message, sizeof(message));

				DEBUG_PRINTF("Sending APDU:\n");
				PRINT_BUFF(&bBufferReader[3], sizeof(message));
				status = phpalI14443p4_Exchange(&palI14443p4,PH_EXCHANGE_DEFAULT, &bBufferReader[3],
											 sizeof(message),(void *)&bBufferReader[0], &bRxLength);

				if(!(ppRxBuffer[0][0] == 0x90 && ppRxBuffer[0][1] == 0x00))
				{
					/* Maybe no NDEF message present */
					DEBUG_PRINTF("\n APDU not successful. Error code: ");
					PRINT_BUFF(ppRxBuffer[0], 2);
					DEBUG_PRINTF("\n");
					continue;
				}

				DEBUG_PRINTF("\n********Select Capability Container********\n");

				FID[0] = 0xE1;
				FID[1] = 0x03;

				uint8_t message_read[sizeof(FID) + sizeof(SELECT_FIRST_OCCURENCE) + 1];

				memcpy(&message_read[0], SELECT_FIRST_OCCURENCE, sizeof(SELECT_FIRST_OCCURENCE));
				message_read[4] = sizeof(FID);
				memcpy(&message_read[5], FID, sizeof(FID));
				memcpy(&bBufferReader[3], message_read, sizeof(message_read));

				DEBUG_PRINTF("Sending APDU:\n");
				PRINT_BUFF(&bBufferReader[3], sizeof(message_read));
				status = phpalI14443p4_Exchange(&palI14443p4,PH_EXCHANGE_DEFAULT, &bBufferReader[3],
										sizeof(message_read),(void *)&bBufferReader[0], &bRxLength);

				if(!(ppRxBuffer[0][0] == 0x90 && ppRxBuffer[0][1] == 0x00))
				{
					DEBUG_PRINTF("\n APDU not successful. Error code: ");
					PRINT_BUFF(ppRxBuffer[0], 2);
					DEBUG_PRINTF("\n");
					continue;
				}

				DEBUG_PRINTF("\n********Read Capability Container********\n");

				OFFSET[0] = 0x00;
				OFFSET[1] = 0x00;
				LENGTH[0] = 0x0F;

				uint8_t cc_read[sizeof(READ_BINARY) + sizeof(OFFSET) + sizeof(LENGTH)];

				memcpy(&cc_read[0], READ_BINARY, sizeof(READ_BINARY));
				memcpy(&cc_read[2], OFFSET, sizeof(OFFSET));
				memcpy(&cc_read[4], LENGTH, sizeof(LENGTH));
				memcpy(&bBufferReader[3], cc_read, sizeof(cc_read));

				DEBUG_PRINTF("Sending APDU:\n");
				PRINT_BUFF(&bBufferReader[3], sizeof(message_read));
				status = phpalI14443p4_Exchange(&palI14443p4,PH_EXCHANGE_DEFAULT, &bBufferReader[3],
											sizeof(cc_read), (void *)&bBufferReader[0], &bRxLength);

				if(!(ppRxBuffer[0][15] == 0x90 && ppRxBuffer[0][16] == 0x00))
				{
					DEBUG_PRINTF("\n APDU not successful. Error code: ");
					PRINT_BUFF(ppRxBuffer[0], 2);
					DEBUG_PRINTF("\n");
					continue;
				}

				DEBUG_PRINTF("\n********Select NDEF message********\n");

				/* Assigning the File ID of the NDEF message */
				FID[0] = ppRxBuffer[0][9];
				FID[1] = ppRxBuffer[0][10];

				uint8_t ndef_select[sizeof(SELECT_FIRST_OCCURENCE) + sizeof(FID) + 1];

				memcpy(&ndef_select[0], SELECT_FIRST_OCCURENCE, sizeof(SELECT_FIRST_OCCURENCE));
				message_read[4] = sizeof(FID);
				memcpy(&ndef_select[5], FID, sizeof(FID));
				memcpy(&bBufferReader[3], ndef_select, sizeof(ndef_select));

				DEBUG_PRINTF("Sending APDU:\n");
				PRINT_BUFF(&bBufferReader[3], sizeof(message_read));
				status = phpalI14443p4_Exchange(&palI14443p4,PH_EXCHANGE_DEFAULT, &bBufferReader[3],
										sizeof(ndef_select), (void *)&bBufferReader[0], &bRxLength);

				if(!(ppRxBuffer[0][00] == 0x90 && ppRxBuffer[0][01] == 0x00))
				{
					/* Maybe no NDEF message present */
					DEBUG_PRINTF("\n APDU not successful. Error code: ");
					PRINT_BUFF(ppRxBuffer[0], 2);
					DEBUG_PRINTF("\n");
					continue;
				}

				DEBUG_PRINTF("\n********Read NDEF message********\n");

				/* First read the length of the NDEF file */
				OFFSET[0] = 0x00;
				OFFSET[1] = 0x00;
				LENGTH[0] = 0x02;

				uint8_t ndef_read[sizeof(READ_BINARY) + sizeof(OFFSET) + sizeof(LENGTH)];

				memcpy(&ndef_read[0], READ_BINARY, sizeof(READ_BINARY));
				memcpy(&ndef_read[2], OFFSET, sizeof(OFFSET));
				memcpy(&ndef_read[4], LENGTH, sizeof(LENGTH));
				memcpy(&bBufferReader[3], ndef_read, sizeof(ndef_read));

				DEBUG_PRINTF("Sending APDU:\n");
				PRINT_BUFF(&bBufferReader[3], sizeof(message_read));
				status = phpalI14443p4_Exchange(&palI14443p4,PH_EXCHANGE_DEFAULT, &bBufferReader[3],
										  sizeof(ndef_read), (void *)&bBufferReader[0], &bRxLength);


				if(!(ppRxBuffer[0][bRxLength - 2] == 0x90 && ppRxBuffer[0][bRxLength - 1] == 0x00))
				{
					DEBUG_PRINTF("\n APDU not successful. Error code: ");
					PRINT_BUFF(ppRxBuffer[0], 2);
					DEBUG_PRINTF("\n");
					continue;
				}

				DEBUG_PRINTF("\nThe length of the NDEF message is: \n");
				PRINT_BUFF(ppRxBuffer[0], bRxLength - 2);
				DEBUG_PRINTF("\n");


				/* Second read the NDEF file */
				uint8_t counter[2];
				/* MSB */
				counter[0] = ppRxBuffer[0][0];
				/* LSB */
				counter[1] = ppRxBuffer[0][1];

				OFFSET[0] = 0x00;
				OFFSET[1] = 0x02;

				DEBUG_PRINTF("NDEF message: \n");

				while (counter[0] > 0)
				{
					--counter[0];
					LENGTH[0] = 0xFF;

					memcpy(&ndef_read[2], OFFSET, sizeof(OFFSET));
					memcpy(&ndef_read[4], LENGTH, sizeof(LENGTH));
					memcpy(&bBufferReader[3], ndef_read, sizeof(ndef_read));
					status = phpalI14443p4_Exchange(&palI14443p4,PH_EXCHANGE_DEFAULT,
													&bBufferReader[3], sizeof(ndef_read),
													(void *)&bBufferReader[0], &bRxLength);

					if ((counter[0] > 0) || (counter[1] > 0))
						PRINT_BUFF(ppRxBuffer[0], bRxLength);
					else
						PRINT_BUFF(ppRxBuffer[0], bRxLength - 2);

					++OFFSET[0];
				}

				if (counter[1] > 0)
				{
					LENGTH[0] = counter[1];

					memcpy(&ndef_read[2], OFFSET, sizeof(OFFSET));
					memcpy(&ndef_read[4], LENGTH, sizeof(LENGTH));
					memcpy(&bBufferReader[3], ndef_read, sizeof(ndef_read));
					status = phpalI14443p4_Exchange(&palI14443p4,PH_EXCHANGE_DEFAULT,
													&bBufferReader[3], sizeof(ndef_read),
													(void *)&bBufferReader[0], &bRxLength);

					PRINT_BUFF(ppRxBuffer[0], bRxLength - 2);
				}

				DEBUG_PRINTF("\n");
			}

			phpalI14443p4_Deselect(&palI14443p4);
		}
		++count_loops;
    }
    return 0;
}
/******************************************************************************
**                            End Of File
******************************************************************************/
