/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/** \file
 * \addtogroup usbd_hid_mouse
 *@{
 */

/*------------------------------------------------------------------------------
 *         Headers
 *------------------------------------------------------------------------------*/


#include <HIDDFunction.h>
#include <HIDDMouseDriver.h>

#include <USBD.h>
#include <USBD_HAL.h>
#include <USBDDriver.h>

#include "AT91SAM3S4.h"
#include "core_cm3.h"
#include <pmc.h>

//#include <USBLib_Trace.h>

//void txmit( unsigned char c )
//{
//  Uart *pUart = UART0 ;
//  while ( (pUart->UART_SR & UART_SR_TXEMPTY) == 0 ) ;
//  pUart->UART_THR = c ;
//}

//unsigned long T2020 ;

//void checkT2020(void)
//{
//	unsigned long value ;
	
//	value = *( ( unsigned long *)0x20002020 ) ;
//	if ( value != T2020 )
//	{
//		txmit('X') ;
//	}
//}

//// Send a single 4 bit value to the RS232 port as a hex digit
//void hex_digit_send( unsigned char c )
//{
//	c &= 0x0F ;
//	if ( c > 9 )
//	{
//		c += 7 ;
//	}
//	c += '0' ;
//	txmit( c ) ;
//}

//// Send the 8 bit value to the RS232 port as 2 hex digits
//void p2hex( unsigned char c )
//{
////	asm("swap %c") ;
//	hex_digit_send( c >> 4 ) ;
////	asm("swap %c") ;
//	hex_digit_send( c ) ;
//}

//// Send the 16 bit value to the RS232 port as 4 hex digits
//void p4hex( unsigned int value )
//{
//	p2hex( value >> 8 ) ;
//	p2hex( value ) ;
//}

//void p8hex( unsigned long value )
//{
//	p4hex( value >> 16 ) ;
//	p4hex( value ) ;
//}


///*------------------------------------------------------------------------------
// *         Internal Defines
// *------------------------------------------------------------------------------*/

/** Tag bit (Always 1) */
#define HIDDMouse_TAG       (1 << 3)
/** Xsign bit */
#define HIDDMouse_Xsign     (1 << 4)
/** Ysign bit */
#define HIDDMouse_Ysign     (1 << 5)

///*------------------------------------------------------------------------------
// *         Internal types
// *------------------------------------------------------------------------------*/

/**
 * Struct for an HID Mouse report.
 */
typedef struct _HIDDMouseReport {
    /** Callback when report done */
    HIDDReportEventCallback fCallback;
    /** Callback arguments */
    void* pArg;

    /** Report size (ID + DATA) */
    uint16_t wMaxSize;
    /** Transfered size */
    uint16_t wTransferred;
    /** Report idle rate */
    uint8_t bIdleRate;
    /** Delay count for Idle */
    uint8_t bDelay;
    /** Report ID */
    uint8_t bID;
    /** Report data block */
    HIDDJoystickInputReport report;
} HIDDMouseReport;

/**
 * Driver structure for an HID device implementing keyboard functionalities.
 */
typedef struct _HIDDMouseDriver {

    /** Mouse function instance */
    HIDDFunction hidDrv;
    /** Mouse input report */
    HIDDReport   *inputReports[1];
} HIDDMouseDriver;

/*------------------------------------------------------------------------------
 *         Internal variables
 *------------------------------------------------------------------------------*/

/** Static instance of the HID mouse device driver. */
/*static*/ HIDDMouseDriver hiddMouseDriver;
/** Input report */
/*static*/ HIDDMouseReport hiddInputReport;

/** Report descriptor used by the driver. */
/*static*/ const uint8_t hiddReportDescriptor[] =
{
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    //     USAGE (Game Pad)
    0xa1, 0x01,                    //     COLLECTION (Application)
    0xa1, 0x00,                    //       COLLECTION (Physical)
    0x05, 0x09,                    //         USAGE_PAGE (Button)
    0x19, 0x01,                    //         USAGE_MINIMUM (Button 1)
    0x29, 0x08,                    //         USAGE_MAXIMUM (Button 8)
    0x15, 0x00,                    //         LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //         LOGICAL_MAXIMUM (1)
    0x95, 0x08,                    //         REPORT_COUNT (8)
    0x75, 0x01,                    //         REPORT_SIZE (1)
    0x81, 0x02,                    //         INPUT (Data,Var,Abs)
    0x05, 0x01,                    //         USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //         USAGE (X)
    0x09, 0x31,                    //         USAGE (Y)
    0x09, 0x32,                    //         USAGE (Z)
    0x09, 0x33,                    //         USAGE (Rx)
    0x09, 0x34,                    //         USAGE (Ry)
    0x09, 0x35,                    //         USAGE (Rz)
    0x09, 0x36,                    //         USAGE (Slider)
    0x09, 0x36,                    //         USAGE (Slider)
    0x15, 0x81,                    //         LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //         LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //         REPORT_SIZE (8)
    0x95, 0x08,                    //         REPORT_COUNT (8)
    0x81, 0x02,                    //         INPUT (Data,Var,Abs)
    0xc0,                          //       END_COLLECTION
    0xc0                           //     END_COLLECTION
	
//    /* Global Usage Page */
//    HIDReport_GLOBAL_USAGEPAGE + 1, HIDGenericDesktop_PAGEID,
//    /* Collection: Application */
//    HIDReport_LOCAL_USAGE + 1, HIDGenericDesktop_MOUSE,
//    HIDReport_COLLECTION + 1, HIDReport_COLLECTION_APPLICATION,
//        /* Physical collection: Pointer */
//        HIDReport_LOCAL_USAGE + 1, HIDGenericDesktop_POINTER,
//        HIDReport_COLLECTION + 1, HIDReport_COLLECTION_PHYSICAL,

//            /* Input report: buttons */
//            HIDReport_GLOBAL_USAGEPAGE + 1, HIDButton_PAGEID,

//            HIDReport_GLOBAL_REPORTCOUNT + 1, 3,
//            HIDReport_GLOBAL_REPORTSIZE + 1, 1,
//            HIDReport_LOCAL_USAGEMINIMUM + 1, 1,
//            HIDReport_LOCAL_USAGEMAXIMUM + 1, 3,
//            HIDReport_GLOBAL_LOGICALMINIMUM + 1, 0,
//            HIDReport_GLOBAL_LOGICALMAXIMUM + 1, 1,
//            HIDReport_INPUT + 1, HIDReport_VARIABLE,    /* 3 button bits */

//            /* Input report: padding */
//            HIDReport_GLOBAL_REPORTCOUNT + 1, 1,
//            HIDReport_GLOBAL_REPORTSIZE + 1, 5,
//            HIDReport_INPUT + 1, HIDReport_CONSTANT,    /* 5 bit padding */

//            /* Input report: pointer */
//            HIDReport_GLOBAL_USAGEPAGE + 1, HIDGenericDesktop_PAGEID,
//            HIDReport_GLOBAL_REPORTSIZE + 1, 8,
//            HIDReport_GLOBAL_REPORTCOUNT + 1, 2,
//            HIDReport_LOCAL_USAGE + 1, HIDGenericDesktop_X,
//            HIDReport_LOCAL_USAGE + 1, HIDGenericDesktop_Y,
//            HIDReport_GLOBAL_LOGICALMINIMUM + 1, (uint8_t) -127,
//            HIDReport_GLOBAL_LOGICALMAXIMUM + 1, 127,
//            HIDReport_INPUT + 1, HIDReport_VARIABLE | HIDReport_RELATIVE,

//        HIDReport_ENDCOLLECTION,
//    HIDReport_ENDCOLLECTION
};

/*------------------------------------------------------------------------------
 *         Internal functions
 *------------------------------------------------------------------------------*/

/**
 * \brief Configure 48MHz Clock for USB
 */
static void _ConfigureUsbClock(void)
{
    /* Enable PLLB for USB */
    PMC->CKGR_PLLBR = CKGR_PLLBR_DIVB(1)
                    | CKGR_PLLBR_MULB(7)
                    | CKGR_PLLBR_PLLBCOUNT_Msk;
    while((PMC->PMC_SR & PMC_SR_LOCKB) == 0);
    /* USB Clock uses PLLB */
    PMC->PMC_USB = PMC_USB_USBDIV(1)    /* /2   */
                 | PMC_USB_USBS;        /* PLLB */
}

/*----------------------------------------------------------------------------
 *         External variables
 *----------------------------------------------------------------------------*/

extern USBDDriverDescriptors hiddMouseDriverDescriptors;


/*------------------------------------------------------------------------------
 *      Exported functions
 *------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *         Callbacks re-implementation
 *----------------------------------------------------------------------------*/

/**
 * Invoked after the USB driver has been initialized. By default, configures
 * the UDP/UDPHS interrupt.
 */
void USBDCallbacks_Initialized(void)
{
    NVIC_EnableIRQ(UDP_IRQn);
}

/**
 *  Invoked whenever a SETUP request is received from the host. Forwards the
 *  request to the standard handler.
 */
void USBDCallbacks_RequestReceived(const USBGenericRequest *request)
{
    HIDDMouseDriver_RequestHandler(request);
}

/**
 * Invoked when the configuration of the device changes. Start reading
 * output reports.
 * \param cfgnum New configuration number.
 */
void USBDDriverCallbacks_ConfigurationChanged(uint8_t cfgnum)
{
    HIDDMouseDriver_ConfigurationChangedHandler(cfgnum);
}

/**
 * Initializes the HID Mouse %device driver.
 * \param pDescriptors Pointer to descriptor list for the HID Mouse.
 */
void HIDDMouseDriver_Initialize(const USBDDriverDescriptors *pDescriptors)
{
    HIDDMouseDriver* pMouse = &hiddMouseDriver;
    HIDDFunction* pHidd = &pMouse->hidDrv;
    USBDDriver* pUsbd = USBD_GetDriver();

    /* One input report */
    pMouse->inputReports[0] = (HIDDReport*)&hiddInputReport;
    HIDDFunction_InitializeReport(pMouse->inputReports[0],
                                  HIDDMouseDriver_REPORTDESCRIPTORSIZE,
                                  0,
                                  0, 0);

    /* Initialize USBD Driver instance */
    USBDDriver_Initialize(pUsbd,
                          pDescriptors,
                          0); /* Multiple interface settings not supported */
    /* Function initialize */
    HIDDFunction_Initialize(pHidd,
                            pUsbd, 0,
                            hiddReportDescriptor,
                            (HIDDReport**)(&pMouse->inputReports), 1,
                            0, 0);
    USBD_Init();
}

/**
 * Handles configureation changed event.
 * \param cfgnum New configuration number
 */
void HIDDMouseDriver_ConfigurationChangedHandler(uint8_t cfgnum)
{
    HIDDMouseDriver * pMouse = &hiddMouseDriver;
    HIDDFunction * pHidd = &pMouse->hidDrv;
    USBDDriver * pUsbd = pHidd->pUsbd;
    USBConfigurationDescriptor *pDesc;

    if (cfgnum > 0) {

        /* Parse endpoints for reports */
        pDesc = USBDDriver_GetCfgDescriptors(pUsbd, cfgnum);
        HIDDFunction_ParseInterface(pHidd,
                                    (USBGenericDescriptor*)pDesc,
                                    pDesc->wTotalLength);
    }
}

/**
 * Handles HID-specific SETUP request sent by the host.
 * \param request Pointer to a USBGenericRequest instance
 */
void HIDDMouseDriver_RequestHandler(const USBGenericRequest *request)
{
    HIDDMouseDriver * pMouse = &hiddMouseDriver;
    HIDDFunction * pHidd = &pMouse->hidDrv;
    USBDDriver * pUsbd = pHidd->pUsbd;
    
//    TRACE_INFO("NewReq ");

    /* Process HID requests */
    if (USBRC_SUCCESS == HIDDFunction_RequestHandler(pHidd,
                                                     request)) {
        return;
    }
    /* Process STD requests */
    else {

        USBDDriver_RequestHandler(pUsbd, request);
    }

}

/**
 * Update the Mouse button status and location changes via input report
 * to host
 * \param bmButtons Bit map of the button status
 * \param deltaX Movment on X direction
 * \param deltaY Movment on Y direction
 */
uint8_t HIDDJoystickDriver_Change( uint8_t *data )
{
	uint32_t i ;
  HIDDMouseDriver * pMouse = &hiddMouseDriver;
  HIDDFunction * pHidd = &pMouse->hidDrv;
  HIDDJoystickInputReport * pReport = &hiddInputReport.report;

  pReport->bmButtons = *data++ ;
	for ( i = 0 ; i < 8 ; i += 1 )
	{
  	pReport->analog[i] = *data++ ;
	}
  /* Send input report through the interrupt IN endpoint */
//	txmit('A') ;
  return USBD_Write(pHidd->bPipeIN,
                      (void*)pReport,
                      9, //sizeof(hiddInputReport),
                      0,
                      0);
}

/**
 * Starts a remote wake-up sequence if the host has explicitely enabled it
 * by sending the appropriate SET_FEATURE request.
 */
void HIDDMouseDriver_RemoteWakeUp(void)
{
    HIDDMouseDriver * pMouse = &hiddMouseDriver;
    HIDDFunction * pHidd = &pMouse->hidDrv;
    USBDDriver * pUsbd = pHidd->pUsbd;

    /* Remote wake-up has been enabled */
    if (USBDDriver_IsRemoteWakeUpEnabled(pUsbd)) {

        USBD_RemoteWakeUp();
    }
}

void startJoystick(void)
{
	/* Enable UPLL for USB */
	_ConfigureUsbClock();
//	T2020 = *( ( unsigned long *)0x20002020 ) ;
	
	HIDDMouseDriver_Initialize(&hiddMouseDriverDescriptors);

	 
}


/**@}*/

