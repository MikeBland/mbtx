/*-----------------------------------------------------------------------*/
/* PFF - transformation of original module for ATMega328                 */
/* Need to be configured of CS pin (in original design is PB0)           */
/* Need proper configuration of SPI interface and ports                  */
/*                                                                       */
/* ORIGINAL COPYRIGHT:                                                   */
/* PFF - Low level disk control module for ATtiny861    (C)ChaN, 2010    */
/*-----------------------------------------------------------------------*/

#include <avr/io.h>
#include "diskio.h"

/* SPI control functions (defined in asmfunc.S) */
void xmit_spi (BYTE);
BYTE rcv_spi (void);
void fwd_blk_part(void*, WORD, WORD);

static inline uint8_t yrcv_spi()
{
	SPDR = 0xFF ;
	while(( SPSR & 0x80 ) == 0 )
		;
	return SPDR ;
}


/* Definitions for MMC/SDC command */
#define CMD0	(0x40+0)	/* GO_IDLE_STATE */
#define CMD1	(0x40+1)	/* SEND_OP_COND (MMC) */
#define	ACMD41	(0xC0+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(0x40+8)	/* SEND_IF_COND */
#define CMD16	(0x40+16)	/* SET_BLOCKLEN */
#define CMD17	(0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD24	(0x40+24)	/* WRITE_BLOCK */
#define CMD55	(0x40+55)	/* APP_CMD */
#define CMD58	(0x40+58)	/* READ_OCR */


/* Port Controls  (Platform dependent) */
#define SELECT()	PORTB &= ~_BV(2)	/* PB2: MMC CS = L */
#define	DESELECT()	PORTB |=  _BV(2)	/* PB2: MMC CS = H */

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

BYTE CardType;


/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void release_spi (void)
{
	DESELECT();
	yrcv_spi();
}


/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
BYTE send_cmd (
	BYTE cmd,		/* Command byte */
	DWORD arg		/* Argument */
)
{
	BYTE n, res;


	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready */
	DESELECT();
	yrcv_spi();
	SELECT();
	yrcv_spi();

	/* Send command packet */
	xmit_spi(cmd);						/* Start + Command index */
	xmit_spi((BYTE)(arg >> 24));		/* Argument[31..24] */
	xmit_spi((BYTE)(arg >> 16));		/* Argument[23..16] */
	xmit_spi((BYTE)(arg >> 8));			/* Argument[15..8] */
	xmit_spi((BYTE)arg);				/* Argument[7..0] */
	n = 0x01;							/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;			/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;			/* Valid CRC for CMD8(0x1AA) */
	xmit_spi(n);

	/* Receive command response */
	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do {
		res = yrcv_spi();
	} while ((res & 0x80) && --n);

	return res;			/* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (void)
{
	BYTE n, cmd, ty, ocr[4];
	WORD t;


//	USIPP = 0b00000001;	/* Attach USI to PORTA */
//	USICR = 0b00001000;	/* Enable the USI. DO pin is controlled by software */

//	SPCR = 0b01010000;
	SPCR = 0b01010000;
#if F_CPU == 12000000L
	SPSR = _BV(0);
#else
	SPSR = 0 ;
#endif


	for (t = 10; t; t--) yrcv_spi();	/* Dummy clocks */
//	yrcv_spi();	/* Dummy clock */
	SELECT();
	for (t = 600; t; t--) yrcv_spi();	/* Dummy clocks */
	ty = 0;
	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
//	if (1 == 1) {			/* Enter Idle state */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDv2 */
			for (n = 0; n < 4; n++) ocr[n] = yrcv_spi();		/* Get trailing return value of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {				/* The card can work at vdd range of 2.7-3.6V */
				for (t = 25000; t && send_cmd(ACMD41, 1UL << 30); t--) ;	/* Wait for leaving idle state (ACMD41 with HCS bit) */
				if (t && send_cmd(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = yrcv_spi();
					ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;	/* SDv2 */
				}
			}
		} else {							/* SDv1 or MMC */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = CT_SD1; cmd = ACMD41;	/* SDv1 */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMCv3 */
			}
			for (t = 25000; t && send_cmd(cmd, 0); t--) ;	/* Wait for leaving idle state */
			if (!t || send_cmd(CMD16, 512) != 0) {			/* Set R/W block length to 512 */
				ty = 0;
			}
		}
	}
	CardType = ty;
	release_spi();

	return ty ? 0 : STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read partial sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp (
	void *dest,		/* Pointer to the destination object to put data */
	DWORD lba,		/* Start sector number (LBA) */
	WORD ofs,		/* Byte offset in the sector (0..511) */
	WORD cnt		/* Byte count (1..512), b15:destination flag */
)
{
	DRESULT res;
	volatile static BYTE rc;
	WORD t;


	if (!(CardType & CT_BLOCK)) lba *= 512;		/* Convert LBA to BA if needed */

	res = RES_ERROR;
	if (send_cmd(CMD17, lba) == 0) {		/* READ_SINGLE_BLOCK */

		t = 30000;
		do {							/* Wait for data packet in timeout of 100ms */
			rc = yrcv_spi();
		} while (rc == 0xFF && --t);

		if (rc == 0xFE) {
			fwd_blk_part(dest, ofs, cnt);
			res = RES_OK;
		}
	}

	release_spi();

	return res;
}

//DRESULT disk_read (
//        BYTE drv,                       /* Physical drive number (0) */
//        BYTE *buff,                     /* Pointer to the data buffer to store read data */
//        DWORD sector,           /* Start sector number (LBA) */
//        BYTE count                      /* Sector count (1..255) */
////	void *dest,		/* Pointer to the destination object to put data */
////	DWORD lba,		/* Start sector number (LBA) */
////	WORD ofs,		/* Byte offset in the sector (0..511) */
////	WORD cnt		/* Byte count (1..512), b15:destination flag */
//)
//{
//	DRESULT res;
//	volatile static BYTE rc;
//	WORD t;


//	if (!(CardType & CT_BLOCK)) sector *= 512;		/* Convert LBA to BA if needed */

//	res = RES_ERROR;
//	if (send_cmd(CMD17, sector) == 0) {		/* READ_SINGLE_BLOCK */

//		t = 30000;
//		do {							/* Wait for data packet in timeout of 100ms */
//			rc = yrcv_spi();
//		} while (rc == 0xFF && --t);

//		if (rc == 0xFE)
//		{
//			for ( t = 0 ; t < 512 ; t += 1 )
//			{
//				rc = yrcv_spi();
//				*buff++ = rc ;
//			}
//			res = RES_OK;
//		}
//	}

//	release_spi();

//	return res;
//}

