#
#       !!!! Do NOT edit this makefile with an editor which replace tabs by spaces !!!!    
#
##############################################################################################
# 
# On command line:
#
# make all = Create project
#
# make clean = Clean project files.
#
# To rebuild project do "make clean" and "make all".
#
#
##############################################################################################
#
# Build commands:
#
# Taranis      make PCB=X9D
# X9E          make PCB=X9D REV9E=1
# TaranisPlus  make PCB=X9D REVPLUS=1
# XLite        make PCB=X9D PCBXLITE=1
# XLiteS       make PCB=X9D PCBXLITE=1 PCBXLITES=1
# QX7          make PCB=X9D PCBX7=1 BT=1
# QX7 ACCESS   make PCB=X9D PCBX7=1 PCBACCESS=1
# Jumper T12   make PCB=X9D PCBX7=1 PCBT12=1
# X9Lite       make PCB=X9D PCBX9LITE=1
# X9LiteS      make PCB=X9D PCBX9LITE=1 X9LS=1 
# 9Xtreme      make PCB=9XT
# SKY board    make REVB=1 SMALL=1 LANG1=0
# SKY board    make REVB=1 SMALL=1 LANG1=1
# 9XR_PRO      make REVB=1 REVX=1
# AR9X board   make REVB=1
# Horus X12S   make PCB=X12D
# Jumper T16   make PCB=X10 PCBT16=1
# RM TX16S     make PCB=X10 PCBT16=1 PCBTX16S=1
# RK TX18S     make PCB=X10 PCBT16=1 PCBTX16S=1 PCBT18=1
# X10Express   make PCB=X10 EXPRESS=1
# X10          make PCB=X10
# XWAVE T-800  make PCBLEM1=1
# ARUNI        make REVB=1 ARUNI=1
#
##############################################################################################
# Start of default section
#

TOOLCHAIN = arm-none-eabi-
CC   = $(TOOLCHAIN)gcc
CP   = $(TOOLCHAIN)objcopy
CLSS = $(TOOLCHAIN)objdump
AS   = $(TOOLCHAIN)as
#gcc -x assembler-with-cpp
BIN  = $(CP) -O ihex 
BINX = $(CP) -O binary 

SHELL=cmd

ifeq ($(PCB), X9D)
 MCU  = cortex-m3
 BASIC = 1
 CPPDEFS += -DBASIC
 ifeq ($(REV19), 1)
  LDSCRIPT = X9D/stm32F4_flash.ld
  UDEFS = -DSTM32F40_41xxx
 else
  LDSCRIPT = X9D/stm32_flash.ld
  UDEFS = -Dstm32f205
 endif
 CPPDEFS += -DHSE_VALUE=12000000
 CPPDEFS += -DPCBX9D 
 CPPDEFS += -DUSB_JOYSTICK
 CPPDEFS += -DXFIRE
 ifeq ($(REVPLUS), 1)
  PROJECT = x9dp
  EXT_MOD=X9DP
  CPPDEFS += -DREVPLUS
  LUA = 1
  CPPDEFS += -DLUA
 else ifeq ($(REV9E), 1)
  PROJECT        = x9e
  EXT_MOD=X9E
  CPPDEFS += -DREV9E
  LUA = 1
  CPPDEFS += -DLUA
 else ifeq ($(PCBX7), 1)
  CPPDEFS += -DPCBX7
  ifeq ($(PCBT12), 1)
   CPPDEFS += -DPCBT12
   PROJECT = t12
   EXT_MOD=T12
   LUA = 1
   CPPDEFS += -DLUA
  else ifeq ($(PCBACCESS), 1)
   CPPDEFS += -DPCBX7ACCESS
   PROJECT = x7Acc
   EXT_MOD=QX7ACCESS
   LUA = 1
   CPPDEFS += -DLUA
  else
   PROJECT = x7
   EXT_MOD=QX7
   LUA = 1
   CPPDEFS += -DLUA
  endif
 else ifeq ($(PCBXLITE), 1)
  CPPDEFS += -DPCBXLITE
  ifeq ($(PCBXLITES), 1)
   BT = 1
   PROJECT = xliteS
   EXT_MOD=XLiteS
   CPPDEFS += -DPCBXLITES
  else 
   PROJECT = xlite
   EXT_MOD=XLITE
  endif 
  LUA = 1
  CPPDEFS += -DLUA
 else ifeq ($(PCBX9LITE), 1)
  CPPDEFS += -DPCBX9LITE
  ifeq ($(X9LS), 1)
   BT = 1
   PROJECT = x9LiteS
   EXT_MOD=X9LiteS
   CPPDEFS += -DX9LS 
   LUA = 1
   CPPDEFS += -DLUA
  else
   PROJECT = x9Lite
   EXT_MOD=X9Lite
   LUA = 1
   CPPDEFS += -DLUA
  endif
 else ifeq ($(REV19), 1)
  PROJECT = x9dp19
  EXT_MOD=X9DP19
#  CPPDEFS += -DREVPLUS
  CPPDEFS += -DREV19
  LUA = 1
  CPPDEFS += -DLUA
 else
  PROJECT = x9d
  EXT_MOD=X9D
  CPPDEFS += -DREVNORM
  LUA = 1
  CPPDEFS += -DLUA
 endif
else ifeq ($(PCB), 9XT)
 MCU  = cortex-m3
 PROJECT      = ersky9x9XT
 BASIC = 1
 CPPDEFS += -DBASIC 
 LDSCRIPT = X9D/stm32_flash.ld
 CPPDEFS += -DHSE_VALUE=12000000
 CPPDEFS += -DPCB9XT 
 CPPDEFS += -DPCBSP
 CPPDEFS += -DUSB_JOYSTICK
 CPPDEFS += -DXFIRE
 EXT_MOD=9XT
 UDEFS = -Dstm32f205
 LUA = 1
 CPPDEFS += -DLUA
else ifeq ($(PCB), X12D)
 MCU  = cortex-m4
 PROJECT = x12d
 BASIC = 1
 CPPDEFS += -DBASIC 
 CDEFS += -D__FPU_USED=1
 LDSCRIPT = X12D/stm32f4_flash.ld
 CPPDEFS += -DHSE_VALUE=12000000
 CPPDEFS += -DPCBX12D 
 CPPDEFS += -DUSB_JOYSTICK
 EXT_MOD=X12D
 UDEFS = -DSTM32F429_439xx
 LUA = 1
 CPPDEFS += -DLUA
else ifeq ($(PCB), X10)
 MCU  = cortex-m4
 BASIC = 1
 CPPDEFS += -DBASIC 
 CDEFS += -D__FPU_USED=1
 LDSCRIPT = X12D/stm32f4_flash.ld
 CPPDEFS += -DHSE_VALUE=12000000
 CPPDEFS += -DPCBX10 
 CPPDEFS += -DUSB_JOYSTICK
 UDEFS = -DSTM32F429_439xx
 ifeq ($(PCBT16), 1)
  ifeq ($(PCBTX16S), 1)
   ifeq ($(PCBT18), 1)
    PROJECT = tx18s
    CPPDEFS += -DPCBT18 
    CPPDEFS += -DPCBT16 
    CPPDEFS += -DPCBTX16S
    EXT_MOD=TX18S
    LUA = 1
	 TOUCH = 1
    CPPDEFS += -DLUA
	 TOUCH = 1
   else
    PROJECT = tx16s
    CPPDEFS += -DPCBT16 
    CPPDEFS += -DPCBTX16S
    LUA = 1
	 TOUCH = 1
    CPPDEFS += -DLUA
    EXT_MOD=TX16S
	 TOUCH = 1
   endif
  else
   PROJECT = t16
   CPPDEFS += -DPCBT16 
   EXT_MOD=T16
   LUA = 1
   CPPDEFS += -DLUA
  endif
 else ifeq ($(EXPRESS), 1)
  PROJECT = x10E
  EXT_MOD=X10E
  CPPDEFS += -DPCBREV_EXPRESS 
  LUA = 1
  CPPDEFS += -DLUA
  BT = 1
 else
  PROJECT = x10
  EXT_MOD=X10
  LUA = 1
  CPPDEFS += -DLUA
 endif
else ifeq ($(PCBLEM1), 1)
 MCU  = cortex-m3
 PROJECT = lemon1
 CPPDEFS += -DBASIC 
 BASIC = 1
 LDSCRIPT = STM32F103VETx_FLASH.ld
 CPPDEFS += -DHSE_VALUE=8000000
 CPPDEFS += -DPCBLEM1 
 FULL_PRJ = $(PROJECT)_rom
 EXT_MOD=Lemon
 UDEFS = -Dstm32f103
 UDEFS = -DSTM32F10X_HD
else
 MCU  = cortex-m3
 USB_JOYSTICK = 1
 BASIC = 1
 CPPDEFS += -DBASIC 
 CPPDEFS += -DPCBSKY 
 CPPDEFS += -DREVB
 UINCDIR = ./sky/joy/inc
 ifeq ($(PCB), DUE)
  PROJECT      = ersky9xd
  LDSCRIPT = due/sam3x8_flash.ld
  UDEFS = -Dat91sam3x8
  UDEFS  += -D__SAM3X8E__
  EXT_MOD=DUE
  CPPDEFS += -DPCBDUE 
 else ifeq ($(REVX), 1)
  PROJECT = ersky9xr
  LDSCRIPT = sam3s8c_flash.ld
  UDEFS = -Dat91sam3s8
  CPPDEFS += -DREVX
  CPPDEFS += -DXFIRE
  EXT_MOD=REVB-X
  LUA = 1
  CPPDEFS += -DLUA
 else ifeq ($(SMALL), 1)
  LDSCRIPT = sam3s4c_flash.ld
  UDEFS = -Dat91sam3s4
  EXT_MOD=REVB
  CPPDEFS += -DSMALL 
  ifeq ($(LANG1), 0)
   PROJECT = ersky9xsf
   CPPDEFS += -DFRENCH
  else
   PROJECT = ersky9xsd
   CPPDEFS += -DGERMAN
  endif
 else ifeq ($(ARUNI), 1)
  PROJECT = aruni
  LDSCRIPT = sam3s8c_flash.ld
  EXT_MOD=ARUNI
  UDEFS = -Dat91sam3s8
  CPPDEFS += -DXFIRE
 else
  PROJECT = ersky9x
  CPPDEFS += -DXFIRE
  LDSCRIPT = sam3s8c_flash.ld
  EXT_MOD=REVB
  UDEFS = -Dat91sam3s8
  LUA = 1
  CPPDEFS += -DLUA
 endif
endif



# List all default C defines here, like -D_DEBUG=1
DDEFS =

# List all default ASM defines here, like -D_DEBUG=1
DADEFS = 

# List all default directories to look for include files here
DINCDIR = 

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS = 


EXT = STD

#ifeq ($(STAMP), 1)
AREV = $(shell sh -c "cat rev.txt | sed -n '1p'")
REV = $(shell echo $$(( $(AREV) + 1 )))
PROV=$(shell sh -c "cat prov.txt")
#REV = $(shell echo $$(( $(AREV) )))
#endif

#
# End of default section
##############################################################################################

FULL_PRJ = $(PROJECT)_rom

ifdef OBJROOT
else
OBJROOT = ./
endif
OBJDIR = $(OBJROOT)/obj_$(PROJECT)

# List all user C define here, like -D_DEBUG=1

#ifeq ($(PCB), X9D)
#UDEFS = 
#endif

ifeq ($(SAM3S2), 1)
UDEFS = -Dat91sam3s2
endif

ifeq ($(USB_JOYSTICK), 1)
  CPPDEFS += -DUSB_JOYSTICK
endif

# Define ASM defines here
UADEFS = 

#    LUASRC = $(LUADIR)/lapi.c $(LUADIR)/lcode.c $(LUADIR)/lctype.c $(LUADIR)/ldebug.c $(LUADIR)/ldo.c $(LUADIR)/ldump.c $(LUADIR)/lfunc.c $(LUADIR)/lgc.c $(LUADIR)/llex.c $(LUADIR)/lmem.c \
#           $(LUADIR)/lobject.c $(LUADIR)/lopcodes.c $(LUADIR)/lparser.c $(LUADIR)/lstate.c $(LUADIR)/lstring.c $(LUADIR)/ltable.c $(LUADIR)/lrotable.c $(LUADIR)/ltm.c $(LUADIR)/lundump.c $(LUADIR)/lvm.c $(LUADIR)/lzio.c \
#           $(LUADIR)/lbaselib.c $(LUADIR)/linit.c $(LUADIR)/lmathlib.c $(LUADIR)/lbitlib.c $(LUADIR)/loadlib.c $(LUADIR)/lauxlib.c $(LUADIR)/ltablib.c $(LUADIR)/lcorolib.c $(LUADIR)/liolib.c

ifeq ($(PCB), X12D)
# List C source files here
SRC  = X12D/system_stm32f4xx.c \
       X12D/stm32f4xx_gpio.c \
       X12D/stm32f4xx_spi.c \
       X12D/stm32f4xx_rtc.c \
       X12D/stm32f4xx_pwr.c \
       X12D/stm32f4xx_rcc.c \
       X12D/misc.c \
		 X12D/sdram_driver.c \
		 X12D/pwr_driver.c \
		 X12D/stm32f4xx_fmc.c \
		 X12D/stm32f4xx_dma2d.c \
		 X12D/stm32f4xx_dma.c \
		 X12D/stm32f4xx_ltdc.c \
       X12D/stm32f4xx_sdio.c \
		 X12D/sdio_sd.c \
       X12D/usbd_desc.c   \
       X12D/usb_core.c    \
       X12D/usb_dcd.c     \
       X12D/usb_bsp.c     \
       X12D/usb_dcd_int.c \
       X12D/usbd_core.c   \
       X12D/usbd_ioreq.c  \
       X12D/usbd_req.c    \
       X12D/usbd_hid_joystick.c \
       X12D/usb_driver.c    \
       port.c \
       coos.c

#		 X12D/stm32f4xx_tim.c \
#		 X12D/led.c

ifeq ($(LUA), 1)
SRC += lua/lapi.c lua/lcode.c lua/lctype.c lua/ldebug.c lua/ldo.c lua/ldump.c lua/lfunc.c lua/lgc.c lua/llex.c lua/lmem.c \
           lua/lobject.c lua/lopcodes.c lua/lparser.c lua/lstate.c lua/lstring.c lua/ltable.c lua/lrotable.c lua/ltm.c lua/lundump.c lua/lvm.c lua/lzio.c \
           lua/lbaselib.c lua/linit.c lua/lmathlib.c lua/lbitlib.c lua/loadlib.c lua/lauxlib.c lua/ltablib.c lua/lcorolib.c lua/liolib.c lua/lstrlib.c \
           syscalls.c
endif

CPPSRC = ersky9x.cpp \
         X12D/lcd_driver.cpp \
         X12D/hdrivers.cpp \
         X12D/sound.cpp \
         X12D/diskio.cpp \
         X12D/watchdog.cpp \
			X12D/i2c_ee.cpp \
         X12D/rtc_driver.cpp \
         X12D/persX12d.cpp \
		 	X12D/pulses_driver.cpp \
		 	X12D/screenshotH.cpp \
         X9D/serial.cpp \
         templates.cpp \
         drivers.cpp \
         pulses.cpp \
         timers.cpp \
         logicio.cpp \
         X12D/adc_driver.cpp \
			frsky.cpp \
         menus.cpp \
         menushres.cpp \
         mixer.cpp \
         trims.cpp \
         ff.cpp \
			logs.cpp \
         audio.cpp \
			en.cpp \
			de.cpp \
			no.cpp \
			fr.cpp \
			se.cpp \
			it.cpp \
			pl.cpp \
			vi.cpp \
			sp.cpp \
			maintenance.cpp \
			mavlink.cpp \
			sbus.cpp \
         X12D/t.cpp \
         X12D/led_driver.cpp \
         X12D/usbd_usr.cpp \
			loadboot.cpp \
			pxx2.cpp \
			gvars.cpp \
			vars.cpp \
			stamp.cpp \
			tml.cpp \
			lcd.cpp

ifeq ($(BASIC), 1)
	  CPPSRC += basic/parser.cpp
endif
ifeq ($(LUA), 1)
CPPSRC += lua/api_lcd.cpp lua/interface.cpp lua/lua_api.cpp bin_allocator.cpp
endif
ifeq ($(DEBUG), 1)
CPPSRC += debug.cpp
endif

#         audio.cpp \
#         X9D/sound.cpp \
#         logs.cpp \

# List ASM source files here
ASRC = X12D/startup_stm32f42_43xxx.s

else

ifeq ($(PCB), X10)
# List C source files here
SRC  = X12D/system_stm32f4xx.c \
       X12D/stm32f4xx_gpio.c \
       X12D/stm32f4xx_spi.c \
       X12D/stm32f4xx_rtc.c \
       X12D/stm32f4xx_pwr.c \
       X12D/stm32f4xx_rcc.c \
       X12D/misc.c \
		 X12D/sdram_driver.c \
		 X12D/pwr_driver.c \
		 X12D/stm32f4xx_fmc.c \
		 X12D/stm32f4xx_dma2d.c \
		 X12D/stm32f4xx_dma.c \
		 X12D/stm32f4xx_ltdc.c \
       X12D/stm32f4xx_sdio.c \
		 X12D/sdio_sd.c \
       X12D/usbd_desc.c   \
       X12D/usb_core.c    \
       X12D/usb_dcd.c     \
       X12D/usb_bsp.c     \
       X12D/usb_dcd_int.c \
       X12D/usbd_core.c   \
       X12D/usbd_ioreq.c  \
       X12D/usbd_req.c    \
       X12D/usbd_hid_joystick.c \
       X12D/usb_driver.c    \
       port.c \
       coos.c

#		 X12D/stm32f4xx_tim.c \
#		 X12D/led.c

ifeq ($(LUA), 1)
SRC += lua/lapi.c lua/lcode.c lua/lctype.c lua/ldebug.c lua/ldo.c lua/ldump.c lua/lfunc.c lua/lgc.c lua/llex.c lua/lmem.c \
           lua/lobject.c lua/lopcodes.c lua/lparser.c lua/lstate.c lua/lstring.c lua/ltable.c lua/lrotable.c lua/ltm.c lua/lundump.c lua/lvm.c lua/lzio.c \
           lua/lbaselib.c lua/linit.c lua/lmathlib.c lua/lbitlib.c lua/loadlib.c lua/lauxlib.c lua/ltablib.c lua/lcorolib.c lua/liolib.c lua/lstrlib.c \
           syscalls.c
endif

CPPSRC = ersky9x.cpp \
         X12D/lcd_driver.cpp \
         X12D/hdrivers.cpp \
         X12D/sound.cpp \
         X12D/diskio.cpp \
         X12D/watchdog.cpp \
			X12D/i2c_ee.cpp \
         X12D/rtc_driver.cpp \
         X12D/persX12d.cpp \
		 	X12D/pulses_driver.cpp \
		 	X12D/screenshotH.cpp \
         X9D/serial.cpp \
			stamp.cpp \
         templates.cpp \
         drivers.cpp \
         pulses.cpp \
         timers.cpp \
         logicio.cpp \
         X12D/adc_driver.cpp \
			frsky.cpp \
         menus.cpp \
         menushres.cpp \
         mixer.cpp \
         trims.cpp \
         ff.cpp \
			logs.cpp \
         audio.cpp \
			en.cpp \
			de.cpp \
			no.cpp \
			fr.cpp \
			se.cpp \
			it.cpp \
			pl.cpp \
			vi.cpp \
			sp.cpp \
			maintenance.cpp \
			mavlink.cpp \
			sbus.cpp \
         X12D/t.cpp \
         X12D/led_driver.cpp \
         X12D/usbd_usr.cpp \
			loadboot.cpp \
			pxx2.cpp \
			gvars.cpp \
			vars.cpp \
			tml.cpp \
			lcd.cpp

ifeq ($(TOUCH), 1)
  CPPSRC += touchMenus.cpp
endif

ifeq ($(BT), 1)
  CPPSRC += bluetooth.cpp
  CPPDEFS += -DBT
endif

ifeq ($(TOUCH), 1)
  CPPSRC += X12D/tp_gt911.cpp
  CPPDEFS += -DTOUCH
endif

ifeq ($(BASIC), 1)
	  CPPSRC += basic/parser.cpp
endif
ifeq ($(LUA), 1)
CPPSRC += lua/api_lcd.cpp lua/interface.cpp lua/lua_api.cpp bin_allocator.cpp
endif
ifeq ($(DEBUG), 1)
CPPSRC += debug.cpp
endif

#         audio.cpp \
#         X9D/sound.cpp \
#         logs.cpp \

# List ASM source files here
ASRC = X12D/startup_stm32f42_43xxx.s

else

ifeq ($(PCB), X9D)

 ifeq ($(REV19), 1)
# List C source files here
SRC  = X12D/system_stm32f4xx.c \
       X12D/stm32f4xx_gpio.c \
       X12D/stm32f4xx_spi.c \
       X12D/stm32f4xx_rtc.c \
       X12D/stm32f4xx_pwr.c \
       X12D/stm32f4xx_rcc.c \
       X12D/misc.c \
       X12D/usbd_desc.c   \
       X12D/usb_core.c    \
       X12D/usb_dcd.c     \
       X12D/usb_bsp.c     \
       X12D/usb_dcd_int.c \
       X12D/usbd_core.c   \
       X12D/usbd_ioreq.c  \
       X12D/usbd_req.c    \
       X12D/usbd_hid_joystick.c \
       X12D/usb_driver.c    \
       port.c \
       coos.c

#       X9D/usb_bsp.c \
#       X9D/usbd_desc.c \
#       X9D/usb/src/usb_core.c \
#       X9D/usb/src/usb_dcd.c \
#       X9D/usb/src/usb_dcd_int.c \
#       X9D/usb/src/usbd_core.c \
#       X9D/usb/src/usbd_ioreq.c \
#       X9D/usb/src/usbd_req.c \
#       X9D/usb/src/usbd_msc_data.c \
#       X9D/usb/src/usbd_msc_scsi.c \
#       X9D/usb/src/usbd_msc_bot.c \
#       X9D/usb/src/usbd_msc_core.c \

ifeq ($(LUA), 1)
SRC += lua/lapi.c lua/lcode.c lua/lctype.c lua/ldebug.c lua/ldo.c lua/ldump.c lua/lfunc.c lua/lgc.c lua/llex.c lua/lmem.c \
           lua/lobject.c lua/lopcodes.c lua/lparser.c lua/lstate.c lua/lstring.c lua/ltable.c lua/lrotable.c lua/ltm.c lua/lundump.c lua/lvm.c lua/lzio.c \
           lua/lbaselib.c lua/linit.c lua/lmathlib.c lua/lbitlib.c lua/loadlib.c lua/lauxlib.c lua/ltablib.c lua/lcorolib.c lua/liolib.c lua/lstrlib.c \
           syscalls.c
endif


CPPSRC = ersky9x.cpp \
         ./X9D/lcd_driver.cpp \
         X9D/aspi.cpp \
         X9D/diskio.cpp \
         X9D/power.cpp \
         X9D/pulses_driver.cpp \
         X9D/watchdog.cpp \
       	X9D/i2c_ee.cpp \
         X9D/rtc_driver.cpp \
         X9D/eeprom_rlc.cpp \
         X9D/serial.cpp \
         X9D/sound.cpp \
			stamp.cpp \
         templates.cpp \
         pulses.cpp \
         drivers.cpp \
         timers.cpp \
         logicio.cpp \
         analog.cpp \
			frsky.cpp \
         menus.cpp \
         mixer.cpp \
         trims.cpp \
         ff.cpp \
			logs.cpp \
         audio.cpp \
			en.cpp \
			de.cpp \
			no.cpp \
			fr.cpp \
			se.cpp \
			it.cpp \
			pl.cpp \
			vi.cpp \
			sp.cpp \
			loadboot.cpp \
			maintenance.cpp \
			mavlink.cpp \
			sbus.cpp \
         X9D/usbd_usr.cpp \
			pxx2.cpp \
			gvars.cpp \
			vars.cpp \
		   lcd.cpp

ifeq ($(BASIC), 1)
	  CPPSRC += basic/parser.cpp
endif

ifeq ($(BT), 1)
  CPPSRC += bluetooth.cpp
  CPPDEFS += -DBT
endif


ifeq ($(LUA), 1)
CPPSRC += lua/api_lcd.cpp lua/interface.cpp lua/lua_api.cpp bin_allocator.cpp
endif

#         gtime.cpp \
#         massstorage.cpp \
#         X9D/usbd_usr.cpp \
#         X9D/usbd_storage_msd.cpp \

CPPSRC += pdi.cpp
ifeq ($(DEBUG), 1)
CPPSRC += debug.cpp
endif

# List ASM source files here
ASRC = X9D/startup_stm32f40_41xxx.s
 else
# List C source files here
SRC  = X9D/system_stm32f2xx.c \
       X9D/stm32f2xx_gpio.c \
       X9D/stm32f2xx_spi.c \
       X9D/stm32f2xx_rtc.c \
       X9D/stm32f2xx_pwr.c \
       X9D/stm32f2xx_rcc.c \
       X9D/misc.c \
       X9D/usbd_desc.c   \
       X9D/usb_core.c    \
       X9D/usb_dcd.c     \
       X9D/usb_bsp.c     \
       X9D/usb_dcd_int.c \
       X9D/usbd_core.c   \
       X9D/usbd_ioreq.c  \
       X9D/usbd_req.c    \
       X9D/usbd_hid_joystick.c \
       X9D/usb_driver.c    \
       port.c \
       coos.c

#       X9D/usb_bsp.c \
#       X9D/usbd_desc.c \
#       X9D/usb/src/usb_core.c \
#       X9D/usb/src/usb_dcd.c \
#       X9D/usb/src/usb_dcd_int.c \
#       X9D/usb/src/usbd_core.c \
#       X9D/usb/src/usbd_ioreq.c \
#       X9D/usb/src/usbd_req.c \
#       X9D/usb/src/usbd_msc_data.c \
#       X9D/usb/src/usbd_msc_scsi.c \
#       X9D/usb/src/usbd_msc_bot.c \
#       X9D/usb/src/usbd_msc_core.c \

ifeq ($(LUA), 1)
SRC += lua/lapi.c lua/lcode.c lua/lctype.c lua/ldebug.c lua/ldo.c lua/ldump.c lua/lfunc.c lua/lgc.c lua/llex.c lua/lmem.c \
           lua/lobject.c lua/lopcodes.c lua/lparser.c lua/lstate.c lua/lstring.c lua/ltable.c lua/lrotable.c lua/ltm.c lua/lundump.c lua/lvm.c lua/lzio.c \
           lua/lbaselib.c lua/linit.c lua/lmathlib.c lua/lbitlib.c lua/loadlib.c lua/lauxlib.c lua/ltablib.c lua/lcorolib.c lua/liolib.c lua/lstrlib.c \
           syscalls.c
endif


CPPSRC = ersky9x.cpp \
         X9D/lcd_driver.cpp \
         X9D/aspi.cpp \
         X9D/sound.cpp \
         X9D/diskio.cpp \
         X9D/power.cpp \
         X9D/pulses_driver.cpp \
         X9D/watchdog.cpp \
       	X9D/i2c_ee.cpp \
         X9D/rtc_driver.cpp \
         X9D/eeprom_rlc.cpp \
         X9D/serial.cpp \
			stamp.cpp \
         templates.cpp \
         pulses.cpp \
         drivers.cpp \
         timers.cpp \
         logicio.cpp \
         analog.cpp \
			frsky.cpp \
         menus.cpp \
         mixer.cpp \
         trims.cpp \
         ff.cpp \
			logs.cpp \
         audio.cpp \
			en.cpp \
			de.cpp \
			no.cpp \
			fr.cpp \
			se.cpp \
			it.cpp \
			pl.cpp \
			vi.cpp \
			sp.cpp \
			loadboot.cpp \
			maintenance.cpp \
			mavlink.cpp \
			sbus.cpp \
         X9D/usbd_usr.cpp \
			pxx2.cpp \
			gvars.cpp \
			vars.cpp \
		   lcd.cpp

ifeq ($(BASIC), 1)
	  CPPSRC += basic/parser.cpp
endif

ifeq ($(BT), 1)
  CPPSRC += bluetooth.cpp
  CPPDEFS += -DBT
endif


ifeq ($(LUA), 1)
CPPSRC += lua/api_lcd.cpp lua/interface.cpp lua/lua_api.cpp bin_allocator.cpp
endif

#         gtime.cpp \
#         massstorage.cpp \
#         X9D/usbd_usr.cpp \
#         X9D/usbd_storage_msd.cpp \

CPPSRC += pdi.cpp
ifeq ($(DEBUG), 1)
CPPSRC += debug.cpp
endif

# List ASM source files here
ASRC = X9D/startup_stm32f2xx.s
 endif
else

ifeq ($(PCB), DUE)

# List C source files here
SRC  = due/startup_sam3x.c \
       due/exceptions.c \
       due/system_sam3x.c \
       port.c \
       coos.c

CPPSRC = stamp.cpp \
         due/dlogicio.cpp \
         due/lcd_driver.cpp \
         lcd.cpp \
			en.cpp \
			de.cpp \
			no.cpp \
			fr.cpp \
			se.cpp \
			it.cpp \
			pl.cpp \
			vi.cpp \
			sp.cpp \
         file.cpp \
         pers.cpp \
         templates.cpp \
			sbus.cpp \
         blink.cpp

else

ifeq ($(PCB), 9XT)

# List C source files here
SRC  = X9D/system_stm32f2xx.c \
       X9D/stm32f2xx_gpio.c \
       X9D/stm32f2xx_spi.c \
       X9D/stm32f2xx_rtc.c \
       X9D/stm32f2xx_pwr.c \
       X9D/stm32f2xx_rcc.c \
       X9D/misc.c \
       X9D/usbd_desc.c   \
       X9D/usb_core.c    \
       X9D/usb_dcd.c     \
       X9D/usb_bsp.c     \
       X9D/usb_dcd_int.c \
       X9D/usbd_core.c   \
       X9D/usbd_ioreq.c  \
       X9D/usbd_req.c    \
       X9D/usbd_hid_joystick.c \
       X9D/usb_driver.c    \
       port.c \
       coos.c

ifeq ($(LUA), 1)
SRC += lua/lapi.c lua/lcode.c lua/lctype.c lua/ldebug.c lua/ldo.c lua/ldump.c lua/lfunc.c lua/lgc.c lua/llex.c lua/lmem.c \
           lua/lobject.c lua/lopcodes.c lua/lparser.c lua/lstate.c lua/lstring.c lua/ltable.c lua/lrotable.c lua/ltm.c lua/lundump.c lua/lvm.c lua/lzio.c \
           lua/lbaselib.c lua/linit.c lua/lmathlib.c lua/lbitlib.c lua/loadlib.c lua/lauxlib.c lua/ltablib.c lua/lcorolib.c lua/liolib.c lua/lstrlib.c \
           syscalls.c
endif

CPPSRC = ersky9x.cpp \
         X9D/aspi.cpp \
         X9D/sound.cpp \
         X9D/diskio.cpp \
         X9D/power.cpp \
         X9D/pulses_driver.cpp \
         X9D/watchdog.cpp \
         X9D/rtc_driver.cpp \
         X9D/i2c_9xt.cpp \
         X9D/serial.cpp \
			stamp.cpp \
         templates.cpp \
         drivers.cpp \
         timers.cpp \
         logicio.cpp \
         analog.cpp \
         file.cpp \
			frsky.cpp \
         pulses.cpp \
         pers.cpp \
         menus.cpp \
         mixer.cpp \
         trims.cpp \
         ff.cpp \
			logs.cpp \
         audio.cpp \
         bluetooth.cpp \
			en.cpp \
			de.cpp \
			no.cpp \
			fr.cpp \
			se.cpp \
			it.cpp \
			pl.cpp \
			vi.cpp \
			sp.cpp \
			loadboot.cpp \
			maintenance.cpp \
			mavlink.cpp \
			sbus.cpp \
			mega64.cpp \
			isp.cpp \
         X9D/usbd_usr.cpp \
			pxx2.cpp \
			gvars.cpp \
			vars.cpp \
		   lcd.cpp

ifeq ($(BASIC), 1)
	  CPPSRC += basic/parser.cpp
endif
ifeq ($(LUA), 1)
CPPSRC += lua/api_lcd.cpp lua/interface.cpp lua/lua_api.cpp bin_allocator.cpp
endif

#         X9D/lcd_driver.cpp \
#       	X9D/i2c_ee.cpp \
#         X9D/eeprom_rlc.cpp \

CPPSRC += pdi.cpp
ifeq ($(DEBUG), 1)
CPPSRC += debug.cpp
endif

# List ASM source files here
ASRC = X9D/startup_stm32f2xx.s

else
ifeq ($(PCBLEM1), 1)


# List C source files here
SRC  = core_cm3.c \
       port.c \
       stm103/system_stm32f1xx.c \
       stm103/stm32f10x_sdio.c \
       stm103/stm32f10x_rcc.c \
       stm103/stm32f10x_gpio.c \
       stm103/stm32f10x_dma.c \
       stm103/sdio_sd.c \
       syscalls.c \
       coos.c

CPPSRC = ersky9x.cpp \
         stm103/t.cpp \
			stm103/lcd_driver.cpp \
			stm103/sound.cpp \
       	stm103/diskio.cpp \
			stm103/power.cpp \
         stm103/watchdog.cpp \
         stm103/pulsesDriver.cpp \
         templates.cpp \
			frsky.cpp \
			menus.cpp \
			mixer.cpp \
         trims.cpp \
			drivers.cpp \
         timers.cpp \
			stm103/logicio103.cpp \
			stm103/analog103.cpp \
			stm103/i2c103.cpp \
         ff.cpp \
			logs.cpp \
         audio.cpp \
			en.cpp \
			de.cpp \
			fr.cpp \
			sbus.cpp \
			pers.cpp \
			gvars.cpp \
			stamp.cpp \
			lcd.cpp

ifeq ($(BASIC), 1)
	  CPPSRC += basic/parser.cpp
endif

#			menus.cpp


#			stm103/stf3210x_sdio.cpp

# List ASM source files here
ASRC = stm103/startup_stm32f103xe.s

else

# List C source files here
SRC  = core_cm3.c \
       board_lowlevel.c \
       crt.c \
       vectors_sam3s.c \
       port.c \
       coos.c

ifeq ($(LUA), 1)
SRC += lua/lapi.c lua/lcode.c lua/lctype.c lua/ldebug.c lua/ldo.c lua/ldump.c lua/lfunc.c lua/lgc.c lua/llex.c lua/lmem.c \
           lua/lobject.c lua/lopcodes.c lua/lparser.c lua/lstate.c lua/lstring.c lua/ltable.c lua/lrotable.c lua/ltm.c lua/lundump.c lua/lvm.c lua/lzio.c \
           lua/lbaselib.c lua/linit.c lua/lmathlib.c lua/lbitlib.c lua/loadlib.c lua/lauxlib.c lua/ltablib.c lua/lcorolib.c lua/liolib.c lua/lstrlib.c \
           syscalls.c
endif


#       sky/usb/udi_hid.c \

#       sky/usb/USBEndpointDescriptor.c sky/usb/USBGenericRequest.c \
#       sky/usb/USBD_UDP.c sky/usb/USBDDriver.c sky/usb/USBDCallbacks.c \
#       sky/usb/USBFeatureRequest.c sky/usb/USBInterfaceRequest.c sky/usb/USBGetDescriptorRequest.c \
#       sky/usb/USBSetAddressRequest.c sky/usb/USBSetConfigurationRequest.c \
#       sky/usb/USBConfigurationDescriptor.c sky/usb/USBGenericDescriptor.c \
#       sky/usb/hidJoystick.c \
#       sky/usb/HIDDMouseDriver.c \
#       sky/usb/hid_device_descriptor.c \


CPPSRC = stamp.cpp \
         drivers.cpp \
         sdcard_driver.cpp \
         ff.cpp \
         sky/sound.cpp \
         sky/diskio.cpp \
         sky/power.cpp \
         sky/serial.cpp \
         lcd.cpp \
         file.cpp \
         templates.cpp \
         pers.cpp \
         menus.cpp \
         mixer.cpp \
         trims.cpp \
         frsky.cpp \
         audio.cpp \
         ersky9x.cpp \
         timers.cpp \
         logicio.cpp \
         pulses.cpp \
         bluetooth.cpp \
         sky/pulses_driver.cpp \
			en.cpp \
			loadboot.cpp \
			maintenance.cpp \
			mavlink.cpp \
			sbus.cpp \
			pxx2.cpp \
			vars.cpp \
			gvars.cpp \
			logs.cpp

ifeq ($(LUA), 1)
CPPSRC += lua/api_lcd.cpp lua/interface.cpp lua/lua_api.cpp
endif
ifeq ($(BASIC), 1)
	  CPPSRC += basic/parser.cpp
endif

	 ifeq ($(SMALL), 1)
     ifeq ($(LANG1), 0)
	   CPPSRC += fr.cpp
	  else
	   CPPSRC += de.cpp
	  endif
	 else
CPPSRC += fr.cpp de.cpp no.cpp se.cpp it.cpp pl.cpp vi.cpp sp.cpp
	 endif

#CPPSRC += sky/gtime.cpp sky/massstorage.cpp
#SRC +=  sky/usb/USBD_UDP.c sky/usb/USBDDriver.c sky/usb/USBDCallbacks.c \
#       sky/usb/USBEndpointDescriptor.c sky/usb/USBGenericRequest.c sky/usb/USBFeatureRequest.c sky/usb/USBInterfaceRequest.c sky/usb/USBGetDescriptorRequest.c \
#       sky/usb/USBSetAddressRequest.c sky/usb/USBSetConfigurationRequest.c \
#       sky/usb/USBConfigurationDescriptor.c sky/usb/USBGenericDescriptor.c \
#       sky/usb/MSDDriver.c sky/usb/MSDLun.c sky/usb/MSDDriverDescriptors.c sky/usb/SBCMethods.c \
#       sky/MEDSdcard.c sky/usb/MSDDStateMachine.c 

ifeq ($(USB_JOYSTICK), 1)
SRC += sky/joy/HIDDJoystickDriver.c
SRC += sky/joy/HIDDFunction.c
SRC += sky/joy/USBDDriver.c
SRC += sky/joy/USBD.c
SRC += sky/joy/USBRequests.c
SRC += sky/joy/USBDescriptors.c
SRC += sky/joy/USBDDriverCallbacks.c
SRC += sky/joy/USBDCallbacks.c
SRC += sky/joy/HIDIdleRequest.c
SRC += sky/joy/HIDReportRequest.c
SRC += sky/joy/HIDDTransferDriver.c
SRC += sky/joy/USBD_HAL.c
SRC += sky/joy/pmc.c
SRC += sky/joy/device_descriptor.c
endif

#SRC += usb/device/core/USBD_UDP.c usb/device/core/USBDDriver.c usb/device/core/USBDCallbacks.c \
#       usb/common/core/USBEndpointDescriptor.c usb/common/core/USBGenericRequest.c \
#		 usb/common/core/USBFeatureRequest.c usb/common/core/USBInterfaceRequest.c usb/common/core/USBGetDescriptorRequest.c \
#       usb/common/core/USBSetAddressRequest.c usb/common/core/USBSetConfigurationRequest.c \
#       usb/common/core/USBConfigurationDescriptor.c usb/common/core/USBGenericDescriptor.c \
#		 usb/device/hid-joystick/HIDDJoystickDriver.c
#		  \
#       usb/device/massstorage/MSDDriver.c usb/device/massstorage/MSDDStateMachine.c \
#		 usb/device/massstorage/MSDLun.c usb/device/massstorage/MSDDriverDescriptors.c usb/device/massstorage/SBCMethods.c \
#		 sky/MEDSdcard.c


#ifeq ($(REVX), 1)
ifneq ($(SMALL), 1)
 CPPSRC += pdi.cpp
endif
#endif
ifeq ($(DEBUG), 1)
CPPSRC += debug.cpp
endif

# List ASM source files here
ASRC =

endif

endif

endif

endif

endif

endif

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = 

# Define optimisation level here
OPT = -Os

#
# End of user defines
##############################################################################################

INCDIR  = $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))

ifeq ($(RUN_FROM_FLASH), 0)
DEFS    = $(DDEFS) $(UDEFS) -DRUN_FROM_FLASH=0
else
DEFS    = $(DDEFS) $(UDEFS) -DRUN_FROM_FLASH=1
endif

ADEFS   = $(DADEFS) $(UADEFS)

# Generate the (raw) object files lists
AXOBJS    = $(ASRC:%.s=%.o)
CXOBJS    = $(SRC:%.c=%.o)
CPPXOBJS  = $(CPPSRC:%.cpp=%.o)

# Remove directory info from the object files lists
AOBJS    = $(patsubst %,$(OBJDIR)/%,$(notdir $(AXOBJS) ))
COBJS    = $(patsubst %,$(OBJDIR)/%,$(notdir $(CXOBJS) ))
CPPOBJS  = $(patsubst %,$(OBJDIR)/%,$(notdir $(CPPXOBJS) ))
DIRS = $(sort $(dir $(AXOBJS) $(CXOBJS) $(CPPXOBJS) ) )

# Search path for sources
VPATH = $(DIRS)

# Generates the dependancy files lists
INCLUDES = $(AOBJS:.o=.o.d) $(COBJS:.o=.o.d) $(CPPOBJS:.o=.o.d)

OBJS    = $(ASRC:%.s=$(OBJDIR)/%.o) $(SRC:%.c=$(OBJDIR)/%.o) $(CPPSRC:%.cpp=$(OBJDIR)/%.o)
LIBS    = $(DLIBS) $(ULIBS)
MCFLAGS = -mcpu=$(MCU)

XOBJS = $(ASRC:%.s=x9dobj/%.o) $(SRC:%.c=x9dobj/%.o) $(CPPSRC:%.cpp=x9dobj/%.o)
ROBJS = $(ASRC:%.s=skyRobj/%.o) $(SRC:%.c=skyRob/%.o) $(CPPSRC:%.cpp=skyRobj/%.o)

#ASFLAGS = $(MCFLAGS) -g -gdwarf-2 -Wa,-amhls=$(<:.s=.lst) $(ADEFS)
CPFLAGS = $(MCFLAGS) $(OPT) -gdwarf-2 -mthumb -fomit-frame-pointer -Wall -Wstrict-prototypes -fverbose-asm $(DEFS)
ifeq ($(SMALL), 1)
LDFLAGS = $(MCFLAGS) -mthumb -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(FULL_PRJ).map,--cref,--no-warn-mismatch $(LIBDIR) --specs=nano.specs
else ifeq ($(ARUNI), 1)
LDFLAGS = $(MCFLAGS) -mthumb -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(FULL_PRJ).map,--cref,--no-warn-mismatch $(LIBDIR) --specs=nano.specs
else
LDFLAGS = $(MCFLAGS) -mthumb -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(FULL_PRJ).map,--cref,--no-warn-mismatch $(LIBDIR) --specs=nano.specs -u _printf_float
endif
CPPFLAGS = $(MCFLAGS) $(OPT) -gdwarf-2 -mthumb -fomit-frame-pointer -Wall -fverbose-asm $(DEFS)

CPPFLAGS += $(CPPDEFS)
CPFLAGS += $(CPPDEFS)

ifeq ($(PCB), X9D)
CPFLAGS += -Ix9d
CPPFLAGS += -IX9D/usb/src -Ix9d
endif

#   ifeq ($(STAMP), 1)
AVER = ${shell sh -c "grep \"SUB_VERS\" stamp-ersky9x.h | egrep -o \"[[:digit:]]*\""}
VER = $(shell echo $$(( $(AVER) + 1 )))
CPPDEFS += -DSTAMP
#   endif

# If ARUNI board
ifeq ($(ARUNI), 1)
 CPPDEFS += -DREVB -DARUNI
 EXT_MOD=ARUNI
endif

CPPDEFS += -DCPUARM

# If serial Debug required
ifeq ($(DEBUG), 1)
 CPPDEFS += -DDEBUG
endif

# Generate dependency information
CPFLAGS += -MD -MP -MF $(OBJDIR)/$(@F).d
CPPFLAGS += -MD -MP -MF $(OBJDIR)/$(@F).d
#CPFLAGS += -MD -MP -MF .dep/$(@F).d
#CPPFLAGS += -MD -MP -MF .dep/$(@F).d

#
# makefile rules
#

all: $(OBJDIR) size_before stamp .WAIT $(AOBJS) $(COBJS) $(CPPOBJS) $(FULL_PRJ).elf $(FULL_PRJ).bin $(FULL_PRJ).lss .WAIT size

size_before:
#	arm-none-eabi-size $(FULL_PRJ).elf
	@if EXIST $(FULL_PRJ).elf arm-none-eabi-size $(FULL_PRJ).elf
#	@if test -f $(FULL_PRJ).elf; then arm-none-eabi-size $(FULL_PRJ).elf; fi

size:
#	arm-none-eabi-size $(FULL_PRJ).elf
	@if EXIST $(FULL_PRJ).elf arm-none-eabi-size $(FULL_PRJ).elf
#	@if test -f $(FULL_PRJ).elf; then arm-none-eabi-size $(FULL_PRJ).elf; fi

# Build stamp-file
#stamp:
#	@echo
##ifeq ($(STAMP), 1)
#	@echo "Generate Version-stamp:"
##	-rm -f $(OBJDIR)/stamp.o
#	@echo "//Automatically generated file - do not edit" > stamp-ersky9x.h
#	@echo "#define DATE_STR \"`date +%d.%m.%Y`\"" >> stamp-ersky9x.h
#	@echo "#define TIME_STR \"`date +%H:%M:%S`\"" >> stamp-ersky9x.h
##	@echo "#define SUB_VERS $(VER)-$(USER)" >> stamp-ersky9x.h
# ifeq ($(PCB), X9D)
#	@echo "#define SVN_VERS \"erx9d-$(PROV)r$(REV)\"" >> stamp-ersky9x.h
#	@echo "#define MOD_VERS \"$(EXT_MOD)\"" >> stamp-ersky9x.h
# else	
#	@echo "#define SVN_VERS \"ersky9x-$(PROV)r$(REV)\"" >> stamp-ersky9x.h
#	@echo "#define MOD_VERS \"$(EXT_MOD)\"" >> stamp-ersky9x.h
# endif
##	@cat stamp-ersky9x.h
##endif

stamp:
	@echo
#ifeq ($(STAMP), 1)
	@echo Generate Version-stamp:
	@echo //Automatically generated file - do not edit > stamp-ersky9x.h
	@echo #define DATE_STR ^"%date:~0,2%.%date:~3,2%.%date:~6,4%%^">> stamp-ersky9x.h
	@echo #define TIME_STR ^"%time:~0,8%^">> stamp-ersky9x.h
 ifeq ($(PCB), X9D)
	@echo #define SVN_VERS ^"erx9d-\>> stamp-ersky9x.h
 else	
	@echo #define SVN_VERS ^"erskyTx-\>> stamp-ersky9x.h
 endif
	@type prov.txt >> stamp-ersky9x.h
	@echo r\>> stamp-ersky9x.h
	@type rev.txt >> stamp-ersky9x.h
	@echo ^">> stamp-ersky9x.h
#	@echo // >> stamp-ersky9x.h
	@echo #define MOD_VERS "$(EXT_MOD)" >> stamp-ersky9x.h
#endif


.NOTPARALLEL: stamp-ersky9x.h

$(OBJDIR):
	if NOT EXIST $(OBJDIR) mkdir $(OBJDIR)
#	test -d $(OBJDIR) || $(shell mkdir -p $(OBJDIR) )

#$(OBJDIR)/%.o : %.cpp
#	$(CC) -c $(CPPFLAGS) -fno-exceptions -I . $(INCDIR) $< -o $@
$(CPPOBJS) : $(OBJDIR)/%.o : %.cpp
#	@mkdir -p $(@D)
	$(CC) -c $(CPPFLAGS) -fno-exceptions -fno-math-errno -I . $(INCDIR) $< -o $@

#$(OBJDIR)/%.o : %.c
#	$(CC) -c $(CPFLAGS) -I . $(INCDIR) $< -o $@
$(COBJS) : $(OBJDIR)/%.o : %.c
#	@mkdir -p $(@D)
	$(CC) -c $(CPFLAGS) -I . $(INCDIR) $< -o $@

#$(OBJDIR)/%.o : %.s
#	$(AS) -c $(ASFLAGS) $< -o $@
$(AOBJS) : $(OBJDIR)/%.o : %.s
#	@mkdir -p $(@D)
	$(AS) -c $(ASFLAGS) $< -o $@

#%elf: $(OBJS)
#	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
%elf: $(AOBJS) $(COBJS) $(CPPOBJS) 
	$(CC) $(AOBJS) $(COBJS) $(CPPOBJS) $(LDFLAGS) $(LIBS) -o $@
  
# Create extended listing file from ELF output file.
%.lss: %.elf
	$(CLSS) -h -S $< > $@

#%hex: %elf
#	$(BIN) $< $@

%bin: %elf
	$(BINX) $< $@

clean:
#	-rm -f x9dobj/*.*
#	-rm -f x9eobj/*.*
#	-rm -f x9dpobj/*.*
#	-rm -f x12dobj/*.*
#	-rm -f skyobj/*.*
#	-rm -f skyRobj/*.*
#	-rm -f skySobj/*.*
#	-rm -f sky9XTobj/*.*
#	-rm -f x7obj/*.*
#	-rm -f xliteobj/*.*
#	-rm -f x9lobj/*.*
#	-rm -f t12obj/*.*
#	-rm -f t16obj/*.*
#	-rm -f lem1obj/*.*
#	-rm -f x9lsobj/*.*
	-rm -f $(OBJROOT)/obj_x9d/*.*
	-rm -f $(OBJROOT)/obj_x9e/*.*
	-rm -f $(OBJROOT)/obj_x9dp/*.*
	-rm -f $(OBJROOT)/obj_x12d/*.*
	-rm -f $(OBJROOT)/obj_ersky9xs/*.*
	-rm -f $(OBJROOT)/obj_ersky9xsf/*.*
	-rm -f $(OBJROOT)/obj_ersky9xsd/*.*
	-rm -f $(OBJROOT)/obj_ersky9xr/*.*
	-rm -f $(OBJROOT)/obj_ersky9x/*.*
	-rm -f $(OBJROOT)/obj_ersky9x9XT/*.*
	-rm -f $(OBJROOT)/obj_x7/*.*
	-rm -f $(OBJROOT)/obj_x7Acc/*.*
	-rm -f $(OBJROOT)/obj_xlite/*.*
	-rm -f $(OBJROOT)/obj_xlites/*.*
	-rm -f $(OBJROOT)/obj_x9Lite/*.*
	-rm -f $(OBJROOT)/obj_t12/*.*
	-rm -f $(OBJROOT)/obj_t16/*.*
	-rm -f $(OBJROOT)/obj_lemon1/*.*
	-rm -f $(OBJROOT)/obj_x9LiteS/*.*
	-rm -f $(OBJROOT)/obj_x10/*.*
	-rm -f $(OBJROOT)/obj_x10E/*.*
	-rm -f $(OBJROOT)/obj_tx16s/*.*
	-rm -f $(OBJROOT)/obj_tx18s/*.*
	-rm -f $(OBJROOT)/obj_aruni/*.*
#	-rm -f $(FULL_PRJ).elf
#	-rm -f $(FULL_PRJ).map
#	-rm -f $(FULL_PRJ).hex
#	-rm -f $(SRC:.c=.c.bak)
#	-rm -f $(SRC:.c=.lst)
#	-rm -f $(SRC:.c=.lss)
#	-rm -f $(ASRC:.s=.s.bak)
#	-rm -f $(ASRC:.s=.lst)
#	-rm -fR .dep

# 
# Include the dependency files, should be the last of the makefile
#
#-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)
# Add in all the dependancy files
-include $(INCLUDES)

# *** EOF ***
