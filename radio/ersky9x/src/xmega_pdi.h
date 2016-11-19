#ifndef _XMEGA_PDI_H_
#define _XMEGA_PDI_H_

#define XNVM_PDI_LDS_INSTR    0x00 //!< LDS instruction.
#define XNVM_PDI_STS_INSTR    0x40 //!< STS instruction.
#define XNVM_PDI_LD_INSTR     0x20 //!< LD instruction.
#define XNVM_PDI_ST_INSTR     0x60 //!< ST instruction.
#define XNVM_PDI_LDCS_INSTR   0x80 //!< LDCS instruction.
#define XNVM_PDI_STCS_INSTR   0xC0 //!< STCS instruction.
#define XNVM_PDI_REPEAT_INSTR 0xA0 //!< REPEAT instruction.
#define XNVM_PDI_KEY_INSTR    0xE0 //!< KEY instruction.

/** Byte size address mask for LDS and STS instruction */
#define XNVM_PDI_BYTE_ADDRESS_MASK 0x00
/** Word size address mask for LDS and STS instruction */
#define XNVM_PDI_WORD_ADDRESS_MASK 0x04
/** 3 bytes size address mask for LDS and STS instruction */
#define XNVM_PDI_3BYTES_ADDRESS_MASK 0x08
/** Long size address mask for LDS and STS instruction */
#define XNVM_PDI_LONG_ADDRESS_MASK 0x0C
/** Byte size data mask for LDS and STS instruction */
#define XNVM_PDI_BYTE_DATA_MASK 0x00
/** Word size data mask for LDS and STS instruction */
#define XNVM_PDI_WORD_DATA_MASK 0x01
/** 3 bytes size data mask for LDS and STS instruction */
#define XNVM_PDI_3BYTES_DATA_MASK 0x02
/** Long size data mask for LDS and STS instruction */
#define XNVM_PDI_LONG_DATA_MASK 0x03
/** Byte size address mask for LDS and STS instruction */
#define XNVM_PDI_LD_PTR_STAR_MASK 0x00
/** Word size address mask for LDS and STS instruction */
#define XNVM_PDI_LD_PTR_STAR_INC_MASK 0x04
/** 3 bytes size address mask for LDS and STS instruction */
#define XNVM_PDI_LD_PTR_ADDRESS_MASK 0x08

#define XNVM_CMD_NOP                         0x00 //!< No Operation.
#define XNVM_CMD_CHIP_ERASE                  0x40 //!< Chip Erase.
#define XNVM_CMD_READ_NVM_PDI                0x43 //!< Read NVM PDI.
#define XNVM_CMD_LOAD_FLASH_PAGE_BUFFER      0x23 //!< Load Flash Page Buffer.
#define XNVM_CMD_ERASE_FLASH_PAGE_BUFFER     0x26 //!< Erase Flash Page Buffer.
#define XNVM_CMD_ERASE_FLASH_PAGE            0x2B //!< Erase Flash Page.
#define XNVM_CMD_WRITE_FLASH_PAGE            0x2E //!< Flash Page Write.
#define XNVM_CMD_ERASE_AND_WRITE_FLASH_PAGE  0x2F //!< Erase & Write Flash Page.
#define XNVM_CMD_CALC_CRC_ON_FLASH           0x78 //!< Flash CRC.

#define XNVM_CMD_ERASE_APP_SECTION           0x20 //!< Erase Application Section.
#define XNVM_CMD_ERASE_APP_PAGE              0x22 //!< Erase Application Section.
#define XNVM_CMD_WRITE_APP_SECTION           0x24 //!< Write Application Section.
#define XNVM_CMD_ERASE_AND_WRITE_APP_SECTION 0x25 //!< Erase & Write Application Section Page.
#define XNVM_CMD_CALC_CRC_APP_SECTION        0x38 //!< Application Section CRC.

#define XNVM_CMD_ERASE_BOOT_SECTION          0x68 //!< Erase Boot Section.
#define XNVM_CMD_ERASE_BOOT_PAGE             0x2A //!< Erase Boot Loader Section Page.
#define XNVM_CMD_WRITE_BOOT_PAGE             0x2C //!< Write Boot Loader Section Page.
#define XNVM_CMD_ERASE_AND_WRITE_BOOT_PAGE   0x2D //!< Erase & Write Boot Loader Section Page.
#define XNVM_CMD_CALC_CRC_BOOT_SECTION       0x39 //!< Boot Loader Section CRC.

#define XNVM_CMD_READ_USER_SIGN              0x03 //!< Read User Signature Row.
#define XNVM_CMD_ERASE_USER_SIGN             0x18 //!< Erase User Signature Row.
#define XNVM_CMD_WRITE_USER_SIGN             0x1A //!< Write User Signature Row.
#define XNVM_CMD_READ_CALIB_ROW              0x02 //!< Read Calibration Row.

#define XNVM_CMD_READ_FUSE                   0x07 //!< Read Fuse.
#define XNVM_CMD_WRITE_FUSE                  0x4C //!< Write Fuse.
#define XNVM_CMD_WRITE_LOCK_BITS             0x08 //!< Write Lock Bits.

#define XNVM_CMD_LOAD_EEPROM_PAGE_BUFFER     0x33 //!< Load EEPROM Page Buffer.
#define XNVM_CMD_ERASE_EEPROM_PAGE_BUFFER    0x36 //!< Erase EEPROM Page Buffer.

#define XNVM_CMD_ERASE_EEPROM                0x30 //!< Erase EEPROM.
#define XNVM_CMD_ERASE_EEPROM_PAGE           0x32 //!< Erase EEPROM Page.
#define XNVM_CMD_WRITE_EEPROM_PAGE           0x34 //!< Write EEPROM Page.
#define XNVM_CMD_ERASE_AND_WRITE_EEPROM      0x35 //!< Erase & Write EEPROM Page.
#define XNVM_CMD_READ_EEPROM                 0x06 //!< Read EEPROM.

#define XNVM_FLASH_BASE                 0x0800000 //!< Adress where the flash starts.
#define XNVM_EEPROM_BASE                0x08C0000 //!< Address where eeprom starts.
#define XNVM_FUSE_BASE                  0x08F0020 //!< Address where fuses start.
#define XNVM_DATA_BASE                  0x1000000 //!< Address where data region starts.
#define XNVM_APPL_BASE            XNVM_FLASH_BASE //!< Addres where application section starts.
#define XNVM_CALIBRATION_BASE          0x008E0200 //!< Address where calibration row starts.
#define XNVM_SIGNATURE_BASE            0x008E0400 //!< Address where signature bytes start.

#define XNVM_FLASH_PAGE_SIZE			512			//

#define XNVM_CONTROLLER_BASE 0x01C0               //!< NVM Controller register base address.
#define XNVM_CONTROLLER_CMD_REG_OFFSET 0x0A       //!< NVM Controller Command Register offset.
#define XNVM_CONTROLLER_STATUS_REG_OFFSET 0x0F    //!< NVM Controller Status Register offset.
#define XNVM_CONTROLLER_CTRLA_REG_OFFSET 0x0B     //!< NVM Controller Control Register A offset.

#define XNVM_CTRLA_CMDEX (1 << 0)                 //!< CMDEX bit offset.
#define XNVM_NVMEN (1 << 1)                       //!< NVMEN bit offset.
#define XNVM_NVM_BUSY (1 << 7)                    //!< NVMBUSY bit offset.

#define XOCD_STATUS_REGISTER_ADDRESS 0x00         //!< PDI status register address.
#define XOCD_RESET_REGISTER_ADDRESS  0x01         //!< PDI reset register address.
#define XOCD_RESET_SIGNATURE         0x59         //!< PDI reset Signature.
#define XOCD_FCMR_ADDRESS 0x05
#define XOCD_CTRL_REGISTER_ADDRESS  0x02

#endif
