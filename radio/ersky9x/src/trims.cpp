/****************************************************************************
*  Copyright (c) 2024 by Michael Blandford. All rights reserved.
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

#include <stdlib.h>

#include "ersky9x.h"
#include "myeeprom.h"
#include "trims.h"
#include "audio.h"
#include "menus.h"
#include "drivers.h"
#ifdef USE_VARS
#include "vars.h"
#endif

uint32_t getFlightPhase()
{
	uint32_t i ;
  for ( i = 0 ; i < MAX_MODES+1 ; i += 1 )
	{
  	PhaseData *phase ;
		phase = (i < MAX_MODES) ? &g_model.phaseData[i] : &g_model.xphaseData ;
    if ( phase->swtch )
		{
    	if ( getSwitch00( phase->swtch ) )
			{
    		if ( phase->swtch2 )
				{
					if ( getSwitch00( phase->swtch2 ) )
					{
						return i + 1 ;
					}
				}
				else
				{
					return i + 1 ;
				}
    	}
		}
		else
		{
    	if ( phase->swtch2 && getSwitch00( phase->swtch2 ) )
			{
    	  return i + 1 ;
    	}
		}
  }
  return 0 ;
}

int16_t getRawTrimValue( uint8_t phase, uint8_t idx )
{
	if ( phase )
	{
	  PhaseData *p ;
		phase -= 1 ;
		p = (phase < MAX_MODES) ? &g_model.phaseData[phase] : &g_model.xphaseData ;
		return p->trim[idx].value ;
	}	
	else
	{
		return g_model.trim[idx] ;
	}
}



t_trim rawTrimFix( uint8_t phase, t_trim v )
{
	if ( phase )
	{
		int16_t tm ;
		phase -= 1 ;
		tm = v.mode ;
		if ( tm < 0 )
		{
			tm = 0 ;
		}
		if ( tm == 0 )
		{
      if (v.value > TRIM_EXTENDED_MAX)
			{
				tm = v.value - (TRIM_EXTENDED_MAX+1) ;
				if ( tm > phase )
				{
					tm += 1 ;
				}
			}
			else
			{
				tm = phase+1 ;
			}
			v.mode = tm << 1 ;
		}
	}	
	else
	{
		v.mode = 0 ;
		v.value = 0 ;
	}
	return v ;
}

t_trim getRawTrimComplete( uint32_t phase, uint32_t idx )
{
	t_trim v ;
	if ( phase )
	{
	  PhaseData *p ;
		phase -= 1 ;
		p = (phase < MAX_MODES) ? &g_model.phaseData[phase] : &g_model.xphaseData ;
		v = p->trim[idx] ;
		return rawTrimFix( phase+1, v ) ;
	}	
	else
	{
		v.mode = 0 ;
		v.value = g_model.trim[idx] ;
		return v ;
	}
}


int16_t getTrimValueAdd( uint32_t phase, uint32_t idx )
{
  int16_t result = 0 ;
  for ( uint32_t i = 0 ; i<MAX_MODES+1 ; i += 1 )
	{
    t_trim v = getRawTrimComplete( phase, idx ) ;
		{
      uint32_t p = v.mode >> 1 ;
      if ( p == phase || phase == 0 )
			{
				result += v.value ;
        return limit( (int16_t)-125, result, (int16_t)125 ) ;
      }
      else
			{
        phase = p ;
//        if ( (v.mode & 1) == 0)
        if (v.mode & 1)
				{
          result += v.value ;
        }
      }
    }
  }
  return 0 ;
}

void setTrimValueAdd(uint32_t phase, uint32_t idx, int16_t trim)
{
	PhaseData *ph = 0 ;
	t_trim tempV ;
	t_trim *pv ;

  for ( uint32_t i = 0 ; i < MAX_MODES+1 ; i += 1 )
	{
		if ( phase )
		{
			ph = (phase-1 < MAX_MODES) ? &g_model.phaseData[phase-1] : &g_model.xphaseData ;
		}
		else
		{
			ph = 0 ;
		}
		if ( ph )
		{
			pv = &ph->trim[idx] ;
		}
		else
		{
			tempV.value = g_model.trim[idx] ;
			tempV.mode = 0 ;
			pv = &tempV ;
		}

		if ( phase )
		{
			int16_t tm ;
			t_trim v ;
		
			v = *pv ;
			tm = v.mode ;
			if ( tm < 0 )
			{
				tm = 0 ;
			}
	
			if ( tm == 0 )
			{
	      if (v.value > TRIM_EXTENDED_MAX)
				{
					tm = v.value - (TRIM_EXTENDED_MAX+1) ;
					v.value = 0 ;
					if ( tm > (int16_t)phase-1 )
					{
						tm += 1 ;
					}
				}
				else
				{
					tm = phase ;
				}
				v.mode = tm << 1 ;
				*pv = v ;
			}
		}	

    uint32_t p = pv->mode >> 1 ;
    if (p == phase || phase == 0)
		{
      pv->value = trim ;
      break ;
    }
    else if ((pv->mode & 1) == 0)
		{
      phase = p ;
    }
    else
		{
      pv->value = limit<int>( -125, trim - getTrimValueAdd(p, idx), 125 ) ;
      break ;
    }
  }
	if ( pv == &tempV )
	{
		g_model.trim[idx] = tempV.value ;
	}
	STORE_MODELVARS_TRIM ;
}

uint8_t checkTrim(uint8_t event)
{
  int8_t  k = (event & EVT_KEY_MASK) - TRM_BASE;
  int8_t  s = g_model.trimInc;
  
		if ( s == 4 )
		{
			s = 8 ;			  // 1=>1  2=>2  3=>4  4=>8
		}
		else
		{
			if ( s == 3 )
			{
				s = 4 ;			  // 1=>1  2=>2  3=>4  4=>8
			}
		}

  if( (k>=0) && (k<8) )
	{
		if ( !IS_KEY_BREAK(event)) // && (event & _MSK_KEY_REPT))
  	{
//			TrimBits |= (1 << k ) ;
			
  	  //LH_DWN LH_UP LV_DWN LV_UP RV_DWN RV_UP RH_DWN RH_UP
  	  uint8_t idx = k/2;
		
	// SORT idx for stickmode if FIX_MODE on
			idx = stickScramble[g_eeGeneral.stickMode*4+idx] ;
			uint8_t ct = g_eeGeneral.crosstrim + ( g_eeGeneral.xcrosstrim << 1 ) ;
			if ( ct )
			{
				idx = 3 - idx ;			
			}
      if ( ct == 2 ) // Vintage style crosstrim
      {
        if (idx == 0 || idx == 3)       // swap back LH/RH trims
				{
					idx = 3 - idx;
				}
      }
#ifdef SMALL
			if ( TrimInUse[idx] )
#else
			if ( TrimInUse[idx] && ((VarUsesTrim & (1 << idx)) == 0) )
#endif
			{
				uint32_t phaseNo = getTrimFlightPhase( CurrentPhase, idx ) ;
  	  	int16_t tm = getTrimValueAdd( phaseNo, idx ) ;
  	  	int8_t  v = (s==0) ? (abs(tm)/4)+1 : s;
  	  	bool thrChan = (2 == idx) ;
				bool thro = (thrChan && (g_model.thrTrim));
  	  	if(thro) v = 2; // if throttle trim and trim trottle then step=2

				if ( GvarSource[idx] )
				{
					v = 1 ;
				}

  	  	if(thrChan && throttleReversed()) v = -v;  // throttle reversed = trim reversed
  	  	int16_t x = (k&1) ? tm + v : tm - v;   // positive = k&1

  	  	if(((x==0)  ||  ((x>=0) != (tm>=0))) && (!thro) && (tm!=0))
				{
					setTrimValueAdd( phaseNo, idx, 0 ) ;
  	  	  killEvents(event);
  	  	  audioDefevent(AU_TRIM_MIDDLE);
  	  	}
				else if(x>-125 && x<125)
				{
					setTrimValueAdd( phaseNo, idx, x ) ;
					if(x <= 125 && x >= -125)
					{
						audio.event(AU_TRIM_MOVE,(abs(x)/4)+60);
					}	
  	  	}
  	  	else
  	  	{
					setTrimValueAdd( phaseNo, idx, (x>0) ? 125 : -125 ) ;
					if(x <= 125 && x >= -125)
					{
						audio.event(AU_TRIM_MOVE,(-abs(x)/4)+60);
					}	
  	  	}
			}
  	  return 0;
  	}
//		else
//		{
//			TrimBits &= ~(1 << k ) ;
//		}
	}
  return event;
}

