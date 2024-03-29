/****************************************************************************
*  Copyright (c) 2011 by Michael Fischer. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*  
*  1. Redistributions of source code must retain the above copyright 
*     notice, this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the 
*     documentation and/or other materials provided with the distribution.
*  3. Neither the name of the author nor the names of its contributors may 
*     be used to endorse or promote products derived from this software 
*     without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
*  THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF 
*  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
*  SUCH DAMAGE.
*
*****************************************************************************
*  History:
*
*  22.05.2011  mifi  First Version
****************************************************************************/


/*
 * In this linker script there is no heap available.
 * The stack start at the end of the ram segment.
 */


/*
 * Take a look in the "The GNU linker" manual, here you get
 * the following information about the "MEMORY":
 *
 * "The MEMORY command describes the location and size of 
 * blocks of memory in the target."
 */



/* Highest address of the user mode stack */
_estack = 0x20010000;    /* end of 64K SRAM */

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0;      /* required amount of heap  */
_Min_Stack_Size = 0 ; /* required amount of stack */

/* Main stack end */
_main_stack_start = _estack - _Min_Stack_Size;
_heap_end = _main_stack_start;

MEMORY
{
   FLASH (rx)  : ORIGIN = 0x00400000, LENGTH = 512K
   RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}


/*
 * And the "SECTION" is used for:
 *
 * "The SECTIONS command tells the linker how to map input
 * sections into output sections, and how to place the output
 * sections in memory.
 */
SECTIONS
{

   /*
    * The ".text" section is used for the code, and
    * read only (.rodata) data. Even the vectors (.vectors)
    * MUST be saved at the start of this section.
    */
   .text :
   {
		FILL(0xFFFF)
      
      CREATE_OBJECT_SYMBOLS

      *(.bootrodata)
      *(.bootrodata.*)
      
      . = ALIGN(32768);       /* Align the start of the exidx part */
      _stext = .;          /* Provide the name for the start of this section */
		KEEP(*(.vectors))
      
      *(.text)
      *(.text.*)
 
/*      . = ALIGN(4);    */     /* Align the start of the exidx part */
/*      *(.ARM.exidx)    */
/*      *(.ARM.exidx.*)  */

      . = ALIGN(4);        /* Align the start of the rodata part */
      *(.rodata)
      *(.rodata.*)
      
      . = ALIGN(4);        /* Align the end of the section */
   } > FLASH = 0
   _etext = .;             /* Provide the name for the end of this section */
   
   .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
    .ARM : {
    __exidx_start = .;
      *(.ARM.exidx*)
      __exidx_end = .;
    } >FLASH

   
  _sidata = .;
   
   /*
    * The ".data" section is used for initialized data
    * and for functions (.fastrun) which should be copied 
    * from flash to ram. This functions will later be
    * executed from ram instead of flash.
    */
   .data : AT (_sidata)
   {
      . = ALIGN(4);        /* Align the start of the section */
      _sdata = .;          /* Provide the name for the start of this section */
      
      *(.data)
      *(.data.*)
      
      . = ALIGN(4);        /* Align the start of the fastrun part */
      *(.fastrun)
      *(.fastrun.*)
      
      . = ALIGN(4);        /* Align the end of the section */
   } > RAM
   _edata = .;             /* Provide the name for the end of this section */
   

   /*
    * The ".bss" section is used for uninitialized data.
    * This section will be cleared by the startup code.
    */
   .bss :
   {
      . = ALIGN(4);        /* Align the start of the section */
      _sbss = .;           /* Provide the name for the start of this section */
      
      *(.bss)
      *(.bss.*)
      
      . = ALIGN(4);        /* Align the end of the section */
   } > RAM
   _ebss = .;              /* Provide the name for the end of this section */
   __bss_end__ = _ebss;
   
  PROVIDE ( end = _ebss );
  PROVIDE ( _end = _ebss );
   
   /* 
    * The ".stack" section is our stack.
    * Here this section starts at the end of the ram segment.
    */
   _estack = ORIGIN(RAM) + LENGTH(RAM);

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
     libc.a ( * ) 
     libm.a ( * )
     libgcc.a ( * )
		*(.eh_frame) 
  }

}

/*** EOF **/

