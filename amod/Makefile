###############################################################################
# Makefile for the project amod_last
###############################################################################

## General Flags

MCU = atmega328p
CC = avr-gcc

CPP = avr-g++


ifeq ($(SPEED), 16)
CDEFS = -DF_CPU=16000000UL
 ifeq ($(SERIAL), 1)
  CDEFS += -DSERIAL_VOICE=1
  PROJECT = amod_lastSer16
  TARGET = amod_last16Ser.elf
 else
  PROJECT = amod_last16
  TARGET = amod_last16.elf
 endif
else
CDEFS = -DF_CPU=12000000UL
 ifeq ($(SERIAL), 1)
  CDEFS += -DSERIAL_VOICE=1
  PROJECT = amod_lastSer
  TARGET = amod_lastSer.elf
 else
  PROJECT = amod_last
  TARGET = amod_last.elf
 endif
endif

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 $(CDEFS) -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=$(PROJECT).map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
#OBJECTS = amod328.o asmfunc.o mmc.o pff.o ff.o fmmc.o ymodem.o ff_lfn.o
OBJECTS = amod328.o asmfunc.o ff.o fmmc.o ymodem.o ff_lfn.o

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss size

## Compile
amod328.o: ../amod328.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

asmfunc.o: ../asmfunc.S
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

mmc.o: ../mmc.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

pff.o: ../pff.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ff.o: ../ff.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

fmmc.o: ../fmmc.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ff_lfn.o: ../ff_lfn.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ymodem.o: ../ymodem.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) amod_last.elf dep/* amod_last.hex amod_last.eep amod_last.lss amod_last.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

