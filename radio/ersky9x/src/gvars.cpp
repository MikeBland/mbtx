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


#if MULTI_GVARS

uint32_t getGVarFlightMode(uint32_t fm, uint32_t gv) // TODO change params order to be consistent!
{
  for (uint32_t i = 0 ; i < MAX_MODES+EXTRA_MODES ; i += 1 )
  {
    if ( fm == 0 )
		{
			return 0 ;
		}
    int16_t val = g_model.mGvars[fm][gv] ;
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
	return 0 ;
}

void setGVar(uint32_t gv, int16_t value )
{
	if ( g_model.flightModeGvars == 0 )
	{
		if ( gv < MAX_GVARS )
		{
			g_model.gvars[gv].gvar = value ;
		}
	}
	else
	{
		
	}
}

void initFmGvars()
{
  for ( uint32_t fmIdx = 1 ; fmIdx < MAX_MODES+EXTRA_MODES+1 ; fmIdx += 1 )
	{
    for ( uint32_t gvarIdx = 0 ; gvarIdx < MAX_GVARS+EXTRA_GVARS ; gvarIdx += 1)
		{
      g_model.mGvars[fmIdx][gvarIdx] = GVAR_MAX + 1;
    }
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



