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

/**\file
 * Declaration of the descriptors used by the HID device keyboard driver.
 */

/** \addtogroup usbd_hid_mouse
 *@{
 */

/*------------------------------------------------------------------------------
 *         Headers
 *------------------------------------------------------------------------------*/
#include "board.h"

#include <USBD_Config.h>

#include <HIDDMouseDriver.h>

/*------------------------------------------------------------------------------
 *         Definitions
 *------------------------------------------------------------------------------*/

/** \addtogroup usbd_hid_mouse_id
 *      @{
 *
 * \section IDs
 * - HIDDMouseDriverDescriptors_PRODUCTID
 * - HIDDMouseDriverDescriptors_VENDORID
 * - HIDDMouseDriverDescriptors_RELEASE
 */

/** Device product ID. */
#define HIDDMouseDriverDescriptors_PRODUCTID       0x6200
/** Device vendor ID. */
#define HIDDMouseDriverDescriptors_VENDORID        0x03EB
/** Device release number. */
#define HIDDMouseDriverDescriptors_RELEASE         0x0100
/**      @}*/

/*------------------------------------------------------------------------------
 *         Internal variables
 *------------------------------------------------------------------------------*/

/** Device descriptor. */

static const USBDeviceDescriptor deviceDescriptor = {

    sizeof(USBDeviceDescriptor),
    USBGenericDescriptor_DEVICE,
    USBDeviceDescriptor_USB2_00,
    HIDDeviceDescriptor_CLASS,
    HIDDeviceDescriptor_SUBCLASS,
    HIDDeviceDescriptor_PROTOCOL,
    CHIP_USB_ENDPOINTS_MAXPACKETSIZE(0),
    HIDDMouseDriverDescriptors_VENDORID,
    HIDDMouseDriverDescriptors_PRODUCTID,
    HIDDMouseDriverDescriptors_RELEASE,
    1, /* Index of manufacturer description */
    2, /* Index of product description */
    3, /* Index of serial number description */
    1  /* One possible configuration */
};

/** Configuration descriptor. */
static const HIDDMouseDriverConfigurationDescriptors configurationDescriptors = {

    /* Configuration descriptor */
    {
        sizeof(USBConfigurationDescriptor),
        USBGenericDescriptor_CONFIGURATION,
        sizeof(HIDDMouseDriverConfigurationDescriptors),
        1, /* One interface in this configuration */
        1, /* This is configuration #1 */
        0, /* No associated string descriptor */
        USBD_BMATTRIBUTES,
        USBConfigurationDescriptor_POWER(100)
    },
    /* Interface descriptor */
    {
        sizeof(USBInterfaceDescriptor),
        USBGenericDescriptor_INTERFACE,
        0, /* This is interface #0 */
        0, /* This is alternate setting #0 */
        1, /* One endpoints used */
        HIDInterfaceDescriptor_CLASS,
        HIDInterfaceDescriptor_SUBCLASS_NONE,
        0, //HIDInterfaceDescriptor_PROTOCOL_MOUSE,
        0  /* No associated string descriptor */
    },
    /* HID descriptor */
    {
        sizeof(HIDDescriptor1),
        HIDGenericDescriptor_HID,
        HIDDescriptor_HID1_11,
        0, /* Device is not localized, no country code */
        1, /* One HID-specific descriptor (apart from this one) */
        HIDGenericDescriptor_REPORT,
        HIDDMouseDriver_REPORTDESCRIPTORSIZE
    },
    /* Interrupt IN endpoint descriptor */
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(
            USBEndpointDescriptor_IN,
            HIDDMouseDriverDescriptors_INTERRUPTIN),
        USBEndpointDescriptor_INTERRUPT,
        sizeof(HIDDJoystickInputReport),
        HIDDMouseDriverDescriptors_INTERRUPTIN_POLLING
    }
};

/**
 *   Variables: String descriptors
 *       languageIdDescriptor - Language ID string descriptor.
 *       manufacturerDescriptor - Manufacturer name.
 *       productDescriptor - Product name.
 *       serialNumberDescriptor - Product serial number.
 *       stringDescriptors - Array of pointers to string descriptors.
 */
static const unsigned char languageIdDescriptor[] = {

    USBStringDescriptor_LENGTH(1),
    USBGenericDescriptor_STRING,
    USBStringDescriptor_ENGLISH_US
};

static const unsigned char manufacturerDescriptor[] = {

    USBStringDescriptor_LENGTH(5),
    USBGenericDescriptor_STRING,
    USBStringDescriptor_UNICODE('E'),
    USBStringDescriptor_UNICODE('R'),
    USBStringDescriptor_UNICODE('S'),
    USBStringDescriptor_UNICODE('K'),
    USBStringDescriptor_UNICODE('Y')
};

static const unsigned char productDescriptor[] = {

    USBStringDescriptor_LENGTH(20),
    USBGenericDescriptor_STRING,
    USBStringDescriptor_UNICODE('E'),
    USBStringDescriptor_UNICODE('R'),
    USBStringDescriptor_UNICODE('S'),
    USBStringDescriptor_UNICODE('K'),
    USBStringDescriptor_UNICODE('Y'),
    USBStringDescriptor_UNICODE('9'),
    USBStringDescriptor_UNICODE('X'),
    USBStringDescriptor_UNICODE(' '),
    USBStringDescriptor_UNICODE('H'),
    USBStringDescriptor_UNICODE('I'),
    USBStringDescriptor_UNICODE('D'),
    USBStringDescriptor_UNICODE(' '),
    USBStringDescriptor_UNICODE('J'),
    USBStringDescriptor_UNICODE('O'),
    USBStringDescriptor_UNICODE('Y'),
    USBStringDescriptor_UNICODE('S'),
    USBStringDescriptor_UNICODE('T'),
    USBStringDescriptor_UNICODE('I'),
    USBStringDescriptor_UNICODE('C'),
    USBStringDescriptor_UNICODE('K'),
};

static const unsigned char serialNumberDescriptor[] = {

    USBStringDescriptor_LENGTH(12),
    USBGenericDescriptor_STRING,
    USBStringDescriptor_UNICODE('0'),
    USBStringDescriptor_UNICODE('1'),
    USBStringDescriptor_UNICODE('2'),
    USBStringDescriptor_UNICODE('3'),
    USBStringDescriptor_UNICODE('4'),
    USBStringDescriptor_UNICODE('5'),
    USBStringDescriptor_UNICODE('6'),
    USBStringDescriptor_UNICODE('7'),
    USBStringDescriptor_UNICODE('8'),
    USBStringDescriptor_UNICODE('9'),
    USBStringDescriptor_UNICODE('A'),
    USBStringDescriptor_UNICODE('F')
};

static const unsigned char *stringDescriptors[] = {

    languageIdDescriptor,
    manufacturerDescriptor,
    productDescriptor,
    serialNumberDescriptor
};

/*------------------------------------------------------------------------------ */
/*         Exported variables */
/*------------------------------------------------------------------------------ */

/** List of descriptors used by the HID keyboard driver. */
USBDDriverDescriptors hiddMouseDriverDescriptors = {

    &deviceDescriptor,
    (USBConfigurationDescriptor *) &configurationDescriptors,
    0, /* No full-speed device qualifier descriptor */
    0, /* No full-speed other speed configuration */
    0, /* No high-speed device descriptor */
    0, /* No high-speed configuration descriptor */
    0, /* No high-speed device qualifier descriptor */
    0, /* No high-speed other speed configuration descriptor */
    stringDescriptors,
    4 /* Four string descriptors in list */
};

/**@}*/
