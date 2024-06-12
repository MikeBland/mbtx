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

#include "ersky9x.h"
#include "myeeprom.h"
#include "gvars.h"

#define TWELVE_GVARS		1

extern uint8_t CurrentPhase ;

SKYModelData  g_model __attribute__ ((aligned (4))) ;

#if MULTI_GVARS

uint32_t getGVarFlightMode(uint32_t fm, uint32_t gvidx) // TODO change params order to be consistent!
{
	
	if ( gvidx >= 12 )
	{
		return 0 ;
	}
	
	
  for (uint32_t i = 0 ; i < MAX_MODES+EXTRA_MODES+1 ; i += 1 )
  {
    if ( fm == 0 )
		{
			return 0 ;
		}
    int16_t val = readMgvar( fm, gvidx ) ;
    if (val <= GVAR_MAX)
		{
			return fm ;
		}
    uint32_t result = val - GVAR_MAX - 1 ;
    if (result >= fm)
		{
			result += 1 ;
		}
    fm = result ;
  }
  return 0 ;
}

int16_t getGvar( int32_t gv )
{
	int32_t mul = 1 ;
	if ( gv < 0 )
	{
		gv = -1 - gv ;
		mul = -1 ;
	}

	if ( g_model.flightModeGvars == 0 )
	{
		return (gv < 7) ? g_model.gvars[gv].gvar * mul : 0 ;
	}
	
	if ( gv >= 12 )
	{
		return 0 ;
	}
	return readMgvar( getGVarFlightMode(CurrentPhase, gv), gv ) * mul ;
}

int16_t getGvarFm( int32_t gv, uint32_t fm )
{
	int32_t mul = 1 ;
	if ( gv < 0 )
	{
		gv = -1 - gv ;
		mul = -1 ;
	}

	if ( g_model.flightModeGvars == 0 )
	{
		return (gv < 7) ? g_model.gvars[gv].gvar * mul : 0 ;
	}
	
	if ( gv >= 12 )
	{
		return 0 ;
	}
	
	return readMgvar( getGVarFlightMode(fm,gv), gv ) * mul ;
}

void setGVar(uint32_t gvidx, int16_t value )
{
	if ( g_model.flightModeGvars == 0 )
	{
		if ( gvidx < MAX_GVARS )
		{
			g_model.gvars[gvidx].gvar = value ;
		}
	}
	else
	{
	
		if ( gvidx >= 12 )
		{
			return ;
		}
		
		setGVarValue( gvidx, value, CurrentPhase ) ;
	}
}

void setGVarFm(uint32_t gvidx, int16_t value, uint32_t fm )
{
	if ( g_model.flightModeGvars == 0 )
	{
		if ( gvidx < MAX_GVARS )
		{
			g_model.gvars[gvidx].gvar = value ;
		}
	}
	else
	{
		if ( gvidx >= 12 )
		{
			return ;
		}
		setGVarValue( gvidx, value, fm ) ;
	}
}

//#define GVAR_VALUE(gv, fm)           g_model.flightModeData[fm].gvars[gv]
//SET_GVAR_VALUE(idx, phase, value)
//{
//	GVAR_VALUE(idx, phase) = value;
//	storageDirty(EE_MODEL);
//	if (g_model.gvars[idx].popup)
//	{
//	  gvarLastChanged = idx;
//	  gvarDisplayTimer = GVAR_DISPLAY_TIME;
//	}
//}

void setGVarValue(uint32_t gvidx, int16_t value, uint32_t fm)
{
	if ( g_model.flightModeGvars == 0 )
	{
		if ( gvidx < MAX_GVARS )
		{
			g_model.gvars[gvidx].gvar = value ;
		}
		(void) fm ;	// Not used
	}
	else
	{
		if ( gvidx >= 12 )
		{
			return ;
		}
		
	  fm = getGVarFlightMode(fm, gvidx) ;
		if ( readMgvar( fm, gvidx ) != value )
		{
			writeMgvar( fm, gvidx, value ) ;
//	if (g_model.xgvars[gvidx].popup)
//	{
//	  gvarLastChanged = gvidx;
//	  gvarDisplayTimer = GVAR_DISPLAY_TIME;
//	}
		}
	}
}

int32_t getGVarValuePrec1(int32_t gv, uint32_t fm)
{
  int8_t idx = (gv >= 0 ? gv : -gv - 1) ;
  int8_t mul = (g_model.xgvars[idx].prec == 0 ? 10 : 1) ; // explicit cast to `int` needed, othervise gv is promoted to double!
  if (gv < 0)
	{
    mul = -mul ;
  }
	
	if ( gv >= 12 )
	{
		return 0 ;
	}

	return readMgvar( getGVarFlightMode(fm,idx), idx ) * mul ;
}

//#define GV_IS_GV_VALUE(x,min,max)    ((max>GV1_SMALL || min<-GV1_SMALL) ? (x>GV_RANGELARGE || x<GV_RANGELARGE_NEG) : (x>max) || (x<min))
//int16_t getGVarFieldValue(int16_t val, int16_t min, int16_t max, int8_t fm)
//{
//  if (GV_IS_GV_VALUE(val, min, max))
//  {
//    int8_t gv = GV_INDEX_CALCULATION(val, max);
//    val = getGVarValue(gv, fm);
//  }
//  return limit(min, val, max);
//}


int16_t readMgvar( uint32_t fmidx, uint32_t gvidx )
{
#ifdef TWELVE_GVARS
	int16_t value ;
	uint32_t index = (gvidx >> 1) * 3 ;
	uint8_t *p = (uint8_t *) g_model.mGvars[fmidx] ;
	if ( gvidx & 1 )
	{
		value = *(p+index+2) | ( ( *(p+index+1) & 0xF0) << 4 ) ;
	}
	else
	{
		value = *(p+index) | ( ( *(p+index+1) & 0x0F) << 8 ) ;
	}
	value <<= 4 ;
	value >>= 4 ;
	return value ;
#else	
	if ( gvidx >= 9 )
	{
		return 0 ;
	}
	
	return g_model.mGvars[fmidx][gvidx] ;
#endif	
}

void writeMgvar( uint32_t fmidx, uint32_t gvidx, int16_t value )
{
#ifdef TWELVE_GVARS
	uint8_t *p = (uint8_t *) g_model.mGvars[fmidx] ;
	uint32_t index = (gvidx >> 1) * 3 ;
	if ( gvidx & 1 )
	{
		uint8_t temp = *(p+index+1) ;
		temp &= 0x0F ;
		temp |= (value >> 4) & 0xF0 ;
		*(p+index+1) = temp ;
		*(p+index+2) = value ;
	}
	else
	{
		uint8_t temp = *(p+index+1) ;
		temp &= 0xF0 ;
		temp |= (value >> 8) & 0x0F ;
		*(p+index+1) = temp ;
		*(p+index) = value ;
	}
#else

	if ( gvidx >= 9 )
	{
		return ;
	}
	
	g_model.mGvars[fmidx][gvidx] = value ;
#endif 
}


void initFmGvars()
{
  for ( uint32_t fmIdx = 0 ; fmIdx < MAX_MODES+EXTRA_MODES+1 ; fmIdx += 1 )
	{
    for ( uint32_t gvarIdx = 0 ; gvarIdx < 12 ; gvarIdx += 1)
		{
			writeMgvar( fmIdx, gvarIdx, (fmIdx == 0) ? 0 : GVAR_MAX + 1 ) ;
//      g_model.mGvars[fmIdx][gvarIdx] = GVAR_MAX + 1;
    }
  }
	uint8_t *p = (uint8_t*)&g_model.gvars ;
  for ( uint32_t gvarIdx = 0 ; gvarIdx < 12 ; gvarIdx += 1)
	{
		*p++ = 0 ;
	}
}

//int16_t getGVarValue(int8_t gv, int8_t fm)
//{
//  int8_t mul = 1;
//  if (gv < 0) {
//    gv = -1-gv;
//    mul = -1;
//  }
//  return GVAR_VALUE(gv, getGVarFlightMode(fm, gv)) * mul;
//	//g_model.flightModeData[fm].gvars[gv]
//}

#else	// MULTI_GVARS

int16_t getGvar( int32_t gv )
{
	int32_t mul = 1 ;
	if ( gv < 0 )
	{
		gv = -1 - gv ;
		mul = -1 ;
	}
	return gv < MAX_GVARS ? g_model.gvars[gv].gvar * mul : 0 ;
}

void setGVar(uint32_t gv, int16_t value )
{
	if ( gv < MAX_GVARS )
	{
		g_model.gvars[gv].gvar = value ;
	}
}



#endif // MULTI_GVARS



