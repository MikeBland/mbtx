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
ifeq ($(PCB), X9D)
 ifeq ($(REVPLUS), 1)
  OBJDIR = x9dpobj
 else
  ifeq ($(REV9E), 1)
   OBJDIR = x9eobj
  else
   OBJDIR = x9dobj
  endif
 endif
else
 ifeq ($(PCB), DUE)
  OBJDIR = skyDobj
 else
  ifeq ($(PCB), 9XT)
    OBJDIR = sky9XTobj
  else
   ifeq ($(REVX), 1)
    OBJDIR = skyRobj
   else
    OBJDIR = skyobj
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
AREV = $(shell sh -c "cat .svn/entries | sed -n '4p'")
REV = $(shell echo $$(( $(AREV) + 1 )))
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
ifeq ($(PCB), X9D)
 ifeq ($(REVPLUS), 1)
  PROJECT        = x9dp
 else
  ifeq ($(REV9E), 1)
   PROJECT        = x9e
  else
   PROJECT        = x9d
  endif
 endif
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
    PROJECT      = ersky9x
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
   else
     LDSCRIPT = sam3s4c_ram.ld
	  CPPDEFS += -DPCBSKY 
     UDEFS = -Dat91sam3s4
	endif
 endif
 FULL_PRJ = $(PROJECT)_ram
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
  else
   ifeq ($(PCB), X9D)
#     ARCH = ARM
     LDSCRIPT = X9D/stm32_flash.ld
#     TRGT = arm-none-eabi-
#     MCU  = cortex-m3
     CPPDEFS += -DHSE_VALUE=12000000
	  CPPDEFS += -DPCBX9D 
     CPPDEFS += -DUSB_JOYSTICK
   else
    CPPDEFS += -DPCBSKY 
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
      LDSCRIPT = sam3s4c_flash.ld
      UDEFS = -Dat91sam3s4
     endif
    endif
	endif
  endif
 endif
 FULL_PRJ = $(PROJECT)_rom
endif

# List all user C define here, like -D_DEBUG=1

ifeq ($(PCB), X9D)
UDEFS = 
endif

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

# Define ASM defines here
UADEFS = 

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
			stamp.cpp \
         templates.cpp \
         drivers.cpp \
         timers.cpp \
         logicio.cpp \
         analog.cpp \
         debug.cpp \
			frsky.cpp \
         menus.cpp \
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
			loadboot.cpp \
			maintenance.cpp \
			mavlink.cpp \
			sbus.cpp \
         X9D/usbd_usr.cpp \
		   lcd.cpp

#         gtime.cpp \
#         massstorage.cpp \
#         X9D/usbd_usr.cpp \
#         X9D/usbd_storage_msd.cpp \

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
         debug.cpp \
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


CPPSRC = ersky9x.cpp \
         X9D/aspi.cpp \
         X9D/sound.cpp \
         X9D/diskio.cpp \
         X9D/power.cpp \
         X9D/pulses_driver.cpp \
         X9D/watchdog.cpp \
         X9D/rtc_driver.cpp \
         X9D/i2c_9xt.cpp \
			stamp.cpp \
         templates.cpp \
         drivers.cpp \
         timers.cpp \
         logicio.cpp \
         analog.cpp \
         debug.cpp \
         file.cpp \
			frsky.cpp \
         pers.cpp \
         menus.cpp \
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
			loadboot.cpp \
			maintenance.cpp \
			mavlink.cpp \
			sbus.cpp \
			mega64.cpp \
			isp.cpp \
         X9D/usbd_usr.cpp \
		   lcd.cpp

#         X9D/lcd_driver.cpp \
#       	X9D/i2c_ee.cpp \
#         X9D/eeprom_rlc.cpp \

# List ASM source files here
ASRC = X9D/startup_stm32f2xx.s

else

# List C source files here
SRC  = core_cm3.c \
       board_lowlevel.c \
       crt.c \
       vectors_sam3s.c \
       port.c \
       coos.c

#       usb/hid_device_descriptor.c \
#       usb/hidJoystick.c \
#       usb/HIDDMouseDriver.c \
#       usb/USBDDriver.c \


#       usb/device/core/USBD_UDP.c usb/device/core/USBDDriver.c usb/device/core/USBDCallbacks.c \
#       usb/device/massstorage/MSDDriver.c usb/device/massstorage/MSDDStateMachine.c usb/device/massstorage/MSDLun.c usb/device/massstorage/MSDDriverDescriptors.c usb/device/massstorage/SBCMethods.c \
#       usb/common/core/USBEndpointDescriptor.c usb/common/core/USBGenericRequest.c usb/common/core/USBFeatureRequest.c usb/common/core/USBInterfaceRequest.c usb/common/core/USBGetDescriptorRequest.c \
#       usb/common/core/USBSetAddressRequest.c usb/common/core/USBSetConfigurationRequest.c \
#       usb/common/core/USBConfigurationDescriptor.c usb/common/core/USBGenericDescriptor.c \
#       MEDSdcard.c

CPPSRC = stamp.cpp \
         drivers.cpp \
         sdcard_driver.cpp \
         ff.cpp \
         sky/sound.cpp \
         sky/diskio.cpp \
         sky/power.cpp \
         lcd.cpp \
         debug.cpp \
         file.cpp \
         templates.cpp \
         pers.cpp \
         menus.cpp \
         frsky.cpp \
         audio.cpp \
         ersky9x.cpp \
         timers.cpp \
         logicio.cpp \
         pulses.cpp \
			en.cpp \
			de.cpp \
			no.cpp \
			fr.cpp \
			se.cpp \
			it.cpp \
			pl.cpp \
			loadboot.cpp \
			maintenance.cpp \
			mavlink.cpp \
			sbus.cpp \
			logs.cpp

#         gtime.cpp \
#         massstorage.cpp \


#ifeq ($(REVX), 1)
# CPPSRC += pl.cpp
#endif

# List ASM source files here
ASRC =

endif

endif

endif


# List all user directories here
UINCDIR = ./inc
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

# If REVX board
ifeq ($(REVX), 1)
 CPPDEFS += -DREVX
 EXT_MOD=REVB-X
endif

# If Flight Phases
ifeq ($(PHASES), 1)
 CPPDEFS += -DPHASES
endif

ifeq ($(PCB), 9XT)
  EXT_MOD=9XT
endif

ifeq ($(PCB), X9D)
# If Taranis+
 ifeq ($(REVPLUS), 1)
  CPPDEFS += -DREVPLUS
  EXT_MOD=X9DP
 else
# If Taranis X9E
  ifeq ($(REV9E), 1)
   CPPDEFS += -DREVPLUS
   CPPDEFS += -DREV9E
   EXT_MOD=X9E
  else
   EXT_MOD=X9D
  endif
 endif
endif

CPPDEFS += -DFRSKY
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

all: size_before stamp $(AOBJS) $(COBJS) $(CPPOBJS) $(FULL_PRJ).elf $(FULL_PRJ).hex $(FULL_PRJ).bin $(FULL_PRJ).lss size

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
	@echo "#define SUB_VERS $(VER)-$(USER)" >> stamp-ersky9x.h
 ifeq ($(PCB), X9D)
	@echo "#define SVN_VERS \"erx9d-r$(REV)\"" >> stamp-ersky9x.h
	@echo "#define MOD_VERS \"$(EXT_MOD)\"" >> stamp-ersky9x.h
 else	
	@echo "#define SVN_VERS \"ersky9x-r$(REV)\"" >> stamp-ersky9x.h
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

%hex: %elf
	$(BIN) $< $@

%bin: %elf
	$(BINX) $< $@

clean:
	-rm -f x9dobj/*.*
	-rm -f x9eobj/*.*
	-rm -f x9dpobj/*.*
	-rm -f skyobj/*.*
	-rm -f skyRobj/*.*
	-rm -f skyDobj/*.*
	-rm -f sky9XTobj/*.*
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
