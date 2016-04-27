# !!!! BETA Makefile !!!!
# !!!! Use at own risk !!!!
#----------------------------------------------------------------------------
# On command line:
#
# make all = Make software.
#
# make clean = Clean out built project files.
#
# make coff = Convert ELF to AVR COFF.
#
# make extcoff = Convert ELF to AVR Extended COFF.
#
# make program = Download the hex file to the device, using avrdude.
#                Please customize the avrdude settings below first!
#
# make debug = Start either simulavr or avarice as specified for debugging,
#              with avr-gdb or avr-insight as the front end for debugging.
#
# make filename.s = Just compile filename.c into the assembler code only.
#
# make filename.i = Create a preprocessed source file for use in submitting
#                   bug reports to the GCC project.
#
# To rebuild project do "make clean" then "make all".
#----------------------------------------------------------------------------


# MCU name
ifeq ($(CPU), 2561)
MCU = atmega2561
else
ifeq ($(CPU), 128)
MCU = atmega128
else
MCU = atmega64
endif
endif

# Processor frequency.
F_CPU = 16000000

# Output format. (can be srec, ihex, binary)
FORMAT = ihex

# Target file name (without extension).

ifeq ($(SLAVE), YES)
 ifeq ($(CPU), 2561)
 TARGET = slave2561
 else
 TARGET = slave
 endif
else
ifeq ($(V2), YES)
TARGET = mbtx
else
TARGET = er9x
endif
endif

# Object files directory
OBJDIR = obj

#---------------- OPTIONS ---------------------------
# Enable JETI-Telemetry or FrSky Telemetry reception on UART0
# For this option you need to modify your hardware!
# More information at [insertURLhere]
EXT = STD


ifeq ($(CPU), 128)
BOOT = YES
endif

# List C++ source files here. (C dependencies are automatically generated.)
#CPPSRC = $(TARGET).cpp stamp.cpp menus.cpp pers.cpp file.cpp lcd.cpp drivers.cpp templates.cpp pulses.cpp audio.cpp

ifeq ($(SLAVE), YES)
CPPSRC = slave.cpp lcdSlave.cpp
else
CPPSRC = er9x.cpp stamp.cpp menus.cpp pers.cpp file.cpp lcd.cpp drivers.cpp templates.cpp pulses.cpp audio.cpp
endif

ifeq ($(BOOT),YES)
 CPPSRC += optiboot.cpp
endif


ifeq ($(EXT), JETI)
 CPPSRC += jeti.cpp
endif

ifeq ($(EXT), FRSKY)
 CPPSRC += frsky.cpp
endif

ifeq ($(EXT), ARDUPILOT)
 CPPSRC += ardupilot.cpp
endif

ifeq ($(EXT), NMEA)
 CPPSRC += nmea.cpp
endif

#ifeq ($(CPU), 2561)
#ifneq ($(SLAVE), YES)
# CPPSRC += arduinoDue.cpp
#endif
#endif


HELI = YES

TEMPLATES = YES

NOPOTSCROLL =

CUSTOM9X = NO

N2F = NO

# BEEPER. Values = BUZZER or BUZZER_MOD  or SPEAKER
# (without a mod, BUZZER can make PPM jack output switch from output to input at random)
# (SPEAKER is used if you have modified your radio gruvin style for the variable sound)
BEEPER = BUZZER



#----Display compile modifications on the version screen---------------------------------------
#   The default for the compile options will not be displayed and is
#	EXT = STD
#	HELI = YES
#	TEMPLATES = YES
#	BUZZER = BUZZER
#     HUB = NO
#     N2F = NO

#   If the variables differ from the above default settings, it will be displayed in the 
#   fifth line of the version screen

#	possible results for EXT are
#	JETI | FRSKY | ARDU | NMEA

#	HELI and TEMPLATES are combined to 
#	NH	HELI = NO
#	NT	TEMPLATES = NO
#	NHT	HELI = NO and TEMPLATES = NO

#  if The FrSky Version supports native FrSky Sensor Hub protocol it will show
#     N2F		NEMEA to FrSky protocol conversion with FrSky Sensor Hub 

#   Support for the NMEA Arduino HUB simulator is activated with HUB = YES
#   This allows to add a private datastream, coded like a normal NMEA record.
#   This record $GPLCL contains analog and digital data, similar to the FrSky HUB 
#	HUB = YES|NO

#	possible values for BUZZER are
#	BZM	BUZZER = BUZZER_MOD
#	SPK	BUZZER = SPEAKER

ifeq ($(CPU), 2561)
	CPU_MOD = 2561
endif

ifeq ($(CPU), 128)
	CPU_MOD = 128
endif

ifeq ($(EXT), ARDUPILOT)
	EXT_MOD = ARDU
else
	ifeq ($(EXT), STD)
#		leave it empty		
	else
		EXT_MOD = $(EXT)
	endif
endif

ifeq ($(HELI), NO)
	ifeq ($(TEMPLATES),NO)
		HT_MOD = NHT
	else
		HT_MOD = NH
	endif
else
	ifeq ($(TEMPLATES),NO)
		HT_MOD = NT
	endif	
endif

ifeq ($(BEEPER), BUZZER_MOD)
	BEEPER_MOD = BZM
else
	ifeq ($(BEEPER), SPEAKER)
		BEEPER_MOD = SPK
	endif
endif

ifeq ($(EXT), FRSKY)
	ifeq ($(N2F), YES)
	  N2F_MOD=N2F
	endif
endif

ifeq ($(NOPOTSCROLL),YES)
	NOPOTSCROLL = YES
	SC_MOD=NPS
endif

#--------------------------------------------------------------------------------

# List Assembler source files here.
#     Make them always end in a capital .S.  Files ending in a lowercase .s
#     will not be considered source files but generated files (assembler
#     output from the compiler), and will be deleted upon "make clean"!
#     Even though the DOS/Win* filesystem matches both .s and .S the same,
#     it will preserve the spelling of the filenames, and gcc itself does
#     care about how the name is spelled on its command-line.
ASRC =


# Optimization level, can be [0, 1, 2, 3, s].
#     0 = turn off optimization. s = optimize for size.
#     (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
OPT = s


# Debugging format.
#     Native formats for AVR-GCC's -g are dwarf-2 [default] or stabs.
#     AVR Studio 4.10 requires dwarf-2.
#     AVR [Extended] COFF format requires stabs, plus an avr-objcopy run.
DEBUG = dwarf-2


# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.
EXTRAINCDIRS =


# Compiler flag to set the C Standard level.
#     c89   = "ANSI" C
#     gnu89 = c89 plus GCC extensions
#     c99   = ISO C99 standard (not yet fully implemented)
#     gnu99 = c99 plus GCC extensions
CSTANDARD = -std=gnu99


# Place -D or -U options here for C sources
CDEFS = -DF_CPU=$(F_CPU)UL


# Place -D or -U options here for C++ sources
CPPDEFS = -DF_CPU=$(F_CPU)UL
#CPPDEFS += -D__STDC_LIMIT_MACROS
#CPPDEFS += -D__STDC_CONSTANT_MACROS

# If HELI-Support is enabled
ifeq ($(HELI), NO)
 CPPDEFS += -DNO_HELI
endif

# If TEMPLATES-Support is enabled
ifeq ($(TEMPLATES), NO)
 CPPDEFS += -DNO_TEMPLATES
endif

# If JETI-Support is enabled
ifeq ($(EXT), JETI)
 CPPDEFS += -DJETI
endif

# If FRSKY-Support is enabled
ifeq ($(EXT), FRSKY)
 CPPDEFS += -DFRSKY
endif

ifeq ($(SERVOICE), YES)
 CPPDEFS += -DSERVOICEONLY
endif


# gruvin: If buzzer speaker replacment supported - ported by rob.thomson
ifeq ($(BEEPER), SPEAKER)
 CPPDEFS += -DBEEPSPKR
endif

# If ARDUPILOT-Support is enabled
ifeq ($(EXT), ARDUPILOT)
 CPPDEFS += -DARDUPILOT
endif

# If NMEA-Support is enabled
ifeq ($(EXT), NMEA)
 CPPDEFS += -DNMEA
# If NMEA and HUB-Support is enabled
  ifeq ($(HUB), YES)
    CPPDEFS += -DHUB
  endif
endif

#if FrSky Sensor Hub native protocol is enabled
ifeq ($(EXT), FRSKY)
	ifeq ($(N2F), YES)
		CPPDEFS += -DN2F
	endif
endif

# If buzzer modified (no interference with PPM jack)
ifeq ($(BEEPER), BUZZER_MOD)
 CPPDEFS += -DBUZZER_MOD
endif

ifeq ($(NOPOTSCROLL),YES)
 CPPDEFS += -DNOPOTSCROLL
endif

ifeq ($(BOOT),YES)
 CPPDEFS += -DBOOTL
endif

ifeq ($(CUSTOM9X),YES)
 CPPDEFS += -DCUSTOM9X
endif

ifeq ($(CPU), 2561)
 CPPDEFS += -DCPUM2561
else
 ifeq ($(CPU), 128)
  CPPDEFS += -DCPUM128
 else
  CPPDEFS += -DCPUM64
 endif
endif

ifeq ($(V2), YES)
 CPPDEFS += -DV2
endif

ifeq ($(LANG), FR)
 CPPDEFS += -DLANG_FR
else
 ifeq ($(LANG), DE)
  CPPDEFS += -DLANG_DE
 else
  ifeq ($(LANG), NO)
   CPPDEFS += -DLANG_NO
  endif
 endif
endif


#---------------- Compiler Options C ----------------
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
CFLAGS = -g$(DEBUG)
CFLAGS += $(CDEFS)
CFLAGS += -O$(OPT)
#CFLAGS += -mint8
#CFLAGS += -mshort-calls
CFLAGS += -funsigned-char
CFLAGS += -funsigned-bitfields
CFLAGS += -fpack-struct
CFLAGS += -fshort-enums
#CFLAGS += -fno-unit-at-a-time
CFLAGS += -Wall
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wundef
#CFLAGS += -Wunreachable-code
#CFLAGS += -Wsign-compare
CFLAGS += -Wa,-adhlns=$(<:%.c=$(OBJDIR)/%.lst)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += $(CSTANDARD)

#CFLAGS+= --combine -fwhole-program
CFLAGS+= -fwhole-program


#---------------- Compiler Options C++ ----------------
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
CPPFLAGS = -g$(DEBUG)
CPPFLAGS += $(CPPDEFS)
CPPFLAGS += -O$(OPT)
#CPPFLAGS += -mint8
#CPPFLAGS += -mshort-calls
CPPFLAGS += -mrelax
CPPFLAGS += -mstrict-X
#CPPFLAGS += -funsigned-char
#CPPFLAGS += -funsigned-bitfields
#CPPFLAGS += -fpack-struct
CPPFLAGS += -fshort-enums
#CPPFLAGS += -fno-exceptions
#CPPFLAGS += -fno-unit-at-a-time
CPPFLAGS += -Wall
#CPPFLAGS += -Wstrict-prototypes
#CPPFLAGS += -fgcse-sm
#CPPFLAGS += -fgcse-las
#CPPFLAGS += -fshrink-wrap
#CPPFLAGS += -fno-tracer
#CPPFLAGS += -Wunreachable-code
#CPPFLAGS += -Wsign-compare
#CPPFLAGS += -Wa,-adhlns=$(<:%.cpp=$(OBJDIR)/%.lst)
CPPFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
#CPPFLAGS += -ffunction-sections -fdata-sections 
#CPPFLAGS += $(CSTANDARD)



#---------------- Assembler Options ----------------
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -gstabs:   have the assembler create line number information; note that
#             for use in COFF files, additional information about filenames
#             and function names needs to be present in the assembler source
#             files -- see avr-libc docs [FIXME: not yet described there]
ASFLAGS = -Wa,-adhlns=$(<:%.S=$(OBJDIR)/%.lst),-gstabs


#---------------- Library Options ----------------
# Minimalistic printf version
PRINTF_LIB_MIN = -Wl,-u,vfprintf -lprintf_min

# Floating point printf version (requires MATH_LIB = -lm below)
PRINTF_LIB_FLOAT = -Wl,-u,vfprintf -lprintf_flt

# If this is left blank, then it will use the Standard printf version.
PRINTF_LIB =
#PRINTF_LIB = $(PRINTF_LIB_MIN)
#PRINTF_LIB = $(PRINTF_LIB_FLOAT)


# Minimalistic scanf version
SCANF_LIB_MIN = -Wl,-u,vfscanf -lscanf_min

# Floating point + %[ scanf version (requires MATH_LIB = -lm below)
SCANF_LIB_FLOAT = -Wl,-u,vfscanf -lscanf_flt

# If this is left blank, then it will use the Standard scanf version.
SCANF_LIB =
#SCANF_LIB = $(SCANF_LIB_MIN)
#SCANF_LIB = $(SCANF_LIB_FLOAT)


MATH_LIB = -lm



#---------------- External Memory Options ----------------

# 64 KB of external RAM, starting after internal RAM (ATmega128!),
# used for variables (.data/.bss) and heap (malloc()).
#EXTMEMOPTS = -Wl,-Tdata=0x801100,--defsym=__heap_end=0x80ffff

# 64 KB of external RAM, starting after internal RAM (ATmega128!),
# only used for heap (malloc()).
#EXTMEMOPTS = -Wl,--defsym=__heap_start=0x801100,--defsym=__heap_end=0x80ffff

EXTMEMOPTS =



#---------------- Linker Options ----------------
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -Wl,-Map=$(TARGET).map,--cref -Wl,--relax
LDFLAGS += $(EXTMEMOPTS)
LDFLAGS += $(PRINTF_LIB) $(SCANF_LIB) $(MATH_LIB)
#LDFLAGS += -Wl,--gc-sections
#LDFLAGS += -T linker_script.x

ifeq ($(CPU), 128)
LDFLAGS += -Wl,--section-start=.boottext=0x1Fd00 -Wl,--section-start=.bootjump=0x1FFFC
else
 ifneq ($(CPU), 2561)
LDFLAGS += -Wl,--section-start=.boottext=0xFe00 -Wl,--section-start=.bootjump=0xFFFC
 endif
endif


#---------------- Programming Options (avrdude) ----------------

# Programming hardware: alf avr910 avrisp bascom bsd
# dt006 pavr picoweb pony-stk200 sp12 stk200 stk500
#
# Type: avrdude -c ?
# to get a full listing.
#
AVRDUDE_PROGRAMMER = usbasp


# com1 = serial port. Use lpt1 to connect to parallel port.
AVRDUDE_PORT = /dev/ttyUSB01

AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex:a
AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(TARGET).bin:a
AVRDUDE_READ_FLASH = -U flash:r:$(TARGET).hex:r
AVRDUDE_READ_EEPROM = -U eeprom:r:$(TARGET).bin:r



# Uncomment the following if you want avrdude's erase cycle counter.
# Note that this counter needs to be initialized first using -Yn,
# see avrdude manual.
#AVRDUDE_ERASE_COUNTER = -y

# Uncomment the following if you do /not/ wish a verification to be
# performed after programming the device.
#AVRDUDE_NO_VERIFY = -V

# Increase verbosity level.  Please use this when submitting bug
# reports about avrdude. See <http://savannah.nongnu.org/projects/avrdude>
# to submit bug reports.
#AVRDUDE_VERBOSE = -v -v

#AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)
AVRDUDE_FLAGS = -p $(MCU)  -c $(AVRDUDE_PROGRAMMER)
AVRDUDE_FLAGS += $(AVRDUDE_NO_VERIFY)
AVRDUDE_FLAGS += $(AVRDUDE_VERBOSE)
AVRDUDE_FLAGS += $(AVRDUDE_ERASE_COUNTER)



#---------------- Debugging Options ----------------

# For simulavr only - target MCU frequency.
DEBUG_MFREQ = $(F_CPU)

# Set the DEBUG_UI to either gdb or insight.
DEBUG_UI = gdb
# DEBUG_UI = insight

# Set the debugging back-end to either avarice, simulavr.
#DEBUG_BACKEND = avarice
DEBUG_BACKEND = simulavr

# GDB Init Filename.
GDBINIT_FILE = __avr_gdbinit

# When using avarice settings for the JTAG
JTAG_DEV = /dev/com1

# Debugging port used to communicate between GDB / avarice / simulavr.
DEBUG_PORT = 4242

# Debugging host used to communicate between GDB / avarice / simulavr, normally
#     just set to localhost unless doing some sort of crazy debugging when
#     avarice is running on a different computer.
DEBUG_HOST = localhost



#============================================================================


# Define programs and commands.
SHELL = sh
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm
AVRDUDE = avrdude
REMOVE = rm -f
REMOVEDIR = rm -rf
COPY = cp
WINSHELL = cmd

# th9x/er9x-specific
XBM2LBM = ruby ../util/xbm2lbm.rb
AREV = $(shell sh -c "cat .svn/entries | sed -n '4p'")
REV = $(shell echo $$(( $(AREV) + 1 )))


# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = -------- begin --------
MSG_END = --------  end  --------
MSG_SIZE_BEFORE = Size before:
MSG_SIZE_AFTER = Size after:
MSG_COFF = Converting to AVR COFF:
MSG_EXTENDED_COFF = Converting to AVR Extended COFF:
MSG_FLASH = Creating load file for Flash:
MSG_EEPROM = Creating load file for EEPROM:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling C:
MSG_COMPILING_CPP = Compiling C++:
MSG_ASSEMBLING = Assembling:
MSG_CLEANING = Cleaning project:
MSG_CREATING_LIBRARY = Creating library:




# Define all object files.
OBJ = $(SRC:%.c=$(OBJDIR)/%.o) $(CPPSRC:%.cpp=$(OBJDIR)/%.o) $(ASRC:%.S=$(OBJDIR)/%.o)

# Define all listing files.
LST = $(SRC:%.c=$(OBJDIR)/%.lst) $(CPPSRC:%.cpp=$(OBJDIR)/%.lst) $(ASRC:%.S=$(OBJDIR)/%.lst)


# Compiler flags to generate dependency files.
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d


# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -mmcu=$(MCU) -I. $(CFLAGS) $(GENDEPFLAGS)
ALL_CPPFLAGS = -mmcu=$(MCU) -I. -x c++ $(CPPFLAGS) $(GENDEPFLAGS)
ALL_ASFLAGS = -mmcu=$(MCU) -I. -x assembler-with-cpp $(ASFLAGS)

AVER = ${shell sh -c "grep \"SUB_VERS\" stamp-er9x.h | egrep -o \"[[:digit:]]*\""}
VER = $(shell echo $$(( $(AVER) + 1 )))



# Default target.
all: begin gccversion sizebefore build sizeafter end

# Change the build target to build a HEX file or a library.
build: stamp font_dblsize.lbm font.lbm sticks.lbm s9xsplash.lbm elf hex eep lss sym
#build: lib




elf: $(TARGET).elf
hex: $(TARGET).hex
eep: $(TARGET).eep
lss: $(TARGET).lss
sym: $(TARGET).sym
LIBNAME=lib$(TARGET).a
lib: $(LIBNAME)


# Build stamp-file
stamp:
	@echo
	@echo "Generate Version-stamp:"
	@echo "//Automatically generated file - do not edit" > stamp-er9x.h
	@echo "#define DATE_STR \"`date +%d.%m.%Y`\"" >> stamp-er9x.h
	@echo "#define TIME_STR \"`date +%H:%M:%S`\"" >> stamp-er9x.h
	@echo "#define SUB_VERS $(VER)-$(USER)" >> stamp-er9x.h
ifeq ($(V2), YES)
	@echo "#define SVN_VERS \"er9xV2-r$(REV)\"" >> stamp-er9x.h
else
	@echo "#define SVN_VERS \"er9x-r$(REV)\"" >> stamp-er9x.h
endif
	@echo "#define MOD_VERS \"$(EXT_MOD) $(HT_MOD) $(BEEPER_MOD) $(CPU_MOD) $(SC_MOD) \"" >> stamp-er9x.h
#	@cat stamp-er9x.h

#font.lbm: font_6x1.xbm
#	@echo
#	@echo "Convert font from xbm to lbm:"
#	$(XBM2LBM) $<

#font_dblsize.lbm: font_dblsize.xbm
#	@echo
#	@echo "Convert doublesize font from xbm to lbm:"
#	$(XBM2LBM) $<

sticks.lbm: sticks_4x1.xbm
	@echo
	@echo "Convert sticks from xbm to lbm:"
	$(XBM2LBM) $<

s9xsplash.lbm: s9xsplash.xbm
	@echo
	@echo "Convert splash screen from xbm to lbm:"
	$(XBM2LBM) $<

# Eye candy.
# AVR Studio 3.x does not check make's exit code but relies on
# the following magic strings to be generated by the compile job.

begin:
	@echo
	@echo $(MSG_BEGIN)

end:
	@echo $(MSG_END)
	@echo



# Display size of file.
HEXSIZE = $(SIZE) --target=$(FORMAT) $(TARGET).hex
#ELFSIZE = $(SIZE) -A $(TARGET).elf
ELFSIZE = $(SIZE) --mcu=$(MCU) --format=avr $(TARGET).elf
AVRMEM = avr-mem.sh $(TARGET).elf $(MCU)

sizebefore:
	@if test -f $(TARGET).elf; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); \
	$(AVRMEM) 2>/dev/null; echo; fi

sizeafter:
	@if test -f $(TARGET).elf; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); \
	$(AVRMEM) 2>/dev/null; echo; fi



# Display compiler version information.
gccversion :
	@$(CC) --version



# Program the device.  Depreciated
#wflash: $(TARGET).hex
#	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)
#
#weeprom: $(TARGET).bin
#	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_EEPROM)
#
#rflash: $(TARGET).hex
#	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_READ_FLASH)
#
#reeprom: $(TARGET).bin
#	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_READ_EEPROM)


# Generate avr-gdb config/init file which does the following:
#     define the reset signal, load the target file, connect to target, and set
#     a breakpoint at main().
gdb-config:
	@$(REMOVE) $(GDBINIT_FILE)
	@echo define reset >> $(GDBINIT_FILE)
	@echo SIGNAL SIGHUP >> $(GDBINIT_FILE)
	@echo end >> $(GDBINIT_FILE)
	@echo file $(TARGET).elf >> $(GDBINIT_FILE)
	@echo target remote $(DEBUG_HOST):$(DEBUG_PORT)  >> $(GDBINIT_FILE)
	ifeq ($(DEBUG_BACKEND),simulavr)
	@echo load  >> $(GDBINIT_FILE)
	endif
	@echo break main >> $(GDBINIT_FILE)

debug: gdb-config $(TARGET).elf
	ifeq ($(DEBUG_BACKEND), avarice)
	@echo Starting AVaRICE - Press enter when "waiting to connect" message displays.
	@$(WINSHELL) /c start avarice --jtag $(JTAG_DEV) --erase --program --file \
$(TARGET).elf $(DEBUG_HOST):$(DEBUG_PORT)
	@$(WINSHELL) /c pause

	else
	@$(WINSHELL) /c start simulavr --gdbserver --device $(MCU) --clock-freq \
	$(DEBUG_MFREQ) --port $(DEBUG_PORT)
	endif
	@$(WINSHELL) /c start avr-$(DEBUG_UI) --command=$(GDBINIT_FILE)




# Convert ELF to COFF for use in debugging / simulating in AVR Studio or VMLAB.
	COFFCONVERT = $(OBJCOPY) --debugging
	COFFCONVERT += --change-section-address .data-0x800000
	COFFCONVERT += --change-section-address .bss-0x800000
	COFFCONVERT += --change-section-address .noinit-0x800000
	COFFCONVERT += --change-section-address .eeprom-0x810000



coff: $(TARGET).elf
	@echo
	@echo $(MSG_COFF) $(TARGET).cof
	$(COFFCONVERT) -O coff-avr $< $(TARGET).cof


extcoff: $(TARGET).elf
	@echo
	@echo $(MSG_EXTENDED_COFF) $(TARGET).cof
	$(COFFCONVERT) -O coff-ext-avr $< $(TARGET).cof



# Create final output files (.hex, .eep) from ELF output file.
%.hex: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

%bin: %elf
	$(OBJCOPY) -O binary $< $@

%.eep: %.elf
	@echo
	@echo $(MSG_EEPROM) $@
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
--change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# Create extended listing file from ELF output file.
%.lss: %.elf
	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	@echo $(MSG_SYMBOL_TABLE) $@
	$(NM) -n $< > $@



# Create library from object files.
.SECONDARY : $(TARGET).a
.PRECIOUS : $(OBJ)
%.a: $(OBJ)
	@echo
	@echo $(MSG_CREATING_LIBRARY) $@
	$(AR) $@ $(OBJ)


# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(OBJ)
%.elf: $(OBJ)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(ALL_CFLAGS) $^ --output $@ $(LDFLAGS)


# Compile: create object files from C source files.
$(OBJDIR)/%.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(ALL_CFLAGS) $< -o $@


# Compile: create object files from C++ source files.
$(OBJDIR)/%.o : %.cpp
	@echo
	@echo $(MSG_COMPILING_CPP) $<
	$(CC) -c $(ALL_CPPFLAGS) $< -o $@


# Compile: create assembler files from C source files.
%.s : %.c
	$(CC) -S $(ALL_CFLAGS) $< -o $@


# Compile: create assembler files from C++ source files.
%.s : %.cpp
	$(CC) -S $(ALL_CPPFLAGS) $< -o $@


# Assemble: create object files from assembler source files.
$(OBJDIR)/%.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@


# Create preprocessed source for use in sending a bug report.
%.i : %.c
	$(CC) -E -mmcu=$(MCU) -I. $(CFLAGS) $< -o $@


# Target: clean project.
clean: begin clean_list end

clean_list :
	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).eep
	$(REMOVE) $(TARGET).cof
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).sym
	$(REMOVE) $(TARGET).lss
	$(REMOVEDIR) $(OBJDIR)
	$(REMOVE) $(SRC:.c=.s)
	$(REMOVE) $(SRC:.c=.d)
	$(REMOVEDIR) .dep


# Create object files directory
$(shell mkdir $(OBJDIR) 2>/dev/null)


# Include the dependency files.
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)


# Listing of phony targets.
.PHONY : all begin finish end sizebefore sizeafter gccversion \
build elf hex eep lss sym coff extcoff \
clean clean_list program debug gdb-config stamp
