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
# make REVB=1 REVX=1
# make PCB=X9D BT=1
# make PCB=X9D REV9E=1
# make PCB=X9D REVPLUS=1
# make PCB=9XT
# make REVB=1 SMALL=1
# make PCB=X9D PCBX7=1 BT=1
# make REVB=1
# make PCB=X12D
# make PCB=X9D PCBXLITE=1
# make PCB=X9D PCBX7=1 PCBT12=1
# make PCB=X9D PCBX9LITE=1
# make PCB=X10 PCBT16=1
# make PCBLEM1=1
# make PCB=X9D PCBX9LITE=1 X9LS=1 
#
##############################################################################################
# Start of default section
#

TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CP   = $(TRGT)objcopy
CLSS = $(TRGT)objdump
AS   = $(TRGT)as
#gcc -x assembler-with-cpp
BIN  = $(CP) -O ihex 
BINX = $(CP) -O binary 

# Object files directory
ifeq ($(PCB), X10)
 ifeq ($(PCBT16), 1)
  CPPDEFS += -DPCBT16
  OBJDIR = t16obj
 else
  OBJDIR = x10obj
 endif
  BASIC = 1
  CPPDEFS += -DBASIC 
#  LUA = 1
#  CPPDEFS += -DLUA
else
ifeq ($(PCB), X12D)
  OBJDIR = x12dobj
  BASIC = 1
  CPPDEFS += -DBASIC 
#  LUA = 1
#  CPPDEFS += -DLUA
else
ifeq ($(PCB), X9D)
 ifeq ($(REVPLUS), 1)
  OBJDIR = x9dpobj
  BASIC = 1
  CPPDEFS += -DBASIC 
#  LUA = 1
#  CPPDEFS += -DLUA
 else
  ifeq ($(REV9E), 1)
   OBJDIR = x9eobj
   BASIC = 1
   CPPDEFS += -DBASIC 
#   LUA = 1
#   CPPDEFS += -DLUA
  else
   ifeq ($(PCBX7), 1)
    ifeq ($(PCBT12), 1)
     CPPDEFS += -DPCBT12
     OBJDIR = t12obj
	 else
     OBJDIR = x7obj
	 endif
    BASIC = 1
    CPPDEFS += -DBASIC 
#    LUA = 1
#    CPPDEFS += -DLUA
   else
    ifeq ($(PCBXLITE), 1)
     OBJDIR = xliteobj
     BASIC = 1
     CPPDEFS += -DBASIC 
    else
     ifeq ($(PCBX9LITE), 1)
	   ifeq ($(X9LS), 1)
       OBJDIR = x9lsobj
		 BT = 1
		else
       OBJDIR = x9lobj
		endif
      BASIC = 1
      CPPDEFS += -DBASIC 
     else
      OBJDIR = x9dobj
      BASIC = 1
      CPPDEFS += -DBASIC 
#      LUA = 1
#      CPPDEFS += -DLUA
	  endif
	 endif
	endif
  endif
 endif
else
ifeq ($(PCBLEM1), 1)
 CPPDEFS += -DBASIC 
 BASIC = 1
 OBJDIR = lem1obj
else
 ifeq ($(PCB), DUE)
  OBJDIR = skyDobj
 else
  ifeq ($(PCB), 9XT)
    OBJDIR = sky9XTobj
    BASIC = 1
    CPPDEFS += -DBASIC 
#    LUA = 1
#    CPPDEFS += -DLUA
  else
   USB_JOYSTICK = 1
   ifeq ($(REVX), 1)
    OBJDIR = skyRobj
    BASIC = 1
    CPPDEFS += -DBASIC 
#    LUA = 1
#    CPPDEFS += -DLUA
   else
    ifeq ($(SMALL), 1)
     OBJDIR = skySobj
     BASIC = 1
     CPPDEFS += -DBASIC 
    else
     BASIC = 1
     CPPDEFS += -DBASIC 
#     LUA = 1
#     CPPDEFS += -DLUA
     OBJDIR = skyobj
    endif
   endif
  endif
 endif
endif
endif
endif
endif

MCU  = cortex-m3

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

##############################################################################################
# Start of user section
#

# 
# Define project name and Ram/Flash mode here
ifeq ($(PCB), X10)
 ifeq ($(PCBT16), 1)
  PROJECT        = t16
 else
  PROJECT        = x10
 endif
else
ifeq ($(PCB), X12D)
  PROJECT        = x12d
else
ifeq ($(PCB), X9D)
 ifeq ($(REVPLUS), 1)
  PROJECT        = x9dp
 else
  ifeq ($(REV9E), 1)
   PROJECT        = x9e
  else
   ifeq ($(PCBX7), 1)
    ifeq ($(PCBT12), 1)
	  PROJECT        = t12
    else
	  PROJECT        = x7
	 endif
   else
    ifeq ($(PCBXLITE), 1)
     PROJECT        = xlite
    else
     ifeq ($(PCBX9LITE), 1)
	   ifeq ($(X9LS), 1)
       PROJECT        = x9LiteS
      else
       PROJECT        = x9Lite
		endif
     else
      PROJECT        = x9d
     endif
    endif
   endif
  endif
 endif
else
ifeq ($(PCBLEM1), 1)
 PROJECT = lemon1
else
 ifeq ($(PCB), 9XT)
   PROJECT      = ersky9x9XT
 else
  ifeq ($(REVX), 1)
   PROJECT      = ersky9xr
  else
   ifeq ($(PCB), DUE)
    PROJECT      = ersky9xd
   else
    ifeq ($(SMALL), 1)
     PROJECT      = ersky9xs
    else
     ifeq ($(LARGE), 1)
      PROJECT      = ersky9xl
     else
      PROJECT      = ersky9x
     endif
    endif
   endif
  endif
 endif
endif
endif
endif
endif

RUN_FROM_FLASH = 1

#
# Define linker script file here
#
ifeq ($(RUN_FROM_FLASH), 0)
 ifeq ($(SAM3S2), 1)
 LDSCRIPT = sam3s2c_ram.ld
 else
   ifeq ($(PCB), X9D)
#     ARCH = ARM
     LDSCRIPT = X9D/stm32_flash.ld
#     TRGT = arm-none-eabi-
#     MCU  = cortex-m3
     CPPDEFS += -DHSE_VALUE=12000000
	  CPPDEFS += -DPCBX9D 
     UDEFS = -Dstm32f205
   else
    ifeq ($(PCB), X12D)
     LDSCRIPT = X12D/stm32f4_flash.ld
     CPPDEFS += -DHSE_VALUE=12000000
	  CPPDEFS += -DPCBX12D 
     UDEFS = -DSTM32F429_439xx
	 else
     LDSCRIPT = sam3s4c_ram.ld
	  CPPDEFS += -DPCBSKY 
     UDEFS = -Dat91sam3s4
	 endif
	endif
 endif
 FULL_PRJ = $(PROJECT)_ram
else
ifeq ($(PCBLEM1), 1)
 LDSCRIPT = STM32F103VETx_FLASH.ld
 CPPDEFS += -DHSE_VALUE=8000000
 CPPDEFS += -DPCBLEM1 
 FULL_PRJ = $(PROJECT)_rom
 UDEFS = -Dstm32f103
 UDEFS = -DSTM32F10X_HD
else
 ifeq ($(SAM3S2), 1)
 CPPDEFS += -DPCBSKY 
 LDSCRIPT = sam3s2c_flash.ld
 else
  ifeq ($(PCB), 9XT)
   LDSCRIPT = X9D/stm32_flash.ld
   CPPDEFS += -DHSE_VALUE=12000000
   CPPDEFS += -DPCB9XT 
   CPPDEFS += -DPCBSP
   CPPDEFS += -DUSB_JOYSTICK
   UDEFS = -Dstm32f205
  else
   ifeq ($(PCB), X9D)
#     ARCH = ARM
     LDSCRIPT = X9D/stm32_flash.ld
#     TRGT = arm-none-eabi-
#     MCU  = cortex-m3
     CPPDEFS += -DHSE_VALUE=12000000
	  CPPDEFS += -DPCBX9D 
     CPPDEFS += -DUSB_JOYSTICK
     UDEFS = -Dstm32f205
     ifeq ($(PCBX7), 1)
      CPPDEFS += -DPCBX7
     endif
     ifeq ($(PCBXLITE), 1)
      CPPDEFS += -DPCBXLITE
     endif
     ifeq ($(PCBX9LITE), 1)
      CPPDEFS += -DPCBX9LITE
	   ifeq ($(X9LS), 1)
       CPPDEFS += -DX9LS 
      endif
     endif
   else
    ifeq ($(PCB), X10)
     LDSCRIPT = X12D/stm32f4_flash.ld
     CPPDEFS += -DHSE_VALUE=12000000
	  CPPDEFS += -DPCBX10 
     CPPDEFS += -DUSB_JOYSTICK
     UDEFS = -DSTM32F429_439xx
	 else
    ifeq ($(PCB), X12D)
     LDSCRIPT = X12D/stm32f4_flash.ld
     CPPDEFS += -DHSE_VALUE=12000000
	  CPPDEFS += -DPCBX12D 
     CPPDEFS += -DUSB_JOYSTICK
     UDEFS = -DSTM32F429_439xx
	 else
    CPPDEFS += -DPCBSKY 
	 ifeq ($(SMALL), 1)
    	CPPDEFS += -DSMALL 
	 endif
    ifeq ($(PCB), DUE)
      LDSCRIPT = due/sam3x8_flash.ld
      UDEFS = -Dat91sam3x8
		UDEFS  += -D__SAM3X8E__
      CPPDEFS += -DPCBDUE 
	 else
     ifeq ($(REVX), 1)
      LDSCRIPT = sam3s8c_flash.ld
      UDEFS = -Dat91sam3s8
     else
      ifeq ($(SMALL), 1)
       LDSCRIPT = sam3s4c_flash.ld
       UDEFS = -Dat91sam3s4
      else
       ifeq ($(LARGE), 1)
        LDSCRIPT = sam4s8c_flash.ld
        UDEFS = -Dat91sam3s8
       else
        LDSCRIPT = sam3s8c_flash.ld
        UDEFS = -Dat91sam3s8
       endif
      endif
     endif
    endif
	 endif
	 endif
	endif
  endif
 endif
 FULL_PRJ = $(PROJECT)_rom
endif
endif

# List all user C define here, like -D_DEBUG=1

#ifeq ($(PCB), X9D)
#UDEFS = 
#endif

ifeq ($(SAM3S2), 1)
UDEFS = -Dat91sam3s2
endif

ifeq ($(LANG), FR)
 CPPDEFS += -DLANG_FR
else
 ifeq ($(LANG), DE)
  CPPDEFS += -DLANG_DE
 endif
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
SRC += LUA/lapi.c LUA/lcode.c LUA/lctype.c LUA/ldebug.c LUA/ldo.c LUA/ldump.c LUA/lfunc.c LUA/lgc.c LUA/llex.c LUA/lmem.c \
           LUA/lobject.c LUA/lopcodes.c LUA/lparser.c LUA/lstate.c LUA/lstring.c LUA/ltable.c LUA/lrotable.c LUA/ltm.c LUA/lundump.c LUA/lvm.c LUA/lzio.c \
           LUA/lbaselib.c LUA/linit.c LUA/lmathlib.c LUA/lbitlib.c LUA/loadlib.c LUA/lauxlib.c LUA/ltablib.c LUA/lcorolib.c LUA/liolib.c \
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
         mixer.cpp \
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
			lcd.cpp

ifeq ($(BASIC), 1)
	  CPPSRC += basic/parser.cpp
endif
ifeq ($(LUA), 1)
CPPSRC += LUA/api_lcd.cpp LUA/interface.cpp LUA/lua_api.cpp
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
SRC += LUA/lapi.c LUA/lcode.c LUA/lctype.c LUA/ldebug.c LUA/ldo.c LUA/ldump.c LUA/lfunc.c LUA/lgc.c LUA/llex.c LUA/lmem.c \
           LUA/lobject.c LUA/lopcodes.c LUA/lparser.c LUA/lstate.c LUA/lstring.c LUA/ltable.c LUA/lrotable.c LUA/ltm.c LUA/lundump.c LUA/lvm.c LUA/lzio.c \
           LUA/lbaselib.c LUA/linit.c LUA/lmathlib.c LUA/lbitlib.c LUA/loadlib.c LUA/lauxlib.c LUA/ltablib.c LUA/lcorolib.c LUA/liolib.c \
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
         mixer.cpp \
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
			lcd.cpp

ifeq ($(BASIC), 1)
	  CPPSRC += basic/parser.cpp
endif
ifeq ($(LUA), 1)
CPPSRC += LUA/api_lcd.cpp LUA/interface.cpp LUA/lua_api.cpp
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
SRC += LUA/lapi.c LUA/lcode.c LUA/lctype.c LUA/ldebug.c LUA/ldo.c LUA/ldump.c LUA/lfunc.c LUA/lgc.c LUA/llex.c LUA/lmem.c \
           LUA/lobject.c LUA/lopcodes.c LUA/lparser.c LUA/lstate.c LUA/lstring.c LUA/ltable.c LUA/lrotable.c LUA/ltm.c LUA/lundump.c LUA/lvm.c LUA/lzio.c \
           LUA/lbaselib.c LUA/linit.c LUA/lmathlib.c LUA/lbitlib.c LUA/loadlib.c LUA/lauxlib.c LUA/ltablib.c LUA/lcorolib.c LUA/liolib.c \
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
		   lcd.cpp

ifeq ($(BASIC), 1)
	  CPPSRC += basic/parser.cpp
endif

ifeq ($(BT), 1)
  CPPSRC += bluetooth.cpp
  CPPDEFS += -DBT
endif


ifeq ($(LUA), 1)
CPPSRC += LUA/api_lcd.cpp LUA/interface.cpp LUA/lua_api.cpp
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
SRC += LUA/lapi.c LUA/lcode.c LUA/lctype.c LUA/ldebug.c LUA/ldo.c LUA/ldump.c LUA/lfunc.c LUA/lgc.c LUA/llex.c LUA/lmem.c \
           LUA/lobject.c LUA/lopcodes.c LUA/lparser.c LUA/lstate.c LUA/lstring.c LUA/ltable.c LUA/lrotable.c LUA/ltm.c LUA/lundump.c LUA/lvm.c LUA/lzio.c \
           LUA/lbaselib.c LUA/linit.c LUA/lmathlib.c LUA/lbitlib.c LUA/loadlib.c LUA/lauxlib.c LUA/ltablib.c LUA/lcorolib.c LUA/liolib.c \
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
		   lcd.cpp

ifeq ($(BASIC), 1)
	  CPPSRC += basic/parser.cpp
endif
ifeq ($(LUA), 1)
CPPSRC += LUA/api_lcd.cpp LUA/interface.cpp LUA/lua_api.cpp
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
       coos.c

CPPSRC = ersky9x.cpp \
         stm103/t.cpp \
			stm103/lcd_driver.cpp \
			stm103/sound.cpp \
       	stm103/diskio.cpp \
			stm103/power.cpp \
         stm103/watchdog.cpp \
         stm103/pulsesDriver.cpp \
			stamp.cpp \
         templates.cpp \
			frsky.cpp \
			menus.cpp \
			mixer.cpp \
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
SRC += LUA/lapi.c LUA/lcode.c LUA/lctype.c LUA/ldebug.c LUA/ldo.c LUA/ldump.c LUA/lfunc.c LUA/lgc.c LUA/llex.c LUA/lmem.c \
           LUA/lobject.c LUA/lopcodes.c LUA/lparser.c LUA/lstate.c LUA/lstring.c LUA/ltable.c LUA/lrotable.c LUA/ltm.c LUA/lundump.c LUA/lvm.c LUA/lzio.c \
           LUA/lbaselib.c LUA/linit.c LUA/lmathlib.c LUA/lbitlib.c LUA/loadlib.c LUA/lauxlib.c LUA/ltablib.c LUA/lcorolib.c LUA/liolib.c \
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
         frsky.cpp \
         audio.cpp \
         ersky9x.cpp \
         timers.cpp \
         logicio.cpp \
         pulses.cpp \
         bluetooth.cpp \
         sky/pulses_driver.cpp \
			en.cpp \
			de.cpp \
			fr.cpp \
			loadboot.cpp \
			maintenance.cpp \
			mavlink.cpp \
			sbus.cpp \
			pxx2.cpp \
			logs.cpp

ifeq ($(LUA), 1)
CPPSRC += LUA/api_lcd.cpp LUA/interface.cpp LUA/lua_api.cpp
endif
ifeq ($(BASIC), 1)
	  CPPSRC += basic/parser.cpp
endif

	 ifeq ($(SMALL), 1)
	 else
CPPSRC += no.cpp se.cpp it.cpp pl.cpp vi.cpp sp.cpp
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

# List all user directories here
UINCDIR = ./sky/joy/inc
# \
#          ./cmsis/core \
#          ./cmsis/device

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
LDFLAGS = $(MCFLAGS) -mthumb -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(FULL_PRJ).map,--cref,--no-warn-mismatch $(LIBDIR)
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

# If REVB board
ifeq ($(REVB), 1)
 CPPDEFS += -DREVB
 EXT_MOD=REVB
endif

# If ARUNI board
ifeq ($(ARUNI), 1)
 CPPDEFS += -DREVB -DARUNI
 EXT_MOD=ARUNI
endif

# If REVX board
ifeq ($(REVX), 1)
 CPPDEFS += -DREVX
 CPPDEFS += -DXFIRE
 EXT_MOD=REVB-X
endif

ifeq ($(PCB), 9XT)
 CPPDEFS += -DXFIRE
 EXT_MOD=9XT
endif

ifeq ($(PCB), X12D)
 EXT_MOD=X12D
endif

ifeq ($(PCB), X10)
 ifeq ($(PCBT16), 1)
  EXT_MOD=T16
 else
  EXT_MOD=X10
 endif
endif

ifeq ($(PCBLEM1), 1)
 EXT_MOD=Lemon
endif

ifeq ($(PCB), X9D)
 CPPDEFS += -DXFIRE
# If Taranis+
 ifeq ($(REVPLUS), 1)
  CPPDEFS += -DREVPLUS
  EXT_MOD=X9DP
 else
# If Taranis X9E
  ifeq ($(REV9E), 1)
#   CPPDEFS += -DREVPLUS
   CPPDEFS += -DREV9E
   EXT_MOD=X9E
  else
   ifeq ($(PCBXLITE), 1)
    EXT_MOD=XLITE
   else
    ifeq ($(PCBX7), 1)
     ifeq ($(PCBT12), 1)
      EXT_MOD=T12
     else
      EXT_MOD=QX7
	  endif
	 else
     ifeq ($(PCBX9LITE), 1)
	   ifeq ($(X9LS), 1)
       EXT_MOD=X9LiteS
	   else
       EXT_MOD=X9Lite
      endif
	  else
     CPPDEFS += -DREVNORM
     EXT_MOD=X9D
     endif
    endif
   endif
  endif
 endif
endif

#CPPDEFS += -DFRSKY
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

all: size_before stamp $(AOBJS) $(COBJS) $(CPPOBJS) $(FULL_PRJ).elf $(FULL_PRJ).bin $(FULL_PRJ).lss size

size_before:
#	arm-none-eabi-size $(FULL_PRJ).elf
	@if test -f $(FULL_PRJ).elf; then arm-none-eabi-size $(FULL_PRJ).elf; fi

size:
#	arm-none-eabi-size $(FULL_PRJ).elf
	@if test -f $(FULL_PRJ).elf; then arm-none-eabi-size $(FULL_PRJ).elf; fi

# Build stamp-file
stamp:
	@echo
#ifeq ($(STAMP), 1)
	@echo "Generate Version-stamp:"
#	-rm -f $(OBJDIR)/stamp.o
	@echo "//Automatically generated file - do not edit" > stamp-ersky9x.h
	@echo "#define DATE_STR \"`date +%d.%m.%Y`\"" >> stamp-ersky9x.h
	@echo "#define TIME_STR \"`date +%H:%M:%S`\"" >> stamp-ersky9x.h
#	@echo "#define SUB_VERS $(VER)-$(USER)" >> stamp-ersky9x.h
 ifeq ($(PCB), X9D)
	@echo "#define SVN_VERS \"erx9d-$(PROV)r$(REV)\"" >> stamp-ersky9x.h
	@echo "#define MOD_VERS \"$(EXT_MOD)\"" >> stamp-ersky9x.h
 else	
	@echo "#define SVN_VERS \"ersky9x-$(PROV)r$(REV)\"" >> stamp-ersky9x.h
	@echo "#define MOD_VERS \"$(EXT_MOD)\"" >> stamp-ersky9x.h
 endif
#	@cat stamp-ersky9x.h
#endif

$(OBJDIR):
	test -d $(OBJDIR) || $(shell mkdir -p $(OBJDIR) )

#$(OBJDIR)/%.o : %.cpp
#	$(CC) -c $(CPPFLAGS) -fno-exceptions -I . $(INCDIR) $< -o $@
$(CPPOBJS) : $(OBJDIR)/%.o : %.cpp
	@mkdir -p $(@D)
	$(CC) -c $(CPPFLAGS) -fno-exceptions -I . $(INCDIR) $< -o $@

#$(OBJDIR)/%.o : %.c
#	$(CC) -c $(CPFLAGS) -I . $(INCDIR) $< -o $@
$(COBJS) : $(OBJDIR)/%.o : %.c
	@mkdir -p $(@D)
	$(CC) -c $(CPFLAGS) -I . $(INCDIR) $< -o $@

#$(OBJDIR)/%.o : %.s
#	$(AS) -c $(ASFLAGS) $< -o $@
$(AOBJS) : $(OBJDIR)/%.o : %.s
	@mkdir -p $(@D)
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
	-rm -f x9dobj/*.*
	-rm -f x9eobj/*.*
	-rm -f x9dpobj/*.*
	-rm -f x12dobj/*.*
	-rm -f skyobj/*.*
	-rm -f skyRobj/*.*
	-rm -f skySobj/*.*
	-rm -f sky9XTobj/*.*
	-rm -f x7obj/*.*
	-rm -f xliteobj/*.*
	-rm -f x9lobj/*.*
	-rm -f t12obj/*.*
	-rm -f t16obj/*.*
	-rm -f lem1obj/*.*
	-rm -f x9lsobj/*.*
	-rm -f $(FULL_PRJ).elf
	-rm -f $(FULL_PRJ).map
	-rm -f $(FULL_PRJ).hex
	-rm -f $(SRC:.c=.c.bak)
	-rm -f $(SRC:.c=.lst)
	-rm -f $(SRC:.c=.lss)
	-rm -f $(ASRC:.s=.s.bak)
	-rm -f $(ASRC:.s=.lst)
	-rm -fR .dep

# 
# Include the dependency files, should be the last of the makefile
#
#-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)
# Add in all the dependancy files
-include $(INCLUDES)

# *** EOF ***
