/*
 * Author - Mike Blandford
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ersky9x.h"
#include "myeeprom.h"
#include "lcd.h"
#include "menus.h"
#include "drivers.h"
#include "stringidx.h"
#include "audio.h"
#include "vars.h"

extern uint8_t InverseBlink ;
extern uint8_t s_currIdx ;

uint32_t doPopup( const char *list, uint16_t mask, uint8_t width, uint8_t event ) ;

#ifndef PACK
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef USE_VARS

extern uint8_t s_pgOfs ;

//#ifndef TOUCH
//extern void alpha\EditName( uint8_t x, uint8_t y, uint8_t *name, uint8_t len, uint16_t type, uint8_t *heading ) ;
//#endif

// Shared memory currently 1408 bytes
// 32*13 = 416
// 128*4 = 512

// Tag 00 = Var
// Tag 01 = Val
// Tag 10 = Act

int16_t VarValues[NUM_VARS] ;

uint16_t VarOffset[NUM_VARS] ;	// Offset into data for each Var

uint16_t TotalStorageUsed ;

#define VAR_PRE_SCALER		10
uint8_t VarPreScaler ;
volatile uint8_t LockVars ;

uint8_t VarUsesTrim ;	// 4 bits
uint8_t CopyUsestrim ;

uint32_t countVarOptions( struct t_varPack *pvar )
{
	uint32_t nVals = 0 ;
	uint32_t nActs = 0 ;
	struct t_valVarPack *pVal = (struct t_valVarPack *)( pvar+1) ;
	while ( pVal->tag == 1 )
	{
		pVal += 1 ;
		nVals += 1 ;
		if ( (uint8_t *)pVal > (uint8_t *) &g_model.varStore[VAR_STORAGE_UINTS-1] )
		{
			break ;
		}		 
	}
	struct t_actVarPack *pAct = (struct t_actVarPack *)pVal ;
	if ( (uint8_t *)pAct <= (uint8_t *) &g_model.varStore[VAR_STORAGE_UINTS-1] )
	{
		while ( pAct->tag == 2 )
		{
			pAct += 1 ;
			nActs += 1 ;		
			if ( (uint8_t *)pAct > (uint8_t *) &g_model.varStore[VAR_STORAGE_UINTS-1] )
			{
				break ;
			}		 
		}
	}
	return ( nActs << 16 ) + nVals ;
}

// Set offsets and count options and actions, fill in counts
void buildVarOffsetTable()
{
	uint32_t i ;
	struct t_varPack *pvar ;
	uint32_t options ;
	uint32_t totalOptions = 0 ;
	uint32_t totalActions = 0 ;

	pvar = (struct t_varPack *) g_model.varStore ;
	for ( i = 0 ; i < NUM_VARS ;  i += 1 )
	{
		VarOffset[i] = (uint8_t *) pvar - (uint8_t *) g_model.varStore ;
		options = countVarOptions( pvar ) ;
		pvar->numOpt = options & 0xFFFF ;
		totalOptions += pvar->numOpt ;
		pvar->numAct = options >> 16 ;
		totalActions += pvar->numAct ;

		uint8_t *t = (uint8_t *) (pvar + 1) ;
		t += pvar->numOpt * sizeof(struct t_valVarPack) + pvar->numAct * sizeof(struct t_actVarPack) ;
		pvar = (t_varPack *) t ;
	}
	TotalStorageUsed = NUM_VARS*sizeof(struct t_varPack) + totalOptions*sizeof(struct t_valVarPack) + totalActions*sizeof(struct t_actVarPack) ;
}

struct t_varPack *getVarAddress( uint32_t index )
{
	struct t_varPack *pvar ;
	uint8_t *p = (uint8_t *) g_model.varStore ;
	p += VarOffset[index] ;	// Offset into data for each Var
	pvar = (struct t_varPack *) p ;
	return pvar ;
}


struct t_valVarPack *getValueAddress( struct t_varPack *pvar, uint32_t position )
{
	struct t_valVarPack *pvalue ;
	pvar += 1 ;	// skip var, point to values
	pvalue = (struct t_valVarPack *) pvar ;	// First value
	pvalue += position ;	// Place to insert
	return pvalue ;
}

void menuVarStorageLimit(uint8_t event)
{
  if ( event == EVT_KEY_FIRST(KEY_EXIT) )
	{
		killEvents(event) ;
		popMenu(false) ;
	}
	PUTS_ATT_LEFT(2*FH, XPSTR("Var Storage Full") ) ;
	PUTS_ATT_LEFT(4*FH, PSTR(STR_PRESS_EXIT_AB));
}


uint32_t insertValue( struct t_varPack *pvar, uint32_t position )
{
	struct t_valVarPack *pvalue ;
	if ( ( sizeof(g_model.varStore) - TotalStorageUsed ) >= sizeof(struct t_valVarPack) )
	{
		LockVars = 1 ;
		pvalue = getValueAddress( pvar, position ) ;
		memmove( pvalue+1, pvalue, (uint8_t *) &g_model.varStore[VAR_STORAGE_UINTS-1] - (uint8_t *) pvalue ) ;
		pvalue->tag = 1 ;
		pvalue->item = 0 ;
		pvalue->value = 0 ;
		pvalue->category = 0 ;
		pvalue->valIsSource = 0 ;
		buildVarOffsetTable() ;
		LockVars = 0 ;
		return 1 ;
	}
	else
	{
		// Report out of memory
		pushMenu(menuVarStorageLimit) ;
	}
	return 0 ;
}

void displayVarName( uint16_t x, uint16_t y, int32_t index, uint16_t attr )
{
	uint8_t chr ;
	uint8_t text[10] ;
	struct t_varPack *pvar ;
	div_t qr ;
	uint8_t *p = text ;

	if ( index < 0 )
	{
		index = -index - 1 ;
		*p++ = '-' ;
	}
	pvar = getVarAddress( index ) ;
	chr = pvar->name[0] ;
	if ( chr && (chr != ' ') )
	{
		p = ncpystr( p, (uint8_t *) pvar->name, sizeof(pvar->name) ) ;
		while ( *(p-1) == ' ' )
		{
			p -= 1 ;
		}
	}
	else
	{
		p = cpystr( p, (uint8_t *) XPSTR("Var") ) ;
		qr = div( (int16_t)index+1, 10 ) ;
		if ( qr.quot )
		{
			*p++ = qr.quot + '0' ;
		}	
		*p++ = qr.rem + '0' ;
	}
	*p = 0 ;
#if defined(PCBX12D) || defined(PCBX10) || defined(TOUCH)
	PUTS_ATT( x, y, (char *)text, attr & LUA_RIGHT ) ;
#else
 #if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
	PUTS_ATT( x, y, (char *)text, attr) ;
 #else
//	PUTS_ATT( x-(p-text)*FW, y, (char *)text, attr ) ;
	PUTS_ATT( x, y, (char *)text, attr ) ;
 #endif
#endif
			 
			
//#ifdef TOUCH
//			lcd_putcAttColour( x+3*FW, y, qr.quot+'0', 0 ) ;
//#else
//			if ( attr & LUA_SMLSIZE )
//			{
//#if defined(PCBX12D) || defined(PCBX10)
//				lcdDrawCharSmall( x+3*FW, y, qr.quot+'0', attr, LcdForeground ) ;
//#else
//				lcd_putcSmall( x+3*FW-2*FW/3, y, qr.quot+'0', attr ) ;
//#endif
//			}
//			else
//			{				
//				lcd_putcAtt( x+3*FW, y, qr.quot+'0', attr ) ;
//			}
//#endif
//		}	
//#ifdef TOUCH
//		lcd_putcAttColour( x+4*FW, y, qr.rem+'0', 0 ) ;
//#else
//		if ( attr & LUA_SMLSIZE )
//		{
//#if defined(PCBX12D) || defined(PCBX10)
//			lcdDrawCharSmall( x+3*FW, y, qr.rem+'0', attr, LcdForeground ) ;
//#else
//			lcd_putcSmall( x+3*FW, y, qr.rem+'0', attr ) ;
//#endif
//		}
//		else
//		{				
//			lcd_putcAtt( x+4*FW, y, qr.rem+'0', attr ) ;
//		}
//#endif
//	}		 
}

void deleteValue( struct t_varPack *pvar, uint32_t position )
{
	struct t_valVarPack *pvalue ;
	LockVars = 1 ;
	pvalue = getValueAddress( pvar, position ) ;
	memmove( pvalue, pvalue+1, (uint8_t *) &g_model.varStore[VAR_STORAGE_UINTS-1] - (uint8_t *) (pvalue+1) ) ;
	buildVarOffsetTable() ;
	LockVars = 0 ;
}

uint32_t insertAction( struct t_varPack *pvar, uint32_t position, uint32_t numValues )
{
	struct t_valVarPack *pvalue ;
	struct t_actVarPack *pact ;
	if ( ( sizeof(g_model.varStore) - TotalStorageUsed ) >= sizeof(struct t_actVarPack) )
	{
		LockVars = 1 ;
		pvar += 1 ;	// skip var, point to values
		pvalue = (struct t_valVarPack *) pvar ;	// First value
		pvalue += numValues ;	// First Action place
		pact = (struct t_actVarPack *) pvalue ;
		pact += position ;
		memmove( pact+1, pact, (uint8_t *) &g_model.varStore[VAR_STORAGE_UINTS-1] - (uint8_t *) (pact) ) ;
		pact->tag = 2 ;
		pact->category = 0 ;
		pact->item = 0 ;
		pact->function = 0 ;
		pact->value = 10 ;
		buildVarOffsetTable() ;
		LockVars = 0 ;
		return 1 ;
	}
	else
	{
		// Report out of memory
		pushMenu(menuVarStorageLimit) ;
	}
	return 0 ;
}

void deleteAction( struct t_varPack *pvar, uint32_t position, uint32_t numValues )
{
	struct t_valVarPack *pvalue ;
	struct t_actVarPack *pact ;
	LockVars = 1 ;
	pvar += 1 ;	// skip var, point to values
	pvalue = (struct t_valVarPack *) pvar ;	// First value
	pvalue += numValues ;	// First Action place
	pact = (struct t_actVarPack *) pvalue ;
	pact += position ;
	memmove( pact, pact+1, (uint8_t *) &g_model.varStore[VAR_STORAGE_UINTS-1] - (uint8_t *) (pact+1) ) ;
	buildVarOffsetTable() ;
	LockVars = 0 ;
}

//struct t_varPack *s_currVar ;
struct t_varPack *PopPvar ;
uint8_t PopOpts ;
uint8_t PopActs ;	// Set to 0xFF indicates delete value

const char StringValueCategory[] = "\006    OnSwitchF.Mode" ;
const char StringActionCategory[] = "\006    OnSwitch" ;
const char StringActionFunction[] = "\010  Assign     AddSubtractMultiply  Divide     Min     MaxUse Trim" ;	// percent, min, max, repurpose trim

//void displayValueCategory( uint16_t x, uint16_t y, uint32_t index, uint16_t attr )
//{
//	lcd_putsAttIdx( x, y, StringValueCategory, index, attr ) ;
//}

//void displayActionCategory( uint16_t x, uint16_t y, uint32_t index, uint16_t attr )
//{
//	lcd_putsAttIdx( x, y, StringActionCategory, index, attr ) ;
//}

void displayActionFunction( uint16_t x, uint16_t y, uint32_t index, uint16_t attr )
{
	PUTS_AT_IDX( x, y, StringActionFunction, index, attr ) ;
}

void displayValueItem( uint16_t x, uint16_t y, struct t_valVarPack *pvalue, uint16_t attr )
{
	switch ( pvalue->category )
	{
		case 0 :
			PUTS_ATT( x, y, XPSTR("---"), attr|LUA_RIGHT ) ;
		break ;
		case 1 :	// Switch
		  putsDrSwitches( x, y, pvalue->item ,attr|LUA_RIGHT );
		break ;
		case 2 :	// Flight Mode
		{
			char text[4] ;
			text[0] = 'F' ;
			text[1] = 'M' ;
			text[2] = pvalue->item+'0' ;	 
			text[3] = 0 ;
			PUTS_ATT( x, y, text, attr|LUA_RIGHT ) ;
//			PUTC_ATT( x-3*FW-1, y, 'F', attr ) ;
//			PUTC_ATT( x-2*FW-1, y, 'M', attr ) ;
//			PUTC_ATT( x-FW, y, pvalue->item+'0', attr ) ;
		}
		break ;
	}
}

void displayActionItem( uint16_t x, uint16_t y, struct t_actVarPack *pAction, uint16_t attr )
{
	switch ( pAction->category )
	{
		case 0 :
			PUTS_ATT( x, y, XPSTR("---"), attr|LUA_RIGHT ) ;
		break ;
		case 1 :	// Switch
		  putsDrSwitches( x, y, pAction->item ,attr|LUA_RIGHT ) ;
		break ;
	}
}


void displayVarSource( uint16_t x, uint16_t y, struct t_valVarPack *pvalue, uint16_t attr )
{
	uint32_t t ;
	t = (uint8_t) pvalue->value ;
	if ( t >= EXTRA_POTS_START )
	{
		PUTS_AT_IDX( x, y, PSTR(STR_CHANS_EXTRA), t - EXTRA_POTS_START, attr ) ;
	}
	else
	{
#ifdef PCBT12
//				if ( ( pgvar->gvsource == 10 ) || ( pgvar->gvsource == 11 ) )
		if ( ( t == 10 ) || ( t == 11 ) )
		{
			PUTS_AT_IDX( x-4, y, "\004AUX4AUX5", t - 10, attr ) ;
		}
		else
#endif 
		PUTS_AT_IDX( x, y, PSTR(STR_GV_SOURCE), t, attr ) ;
	}
}			 

void menuDeleteValue(uint8_t event)
{
	uint8_t action ;
	action = yesNoMenuExit( event, XPSTR("Delete Value?") ) ;
  
	switch( action )
	{
    case YN_YES :
			deleteValue( PopPvar, PopOpts ) ;
      //fallthrough
		case YN_NO :
    break;
  }
}

void menuDeleteAction(uint8_t event)
{
	uint8_t action ;
	action = yesNoMenuExit( event, XPSTR("Delete Action?") ) ;
  
	switch( action )
	{
    case YN_YES :
			deleteAction( PopPvar, PopActs, PopOpts ) ;
      //fallthrough
		case YN_NO :
    break;
  }
}


void varpopup( uint8_t event )
{
	uint8_t popaction = doPopup( PSTR(STR_MIX_POPUP), 0x11, 8, event ) ;
	
  if ( popaction == POPUP_SELECT )
	{
		uint8_t popidx = PopupData.PopupSel ;
		if ( popidx == 4 )		// Delete
		{
			killEvents(event);
			Tevent = 0 ;
			if ( PopActs == 0xFF )
			{
				pushMenu(menuDeleteValue) ;
			}
			else
			{
				pushMenu(menuDeleteAction) ;
			}
		}
		else if ( popidx == 0 )		// Edit
		{
			killEvents(event);
			Tevent = 0 ;
			if ( PopActs == 0xFF )
			{
	  	  pushMenu(menuOneValue) ;
			}
			else
			{
	  	  pushMenu(menuOneAction) ;
			}
		}
		PopupData.PopupActive = 0 ;
		s_editMode = 0 ;
	}
}



// From Ethos
// Category - ALways ON, Direction, Switch positions,Logic Switches, Trim positions, telemetry, sys event. Fn Switches(6pos)
// Actions (up to 10) - ALways ON, Switch positions, Fn Switches(6pos), Logic Switches, Trim positions, telemetry, sys event
//    Function - Assign(=), Add, subtract, multiply, divide, percent, min, max, repurpose trim

//#ifndef TOUCH
#if not (defined(PCBX12D) || defined(PCBX10) || defined(TOUCH) )
void menuOneAction( uint8_t event )
{
	TITLE(XPSTR("Action")) ;
	uint32_t rows = 4 ;
	static MState2 mstate2 ;
	struct t_actVarPack	*pAction ;
	struct t_valVarPack *pvalue ;
	uint32_t i ;
	uint8_t blink = InverseBlink ;

	pvalue = getValueAddress( PopPvar, PopOpts ) ;
	pAction = (struct t_actVarPack *)	pvalue ;
	pAction += PopActs ;

	if ( pAction->function == 7 )
	{
		rows = 5 ;
	}

	mstate2.check_columns( event, rows-1 ) ;

	uint8_t sub = mstate2.m_posVert ;

 	for( i = 0 ; i < rows ; i += 1 )
	{
		uint16_t y = (1+i)*FH ;
		uint16_t attr = (sub==i) ? blink : 0 ;
		
		switch ( i )
		{
			case 0 :
			{
				int32_t oldCategory = pAction->category ;
				PUTS_ATT_LEFT( y, XPSTR("Category") ) ;
				PUTS_AT_IDX( 12*FW, y, StringActionCategory, pAction->category, attr ) ;
	 			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( pAction->category, 1 ) ;
				}
				if ( pAction->category != oldCategory )
				{
					pAction->item = 0 ;
					pAction->function = 0 ;
					pAction->value = 10 ;
				}
			}
			break ;
			case 1 :
				PUTS_ATT_LEFT( y, XPSTR("Item") ) ;
				displayActionItem( 18*FW, y, pAction, attr ) ;
				switch ( pvalue->category )
				{
					case 1 :	// Switch
					{	
						int32_t oldSwitch = pAction->item ;
						if ( attr )
						{
							CHECK_INCDEC_MODELSWITCH( pAction->item, -MaxSwitchIndex, MaxSwitchIndex) ;
						}
						if ( oldSwitch != pAction->item )
						{
							pAction->swPosition = getSwitch00( pAction->item ) ;
						}
					}
					break ;
				}
			break ;
			case 2 :
				PUTS_ATT_LEFT( y, XPSTR("Function") ) ;
				PUTS_AT_IDX( 10*FW, y, StringActionFunction, pAction->function, attr ) ;
	 			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( pAction->function, 7 ) ;
					if ( pAction->function == 7 )
					{
						if ( pAction->value > 3 )
						{
							pAction->value = 0 ;
						}
					}
				}
			break ;
			case 3 :
			{	
				int32_t lvalue = pAction->value ;
				uint32_t lindex = (pAction->value >> 9) & 0x03 ;
				if ( pAction->function == 7 )
				{
					PUTS_ATT_LEFT( y, XPSTR("Trim") ) ;
					PUTS_AT_IDX( 16*FW, y, XPSTR("\002LHLVRVRH"), lindex, attr ) ;
				}
				else
				{
					PUTS_ATT_LEFT( y, XPSTR("Value") ) ;
					PUTS_NUM( 18*FW, y, pAction->value, attr|PREC1) ;
				}
				if(attr)
				{
					if ( pAction->function == 7 )
					{
						lindex = checkIncDec16( lindex, 0, 3, EE_MODEL ) ;
						pAction->value = (lindex << 9) | ( lvalue & 0x01FF ) ;
					}
					else
					{
						pAction->value = checkIncDec16( pAction->value, -1000, 1000, EE_MODEL ) ;
					}
 	 			}
			}
			break ;
			case 4 :
			{
				int32_t lvalue = pAction->value & 0x00FF ;
				if ( pAction->value & 0x0100 )
				{
					lvalue -= 256 ;
				}
				uint32_t lindex = (pAction->value >> 9) & 0x03 ;
				PUTS_ATT_LEFT( y, XPSTR("Value") ) ;
				PUTS_NUM( 18*FW, y, lvalue, attr|PREC1) ;
				if(attr)
				{
					lvalue = checkIncDec16( lvalue, -250, 250, EE_MODEL ) ;
					pAction->value = (lindex << 9) | ( lvalue & 0x01FF ) ;
 	 			}
			}
			break ;
		}
	}
}

void menuOneValue( uint8_t event )
{
	TITLE(XPSTR("Value")) ;
	uint32_t rows = 4 ;
	static MState2 mstate2 ;
	mstate2.check_columns( event, rows-1 ) ;
	struct t_valVarPack *pvalue ;
	uint32_t i ;
	uint8_t sub = mstate2.m_posVert ;
	uint8_t blink = InverseBlink ;
	uint8_t sItem ;

	pvalue = getValueAddress( PopPvar, PopOpts ) ;

 	for( i = 0 ; i < rows ; i += 1 )
	{
		uint16_t y = (1+i)*FH ;
		uint16_t attr = (sub==i) ? blink : 0 ;
		
		switch ( i )
		{
			case 0 :
			{
				int32_t oldCategory = pvalue->category ;
				PUTS_ATT_LEFT( y, XPSTR("Category") ) ;
				PUTS_AT_IDX( 12*FW, y, StringValueCategory, pvalue->category, attr ) ;
	 			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( pvalue->category, 2 ) ;
				}
				if ( pvalue->category != oldCategory )
				{
					pvalue->item = 0 ;
				}
			}
			break ;
			case 1 :
				PUTS_ATT_LEFT( y, XPSTR("Item") ) ;
				displayValueItem( 18*FW, y, pvalue, attr ) ;
				switch ( pvalue->category )
				{
					case 1 :	// Switch
						if ( attr )
						{
							CHECK_INCDEC_MODELSWITCH( pvalue->item, -MaxSwitchIndex, MaxSwitchIndex) ;
						}
					break ;
					case 2 :	// Flight Mode
			 			if(attr)
						{
							CHECK_INCDEC_H_MODELVAR_0( pvalue->item, 7 ) ;
						}
					break ;
				}
			break ;
	 
			case 2 :
			{
				uint32_t old = pvalue->valIsSource ;
//extern uint8_t onoffMenuItem( uint8_t value, uint8_t y, const prog_char *s, uint8_t condition ) ;
				pvalue->valIsSource = onoffMenuItem( pvalue->valIsSource, y, XPSTR("Use Source"), attr ) ;
				if ( pvalue->valIsSource != old )
				{
					pvalue->value = 0 ;
				}
			}
			break ;
			case 3 :
				PUTS_ATT_LEFT( y, XPSTR("Value") ) ;
 				if ( pvalue->valIsSource )
				{
					displayVarSource( 17*FW, y, pvalue, attr ) ;
			 		if(attr)
					{
void editGvarSource(uint8_t *p) ;
						sItem = pvalue->value ;
						editGvarSource(&sItem) ;
						pvalue->value = sItem ;
					}
				}
				else
				{
					PUTS_NUM( 17*FW, y, pvalue->value, attr|PREC1) ;
					if(attr)
					{
						pvalue->value = checkIncDec16( pvalue->value, -2000, 2000, EE_MODEL ) ;
  	 			}
				}
			break ;
		}
	}
}


void menuOneVar(uint8_t event)
{
	TITLE(XPSTR("Var")) ;
	if ( s_currIdx > 8 )
	{
		uint8_t t = s_currIdx+1 ;
#if defined(PCBX12D) || defined(PCBX10)
		PUTC(7*FW, 0, t/10+'0' ) ;
#else
		PUTC(3*FW, 0, t/10+'0' ) ;
#endif
		t %= 10 ; 
#if defined(PCBX12D) || defined(PCBX10)
		PUTC(8*FW, 0, t+'0' ) ;
#else
		PUTC(4*FW, 0, t+'0' ) ;
#endif
	}
	else
	{
#if defined(PCBX12D) || defined(PCBX10)
		PUTC(8*FW, 0, s_currIdx+'1' ) ;
#else
		PUTC(4*FW, 0, s_currIdx+'1' ) ;
#endif
	}
		
	PUTC(10*FW, 0, '=' ) ;
	PUTS_NUM( 16*FW, 0, VarValues[s_currIdx], PREC1) ;

	struct t_varPack *pvar ;
	uint32_t options ;
	uint32_t actions ;
	uint32_t i ;
	uint32_t j ;
	uint32_t k ;
	uint32_t m ;
	pvar = getVarAddress( s_currIdx ) ;
	options = pvar->numOpt ;
	actions = pvar->numAct ;

	static MState2 mstate2 ;
	uint32_t rows = 4 + ( options + actions ) ;
	if ( options < MAX_VALS )
	{
		rows += 1 ;
	}
	if ( actions < MAX_ACTS )
	{
		rows += 1 ;
	}
	
	if ( !PopupData.PopupActive )
	{
		mstate2.check_columns( event, rows-1 ) ;
	}
	
	uint32_t sub = mstate2.m_posVert ;
	uint32_t blink = InverseBlink ;
	uint32_t t_pgOfs = evalOffset( sub ) ;
	
	if ( PopupData.PopupActive )
	{
		Tevent = 0 ;
	}
	 
//	pvar = (struct t_varPack *) s_currVar ;

	j = 0 ;
	if ( t_pgOfs > 4)
	{
		j = t_pgOfs - 4 ;
	}
	k = 0 ;
	m = 0 ;
 	for( i = 0 ; i < rows ; i += 1 )
	{
		coord_t y = (1+i)*FH ;
    k = i + t_pgOfs ;
		uint32_t attr = (sub==k) ? blink : 0 ;

		switch ( k )
		{
			case 0 :
				PUTS_ATT_LEFT( y, XPSTR("Value") ) ;
 				PUTS_NUM( 17*FW, y, pvar->value, attr|PREC1) ;
				if(attr)
				{
					pvar->value = checkIncDec16( pvar->value, -2000, 2000, EE_MODEL ) ;
  	 		}
			break ;
			case 1 :
				PUTS_ATT_LEFT( y, XPSTR("Min") ) ;
	 			PUTS_NUM( 17*FW, y, pvar->min, attr|PREC1) ;
				if(attr)
				{
					pvar->min = checkIncDec16( pvar->min, -2000, 2000, EE_MODEL ) ;
  	 		}
			break ;
			case 2 :
				PUTS_ATT_LEFT( y, XPSTR("Max") ) ;
	 			PUTS_NUM( 17*FW, y, pvar->max, attr|PREC1) ;
				if(attr)
				{
					pvar->max = checkIncDec16( pvar->max, -2000, 2000, EE_MODEL ) ;
  	 		}
			break ;
			case 3 :
				alphaEditName( 11*FW-2, y, (uint8_t *)pvar->name, sizeof(pvar->name), attr, (uint8_t *)XPSTR( "Name") ) ;
			break ;
			default :
				// j = ???
				if ( j < options )
				{
//					lcd_putsAtt( 0, y, XPSTR("Value"), attr ) ;
//					PUTC_ATT(6*FW, y, j+'1', attr ) ;
	
					struct t_valVarPack *pvalue ;
					pvalue = getValueAddress( pvar, j ) ;
					PUTS_AT_IDX( 0, y, StringValueCategory, pvalue->category, attr ) ;
					displayValueItem( 12*FW, y, pvalue, attr ) ;
 					
	 				if ( pvalue->valIsSource )
					{
						displayVarSource( 14*FW, y, pvalue, attr ) ;
					}
					else
					{
						PUTS_NUM( 17*FW, y, pvalue->value, attr|PREC1) ;
					}

					if (sub==k)
					{
						if ( !PopupData.PopupActive )
						{
							if (event == EVT_KEY_FIRST(KEY_MENU) || event == EVT_KEY_BREAK(BTN_RE) )
							{
  	   					killEvents(event) ;
								s_editMode = 0 ;
								PopPvar = pvar ;
								PopOpts = j ;
								PopActs = 0xFF ;
								PopupData.PopupIdx = 0 ;
								PopupData.PopupActive = 1 ;
								event = 0 ;		// Kill this off
							}
						}
					}
					j += 1 ;
				}
				else if ( ( j == options) && (options < MAX_VALS) )
				{
					PUTS_ATT( 9*FW, y, XPSTR("Add Value"), attr ) ;
					if (sub==k)
					{
						if ( !PopupData.PopupActive )
						{
							if (event == EVT_KEY_FIRST(KEY_MENU) || event == EVT_KEY_BREAK(BTN_RE) )
							{
 	   						killEvents(event) ;
								s_editMode = 0 ;
								options += insertValue( pvar, options ) ;
							}
						}
					}
					j += 1 ;
				}
				else
				{
					if ( m < actions )
					{
						struct t_actVarPack	*pAction ;
						struct t_valVarPack *pvalue ;
						pvalue = getValueAddress( pvar, options ) ;
						pAction = (struct t_actVarPack *)	pvalue ;
						pAction += m ;

						PUTS_AT_IDX( 0, y, StringActionCategory, pAction->category, attr ) ;
						displayActionItem( 11*FW, y, pAction, attr ) ;
						PUTS_AT_IDX( 12*FW, y, XPSTR("\001=+-*/v^T"), pAction->function, attr ) ;
						int32_t lvalue = pAction->value ;
						if ( (pAction->function == 7) )
						{
							lvalue &= 0x01FF ;
						}
						PUTS_NUM( 17*FW, y, lvalue, attr|PREC1) ;

//						lcd_putsAtt( 0, y, XPSTR("Action"), attr ) ;
//						PUTC_ATT( 7*FW, y, m+'1', attr ) ;
						if (sub==k)
						{
							if ( !PopupData.PopupActive )
							{
								if (event == EVT_KEY_FIRST(KEY_MENU) || event == EVT_KEY_BREAK(BTN_RE) )
								{
 	   							killEvents(event) ;
									s_editMode = 0 ;
									PopPvar = pvar ;
									PopOpts = options ;
									PopActs = m ;
									PopupData.PopupIdx = 0 ;
									PopupData.PopupActive = 1 ;
									event = 0 ;		// Kill this off
								}
							}
						}
						m += 1 ;
					}
					else if ( ( m == actions) && (actions < MAX_ACTS) )
					{
						PUTS_ATT( 8*FW, y, XPSTR("Add Action"), attr ) ;
						if (sub==k)
						{
							if ( !PopupData.PopupActive )
							{
								if (event == EVT_KEY_FIRST(KEY_MENU) || event == EVT_KEY_BREAK(BTN_RE) )
								{
 	   							killEvents(event) ;
									s_editMode = 0 ;
									actions += insertAction( pvar, actions, options ) ;
								}
							}
						}
						m += 1 ;
					}
				}
			break ;
		}
		if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
	}

	if ( PopupData.PopupActive )
	{
		Tevent = event ;
		varpopup( event ) ;
    s_editMode = false;
	}
}

void menuVars(uint8_t event)
{
	TITLE(XPSTR("Vars"));
	static MState2 mstate2 ;
	uint32_t rows = NUM_VARS ;
	mstate2.check_columns( event, rows-1 ) ;
	static uint8_t savedOffset ;

	if ( event == EVT_ENTRY_UP )
	{
		s_pgOfs = savedOffset ;
	}

	uint32_t sub = mstate2.m_posVert ;
	uint32_t blink = InverseBlink ;
	uint32_t i ;

  uint32_t t_pgOfs ;
	t_pgOfs = evalOffset( sub ) ;
  uint32_t k ;

//	s_currVar = getVarAddress( 0 ) ;

  switch (event)
	{
  	case EVT_KEY_FIRST(KEY_MENU) :
  	case EVT_KEY_BREAK(BTN_RE) :
  	   s_currIdx = sub ;	// s_currVar is also set
  	   killEvents(event);
  	   pushMenu(menuOneVar) ;
		break ;
  }

	savedOffset = t_pgOfs ;

	uint32_t lines = rows > SCREEN_LINES-1 ? SCREEN_LINES-1 : rows ;
 	for( i = 0 ; i < lines ; i += 1 )
	{
		uint32_t attr = 0 ;
		coord_t y = (1+i)*FHPY ;
    k = i + t_pgOfs ;

		if (sub==k)
		{
//			s_currVar = pvar ;
			attr = blink ;
		}
		
 #if defined(PCBX12D) || defined(PCBX10) || defined(TOUCH)
		displayVarName( THOFF, y, k, 0 ) ;
 #else
		displayVarName( 0, y, k, 0 ) ;
 #endif
// 		lcd_outdezAtt( 10*FW, y, pvar->numOpt, 0) ;
// 		lcd_outdezAtt( 12*FW, y, pvar->numAct, 0) ;

 #if defined(PCBX12D) || defined(PCBX10) || defined(TOUCH)
		PUTS_NUM( TRIGHT-TRMARGIN/2, y, VarValues[k], attr|PREC1) ;
 #else
		PUTS_NUM( 17*FW, y, VarValues[k], attr|PREC1) ;
 #endif
	}
}


#endif

#ifdef TOUCH

extern uint8_t TlExitIcon ;
extern uint8_t evalHresOffset(int8_t sub) ;

#endif

extern uint8_t EditType ;
extern uint8_t s_pgOfs ;
extern uint8_t evalHresOffset(int8_t sub) ;
extern void drawIdxText( uint16_t y, char *s, uint32_t index, uint16_t mode ) ;

#if defined(PCBX12D) || defined(PCBX10) || defined(TOUCH)

void menuOneAction( uint8_t event )
{
	TITLE(XPSTR("Action ")) ;
	EditType = EE_MODEL ;
	static MState2 mstate2 ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	uint32_t rows = 4 ;
	struct t_actVarPack	*pAction ;
	struct t_valVarPack *pvalue ;

	pvalue = getValueAddress( PopPvar, PopOpts ) ;
	pAction = (struct t_actVarPack *)	pvalue ;
	pAction += PopActs ;

	if ( pAction->function == 7 )
	{
		rows = 5 ;
	}
	 
	event = mstate2.check_columns( event, rows-1 ) ;
	
	uint8_t sub = mstate2.m_posVert ;
	
	uint16_t colour = dimBackColour() ;
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif
//	sub = mstate2.m_posVert = handleTouchSelect( rows, 0, sub ) ;

	lcd_hline( 0, TTOP, TRIGHT ) ;

 	for( uint32_t i = 0 ; i < rows ; i += 1 )
	{
    uint16_t y = i * TFH + TTOP ;
    uint16_t attr = (sub==i) ? INVERS : 0 ;
		
		switch ( i )
		{
			case 0 :
			{
				int32_t oldCategory = pAction->category ;
				drawItem( (char *)XPSTR("Category"), y, attr ) ;
				drawIdxText( y*2+TVOFF, (char *)StringActionCategory, pAction->category, attr ) ;
	 			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR( pAction->category, 0, 1 ) ;
				}
				if ( pAction->category != oldCategory )
				{
					pAction->item = 0 ;
					pAction->function = 0 ;
					pAction->value = 10 ;
				}
			}
			break ;
			case 1 :
				drawItem( (char *)XPSTR("Item"), y, attr ) ;
				if ( attr & INVERS )
				{
					if ( s_editMode && BLINK_ON_PHASE )
					{
						attr = 0 ;
					}
				}
				saveEditColours( attr, colour ) ;
				displayActionItem( TRIGHT-TRMARGIN, y+TVOFF, pAction, attr & ~INVERS ) ;
				restoreEditColours() ;
				switch ( pvalue->category )
				{
					case 0 :	// Nothing to edit
						if ( attr )
						{
							s_editMode = 0 ;
						}
					break ;
					case 1 :	// Switch
					{	
						int32_t oldSwitch = pAction->item ;
						if ( attr )
						{
							CHECK_INCDEC_MODELSWITCH( pAction->item, -MaxSwitchIndex, MaxSwitchIndex) ;
						}
						if ( oldSwitch != pAction->item )
						{
							pAction->swPosition = getSwitch00( pAction->item ) ;
						}
					}
					break ;
				}
			break ;
			case 2 :
				drawItem( (char *)XPSTR("Function"), y, attr ) ;
				drawIdxText( y*2+TVOFF, (char *)StringActionFunction, pAction->function, attr ) ;
	 			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR( pAction->function, 0, 7 ) ;
					if ( pAction->function == 7 )
					{
						if ( pAction->value > 3 )
						{
							pAction->value = 0 ;
						}
					}
				}
			break ;
			case 3 :
			{
				int32_t lvalue = pAction->value ;
				uint32_t lindex = (pAction->value >> 9) & 0x03 ;
				if ( pAction->function == 7 )
				{
			 		drawItem( (char *)XPSTR("Trim"), y, attr ) ;
					drawIdxText( y*2+TVOFF, (char *)XPSTR("\002LHLVRVRH"), lindex, attr ) ;
				}
				else
				{
			 		drawItem( (char *)XPSTR("Value"), y, attr ) ;
					drawNumber( TRIGHT-TRMARGIN, y, pAction->value, attr|PREC1 ) ;
				}
				if(attr)
				{
					if ( pAction->function == 7 )
					{
						lindex = checkIncDec16( lindex, 0, 3, EE_MODEL ) ;
						pAction->value = (lindex << 9) | ( lvalue & 0x01FF ) ;
					}
					else
					{
						pAction->value = checkIncDec16( pAction->value, -1000, 1000, EE_MODEL ) ;
					}
 	 			}
			}
			break ;
			case 4 :
			{
				int32_t lvalue = pAction->value & 0x01FF ;
				uint32_t lindex = (pAction->value >> 9) & 0x03 ;
				drawItem( (char *)XPSTR("Value"), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, lvalue, attr|PREC1 ) ;
				if(attr)
				{
					lvalue = checkIncDec16( lvalue, -250, 250, EE_MODEL ) ;
					pAction->value = (lindex << 9) | ( lvalue & 0x01FF ) ;
 	 			}
			}
			break ;
		}
	}
}

void menuOneValue( uint8_t event )
{
	TITLE(XPSTR("Value ")) ;
	EditType = EE_MODEL ;
	static MState2 mstate2 ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	uint32_t rows = 4 ;
	struct t_valVarPack *pvalue ;
	uint8_t sItem ;
	
	event = mstate2.check_columns( event, rows-1 ) ;
	
	uint8_t sub = mstate2.m_posVert ;

	uint16_t colour = dimBackColour() ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif
//	sub = mstate2.m_posVert = handleTouchSelect( rows, 0, sub ) ;

	lcd_hline( 0, TTOP, TRIGHT ) ;

	pvalue = getValueAddress( PopPvar, PopOpts ) ;

 	for( uint32_t i = 0 ; i < rows ; i += 1 )
	{
    uint16_t y = i * TFH + TTOP ;
    uint16_t attr = (sub==i) ? INVERS : 0 ;
		
		switch ( i )
		{
			case 0 :
			{
				drawItem( (char *)XPSTR("Category"), y, attr ) ;
				drawIdxText( y*2+TVOFF, (char *)StringValueCategory, pvalue->category, attr ) ;
				int32_t oldCategory = pvalue->category ;
				if(attr)
				{
					CHECK_INCDEC_H_MODELVAR( pvalue->category, 0, 2 ) ;
				}
				if ( pvalue->category != oldCategory )
				{
					pvalue->item = 0 ;
				}
			}
			break ;
			case 1 :
				drawItem( (char *)XPSTR("Item"), y, attr ) ;
				if ( attr & INVERS )
				{
					if ( s_editMode && BLINK_ON_PHASE )
					{
						attr = 0 ;
					}
				}
				saveEditColours( attr, colour ) ;
				displayValueItem( TRIGHT-TRMARGIN, y+TVOFF/2, pvalue, attr & ~INVERS ) ;
				restoreEditColours() ;
				switch ( pvalue->category )
				{
					case 0 :	// Nothing to edit
						if ( attr )
						{
							s_editMode = 0 ;
						}
					break ;
					case 1 :	// Switch
						if ( attr )
						{
							CHECK_INCDEC_MODELSWITCH( pvalue->item, -MaxSwitchIndex, MaxSwitchIndex) ;
						}
					break ;
					case 2 :	// Flight Mode
			 			if(attr)
						{
							CHECK_INCDEC_H_MODELVAR( pvalue->item, 0, 7 ) ;
						}
					break ;
				}
			break ;
	 
			case 2 :
			{
				uint32_t old = pvalue->valIsSource ;
				pvalue->valIsSource = touchOnOffItem( pvalue->valIsSource, y, XPSTR("Use Source"), ( sub==i ), colour ) ;
				if ( pvalue->valIsSource != old )
				{
					pvalue->value = 0 ;
				}
			}
			break ;
			case 3 :
				drawItem( (char *)XPSTR("Value"), y, attr ) ;
 				if ( pvalue->valIsSource )
				{
					if ( attr & INVERS )
					{
						if ( s_editMode && BLINK_ON_PHASE )
						{
							attr = 0 ;
						}
					}
					saveEditColours( attr, colour ) ;
					displayVarSource( TRIGHT-TRMARGIN-3*FW, y+TVOFF, pvalue, attr & ~INVERS ) ;
					restoreEditColours() ;
			 		if(attr)
					{
void editGvarSource(uint8_t *p) ;
						sItem = pvalue->value ;
						editGvarSource(&sItem) ;
						pvalue->value = sItem ;
					}
				}
				else
				{
					drawNumber( TRIGHT-TRMARGIN, y, pvalue->value, attr|PREC1 ) ;
					if(attr)
					{
						pvalue->value = checkIncDec16( pvalue->value, -2000, 2000, EE_MODEL ) ;
  	 			}
				}
			break ;
		}
	}
}

void menuOneVar(uint8_t event)
{
	TITLE(XPSTR("Var ")) ;
	if ( s_currIdx > 8 )
	{
//		lcd_2_digits( 9*FW, 0, s_currIdx+1, 0 ) ;
		PUTS_NUM_N( 9*FW, 0, s_currIdx+1, 0, 2 ) ;
	}
	else
	{
		PUTC(8*FW, 0, s_currIdx+'1' ) ;
	}

	PUTC(10*FW, 0, '=' ) ;
	PUTS_NUM( 16*FW, 0, VarValues[s_currIdx], PREC1) ;

	EditType = EE_MODEL ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	static MState2 mstate2 ;
	
	struct t_varPack *pvar ;
	uint32_t options ;
	uint32_t actions ;
	uint32_t j ;
	uint32_t k ;
	uint32_t m ;
	uint32_t t_pgOfs ;
#ifdef TOUCH
	uint32_t selected = 0 ;
#endif
	 
	pvar = getVarAddress( s_currIdx ) ;
	options = pvar->numOpt ;
	actions = pvar->numAct ;

	uint32_t rows = 4 + ( options + actions ) ;
	if ( options < MAX_VALS )
	{
		rows += 1 ;
	}
	if ( actions < MAX_ACTS )
	{
		rows += 1 ;
	}
	
	if ( !PopupData.PopupActive )
	{
		event = mstate2.check_columns( event, rows-1 ) ;
	}
	
	uint8_t sub = mstate2.m_posVert ;

	if ( PopupData.PopupActive )
	{
		Tevent = 0 ;
	}

	t_pgOfs = evalHresOffset( sub ) ;
	
#ifdef TOUCH
	if ( !PopupData.PopupActive )
	{
		int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 0 ) ;
		uint16_t newVpos = processSelection( sub , newSelection ) ;
		sub = mstate2.m_posVert = newVpos & 0x00FF ;
		selected = newVpos & 0x0100 ;

		if ( rows > TLINES )
		{
			newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
			if ( newVpos != t_pgOfs )
			{
				s_pgOfs = t_pgOfs = newVpos ;
				if ( sub < t_pgOfs )
				{
					mstate2.m_posVert = sub = t_pgOfs ;
				}
				else if ( sub > t_pgOfs + TLINES - 1 )
				{
					mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
				}
			}
		}
	}
#endif
	lcd_hline( 0, TTOP, TRIGHT ) ;
	 
//	pvar = (struct t_varPack *) s_currVar ;

	j = 0 ;
	if ( t_pgOfs > 4)
	{
		j = t_pgOfs - 4 ;
	}
	k = 0 ;
	m = 0 ;
	
	uint16_t colour = dimBackColour() ;
	
	uint32_t lines = rows > TLINES ? TLINES : rows ;
	for (uint32_t i = 0 ; i < lines ; i += 1 )
	{
    uint16_t y = TTOP + i*TFH ;
    k = i + t_pgOfs ;
		uint16_t attr = (sub==k) ? INVERS : 0 ;
		uint32_t process = 0 ;

		switch ( k )
		{
			case 0 :
				drawItem( (char *)XPSTR("Value"), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, pvar->value, attr|PREC1 ) ;
#ifdef TOUCH
				checkTouchEnterEdit( selected ) ;
#endif
				if(attr)
				{
					pvar->value = checkIncDec16( pvar->value, -2000, 2000, EE_MODEL ) ;
  	 		}
			break ;
			case 1 :
				drawItem( (char *)XPSTR("Min"), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, pvar->min, attr|PREC1 ) ;
#ifdef TOUCH
				checkTouchEnterEdit( selected ) ;
#endif
				if(attr)
				{
					pvar->min = checkIncDec16( pvar->min, -2000, 2000, EE_MODEL ) ;
  	 		}
			break ;
			case 2 :
				drawItem( (char *)XPSTR("Max"), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, pvar->max, attr|PREC1 ) ;
#ifdef TOUCH
				checkTouchEnterEdit( selected ) ;
#endif
				if(attr)
				{
					pvar->max = checkIncDec16( pvar->max, -2000, 2000, EE_MODEL ) ;
  	 		}
			break ;
			case 3 :
				drawItem( (char *)XPSTR("Name"), y, attr ) ;
#ifdef TOUCH
				checkTouchEnterEdit( selected ) ;
#endif
				saveEditColours( attr, colour ) ;
				alphaEditName( TRIGHT-6*FW-TRMARGIN, y+TVOFF, (uint8_t *)pvar->name, sizeof(pvar->name), attr|ALPHA_NO_NAME, (uint8_t *)0 ) ;
				restoreEditColours() ;
			break ;
			default :
				if ( attr )
				{
#ifdef TOUCH
					if ( handleSelectIcon() || selected || (event == EVT_KEY_BREAK(BTN_RE) ) )
#else
	if ( event == EVT_KEY_BREAK(BTN_RE) )
#endif
					{
						process = 1 ;
					}
				}

				if ( j < options )
				{
					struct t_valVarPack *pvalue ;
					pvalue = getValueAddress( pvar, j ) ;

					drawItem( (char *)XPSTR("Value"), y, attr ) ;

					PUTS_AT_IDX( 8*FW, y+TVOFF/2, StringValueCategory, pvalue->category, 0 ) ;
					
					saveEditColours( attr, colour ) ;
					displayValueItem( TRIGHT-6*FW-TRMARGIN, y+TVOFF/2, pvalue, 0 ) ;
 					
	 				if ( pvalue->valIsSource )
					{
						displayVarSource( TRIGHT-4*FW-TRMARGIN, y+TVOFF/2, pvalue, 0 ) ;
					}
					else
					{
						PUTS_NUM( TRIGHT-TRMARGIN, y+TVOFF/2, pvalue->value, PREC1) ;
					}
					restoreEditColours() ;

					if (sub==k)
					{
						if ( !PopupData.PopupActive )
						{
							if ( process )
							{
  	   					killEvents(event) ;
								s_editMode = 0 ;
								PopPvar = pvar ;
								PopOpts = j ;
								PopActs = 0xFF ;
								PopupData.PopupIdx = 0 ;
								PopupData.PopupActive = 1 ;
								event = 0 ;		// Kill this off
							}
						}
					}
					j += 1 ;
				}
				else if ( ( j == options) && (options < MAX_VALS) )
				{
					drawItem( (char *)XPSTR("Add"), y, attr ) ;
					drawText( TRIGHT-TRMARGIN, y, XPSTR("Value"), attr|LUA_RIGHT ) ;
					if (sub==k)
					{
						if ( !PopupData.PopupActive )
						{
							if ( process )
							{
 	   						killEvents(event) ;
								s_editMode = 0 ;
								insertValue( pvar, options ) ;
							}
						}
					}
					j += 1 ;
				}
				else
				{
					if ( m < actions )
					{
						struct t_actVarPack	*pAction ;
						struct t_valVarPack *pvalue ;
						pvalue = getValueAddress( pvar, options ) ;
						pAction = (struct t_actVarPack *)	pvalue ;
						pAction += m ;

						drawItem( (char *)XPSTR("Action"), y, attr ) ;

						PUTS_AT_IDX( 8*FW, y+TVOFF/2, StringActionCategory, pAction->category, 0 ) ;
						saveEditColours( attr, colour ) ;
						displayActionItem( TRIGHT-6*FW-TRMARGIN, y+TVOFF/2, pAction, 0 ) ;
						PUTS_AT_IDX( TRIGHT-5*FW-TRMARGIN, y+TVOFF/2, XPSTR("\001=+-*/v^T"), pAction->function, 0 ) ;
						restoreEditColours() ;
						int32_t lvalue = pAction->value ; 
						if ( (pAction->function == 7) )
						{
							lvalue &= 0x01FF ;
						}
						drawNumber( TRIGHT-TRMARGIN, y, lvalue, attr|PREC1 ) ;

//						lcd_putsAtt( 0, y, XPSTR("Action"), attr ) ;
//						PUTC_ATT( 7*FW, y, m+'1', attr ) ;
						if (sub==k)
						{
							if ( !PopupData.PopupActive )
							{
								if ( process )
								{
 	   							killEvents(event) ;
									s_editMode = 0 ;
									PopPvar = pvar ;
									PopOpts = options ;
									PopActs = m ;
									PopupData.PopupIdx = 0 ;
									PopupData.PopupActive = 1 ;
									event = 0 ;		// Kill this off
								}
							}
						}
						m += 1 ;
					}
					else if ( ( m == actions) && (actions < MAX_ACTS) )
					{
						drawItem( (char *)XPSTR("Add"), y, attr ) ;
						drawText( TRIGHT-TRMARGIN, y, XPSTR("Action"), attr|LUA_RIGHT ) ;
						if (sub==k)
						{
							if ( !PopupData.PopupActive )
							{
								if ( process )
								{
 	   							killEvents(event) ;
									s_editMode = 0 ;
									insertAction( pvar, actions, options ) ;
									actions += 1 ;
								}
							}
						}
						m += 1 ;
					}
				}
			break ;
		}
//		if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
	}

	if ( PopupData.PopupActive )
	{
		Tevent = event ;
		varpopup( event ) ;
    s_editMode = false;
	}
}

void menuVars(uint8_t event)
{
	TITLE(XPSTR("Vars "));
	EditType = EE_MODEL ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	uint32_t rows = NUM_VARS ;
	static MState2 mstate2 ;
	mstate2.check_columns( event, rows-1 ) ;
	static uint8_t savedOffset ;

	if ( event == EVT_ENTRY_UP )
	{
		s_pgOfs = savedOffset ;
	}

	uint32_t sub = mstate2.m_posVert ;
	coord_t y ;
	
#ifdef TOUCH
	uint32_t selected = 0 ;
	uint32_t newVpos ;
#endif
	uint32_t k = 0 ;
  uint32_t t_pgOfs ;
	t_pgOfs = evalHresOffset( sub ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint32_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	selected = newVert & 0x0100 ;
#endif

#ifdef TOUCH
	if ( handleSelectIcon() || selected || ( event == EVT_KEY_BREAK(BTN_RE) ) )
#else
	if ( event == EVT_KEY_BREAK(BTN_RE) )
#endif
	{
		s_currIdx = sub ;
		killEvents(event);
		s_editMode = false ;
		pushMenu(menuOneVar) ;
  }

#ifdef TOUCH
	if ( rows > TLINES )
	{
		newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
		if ( newVpos != t_pgOfs )
		{
			s_pgOfs = t_pgOfs = newVpos ;
			if ( sub < t_pgOfs )
			{
				mstate2.m_posVert = sub = t_pgOfs ;
			}
			else if ( sub > t_pgOfs + TLINES - 1 )
			{
				mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
			}
		}
	}
#endif
	lcd_hline( 0, TTOP, TRIGHT ) ;

	savedOffset = t_pgOfs ;

	uint32_t lines = rows > TLINES ? TLINES : rows ;
	for( uint32_t i = 0 ; i < lines ; i += 1 )
	{
    y = (i)*TFH + TTOP ;
    k = i + t_pgOfs ;
		uint16_t attr = ( (sub==k) ? INVERS : 0 ) ;

		uint16_t oldFcolour = LcdForeground ;
		
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, (y)*TSCALE+2, (TRIGHT)*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdForeground = ~LcdForeground ;
		}
		 
		displayVarName( THOFF, y+TVOFF/2, k, 0 ) ;
		PUTS_NUM( TRIGHT-TRMARGIN, y+TVOFF/2, VarValues[k], PREC1) ;
		
		LcdForeground = oldFcolour ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
	}
}


#endif

extern int16_t getVarSourceValue( uint8_t src ) ;

int16_t getVarSingleValue( struct t_valVarPack *pVal )
{
	int16_t value ;
	if ( pVal->valIsSource )
	{
		if ( pVal->value )
		{
			value = getVarSourceValue( pVal->value ) ;
		}
		else
		{
			return 0 ;
		}
	}
	else
	{
		value = pVal->value ;
	}
	return value ;
}

// Needs to process vals
int16_t getVarValue( struct t_varPack *pvar, uint8_t event )
{
	uint32_t i ;
	int16_t value = pvar->value ;
	uint32_t options ;
	int32_t item ;
	struct t_valVarPack *pvalue ;
	struct t_valVarPack *pCurrentvalue = 0 ;

	options = pvar->numOpt ;
	
	pvalue = (struct t_valVarPack *) (pvar+1) ;	// First value
	struct t_actVarPack	*pAction ;
	pAction = (struct t_actVarPack *)	(pvalue + options) ;
	for ( i = 0 ; i < options ; i += 1 )
	{
		item = pvalue->item ;
		switch ( pvalue->category )
		{
			case 0 :	// Always On
				value = getVarSingleValue( pvalue ) ;
				i = options ;
				pCurrentvalue = pvalue ;
			break ;
			case 1 :	// Switch
				if ( item )
				{
					if ( getSwitch00( item ) )
					{
						value = getVarSingleValue( pvalue ) ;
						i = options ;
						pCurrentvalue = pvalue ;
					}
				}
			break ;		// Flight Mode
			case 2 :
				if ( CurrentPhase == item )
				{
					value = getVarSingleValue( pvalue ) ;
					i = options ;
					pCurrentvalue = pvalue ;
				}
			break ;
		}
		pvalue += 1 ;
	}
	
// We have the raw value, now process actions

	uint32_t actionOn ;
	uint32_t actionDone = 0 ;
	for ( i = 0 ; i < pvar->numAct ; i += 1 )
	{
		actionOn = 0 ;
		if ( pAction->function <= 7 )
		{
			switch ( pAction->category )
			{
				case 0 :
					actionOn = 1 ;
				break ;
				case 1 :
					if ( pAction->swPosition )
					{
						if ( VarPreScaler == VAR_PRE_SCALER )
						{
							if ( pAction->timer )
							{
								if ( --pAction->timer == 0 )
								{
									actionOn = 1 ;
//									if ( pAction->tFlag )
									{
//										pAction->tFlag = 0 ;
										pAction->timer = 3 ;
									}
								}
							}
						}
					}
					else
					{
						pAction->timer = 0 ;
					}
					if ( getSwitch00( pAction->item ) )
					{
						if ( pAction->swPosition == 0 )
						{
							actionOn = 1 ;
//							pAction->tFlag = 1 ;
							pAction->timer = 7 ;
						}
						pAction->swPosition = 1 ;
						if ( pAction->function == 7 )
						{
							actionOn = 1 ;
						}
					}
					else
					{
						pAction->swPosition = 0 ;
					}
				break ;
			}
			if ( actionOn )
			{
				actionDone = 1 ;
				switch ( pAction->function )
				{
					case 0 :	// =
						value = pAction->value ;
					break ;
					case 1 :	// +
						value += pAction->value ;
					break ;
					case 2 :	// -
						value -= pAction->value ;
					break ;
					case 3 :	// *
						value *= pAction->value ;
						value /= 10 ;
					break ;
					case 4 :	// /
						value *= 10 ;
						value /= pAction->value ;
					break ;
					case 5 :
						value = (value < pAction->value) ? value : pAction->value ;
					break ;
					case 6 :
						value = (value > pAction->value) ? value : pAction->value ;
					break ;
					case 7 :
					{	
						// Handle repurposed trims (event)
						// pAction->value (top 2 bits) is 0-3 for LHLVRVRH
						uint32_t lindex = (pAction->value >> 9) & 0x03 ;
						CopyUsestrim |= 1 << lindex ;		// Flag trim used
  					uint8_t k = (event & EVT_KEY_MASK) - TRM_BASE ;
						if ( ( k / 2 ) == lindex )
						{
							if ( !IS_KEY_BREAK(event)) // && (event & _MSK_KEY_REPT))
							{
								int32_t lvalue = pAction->value & 0x01FF ;
				  	  	value = (k&1) ? value + lvalue : value - lvalue ;   // positive = k&1
  	  	  			audioDefevent(AU_TRIM_MOVE) ;
							}
						}
					}	
					break ;
				}
			}
		}
		pAction += 1 ;	 
	}
	if ( value > pvar->max )
	{
		value = pvar->max ;
	}
	if ( value < pvar->min )
	{
		value = pvar->min ;
	}
	if ( actionDone )
	{
		if ( pCurrentvalue )
		{
			if ( pCurrentvalue->valIsSource == 0 )
			{
				pCurrentvalue->value = value ;
			}
		}
		else
		{
			pvar->value = value ;
		}
	}
	return value ;
}

int16_t getVarValue( int32_t index )
{
	uint32_t negative = 0 ;
	int16_t value ;
	if ( index < 0 )
	{
		index = -index - 1 ;
		negative = 1 ;
	}
	value = VarValues[index] ;
	return negative ? -value : value ;
}

void initVars()
{
	buildVarOffsetTable() ;
	// Also need to note Action switch positions
	uint32_t i ;
	uint32_t j ;
	struct t_varPack *pvar ;
	struct t_valVarPack *pvalue ;
	struct t_actVarPack	*pAction ;
	for ( i = 0 ; i < NUM_VARS ; i += 1 )
	{
		pvar = getVarAddress( i ) ;
		if ( pvar->numAct )
		{
			pvalue = (struct t_valVarPack *) (pvar+1) ;	// First value
			pAction = (struct t_actVarPack *)	(pvalue + pvar->numOpt) ;
			for ( j = 0 ; j < pvar->numAct ; j += 1 )
			{
				if ( pAction->category == 1 )		// A switch
				{
					pAction->swPosition = getSwitch00( pAction->item ) ;
				}
				pAction->timer = 0 ;
				pAction->tFlag = 0 ;
			}
		}
	}
}

void processVars( uint8_t event )
{
	uint32_t i ;

	if ( LockVars )
	{
		return ;
	}

	if ( --VarPreScaler == 0 )
	{
		VarPreScaler = VAR_PRE_SCALER ;
	}
	CopyUsestrim = 0 ;

 	for( i = 0 ; i < NUM_VARS ; i += 1 )
	{
		VarValues[i] = getVarValue( getVarAddress( i ), event ) ;
	}
	VarUsesTrim = CopyUsestrim ;

}

// Value in is 1000 + var (positive or negative if var)
// If varLimit is negative, then var select is 0 - abs(varLimit)-1
int16_t editVarCapableValue( uint16_t x, uint16_t y, int16_t value, int16_t min, int16_t max, int16_t varLimit, uint32_t attr, uint8_t event )
{
	int16_t lowVarLimit ;
  uint8_t invers = attr&(INVERS|BLINK) ;
#if defined(PCBX12D) || defined(PCBX10)
//	x *= HVSCALE ;
//	y *= HVSCALE ;
#endif
	if ( value > 900 )	// Is a var
	{
		value -= 1000 ;
#if defined(PCBX12D) || defined(PCBX10)
		displayVarName( x-FW, y, value, (attr & ~SMLSIZE)|LUA_RIGHT ) ;
#else
 #ifdef PROP_TEXT
		uint16_t x1 = x+FW ;
		if ( ( attr & SMLSIZE ) == SMLSIZE )
		{
			x += 3*FW ;
		}
		displayVarName( x1, y, value, attr|LUA_RIGHT ) ;
 #else		
		uint16_t x1 = x+FW ;
		if ( attr & LUA_SMLSIZE )
		{
			x += 3*FW ;
		}
		displayVarName( x1, y, value, attr|LUA_RIGHT ) ;
 #endif
#endif
		if ( invers )
		{
			if ( event == EVT_TOGGLE_GVAR )
			{
				value = VarValues[value] / 10 ;
				return value ;
			}
			lowVarLimit = -varLimit ;
			if ( varLimit < 0 )
			{
				lowVarLimit = 0 ;
				varLimit = -varLimit ;
			}
			value = checkIncDec16( value, lowVarLimit, varLimit-1, EE_MODEL) ;
		}
		value += 1000 ;
	}
	// Numeric	
	else
	{
 #if defined(PCBX12D) || defined(PCBX10) || defined(TOUCH)
//		lcd_outdezAtt( x-FW, y, value, 0 ) ;
		PUTS_NUM( x-FW, y, value, 0 ) ;
 #else
  #if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
		PUTS_NUM( x, y, value, (attr & ~LUA_SMLSIZE) ) ;
	#else
		PUTS_NUM( x, y, value, attr & ~LUA_SMLSIZE ) ;
	#endif
  #if defined(PCBX12D) || defined(PCBX10)
  #else
   #ifdef PROP_TEXT
		if ( ( attr & SMLSIZE ) == SMLSIZE )
	 #else
		if ( attr & LUA_SMLSIZE )
	 #endif
		{
			x += 3*FW ;
		}
  #endif
 #endif
		if ( invers )
		{
			if ( event == EVT_TOGGLE_GVAR )
			{
				return 1000 ;
			}
			value = checkIncDec16( value, min, max, EE_MODEL) ;
		}
	}
#ifndef TOUCH
 #if defined(PCBX12D) || defined(PCBX10)
//	PUTC(x-6*FW, y-2, 'v' ) ;
 #else	
	lcd_plot( x-7*FW, y ) ;
	lcd_plot( x-7*FW+2, y ) ;
	lcd_plot( x-7*FW+1, y+1 ) ;
 #endif
#endif
	return value ;
}

int16_t editVarCapable100Value( uint16_t x, uint16_t y, int16_t value, uint32_t attr, uint8_t event )
{
	if ( ( value > 100 ) && ( value <= 125 ) )
	{
		value += 899 ; // gives 1000 to 1024
	}
	else if ( ( value < -100 ) && ( value >= -125 ) )
	{
		value += 1100 ;	// gives 999 to 975
	}
#if defined(PCBX12D) || defined(PCBX10) || defined(TOUCH)
	x -= FW*2/3 ;
#endif				 
	value = editVarCapableValue( x, y, value, -100, 100, NUM_VAR25, attr | LUA_SMLSIZE, event ) ;
	if ( value > 900 )
	{
		if ( value < 1000 )
		{
			value -= 1100 ;
		}
		else
		{
			value -= 899 ;
		}
	}
	return value ;
	
}



int32_t getVarValWholeL100( int32_t index )
{
	index = getVarValue(index) / 10 ;
	if ( index > 100 )
	{
		index = 100 ;
	}
	else if ( index < -100 )
	{
		index = -100 ;
	}
	return index ;
}



#endif // USE_VARS

