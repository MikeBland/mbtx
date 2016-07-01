

#include <stdint.h>
#include "stm32f4xx_rcc.h"
#include "hal.h"
//#include "lcd.h"
//#include "board_horus.h"

#define PROGMEM const char
#include "..\font.lbm"
#define font_5x8_x20_x7f (font)


void ledInit() ;
void ledRed() ;
void ledBlue() ;

extern "C" void pwrInit(void) ;
extern "C" void pwrOn(void) ;
extern "C" void pwrOff(void) ;
extern "C" uint32_t pwrPressed(void) ;

extern "C" void lcdInit(void) ;
void lcdColorsInit(void) ;
extern "C" void lcdDrawSolidFilledRectDMA(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) ;

extern uint32_t CurrentFrameBuffer ;

#define SDRAM_BANK_ADDR     ((uint32_t)0xD0000000)
#define LCD_W	480
#define LCD_H	272

// Using "double size" chars this to 17 rows of 40 characters


#define FW	6
#define FH	8

#define PERI1_FREQUENCY                45000000
#define PERI2_FREQUENCY                90000000
#define TIMER_MULT_APB1                2
#define TIMER_MULT_APB2                2

uint32_t TimeCounter ;
uint32_t SecCounter ;

// Starts TIMER at 200Hz, 5mS period
void init5msTimer()
{
  INTERRUPT_5MS_TIMER->ARR = 4999 ;     // 5mS
  INTERRUPT_5MS_TIMER->PSC = (PERI1_FREQUENCY * TIMER_MULT_APB1) / 1000000 - 1 ; // 1uS from 30MHz
  INTERRUPT_5MS_TIMER->CCER = 0 ;
  INTERRUPT_5MS_TIMER->CCMR1 = 0 ;
  INTERRUPT_5MS_TIMER->EGR = 0 ;
  INTERRUPT_5MS_TIMER->CR1 = 5 ;
  INTERRUPT_5MS_TIMER->DIER |= 1 ;
  NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, 7);
  NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn) ;
}

void interrupt5ms()
{
  static uint32_t pre_scale ;       // Used to get 10 Hz counter

  ++pre_scale;

//  if (pre_scale == 5 || pre_scale == 10) {

//#if defined(HAPTIC)
//    HAPTIC_HEARTBEAT();
//#endif

//  }

  if ( pre_scale >= 2 )
	{
    pre_scale = 0 ;
		TimeCounter += 1 ;
		if ( TimeCounter >= 100 )
		{
			TimeCounter = 0 ;
			SecCounter += 1 ;
		}
//    per10ms();
  }

//  checkRotaryEncoder();
}

extern "C" void TIM8_TRG_COM_TIM14_IRQHandler()
{
  TIM14->SR &= ~TIM_SR_UIF ;
  interrupt5ms() ;
}

uint8_t lcd_putcAttDbl(uint8_t x,uint8_t y,const char c,uint8_t mode)
{
	uint16_t *p ;
  uint8_t *q ;
	uint8_t mask ;
	
	p = ( uint16_t *) CurrentFrameBuffer ;
	p += x ;
	p += y * LCD_W ;

	q = (uint8_t *) &font_5x8_x20_x7f[(c-0x20)*5] ;
	for ( mask = 1 ; mask ; mask <<= 1 )
	{
		if ( *q & mask )
		{
			*p ^= 0xFFFF ;
			*(p+1) ^= 0xFFFF ;
			*(p+LCD_W) ^= 0xFFFF ;
			*(p+LCD_W+1) ^= 0xFFFF ;
		}
		if ( *(q+1) & mask )
		{
			*(p+2) ^= 0xFFFF ;
			*(p+3) ^= 0xFFFF ;
			*(p+LCD_W+2) ^= 0xFFFF ;
			*(p+LCD_W+3) ^= 0xFFFF ;
		}
		if ( *(q+2) & mask )
		{
			*(p+4) ^= 0xFFFF ;
			*(p+5) ^= 0xFFFF ;
			*(p+LCD_W+4) ^= 0xFFFF ;
			*(p+LCD_W+5) ^= 0xFFFF ;
		}
		if ( *(q+3) & mask )
		{
			*(p+6) ^= 0xFFFF ;
			*(p+7) ^= 0xFFFF ;
			*(p+LCD_W+6) ^= 0xFFFF ;
			*(p+LCD_W+7) ^= 0xFFFF ;
		}
		if ( *(q+4) & mask )
		{
			*(p+8) ^= 0xFFFF ;
			*(p+9) ^= 0xFFFF ;
			*(p+LCD_W+8) ^= 0xFFFF ;
			*(p+LCD_W+9) ^= 0xFFFF ;
		}
		p += LCD_W * 2 ;
	}
	return x+12 ;
}

uint8_t lcd_putcAtt(uint8_t x,uint8_t y,const char c,uint8_t mode)
{
	uint16_t *p ;
  uint8_t *q ;
	uint8_t mask ;

	p = ( uint16_t *) CurrentFrameBuffer ;
	p += x ;
	p += y * LCD_W ;
	// p -> char position
	q = (uint8_t *) &font_5x8_x20_x7f[(c-0x20)*5] ;
	for ( mask = 1 ; mask ; mask <<= 1 )
	{
		if ( *q & mask )
		{
			*p ^= 0xFFFF ;
		}
		if ( *(q+1) & mask )
		{
			*(p+1) ^= 0xFFFF ;
		}
		if ( *(q+2) & mask )
		{
			*(p+2) ^= 0xFFFF ;
		}
		if ( *(q+3) & mask )
		{
			*(p+3) ^= 0xFFFF ;
		}
		if ( *(q+4) & mask )
		{
			*(p+4) ^= 0xFFFF ;
		}
		p += LCD_W ;
	}
	return x+6 ;
}

void lcd_putsnAtt(uint8_t x,uint8_t y, const char * s,uint8_t len,uint8_t mode)
{
	register char c ;
  while(len!=0) {
    c = *s++ ;
    x = lcd_putcAtt(x,y,c,mode);
    len--;
  }
}

uint8_t lcd_putsAtt( uint8_t x, uint8_t y, const char *s, uint8_t mode )
{

  while(1)
	{
    char c = *s++ ;
    if(!c) break ;
		if ( c == 31 )
		{
			if ( (y += FH) >= LCD_H )	// Screen height
			{
				break ;
			}	
			x = 0 ;
		}
		else
		{
    	x = lcd_putcAtt(x,y,c,mode) ;
		}
  }
  return x;
}

void lcd_outhex4(uint8_t x,uint8_t y,uint16_t val)
{
	uint8_t i ;
  x+=FW*4;
  for(i=0; i<4; i++)
  {
    x-=FW;
    char c = val & 0xf ;
    c = c>9 ? c+'A'-10 : c+'0' ;
    lcd_putcAtt(x,y,c,0) ;
    val>>=4;
  }
}

void lcd_outhex8(uint8_t x,uint8_t y,uint32_t val)
{
	lcd_outhex4( x, y, val>>16 ) ;
	lcd_outhex4( x+FW*4, y, val ) ;
}


int main()
{
	uint32_t i ;
	uint32_t j ;
	uint32_t *p ;

  RCC_AHB1PeriphClockCmd(PWR_RCC_AHB1Periph | LCD_RCC_AHB1Periph | KEYS_RCC_AHB1Periph_GPIO | ADC_RCC_AHB1Periph | SERIAL_RCC_AHB1Periph | TELEMETRY_RCC_AHB1Periph | AUDIO_RCC_AHB1Periph | HAPTIC_RCC_AHB1Periph, ENABLE);
  RCC_APB1PeriphClockCmd(INTERRUPT_5MS_APB1Periph | TIMER_2MHz_APB1Periph | SERIAL_RCC_APB1Periph | TELEMETRY_RCC_APB1Periph | AUDIO_RCC_APB1Periph, ENABLE);
  RCC_APB2PeriphClockCmd(LCD_RCC_APB2Periph | ADC_RCC_APB2Periph | HAPTIC_RCC_APB2Periph, ENABLE);

	pwrInit() ;
	ledInit() ;
	
	j = 0 ;
	for( j = 0 ; j < 3 ; j += 1 )
	{
		uint32_t i ;
		ledRed() ;
		for ( i = 0 ; i < 20000000 ; i += 1 )
		{
			asm("nop") ;
		}
		ledBlue() ;
		for ( i = 0 ; i < 20000000 ; i += 1 )
		{
			asm("nop") ;
		}
		j += 1 ;
		if ( j > 3 )
		{
			if ( pwrPressed() )
			{
				pwrOff() ;
			}
		}
	}
	lcdColorsInit() ;
	lcdInit() ;
	j = 0 ;
	lcdDrawSolidFilledRectDMA( 0, 0, 180, 120, 0xFFFF ) ;
	lcdDrawSolidFilledRectDMA( 180, 0, 100, 100, 0xF800 ) ;	// Red?
	lcdDrawSolidFilledRectDMA( 400, 200, 80, 72, 0x001F ) ;	// Blue?

	lcd_putcAtt( 20, 20, 'A', 0 ) ;
	lcd_putcAtt( 30, 20, '0', 0 ) ;
	lcd_putcAtt( 40, 20, '7', 0 ) ;
	lcd_putcAttDbl( 60 , 20, 'T', 0 ) ;

	lcd_putsAtt( 10, 60, "Hello", 0 ) ;
	lcd_outhex8( 10, 70, CurrentFrameBuffer ) ;

	init5msTimer() ;
	__enable_irq() ;

	p = (uint32_t *) CurrentFrameBuffer ;
	p += (LCD_W / 2) * 140 ;

	j = INTERRUPT_5MS_TIMER->CNT ;
	for ( i = 0 ; i < (LCD_W / 2) * 50 ; i += 1 )
	{
		*p++ = 0xFFFFFFFF ;
	}
	j = INTERRUPT_5MS_TIMER->CNT - j ;
	j &= 0x0000FFFF ;
	lcd_outhex8( 10, 142, j ) ;

	for(;;)
	{
		uint32_t i ;
		ledRed() ;
		for ( i = 0 ; i < 20000000 ; i += 1 )
		{
			asm("nop") ;
		}
		ledBlue() ;
		
		lcdDrawSolidFilledRectDMA( 0, 100, 50, 10, 0xFFFF ) ;
		lcd_outhex4( 10, 101, TimeCounter ) ;
		
		lcdDrawSolidFilledRectDMA( 0, 200, 50, 10, 0 ) ;
		lcd_outhex4( 10, 200, SecCounter ) ;
		
		for ( i = 0 ; i < 20000000 ; i += 1 )
		{
			asm("nop") ;
		}
		j += 1 ;
		if ( j > 3 )
		{
			if ( pwrPressed() )
			{
				pwrOff() ;
			}
		}
	}

}
	
