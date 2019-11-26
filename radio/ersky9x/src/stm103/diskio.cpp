/*
 * Authors (alphabetical order)
 * - Andre Bernet <bernet.andre@gmail.com>
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Jean-Pierre Parisy
 * - Karl Szmutny <shadow@privy.de>
 * - Michael Blandford
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
 *
 * open9x is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <string.h>
#include "../ersky9x.h"
#include "../CoOS.h"
#include "../diskio.h"
#include "../ff.h"
//#include "stm32f4xx_gpio.h"
//#include "stm32f4xx_spi.h"
//#include "stm32f4xx_rcc.h"
#include "hal.h"
#include "stm32_sdio_sd.h"
#include "logicio103.h"

/* Definitions for MMC/SDC command */
#define CMD0    (0x40+0)        /* GO_IDLE_STATE */
#define CMD1    (0x40+1)        /* SEND_OP_COND (MMC) */
#define ACMD41  (0xC0+41)       /* SEND_OP_COND (SDC) */
#define CMD8    (0x40+8)        /* SEND_IF_COND */
#define CMD9    (0x40+9)        /* SEND_CSD */
#define CMD10   (0x40+10)       /* SEND_CID */
#define CMD12   (0x40+12)       /* STOP_TRANSMISSION */
#define ACMD13  (0xC0+13)       /* SD_STATUS (SDC) */
#define CMD16   (0x40+16)       /* SET_BLOCKLEN */
#define CMD17   (0x40+17)       /* READ_SINGLE_BLOCK */
#define CMD18   (0x40+18)       /* READ_MULTIPLE_BLOCK */
#define CMD23   (0x40+23)       /* SET_BLOCK_COUNT (MMC) */
#define ACMD23  (0xC0+23)       /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24   (0x40+24)       /* WRITE_BLOCK */
#define CMD25   (0x40+25)       /* WRITE_MULTIPLE_BLOCK */
#define CMD55   (0x40+55)       /* APP_CMD */
#define CMD58   (0x40+58)       /* READ_OCR */

/* Card-Select Controls  (Platform dependent) */
#define SELECT()        GPIO_ResetBits(GPIO_SPI_SD, GPIO_Pin_SPI_SD_CS)    /* MMC CS = L */
#define DESELECT()      GPIO_SetBits(GPIO_SPI_SD, GPIO_Pin_SPI_SD_CS)      /* MMC CS = H */

#if (_MAX_SS != 512) || (_FS_READONLY == 0) || (STM32_SD_DISK_IOCTRL_FORCE == 1)
#define STM32_SD_DISK_IOCTRL   1
#else
#define STM32_SD_DISK_IOCTRL   0
#endif

#define BLOCK_SIZE	512

#define BOOL   bool
//#define FALSE  false
//#define TRUE   true

/* Card type flags (CardType) */
#define CT_MMC              0x01
#define CT_SD1              0x02
#define CT_SD2              0x04
#define CT_SDC              (CT_SD1|CT_SD2)
#define CT_BLOCK            0x08

uint32_t Card_state; //  = SD_ST_STARTUP ;
/*uint32_t Card_ID[4] ;
uint32_t Card_SCR[2] ;
uint32_t Card_CSD[4] ;
volatile uint32_t Card_initialized = 0;
uint32_t Sd_rca ;
uint32_t Cmd_A41_resp ;
uint8_t  cardType;
uint32_t transSpeed; */

/*-----------------------------------------------------------------------*/
/* Lock / unlock functions                                               */
/*-----------------------------------------------------------------------*/

//void where( uint8_t chr ) ;

static _SYNC_t s_sync ;
static uint8_t s_sync_set ;

int ff_cre_syncobj (BYTE vol, _SYNC_t *mutex)
{
	if ( s_sync_set == 0 )
	{
  	s_sync = CoCreateMutex();
		s_sync_set = 1 ;
	}
  *mutex = s_sync ;
  return 1;
}

int ff_req_grant (_SYNC_t mutex)
{
  CoEnterMutexSection(mutex);
  return 1;
}

void ff_rel_grant (_SYNC_t mutex)
{
  CoLeaveMutexSection(mutex);
}

int ff_del_syncobj (_SYNC_t mutex)
{
  return 1;
}

static const DWORD socket_state_mask_cp = (1 << 0);
static const DWORD socket_state_mask_wp = (1 << 1);

static volatile
DSTATUS Stat = STA_NOINIT;      /* Disk status */

static volatile
DWORD Timer1, Timer2;   /* 100Hz decrement timers */

//static
BYTE CardType;                  /* Card type flags */

enum speed_setting { INTERFACE_SLOW, INTERFACE_FAST };

DSTATUS disk_initialize ( BYTE drv ) /* Physical drive nmuber (0..) */
{
  DSTATUS stat = 0;

  /* Supports only single drive */
  if (drv)
  {
    stat |= STA_NOINIT;
  }

  /*-------------------------- SD Init ----------------------------- */
//  SD_Error res = SD_Init();
//  if (res != SD_OK)
//  {
//    stat |= STA_NOINIT;
//  }

  return(stat);
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status ( BYTE drv )   /* Physical drive number (0) */
{
  DSTATUS stat = 0;

  if (SD_Detect() != SD_PRESENT)
    stat |= STA_NODISK;

  // STA_NOTINIT - Subsystem not initailized
  // STA_PROTECTED - Write protected, MMC/SD switch if available

  return(stat);
}

uint32_t sdReadRetries = 0;

DRESULT disk_read_dma(BYTE drv, BYTE * buff, DWORD sector, UINT count)
{
  // this functions assumes that buff is properly aligned and in the right RAM segment for DMA
  DRESULT res;
  SD_Error Status;
  SDTransferState State;
  for (int retry=0; retry<3; retry++)
	{
    res = RES_OK;
    if (count == 1)
		{
      Status = SD_ReadBlock(buff, sector, BLOCK_SIZE); // 4GB Compliant
			if ( Status == SD_UNSUPPORTED_FEATURE )
			{
				return (DRESULT)5000 ;
			}
    }
    else
		{
      Status = SD_ReadMultiBlocks(buff, sector, BLOCK_SIZE, count); // 4GB Compliant
    }
    if (Status == SD_OK)
		{
//      Status = SD_WaitReadOperation(200*count); // Check if the Transfer is finished
      Status = SD_WaitReadOperation(); // Check if the Transfer is finished
      while ((State = SD_GetStatus()) == SD_TRANSFER_BUSY); // BUSY, OK (DONE), ERROR (FAIL)
      if (State == SD_TRANSFER_ERROR)
			{
//	      TRACE("State=SD_TRANSFER_ERROR, c: %u", sector, (uint32_t)count);
        res = RES_ERROR;
      }
      else if (Status != SD_OK)
			{
//        TRACE("Status(WaitRead)=%d, s:%u c: %u", Status, sector, (uint32_t)count);
        res = RES_ERROR;
      }
    }
    else
		{
//      TRACE("Status(ReadBlock)=%d, s:%u c: %u", Status, sector, (uint32_t)count);
      res = RES_ERROR;
    }
    if (res == RES_OK) break;
    sdReadRetries += 1;
  }
  return res;
}

DWORD scratch[BLOCK_SIZE / 4] ;

DRESULT disk_read(BYTE drv, BYTE * buff, DWORD sector, BYTE count)
{
  // If unaligned, do the single block reads with a scratch buffer.
  // If aligned and single sector, do a single block read.
  // If aligned and multiple sectors, try multi block read.
  //    If multi block read fails, try single block reads without
  //    an intermediate buffer (move trough the provided buffer)

  // TRACE("disk_read %d %p %10d %d", drv, buff, sector, count);
  if (SD_Detect() != SD_PRESENT)
	{
//    TRACE("SD_Detect() != SD_PRESENT");
    return RES_NOTRDY;
  }

  DRESULT res = RES_OK;
  if (count == 0) return res;

  if ((DWORD)buff < 0x20000000 || ((DWORD)buff & 3))
	{
    // buffer is not aligned, use scratch buffer that is aligned
//    TRACE("disk_read bad alignment (%p)", buff);
    while (count--)
		{
      res = disk_read_dma(drv, (BYTE *)scratch, sector++, 1);
      if (res != RES_OK) break;
      memcpy(buff, scratch, BLOCK_SIZE);
      buff += BLOCK_SIZE;
    }
    return res;
  }


  res = disk_read_dma(drv, buff, sector, count);
  if (res != RES_OK && count > 1)
	{
    // multi-read failed, try reading same sectors, one by one
//    TRACE("disk_read() multi-block failed, trying single block reads...");
    while (count--)
		{
      res = disk_read_dma(drv, buff, sector++, 1);
      if (res != RES_OK) break;
      buff += BLOCK_SIZE;
    }
  }
  return res;
}

DRESULT disk_write(
  BYTE drv,                       /* Physical drive nmuber (0..) */
  const BYTE *buff,               /* Data to be written */
  DWORD sector,                   /* Sector address (LBA) */
  BYTE count                      /* Number of sectors to write (1..255) */
)
{
  SD_Error Status;
  DRESULT res = RES_OK;

  // TRACE("disk_write %d %p %10d %d", drv, buff, sector, count);

  if (SD_Detect() != SD_PRESENT)
    return(RES_NOTRDY);

  if ((DWORD)buff < 0x20000000 || ((DWORD)buff & 3)) {
//    TRACE("disk_write bad alignment (%p)", buff);
    while(count--) {
      memcpy(scratch, buff, BLOCK_SIZE);

      res = disk_write(drv, (BYTE *)scratch, sector++, 1);

      if (res != RES_OK)
        break;

      buff += BLOCK_SIZE;
    }
    return(res);
  }

  if (count == 1) {
    Status = SD_WriteBlock((uint8_t *)buff, sector, BLOCK_SIZE); // 4GB Compliant
  }
  else {
    Status = SD_WriteMultiBlocks((uint8_t *)buff, sector, BLOCK_SIZE, count); // 4GB Compliant
  }

  if (Status == SD_OK) {
    SDTransferState State;

//    Status = SD_WaitWriteOperation(500*count); // Check if the Transfer is finished
    Status = SD_WaitWriteOperation(); // Check if the Transfer is finished

    while((State = SD_GetStatus()) == SD_TRANSFER_BUSY); // BUSY, OK (DONE), ERROR (FAIL)

    if ((State == SD_TRANSFER_ERROR) || (Status != SD_OK)) {
//      TRACE("__disk_write() err, st:%d,%d, s:%u c: %u", Status, State, sector, (uint32_t)count);
      res = RES_ERROR;
    }
  }
  else {
    res = RES_ERROR;
  }

  // TRACE("result=%d", res);
  return res;
}

DRESULT disk_ioctl (
  BYTE drv,               /* Physical drive nmuber (0..) */
  BYTE ctrl,              /* Control code */
  BYTE *buff              /* Buffer to send/receive control data */
)
{
  DRESULT res;

  if (drv) return RES_PARERR;

  res = RES_ERROR;

  switch (ctrl) {
    case GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
      // use 512 for sector size, SDCardInfo.CardBlockSize is not sector size and can be 1024 for 2G SD cards!!!!
      *(DWORD*)buff = SDCardInfo.CardCapacity / BLOCK_SIZE;
      res = RES_OK;
      break;

    case GET_SECTOR_SIZE :  /* Get R/W sector size (WORD) */
      *(WORD*)buff = BLOCK_SIZE;   // force sector size. SDCardInfo.CardBlockSize is not sector size and can be 1024 for 2G SD cards!!!!
      res = RES_OK;
      break;

    case GET_BLOCK_SIZE :   /* Get erase block size in unit of sector (DWORD) */
      // TODO verify that this is the correct value
      *(DWORD*)buff = (uint32_t)SDCardInfo.SD_csd.EraseGrSize * (uint32_t)SDCardInfo.SD_csd.EraseGrMul;
      res = RES_OK;
      break;

    case CTRL_SYNC:
      while (SD_GetStatus() == SD_TRANSFER_BUSY); /* Complete pending write process (needed at _FS_READONLY == 0) */
      res = RES_OK;
      break;

    default:
      res = RES_OK;
      break;

  }

  return res;
}



static inline DWORD socket_is_write_protected(void)
{
        return 0; /* fake not protected */
}

//static void socket_cp_init(void)
//{
//        return;
//}

DWORD socket_is_empty(void)
{
        return 0; /* fake inserted */
//	return SD_PRESENT_GPIO->IDR & SD_PRESENT_GPIO_PIN ;
}

//static void card_power(BYTE on)
//{
//        on=on;
//}

////#if (STM32_SD_DISK_IOCTRL == 1)
//static int chk_power(void)
//{
//        return 1; /* fake powered */
//}
////#endif

///*-----------------------------------------------------------------------*/
///* Transmit/Receive a byte to MMC via SPI  (Platform dependent)          */
///*-----------------------------------------------------------------------*/
//static BYTE stm32_spi_rw( BYTE out )
//{
//        /* Loop while DR register in not empty */
//        /// not needed: while (SPI_I2S_GetFlagStatus(SPI_SD, SPI_I2S_FLAG_TXE) == RESET) { ; }

//        /* Send byte through the SPI peripheral */
//        SPI_I2S_SendData(SPI_SD, out);

//        /* Wait to receive a byte */
//        while (SPI_I2S_GetFlagStatus(SPI_SD, SPI_I2S_FLAG_RXNE) == RESET) { ; }

//        /* Return the byte read from the SPI bus */
//        return SPI_I2S_ReceiveData(SPI_SD);
//}


/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure  (Platform dependent)                */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 10ms                        */

void sdPoll10mS()
{
        static DWORD pv;
        DWORD ns;
        BYTE n, s;


        n = Timer1;                /* 100Hz decrement timers */
        if (n) Timer1 = --n;
        n = Timer2;
        if (n) Timer2 = --n;

        ns = pv;
        pv = socket_is_empty() | socket_is_write_protected();   /* Sample socket switch */

        if (ns == pv) {                         /* Have contacts stabled? */
                s = Stat;

                if (pv & socket_state_mask_wp)      /* WP is H (write protected) */
                        s |= STA_PROTECT;
                else                                /* WP is L (write enabled) */
                        s &= ~STA_PROTECT;

                if (pv & socket_state_mask_cp)      /* INS = H (Socket empty) */
                        s |= (STA_NODISK | STA_NOINIT);
                else                                /* INS = L (Card inserted) */
                        s &= ~STA_NODISK;

                Stat = s;
        }
}

extern FATFS g_FATFS ;

void sdInit()
{
	FRESULT fr ;
	if ( socket_is_empty() )
	{
		return ;
	}
  if (( fr = f_mount(0, &g_FATFS)) == FR_OK)
	{
		
//    refreshSystemAudioFiles() ;
    Card_state = 100 ;
	}
}

uint32_t sdMounted()
{
	// Should check for card present as well
	if ( socket_is_empty() )
	{
		return 0 ;
	}
  return Card_state == 100 ;
}

//void sdMountPoll()
//{
//  /* TODO a define
//  static uint8_t mountTimer;
//  if (mountTimer-- == 0) {
//    mountTimer = 100;
//    if (!sdMounted()) {
//      sdInit();
//    }
//  }*/
//}


// Checks for card ready for read/write
// returns 1 for YES, 0 for NO
uint32_t sd_card_ready( void )
{
//  if ( CardIsPresent() )
//  {
    if ( Card_state == 100 )
    {
      return 1 ;
    }
//  }
  return 0 ;
}


