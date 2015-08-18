/****************************************************************************
*  Copyright (c) 2011 by Michael Blandford. All rights reserved.
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
****************************************************************************/
// Developed from Atmel AVR112: TWI Bootloader for devices without boot section
// and changed so that it actually works


#include "USI_TWI_Slave.h"
#include "Common_Define.h"

/*****************************************************************************/

// USI_TWI state values.
static uint8_t USI_TWI_SLAVE_Write_State;
//static uint8_t USI_TWI_SLAVE_Overflow_State;
//static uint8_t Command_Flag;
#define Command_Flag		GPIOR1

#define USI_TWI_SLAVE_Overflow_State	GPIOR0
// TWI flag values.
// static uint8_t USI_TWI_SLAVE_Address_Update;

// Slave Each page address values.
static uint16_t USI_TWI_SLAVE_PAGE_Address;
static uint8_t USI_TWI_SLAVE_PAGE_High_Address;

/******************************************************************************/

/* Local variables   */

//static unsigned char USI_TWI_SlaveAddress;
#define USI_TWI_SlaveAddress 0x35

//********** USI_TWI functions **********//
static void USI_TWI_SLAVE_Init()
{
  
//  USI_TWI_SlaveAddress = TWI_OwnAddress;
  
  DDR_USI  &= ~(1<<PORT_USI_SDA);      // Set SDA as input
  PORT_USI &=  ~(1<<PORT_USI_SDA);    // Set SDA high
  
  DDR_USI  |=  (1<<PORT_USI_SCL);      // Set SCL as output
  PORT_USI |=  (1<<PORT_USI_SCL);      // Set SCL high
   
  
  USICR    =  (0<<USISIE)|(0<<USIOIE)|                         // Disable Start Condition Interrupt. Disable Overflow Interrupt.
               (1<<USIWM1)|(1<<USIWM0)|                        // Set USI in Two-wire mode. No USI Counter overflow prior
                                                               // to first Start Condition (potentail failure)
               (1<<USICS1)|(0<<USICS0)|(0<<USICLK)|            // Shift Register Clock Source = External, positive edge
              (0<<USITC);
   
  // Clear the interrupt flags and reset the counter.   
  USISR = (1<<USISIF) | (1<<USIOIF) | (1<<USIPF) |        // Clear interrupt flags.
          (1<<USIDC) |(0x0<<USICNT0);                     // USI to sample 8 bits or 16 edge toggles.
 }

/*****************************************************************************/ 

#define NOINLINE __attribute__ ((noinline))
 
static void USI_TWI_SLAVE_ReadAndProcessPacket( void) __attribute__((noreturn)) ;

void set_clock( uint8_t speed ) ;

static void USI_TWI_SLAVE_ReadAndProcessPacket ()
{
	set_clock( 5 ) ;		// Slow to 0.25MHz
	while(1)
	{
   if (USISR & (1<<USISIF))   // Check for USI TWI start condition.
   {
			CLKPR = 0x80 ;		// Do inline for quick response
			CLKPR = 0 ;				// Full speed 8.0MHz
      USI_TWI_SLAVE_Process_Start_Condition();     // Process the USI TWI start condition.
   }
  
   if (USISR & (1<<USIOIF))   // Check for USI_TWI_SLAVE overflow condition.
   {
      // Handle the TWI overflow condition.
      USI_TWI_SLAVE_Process_Overflow_Condition();
   }
   
   if (USISR & (1<<USIPF))   // Check for TWI stop condition.
   {
      USISR = (1<<USIPF);    // Clear the stop condition flag.
      if ( Run_app )		     // Check for run the loaded application
      {
  			DDR_USI = 0 ;			// All inputs
				USICR = 0 ;

				((void (*)(void)) (0x0080>>1))() ;		// Go execute the loaded application
      }
      else
      {
			  USI_TWI_SLAVE_Overflow_State = USI_TWI_SLAVE_OVERFLOW_STATE_NONE ;
				Command_Flag = 0 ;
      }
			if ( Read_program == 2 )
			{
				Read_program = 0 ;		// Cancel reading
			}
			set_clock( 5 ) ;		// Slow to 0.25MHz
		}
		wdt_reset() ;
  }   
}


/******************************************************************************/
//Description: 
// Process the USI TWI start condition.  This is called when the TWI master initiates
// communication with a USI TWI slave by asserting the TWI start condition.
/******************************************************************************/
static void USI_TWI_SLAVE_Process_Start_Condition(void)
{    
    // Wait until the "Start Condition" is complete when SCL goes low. If we fail to wait
    // for SCL to go low we may miscount the number of clocks pulses for the data because
    // the transition of SCL could be mistaken as one of the data clock pulses.
    while ((PIN_USI & (1<<PORT_USI_SCL)));
    // Reset the overflow state.
    USI_TWI_SLAVE_Overflow_State = USI_TWI_SLAVE_OVERFLOW_STATE_NONE;
    // Clear the interrupt flags and reset the counter.
    USISR = (1<<USISIF) | (1<<USIOIF) | (1<<USIPF) |    // Clear interrupt flags.
            (0x00<<USICNT0);                            // USI to sample 8 bits or 16 edge toggles.
    
    // Update the interrupt enable, wire mode and clock settings.
    USICR = (0<<USISIE) | (0<<USIOIE) |                 // Don't enable Overflow and Start Condition Interrupt.
            (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in two-wire mode with clock stretching.
            (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
            (0<<USITC);                                 // No toggle of clock pin.
}

/******************************************************************************/

 //Buffer the entire page
uint8_t *bufferPtr ; // = pageBuffer;
static uint8_t Value;

static uint8_t Tx_buffer[22] ;
static uint8_t Tx_index ;

/*******************************************************************************
Description: 
Processes the USI_TWI_SLAVE overflow condition.  
This is called when the USI TWI 4-bit counterboverflows indicating the 
TWI master has clocked in/out a databyte or a singleback/nack byte following a 
databyte transfer.

*******************************************************************************/
void USI_TWI_SLAVE_Process_Overflow_Condition(void)
{    
    // Buffer the USI data.
	uint8_t Usi_Data = USIDR;
	uint8_t Usi_Ack_Nak = 0 ;
  
  // Handle the interrupt based on the overflow state.
	switch (USI_TWI_SLAVE_Overflow_State)
	{
    /***********************************************************************/
    // Handle the first byte transmitted from master -- the slave address.
    case USI_TWI_SLAVE_OVERFLOW_STATE_NONE:
      
      // Are we receiving our address?
      if ((Usi_Data >> 1) == USI_TWI_SlaveAddress)
      {
        // Yes. Are we to send or receive data?
        if((Usi_Data & 0x01) == 0x01)
        {
            //USI TWI Slave has to transmit the data byte
            USI_TWI_SLAVE_Overflow_State = USI_TWI_SLAVE_OVERFLOW_STATE_ACK_PR_TX;
						Tx_buffer[0] = 0x81 ;
						Tx_index = 0 ;
        }
        else
        {
            //USI TWI Slave has to Receive the data byte
            USI_TWI_SLAVE_Overflow_State = USI_TWI_SLAVE_OVERFLOW_STATE_ACK_PR_RX;
        }
       
        
        // Reset the write state.
       //.. added the below statement at Command_flag=1
        //USI_TWI_SLAVE_Write_State = USI_TWI_SLAVE_WRITE_ADDR_HI_BYTE;
        
        // Set SDA for output.
        PORT_USI |= (1<<PORT_USI_SDA);
        DDR_USI |= (1<<PORT_USI_SDA);
        
        // Load data for ACK.
        USIDR = Usi_Ack_Nak;
        
        // Reload counter for ACK -- two clock transitions.
        USISR = 0x0E;
        
      }
      else
      {
        // No. Reset USI to detect start condition.  Update the interrupt enable, 
        // wire mode and clock settings.  Note: At this time the wire mode must
        // not be set to hold the SCL line low when the counter overflows.  
        // Otherwise, this TWI slave will interfere with other TWI slaves.
        USICR = (1<<USISIE) | (0<<USIOIE) |                 // Enable Start Condition Interrupt. Disable overflow.
          (1<<USIWM1) | (0<<USIWM0) |                 // Maintain USI in two-wire mode without clock stretching.
            (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
              (0<<USITC);                                 // No toggle of clock pin.
      }
      
    break;
       /***********************************************************************/   
      // Ack sent to master so prepare to receive more data.
		case USI_TWI_SLAVE_OVERFLOW_STATE_ACK_PR_RX:
    
      if(Command_Flag == 0)
      {
         // Update our state.
         USI_TWI_SLAVE_Overflow_State = USI_TWI_SLAVE_OVERFLOW_STATE_CMD_RX;
      }
      else if (Command_Flag == 1)
      {
         // Update our state.
         USI_TWI_SLAVE_Overflow_State = USI_TWI_SLAVE_OVERFLOW_STATE_DATA_RX;
      }
    
     // Set SDA for input
     DDR_USI &= ~(1<<PORT_USI_SDA);
     PORT_USI &= ~(1<<PORT_USI_SDA);
    
		break;
    /**************************************************************************/
    
    
  	case USI_TWI_SLAVE_OVERFLOW_STATE_CMD_RX:
       
	    // Update our state
    	USI_TWI_SLAVE_Overflow_State = USI_TWI_SLAVE_OVERFLOW_STATE_NONE;
      
      // Check the command recieved type.
      if(Usi_Data == TWI_CMD_PAGEUPDATE)
      {
          // set this flag to receive data
          Command_Flag =1 ; 
					Read_program = 0 ;
          // Reset the write state.
          USI_TWI_SLAVE_Write_State = USI_TWI_SLAVE_WRITE_ADDR_HI_BYTE ;
					USI_TWI_SLAVE_Overflow_State = USI_TWI_SLAVE_OVERFLOW_STATE_ACK_PR_RX ;
      }
      else if(Usi_Data == TWI_CMD_EXECUTEAPP)
      {
          // reset the controller ... 
        
          // set this flag to receive data
          Command_Flag =0; 
					Run_app = 1 ;
					USI_TWI_SLAVE_Overflow_State = USI_TWI_SLAVE_OVERFLOW_STATE_ACK_PR_RX ;
      }
      else if(Usi_Data == TWI_CMD_SETREAD_ADDRESS )
      {
          // set this flag to receive data
          Command_Flag = 1 ;
					Read_program = 1 ;
          USI_TWI_SLAVE_Write_State = USI_TWI_SLAVE_WRITE_ADDR_HI_BYTE ;
					USI_TWI_SLAVE_Overflow_State = USI_TWI_SLAVE_OVERFLOW_STATE_ACK_PR_RX ;
      }
      else
      {
          // set this flag to receive data
          Command_Flag = 0 ;
					Usi_Ack_Nak = 0x80 ;
					USI_TWI_SLAVE_Overflow_State = USI_TWI_SLAVE_OVERFLOW_STATE_ACK_PR_RX ;
//          USI_TWI_SLAVE_Abort();
      }
  	  // Set SDA for output.
	    PORT_USI |= (1<<PORT_USI_SDA);
    	DDR_USI |= (1<<PORT_USI_SDA);
    
	    // Load data for ACK/NAK.
  	  USIDR = Usi_Ack_Nak ;
    
	    // Reload counter for ACK/NAK -- two clock transitions.
  	  USISR = 0x0E;
	  break;
    
    // Data received from master so prepare to send ACK.
  	case USI_TWI_SLAVE_OVERFLOW_STATE_DATA_RX:
    
    // Update our state.
	    USI_TWI_SLAVE_Overflow_State = USI_TWI_SLAVE_OVERFLOW_STATE_ACK_PR_RX;
    
	    // Always make sure the Command_Flag is 1
  	  Command_Flag = 1;
    
    	// Check the TWI write state to determine what type of byte we received.
    	if (USI_TWI_SLAVE_Write_State == USI_TWI_SLAVE_WRITE_ADDR_HI_BYTE)
    	{
    	  // Set the twi address high byte.
    	  USI_TWI_SLAVE_PAGE_High_Address = Usi_Data;
      
    	  // Set the next state.
    	  USI_TWI_SLAVE_Write_State = USI_TWI_SLAVE_WRITE_ADDR_LO_BYTE;
    	}
    	else if (USI_TWI_SLAVE_Write_State == USI_TWI_SLAVE_WRITE_ADDR_LO_BYTE)
    	{
    	  // Set the address low byte.
    	  USI_TWI_SLAVE_PAGE_Address = (USI_TWI_SLAVE_PAGE_High_Address << 8) | Usi_Data;
      
    	  // Set the programming address.
    	  Value = 0;
    	  bufferPtr = pageBuffer ;
				if (Read_program != 1)
				{
    	  	// Set the next state.
    	  	USI_TWI_SLAVE_Write_State = USI_TWI_SLAVE_WRITE_DATA_BYTE;
				}
    	}
    	else
    	{
    	    *bufferPtr++ = Usi_Data;    	    // Write the data to the buffer.
  
    	    if (++Value >(PAGE_SIZE - 1))			// Check the byte address for wrapping.
    	    {
    	      UpdatePage (USI_TWI_SLAVE_PAGE_Address);  
    	    }
    	  }
    
    	// Set SDA for output.
    	PORT_USI |= (1<<PORT_USI_SDA);
    	DDR_USI |= (1<<PORT_USI_SDA);
    
    	// Load data for ACK.
    	USIDR = Usi_Ack_Nak ;
    
    	// Reload counter for ACK -- two clock transitions.
    	USISR = 0x0E;
    
    break;
     /***********************************************************************/
    
	  case USI_TWI_SLAVE_OVERFLOW_STATE_DATA_TX_AVERSION:
    
//     	// Update our state.
    	USI_TWI_SLAVE_Overflow_State = USI_TWI_SLAVE_OVERFLOW_STATE_PR_ACK_TX;
    
//    	// Set SDA for input.
    	DDR_USI &= ~(1<<PORT_USI_SDA);
    	PORT_USI &= ~(1<<PORT_USI_SDA);
    
//    	// Reload counter for ACK -- two clock transitions.
    	USISR = 0x0E;
    
  	break;
//	  /***********************************************************************/  
    // ACK received from master.  Reset USI state if NACK received.
	  case USI_TWI_SLAVE_OVERFLOW_STATE_PR_ACK_TX:
    
    	// Check the lowest bit for NACK?  If set, the master does not want more data.
    	if (Usi_Data & 0x01)
    	{
    	  // Update our state.
    	  USI_TWI_SLAVE_Overflow_State = USI_TWI_SLAVE_OVERFLOW_STATE_NONE;
      
    	  // Reset USI to detect start condition. Update the interrupt enable,
    	  // wire mode and clock settings. Note: At this time the wire mode must
    	  // not be set to hold the SCL line low when the counter overflows.
    	  // Otherwise, this TWI slave will interfere with other TWI slaves.
    	  USICR = (1<<USISIE) | (0<<USIOIE) |                 // Enable Start Condition Interrupt. Disable overflow.
    	    (1<<USIWM1) | (0<<USIWM0) |                 // Maintain USI in two-wire mode without clock stretching.
    	      (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
    	        (0<<USITC);                                 // No toggle of clock pin.
      
      
    	  // Clear the overflow interrupt flag and release the hold on SCL.
    	  USISR |= (1<<USIOIF);
      
    	  return;
    	}
    
     /***********************************************************************/   
    // Handle sending a byte of data.
  case USI_TWI_SLAVE_OVERFLOW_STATE_ACK_PR_TX:
    
    // Update our state.
     USI_TWI_SLAVE_Overflow_State = USI_TWI_SLAVE_OVERFLOW_STATE_DATA_TX_AVERSION;
    
    // Set SDA for output.
    PORT_USI |= (1<<PORT_USI_SDA);
    DDR_USI |= (1<<PORT_USI_SDA);
    
		if ( Read_program )
		{
			Read_program = 2 ;		// Flag reading data
			USIDR = pgm_read_byte(USI_TWI_SLAVE_PAGE_Address++) ;
		}
		else
		{
			USIDR = Tx_buffer[Tx_index++] ;		// Send this
		}
    
    break;
  }
  USISR |= (1<<USIOIF);  // Clear the overflow interrupt flag and release the hold on SCL.
}



