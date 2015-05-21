;---------------------------------------------------------------------------;
; USI control functions
;---------------------------------------------------------------------------;

.nolist
#include <avr/io.h>	// Include device specific definitions.
#include <stdio.h>
.list
#define	_FLAGS	_SFR_IO_ADDR(GPIOR0)

;---------------------------------------------------------------------------;
; Simple Delay
;---------------------------------------------------------------------------;
; void delay_ms(WORD ms);
; void delay_us(WORD us);

;.global delay_ms
;.func delay_ms
;delay_ms:
;	wdr			; Reset WDT
;	sbiw	r24, 1		; if (ms-- == 0) return;
;	brcs	9f		; /
;	ldi	ZL, lo8(4000)	; 1ms delay (16MHz)
;	ldi	ZH, hi8(4000)	; 
;1:	sbiw	ZL, 1		; 
;	brne	1b		; /
;	rjmp	delay_ms
;9:	ret
;.endfunc


.global delay_us
.func delay_us
delay_us:
	ldi	r23, 2
1:	dec	r23
	brne	1b
	sbiw	r24, 1
	brne	delay_us
	wdr
	ret
.endfunc


;---------------------------------------------------------------------------;
; Receive a byte from the MMC
;---------------------------------------------------------------------------;
; BYTE rcv_spi (void);

;.global rcv_spi
;.func rcv_spi
;rcv_spi:
;	ldi r24, 0xFF
;	out  _SFR_IO_ADDR(SPDR),r24
;Wait_TransmitReceive: ; Wait for transmission complete
;	in   r24, _SFR_IO_ADDR(SPSR)
;	sbrs r24, 7
;	rjmp Wait_TransmitReceive
;	in   r24, _SFR_IO_ADDR(SPDR)

;	ret
;.endfunc


;---------------------------------------------------------------------------;
; Transmit a byte to the MMC
;---------------------------------------------------------------------------;
; void xmit_spi (BYTE);

;.global xmit_spi
;.func xmit_spi
;xmit_spi:

;	out  _SFR_IO_ADDR(SPDR),r24
;Wait_Transmit: ; Wait for transmission complete
;	in   r24, _SFR_IO_ADDR(SPSR)
;	sbrs r24, 7
;	rjmp Wait_Transmit
	
;	in   r24, _SFR_IO_ADDR(SPDR)

;	ret
;.endfunc

;---------------------------------------------------------------------------;
; Read and forward a part of the 512 byte data block
;---------------------------------------------------------------------------;
; void fwd_blk_part (void*, WORD, WORD);

;.global fwd_blk_part
;.func fwd_blk_part
;fwd_blk_part:
;	movw	XL, r24			;X = R25:R24 (memory address)
;	movw	ZL, r22			;Z = R23:R22 (byte offset in the sector)

;	ldi	r18, lo8(514)		;R19:R18 = 514, Number of bytes to receive
;	ldi	r19, hi8(514)		;/
;	sub	r18, ZL			;R19:R18 -= Z
;	sbc	r19, ZH			;/
;	sub	r18, r20		;R19:R18 -= R21:R20
;	sbc	r19, r21		;/
;	; Skip leading data bytes
;//	ldi	r24, _BV(2)		;SCK(PA2)
;;	ldi	r24, _BV(5)		;SCK(PB5)

;1:	sbiw	ZL, 1			;Skip leading data...
;	brcs	2f			; None

;	sbiw	ZL, 1			;Skip leading data...
;	brcs	3f			; One

;	ldi r25, 0xFF
;	out  _SFR_IO_ADDR(SPDR),r25
;wtr1: ; Wait for transmission complete
;	in   r24, _SFR_IO_ADDR(SPSR)
;	sbrs r24, 7
;	rjmp wtr1
;	in   r24, _SFR_IO_ADDR(SPDR)
;	out  _SFR_IO_ADDR(SPDR),r25		; Start next SPI access
;	sbiw	ZL, 1			;Skip leading data...
;	brcs	wtr3		
;	rjmp	wtr1			;

;3:
;	ldi r24, 0xFF
;	out  _SFR_IO_ADDR(SPDR),r24
;wtr3: ; Wait for transmission complete
;	in   r24, _SFR_IO_ADDR(SPSR)
;	sbrs r24, 7
;	rjmp wtr3
;	in   r24, _SFR_IO_ADDR(SPDR)

;2:	sbiw	XL, 0			;Destination?
;	breq	fb_wave

;fb_mem:	; Store intermediate data bytes to the memory
;;	rcall	rcv_spi			;do

;	subi	r20, 1			;while (--r21:r20)
;	sbci	r21, 0			;
;	breq	mem_one			;
	
;	ldi r25, 0xFF
;	out  _SFR_IO_ADDR(SPDR),r25
;wtr4: ; Wait for transmission complete
;	in   r24, _SFR_IO_ADDR(SPSR)
;	sbrs r24, 7
;	rjmp wtr4
;	in   r24, _SFR_IO_ADDR(SPDR)
;	out  _SFR_IO_ADDR(SPDR),r25		; Start next SPI access
;	st	X+, r24			; *X++ = rcv_spi()
;	subi	r20, 1			;while (--r21:r20)
;	sbci	r21, 0			;
;	breq	wtr2				;
;	rjmp wtr4
	
;mem_one:
;	LDI R24, 0XFF
;	out  _SFR_IO_ADDR(SPDR),r24
;wtr2: ; Wait for transmission complete
;	in   r24, _SFR_IO_ADDR(SPSR)
;	sbrs r24, 7
;	rjmp wtr2
;	in   r24, _SFR_IO_ADDR(SPDR)

;	st	X+, r24			; *X++ = rcv_spi()
;	rjmp	fb_exit


;fb_wave: ; Forward intermediate data bytes to the wave FIFO
;	sbic	_FLAGS, 7		;if (16bit data) R21:R20 /= 2;
;	lsr	r21			;
;	sbic	_FLAGS, 7		;if (16bit data) R21:R20 /= 2;
;	ror	r20			;/
;	sbic	_FLAGS, 6		;if (Stereo data) R21:R20 /= 2;
;	lsr	r21			;
;	sbic	_FLAGS, 6		;if (Stereo data) R21:R20 /= 2;
;	ror	r20			;/
;	lds	r22, FifoWi		;r22 = FIFO write index

;3:	ldi	XL, lo8(Abuff)		;X = Buff + R22
;	ldi	XH, hi8(Abuff)		;
;	add	XL, r22			;
;	adc	XH, r1			;/
;4:	lds	r24,FifoCt		;wait while FIFO full
;	cpi	r24, 252		;
;	brcc	4b			;/

;	rcall	rcv_spi			;Get L-ch/Mono data into Z
;	clr	ZL			;
;	sbis	_FLAGS, 7		;
;	rjmp	5f			;
;	mov	ZL, r24			;
;	rcall	rcv_spi			;
;	subi	r24, 0x80		;
;5:	mov	ZH, r24			;/
;	sbis	_FLAGS, 6		;if Mono file, do not process R-ch data
;	rjmp	8f			;/
;	rcall	rcv_spi			;Get R-ch data and mix it to Z
;	clr	r25			;
;	sbis	_FLAGS, 7		;
;	rjmp	6f			;
;	mov	r25, r24		;
;	rcall	rcv_spi			;
;	subi	r24, 0x80		;
;6:	add	ZL, r25			;
;	adc	ZH, r24			;
;	ror	ZH			;
;	ror	ZL			;/

;8:
;	push	r16
;	lds r16,Volume
;	and r16,r16
;	breq	9f			; Volume is 0 => full

;	subi	ZH, 0x80		;
;	push	r17
;	mul ZL,r16
;	mov r25,r1
;	mov	r17,ZH
;	mulsu r17,r16
;	movw	ZL,r0
;	eor	r1,r1
;	add	ZL,r25
;	adc	ZH,r1
;	subi	ZH, 0x80		;
;	pop	r17
;9:	
;	pop	r16
;	st	X+, ZL			;Store -/Rch/LSB data
;	st	X+, ZH			;Store +/Lch/MSB data
;	cli				;
;	lds	r24, FifoCt		;
;	subi	r24, -2			;
;	sts	FifoCt, r24		;
;	sei				;
;	subi	r22, -2			;/

;	subi	r20, lo8(1)		;while(--R21:R20)
;	sbci	r21, hi8(1)		;
;	brne	3b			;/
;	sts	FifoWi, r22		;Save FIFO write index

;fb_exit:
;//	ldi	r24, _BV(2)		;SCK(PA2)
;//	ldi	r24, _BV(5)		;SCK(PB5)
;10:	rcall rcv_spi;
;	subi	r18, lo8(1)		;Repeat r19:r18 times
;	sbci	r19, hi8(1)		;
;	brne	10b			;/

;	ret
;.endfunc

;---------------------------------------------------------------------------;
; Put a single sample in the audio fifo (with volume adjust)
;---------------------------------------------------------------------------;
; void put_audio_fifo (WORD);
.global put_audio_fifo
.func put_audio_fifo
put_audio_fifo:

	lds r31,Volume
	and r31,r31
	breq	11f			; Volume is 0 => full

	push	r16
	push	r17
	subi	r25, 0x80		;
	mul r24,r31
	mov r24,r1
	mov r16,r31
	mov r17,r25
	mulsu r17,r16
	mov	r25,r1
	eor	r1,r1
	add	r24,r0
	adc	r25,r1
	subi	r25, 0x80		;
	pop	r17
	pop	r16
11:
	lds	r30, FifoCt		;wait while FIFO full
	cpi	r30, 252		;
	brcc	11b			;/

	lds	r0, FifoWi		;r22 = FIFO write index
	ldi	ZL, lo8(Abuff)		;X = Buff + R22
	ldi	ZH, hi8(Abuff)		;
	add	ZL, r0			;
	adc	ZH, r1			;/

	st	Z+, r24			;Store -/Rch/LSB data
	st	Z+, r25			;Store +/Lch/MSB data

	cli				;
	lds	r24, FifoCt		;
	subi	r24, -2			;
	sts	FifoCt, r24		;
	sei				;

	mov r25, r0
	subi	r25, -2			;/
	sts	FifoWi, r25		;Save FIFO write index
	ret

.endfunc


;---------------------------------------------------------------------------;
; ISR for Audio Output Interrupt
;---------------------------------------------------------------------------;
; ISR(TIMER0_COMPA_vect);
;
; Pop an audio sample from FIFO and put it to the DAC.

.global TIMER1_COMPA_vect
.func TIMER1_COMPA_vect
TIMER1_COMPA_vect:
	push	r24				;Save regs.
	in	r24, _SFR_IO_ADDR(SREG)		;
	push	r24				;
	push	ZL				;
	push	ZH				;/

	lds	r24, FifoCt			;Load FIFO data counter
	subi	r24, 2			;Check if a sample is available
	brcs	9f				; If not, exit function
	sts	FifoCt, r24			;Save FIFO data counter
	lds	ZL, FifoRi			;Get FIFO read index
	clr	ZH				;Z = pointer to the top of FIFO
	subi	ZL, lo8(-(Abuff))		;
	sbci	ZH, hi8(-(Abuff))		;/
	ld	r24, Z+				;Send -/Rch/LSB data to OC1A

	out	_SFR_IO_ADDR(OCR0A), r24	;/

	ld	r24, Z+				;Send +/Lch/MSB data to OC1B
	out	_SFR_IO_ADDR(OCR0B), r24	;/
	subi	ZL, lo8(Abuff)			;Save FIFO read index
	sts	FifoRi, ZL			;/
9:
	pop	ZH				;Restore regs.
	pop	ZL				;
	pop	r24				;
	out	_SFR_IO_ADDR(SREG), r24		;
	pop	r24				;/

//	wdr //after all debugging need to realize sleep mode, then we need wdt

	reti
.endfunc

;---------------------------------------------------------------------------;
; ISR for Command input
;---------------------------------------------------------------------------;
; ISR(INT0_vect);

	//GPIOR0:	7bit - HIGH:16bit samples, LOW:8bit samples
	//			6bit - HIGH:stereo, LOW:mono
	//			5bit - HIGH:new PLAY/STOP/VOLUME command recieved
	//			4bit - last a_clock state
	//			3:0bits - bit counter pointer

.global INT0_vect 
.func INT0_vect 
INT0_vect:
	push r24
	push r23
	in	r24, _SFR_IO_ADDR(SREG)		;
	push	r24				;Save regs.

	in r23, _SFR_IO_ADDR(PIND) //reading the data pins

	BST r23, 7					//read the command for backlight state
	in r24, _SFR_IO_ADDR(PORTB) 
	BLD r24, 1					//switch ON/OFF backlight
	out _SFR_IO_ADDR(PORTB), r24 


//	rjmp	_exit_INT0_vect

	sbis	_FLAGS, 4	;if a_clk last state was HIGH - then it is not rising edge
	sbrs	r23, 3 		;if a_clk last state was LOW and current state is LOW - then it is not rising edge
	rjmp	_exit_INT0_vect

	in r24,_FLAGS
	andi r24, 0b00001111 //take the only bit counter part from GPIOR0

	inc r24
	sbrs r24, 4 //If pointer counts 16, then clear the counter part and set flag, else not
	rjmp _shifting

_command_received:
	in r24, _FLAGS
	andi r24, 0b11110000 //clear pointer part to 0
	out _FLAGS, r24
	sbi _FLAGS, 5

_shifting: //I use shifting with CARRY, place bit from input into carry-flag and shift it into buffer
	clc
	sbrc	r23, 4 ;if a_data state is HIGH - set bit, else not
	sec
	in r24, _SFR_IO_ADDR(GPIOR1)
	rol r24
	out _SFR_IO_ADDR(GPIOR1), r24
	in r24, _SFR_IO_ADDR(GPIOR2)
	rol r24
	out _SFR_IO_ADDR(GPIOR2), r24


	sbic _FLAGS, 5 //If pointer counts 16 (bit 5 is set), then we need to copy from buffer, else inc counter
	rjmp _store_command

_inc_counter:
	in r24, _FLAGS
	inc r24			//count next step of the pointer
	out _FLAGS, r24
	rjmp _exit_INT0_vect	

_store_command:

; Quicker/shorter code
	in r24, _SFR_IO_ADDR(GPIOR1)
	sts Command, r24
	in r24, _SFR_IO_ADDR(GPIOR2)
	sts Command+1, r24

;	push	ZL				;
;	push	ZH				;Save regs.

;	ldi ZH, hi8(Command) ; Storing from command buffer into memory variable
;	ldi ZL, lo8(Command)

;	in r24, _SFR_IO_ADDR(GPIOR1)
;	st Z+, r24
;	in r24, _SFR_IO_ADDR(GPIOR2)
;	st Z, r24

;	pop	ZH				;Restore regs.
;	pop	ZL				;

_exit_INT0_vect:

	in r24,_FLAGS
	BST r23, 3
	BLD r24, 4		//store current a_clk state
	out _FLAGS, r24

; Debug starts
;	in r24, _SFR_IO_ADDR(PORTC) 
;	BST r23, 3
;	BLD r24, 2
;	BST r23, 4
;	BLD r24, 3
;	andi r24, 0b11001111
;	sbic	_FLAGS, 0	;
;	ori r24, 0b00010000
;	sbic	_FLAGS, 1	;
;	ori r24, 0b00100000
;	out _SFR_IO_ADDR(PORTC), r24 
; Debug ends

	pop r24
	out	_SFR_IO_ADDR(SREG), r24		;
	pop	r23				;/
	pop	r24				;/
	reti
.endfunc
