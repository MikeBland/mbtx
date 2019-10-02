/*
 * Author - Mike Blandford
 *
 * Based on er9x by Erez Raviv <erezraviv@gmail.com>
 *
 * Based on th9x -> http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ersky9x.h"
#include "myeeprom.h"
#include "mixer.h"
#include "menus.h"
#include "audio.h"
#include "logicio.h"
#include "sound.h"

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
#ifdef REV9E
const uint8_t switchIndex[18] = { HSW_SA0, HSW_SB0, HSW_SC0, HSW_SD0, HSW_SE0, HSW_SF2, HSW_SG0, HSW_SH2, HSW_SI0, 
																 HSW_SJ0, HSW_SK0, HSW_SL0, HSW_SM0, HSW_SN0, HSW_SO0, HSW_SP0, HSW_SQ0, HSW_SR0, } ;
#else
const uint8_t switchIndex[8] = { HSW_SA0, HSW_SB0, HSW_SC0, HSW_SD0, HSW_SE0, HSW_SF2, HSW_SG0, HSW_SH2 } ;
#endif	// REV9E
#endif

//#define LATENCY 1

extern SKYMixData *mixAddress( uint32_t index ) ;

// static variables used in perOut - moved here so they don't interfere with the stack
// It's also easier to initialize them here.
int16_t  anas [NUM_SKYXCHNRAW+1+MAX_GVARS+1] ;		// To allow for 3POS and THIS and 8 extra PPM inputs
int32_t  chans[NUM_SKYCHNOUT+EXTRA_SKYCHANNELS] = {0};
uint8_t inacPrescale ;
uint16_t inacSum = 0;
uint16_t InacCounter = 0;
uint16_t  bpanaCenter = 0;
uint16_t  sDelay[MAX_SKYMIXERS+EXTRA_SKYMIXERS] = {0};
int32_t  act   [MAX_SKYMIXERS+EXTRA_SKYMIXERS] = {0};
uint8_t  swOn  [MAX_SKYMIXERS+EXTRA_SKYMIXERS] = {0};
uint8_t	CurrentPhase = 0 ;
int16_t rawSticks[4] ;
uint8_t TrimInUse[4] = { 1, 1, 1, 1 } ;
uint8_t ThrottleStickyOn = 0 ;

struct t_fade
{
uint8_t  fadePhases ;
uint16_t fadeRate ;
uint16_t fadeWeight ;
uint16_t fadeScale[MAX_MODES+1] ;
int32_t  fade[NUM_SKYCHNOUT+EXTRA_SKYCHANNELS];
} Fade ;

void trace()   // called in perOut - once every 0.01sec
{
    //value for time described in g_model.tmrMode
    //OFFABSRUsRU%ELsEL%THsTH%ALsAL%P1P1%P2P2%P3P3%
    //now OFFABSTHsTH%
  int16_t val ;
  val = calibratedStick[3-1];

// ***** Handle sticky throttle here
		if ( ThrottleStickyOn )
		{
			val = -RESX ;
		}
    timer(val);

  val = (val+RESX) / (RESX/16); //calibrate it
  
	static uint16_t s_time;
  static uint16_t s_cnt;
  static uint16_t s_sum;
  s_cnt++;
  s_sum+=val;
  if((uint16_t)( get_tmr10ms()-s_time)<1000) //10 sec
      return;
  s_time= get_tmr10ms() ;
 
#ifdef PCBSKY
  if ((g_model.Module[1].protocol==PROTO_DSM2)&&getSwitch00(MAX_SKYDRSWITCH-1) ) audioDefevent(AU_TADA);   //DSM2 bind mode warning
#endif
#if defined(PCBX9D) || defined(PCB9XT)
  if ((g_model.Module[1].protocol==PROTO_DSM2)&&getSwitch00(MAX_SKYDRSWITCH-1) ) audioDefevent(AU_TADA);   //DSM2 bind mode warning
#endif
  val   = s_sum/s_cnt;
  s_sum = 0;
  s_cnt = 0;

  s_traceCnt++;
  s_traceBuf[s_traceWr++] = val;
  if(s_traceWr>=MAXTRACE) s_traceWr=0;
}


uint16_t isqrt32(uint32_t n)
{
  uint16_t c = 0x8000;
  uint16_t g = 0x8000;

  for(;;)
	{
    if((uint32_t)g*g > n)
      g ^= c;
    c >>= 1;
    if(c == 0)
      return g;
    g |= c;
  }
}

static void inactivityCheck()
{
  if(s_noHi) s_noHi--;
  uint16_t tsum = 0;
  for(uint8_t i=0;i<4;i++) tsum += anas[i];
  if(abs(int16_t(tsum-inacSum))>INACTIVITY_THRESHOLD)
	{
		inacSum = tsum;
		InactivityMonitor = 1 ;  // reset in perMain
		InacCounter = 0 ;
  }
  if( (g_eeGeneral.inactivityTimer + 10) && (g_vbat100mV>49))
	{
    if (++inacPrescale > 15 )
    {
      InacCounter += 1 ;
      inacPrescale = 0 ;
  	  if( InacCounter >((uint16_t)(g_eeGeneral.inactivityTimer+10)*(100*60/16)))
      if( ( InacCounter & 0x1F ) == 1 )
			{
				SystemOptions &= ~SYS_OPT_MUTE ;						
				putVoiceQueue( VOLUME_MASK + g_eeGeneral.inactivityVolume + ( NUM_VOL_LEVELS-3 ) ) ;
				voiceSystemNameNumberAudio( SV_INACTV, V_INACTIVE, AU_INACTIVITY ) ;
				putVoiceQueue( 0xFFFF ) ;

      }
    }
  }
}

//#ifdef PCBX9D
//extern uint16_t MixerRunAtTime ;
//void checkMixerNeeded()
//{
//	uint16_t t1 = getTmr2MHz() ;
//	if ( (uint16_t)( t1 - MixerRunAtTime ) > 4095 )
//	{
//		MixerRunAtTime = t1 ;
//		perOutPhase( g_chans512, 0) ;
//	}
//}
//#endif

void perOutPhase( int16_t *chanOut, uint8_t att ) 
{
	static uint8_t lastPhase ;
	uint8_t thisPhase ;
	struct t_fade *pFade ;
	pFade = &Fade ;

//#ifdef PCBX9D
// #ifdef LATENCY
//	GPIOA->ODR |= 0x4000 ;
// #endif	
//#endif

	thisPhase = getFlightPhase() ;
	if ( thisPhase != lastPhase )
	{
		uint8_t time1 = 0 ;
		uint8_t time2 ;
		
		if ( lastPhase )
		{
      time1 = g_model.phaseData[(uint8_t)(lastPhase-1)].fadeOut ;
		}
		if ( thisPhase )
		{
      time2= g_model.phaseData[(uint8_t)(thisPhase-1)].fadeIn ;
			if ( time2 > time1 )
			{
        time1 = time2 ;
			}
		}
		if ( time1 )
		{
			pFade->fadeRate = (25600 / 50) / time1 ;
			pFade->fadePhases |= ( 1 << lastPhase ) | ( 1 << thisPhase ) ;
		}
		lastPhase = thisPhase ;
	}
	att |= FADE_FIRST ;
	if ( pFade->fadePhases )
	{
		pFade->fadeWeight = 0 ;
		uint8_t fadeMask = 1 ;
    for (uint8_t p=0; p<MAX_MODES+1; p++)
		{
			if ( pFade->fadePhases & fadeMask )
			{
				if ( p != thisPhase )
				{
					CurrentPhase = p ;
					pFade->fadeWeight += pFade->fadeScale[p] ;
					perOut( chanOut, att ) ;
					att &= ~FADE_FIRST ;				
				}
			}
			fadeMask <<= 1 ;
		}	
	}
	else
	{
		pFade->fadeScale[thisPhase] = 25600 ;
	}
	pFade->fadeWeight += pFade->fadeScale[thisPhase] ;
	CurrentPhase = thisPhase ;
	perOut( chanOut, att | FADE_LAST ) ;
	
	if ( pFade->fadePhases && tick10ms )
	{
		uint8_t fadeMask = 1 ;
    for (uint8_t p=0; p<MAX_MODES+1; p+=1)
		{
			uint16_t l_fadeScale = pFade->fadeScale[p] ;
			
			if ( pFade->fadePhases & fadeMask )
			{
				uint16_t x = pFade->fadeRate * tick10ms ;
				if ( p != thisPhase )
				{
          if ( l_fadeScale > x )
					{
						l_fadeScale -= x ;
					}
					else
					{
						l_fadeScale = 0 ;
						pFade->fadePhases &= ~fadeMask ;						
					}
				}
				else
				{
          if ( 25600 - l_fadeScale > x )
					{
						l_fadeScale += x ;
					}
					else
					{
						l_fadeScale = 25600 ;
						pFade->fadePhases &= ~fadeMask ;						
					}
				}
			}
			else
			{
				l_fadeScale = 0 ;
			}
			pFade->fadeScale[p] = l_fadeScale ;
			fadeMask <<= 1 ;
		}
	}
//#ifdef PCBX9D
// #ifdef LATENCY
//	GPIOA->ODR &= ~0x4000 ;
// #endif	
//#endif
}



int16_t scaleAnalog( int16_t v, uint8_t channel )
{
	int16_t mid ;
	int16_t neg ;
	int16_t pos ;
	int16_t deadband = ( channel < 4 ) ? g_eeGeneral.stickDeadband[channel] : 0 ;

#ifndef SIMU
	mid = *CalibMid[channel] ;
  pos = *CalibSpanPos[channel] - deadband ;
  neg = *CalibSpanNeg[channel] - deadband ;

	v -= mid ;

	if ( channel < 4 )
	{
		if ( ( v > -deadband) && ( v < deadband ) )
		{
			v = 0 ;
		}
		else
		{
			if ( v > 0 )
			{
				v -= deadband ;
			}
			else
			{
				v += deadband ;
			}
		}
	}
#if defined(REVPLUS) || defined(REV9E)
	else if ( ( channel < 6 ) || ( channel == 8 ) )
	{
		uint8_t chan = channel - 3 ;
		if ( chan == 5 )
		{
			chan = 3 ;
		}
		if ( chan == ( ( g_eeGeneral.analogMapping & MASK_6POS ) >> 2 ) )
		{
			v = ((int32_t)switchPosition( HSW_Ele6pos0 ) * (int32_t)RESX*2 - (int32_t)RESX*5)/5 ;
			return v ;
		}
	}
#else
	else if ( channel < 7 )
	{
		if ( ( channel - 3 ) == ( ( g_eeGeneral.analogMapping & MASK_6POS ) >> 2 ) )
		{
			// Pot mapped as 6-pos switch
			v = ((int32_t)switchPosition( HSW_Ele6pos0 ) * (int32_t)RESX*2 - (int32_t)RESX*5)/5 ;
			return v ;
		}
	}
#endif

//#ifdef REVX
//	if ( channel < 4 )
//	{
//		if ( ( v > -1) && ( v < 1 ) )
//		{
//			v = 0 ;
//		}
//	}
//#endif
	v  =  v * (int32_t)RESX /  (max((int16_t)100,(v>0 ? pos : neg ) ) ) ;
	
#endif // SIMU
	if(v <= -RESX) v = -RESX;
	if(v >=  RESX) v =  RESX;
	if ( throttleReversed() )
	{
		if ( channel == THR_STICK )
		{
			v = -v ;
		}
	}
	return v ;
}

void perOut(int16_t *chanOut, uint8_t att )
{
    int16_t  trimA[4];
    uint16_t  anaCenter = 0;
//    uint16_t d = 0;
		int16_t trainerThrottleValue = 0 ;
		uint8_t trainerThrottleValid = 0 ;
		if ( check_soft_power() == POWER_TRAINER )		// On trainer power
		{
			TrainerChannel *tChan = &g_eeGeneral.trainerProfile[g_model.trainerProfile].channel[0] ;
			
#ifdef PCBX9D
#ifdef PCBX7
			if ( ( tChan->source != TRAINER_SBUS ) && ( tChan->source != TRAINER_CPPM ) && ( tChan->source != TRAINER_BT ) )
#else
			if ( ( tChan->source != TRAINER_SBUS ) && ( tChan->source != TRAINER_CPPM ) )
#endif			
#else
			if ( ( tChan->source != TRAINER_BT ) && ( tChan->source != TRAINER_COM2 ) )
#endif
			{
				att |= NO_TRAINER ;
			}
		}
    {
        uint8_t ele_stick, ail_stick ;
        ele_stick = 1 ; //ELE_STICK ;
        ail_stick = 3 ; //AIL_STICK ;

				uint8_t stickIndex = g_eeGeneral.stickMode*4 ;
#ifdef PCBX7
        for( uint8_t i = 0 ; i < 6+NumExtraPots ; i += 1 ) // calc Sticks
#else
#if defined(PCBX9LITE)
        for( uint8_t i = 0 ; i < 5+NumExtraPots ; i += 1 ) // calc Sticks
#else
        for( uint8_t i = 0 ; i < 7+NumExtraPots ; i += 1 ) // calc Sticks
#endif
#endif
				{
            //Normalization  [0..2048] ->   [-1024..1024]
					int16_t v = anaIn( i ) ;
					v = scaleAnalog( v, i ) ;

						uint8_t index = i ;
						if ( i < 4 )
						{
            	phyStick[i] = v >> 4 ;
							index = stickScramble[stickIndex+i] ;
						}
            calibratedStick[index] = v; //for show in expo

						// Filter beep centre
						{
							int8_t t = v/16 ;
							uint16_t mask = 1 << index ;
							if ( t < 0 )
							{
								t = -t ;		//abs(t)
							}
							if ( t <= 1 )
							{
            		anaCenter |= ( t == 0 ) ? mask : bpanaCenter & mask ;
							}
						}

            if(i<4) { //only do this for sticks
                //===========Trainer mode================
                if (!(att&NO_TRAINER) && g_model.traineron)
								{
									TrainerChannel *tChan = &g_eeGeneral.trainerProfile[g_model.trainerProfile].channel[index] ;
                  if (tChan->mode && getSwitch00(tChan->swtch))
									{
										if ( ppmInValid )
										{
                      uint8_t chStud = tChan->srcChn ;
                      int32_t vStud  = (g_ppmIns[chStud] - tChan->calib) /* *2 */ ;
                      vStud *= tChan->studWeight ;
                      vStud /= 50 ;
                      switch ((uint8_t)tChan->mode)
											{
                        case 1: v += vStud;   break; // add-mode
                        case 2: v  = vStud;   break; // subst-mode
                      }
											if ( index == 2 )
											{
												trainerThrottleValue = vStud ;
												trainerThrottleValid = 1 ;
											}												 
										}
									}
									
                }


								if ( att & FADE_FIRST )
								{
    		        	rawSticks[index] = v; //set values for mixer
								}
								v = calcExpo( index, v ) ;

                trimA[i] = getTrimValue( CurrentPhase, i )*2 ; //    if throttle trim -> trim low end
            }
						if ( att & FADE_FIRST )
						{
							if ( i < 7 )
							{
        	    	anas[index] = v ; //set values for mixer
							}
						}
			    	if(att&NO_INPUT)
						{ //zero input for setStickCenter()
		      	  if ( i < 4 )
							{
    	    	    if(!IS_THROTTLE(index))
								{
									if ( ( v > (RESX/100 ) ) || ( v < -(RESX/100) ) )
									{
				            anas[index] = 0; //set values for mixer
									}
          	      trimA[index] = 0;
      	  	    }
        				anas[i+PPM_BASE] = 0;
        			}
    				}
        }
				//    if throttle trim -> trim low end
        if(g_model.thrTrim)
				{
					int8_t ttrim ;
					ttrim = getTrimValue( CurrentPhase, 2 ) ;
					if(throttleReversed())
					{
						ttrim = -ttrim ;
					}
		      int16_t tmp = calc100toRESX( 100 - g_model.throttleIdleScale ) * 2 ;	// 0 to 2 * RESX
					if ( ( anas[2] + RESX) > tmp )
					{
						trimA[2] = 0 ;
					}
					else
					{
						tmp = ( tmp - ( anas[2] + RESX ) ) * RESX / tmp ;
	         	trimA[2] = ((int32_t)ttrim+125) * tmp / (RESX) ;
					}
				}
			if ( att & FADE_FIRST )
			{

        //===========BEEP CENTER================
        anaCenter &= g_model.beepANACenter;
        if(((bpanaCenter ^ anaCenter) & anaCenter)) audioDefevent(AU_POT_STICK_MIDDLE);
        bpanaCenter = anaCenter;

				// Set up anas[] array
        anas[MIX_MAX-1]  = RESX;     // MAX
        anas[MIX_FULL-1] = RESX;     // FULL
#if defined(PCBSKY) || defined(PCB9XT)
        anas[MIX_3POS-1] = keyState(SW_ID0) ? -1024 : (keyState(SW_ID1) ? 0 : 1024) ;
#endif
#ifdef PCBX9D
        anas[MIX_3POS-1] = keyState(SW_SC0) ? -1024 : (keyState(SW_SC1) ? 0 : 1024) ;
#endif
        for(uint8_t i=0;i<4;i++) anas[i+PPM_BASE] = (g_ppmIns[i] - g_eeGeneral.trainerProfile[g_model.trainerProfile].channel[i].calib)*2; //add ppm channels
        for(uint8_t i=4;i<NUM_PPM;i++)    anas[i+PPM_BASE]   = g_ppmIns[i]*2; //add ppm channels
        for(uint8_t i=0;i<NUM_SKYCHNOUT+EXTRA_SKYCHANNELS;i++) anas[i+CHOUT_BASE] = chans[i]; //other mixes previous outputs
        for(uint8_t i=0;i<MAX_GVARS;i++) anas[i+MIX_3POS] = g_model.gvars[i].gvar * 1024 / 100 ;

				int16_t heliEle = anas[ele_stick] ;
				int16_t heliAil = anas[ail_stick] ;

        //===========Swash Ring================
        if(g_model.swashRingValue)
        {
          uint32_t v = ((int32_t)heliEle*heliEle + (int32_t)heliAil*heliAil);
		      int16_t tmp = calc100toRESX(g_model.swashRingValue) ;
          uint32_t q ;
          q = (int32_t)tmp * tmp ;
          if(v>q)
          {
            uint16_t d = isqrt32(v);
            heliEle = (int32_t)heliEle*tmp/((int32_t)d) ;
            heliAil = (int32_t)heliAil*tmp/((int32_t)d) ;
          }
        }

#define REZ_SWASH_X(x)  ((x) - (x)/8 - (x)/128 - (x)/512)   //  1024*sin(60) ~= 886
#define REZ_SWASH_Y(x)  ((x))   //  1024 => 1024

        if(g_model.swashType)
        {
            int16_t vp = 0 ;
            int16_t vr = 0 ;

            if( !(att & NO_INPUT) )  //zero input for setStickCenter()
						{
	            vp = heliEle+trimA[ele_stick];
  	          vr = heliAil+trimA[ail_stick];
							TrimInUse[ele_stick] |= 1 ;
							TrimInUse[ail_stick] |= 1 ;
						}

            int16_t vc = 0;
            if(g_model.swashCollectiveSource)
						{
							if ( g_model.swashCollectiveSource >= EXTRA_POTS_START )
							{
								vc = calibratedStick[g_model.swashCollectiveSource-EXTRA_POTS_START+7] ;
							}
							else
							{
              	vc = anas[g_model.swashCollectiveSource-1];
							}
						}

            if(g_model.swashInvertELE) vp = -vp;
            if(g_model.swashInvertAIL) vr = -vr;
            if(g_model.swashInvertCOL) vc = -vc;

            switch (( uint8_t)g_model.swashType)
            {
            case (SWASH_TYPE_120):
                vp = REZ_SWASH_Y(vp);
                vr = REZ_SWASH_X(vr);
                anas[MIX_CYC1-1] = vc - vp;
                anas[MIX_CYC2-1] = vc + vp/2 + vr;
                anas[MIX_CYC3-1] = vc + vp/2 - vr;
                break;
            case (SWASH_TYPE_120X):
                vp = REZ_SWASH_X(vp);
                vr = REZ_SWASH_Y(vr);
                anas[MIX_CYC1-1] = vc - vr;
                anas[MIX_CYC2-1] = vc + vr/2 + vp;
                anas[MIX_CYC3-1] = vc + vr/2 - vp;
                break;
            case (SWASH_TYPE_140):
                vp = REZ_SWASH_Y(vp);
                vr = REZ_SWASH_Y(vr);
                anas[MIX_CYC1-1] = vc - vp;
                anas[MIX_CYC2-1] = vc + vp + vr;
                anas[MIX_CYC3-1] = vc + vp - vr;
                break;
            case (SWASH_TYPE_90):
                vp = REZ_SWASH_Y(vp);
                vr = REZ_SWASH_Y(vr);
                anas[MIX_CYC1-1] = vc - vp;
                anas[MIX_CYC2-1] = vc + vr;
                anas[MIX_CYC3-1] = vc - vr;
                break;
            default:
                break;
            }
	        }
  		  }

    		if(tick10ms)
				{
					inactivityCheck() ;
					trace(); //trace thr 0..32  (/32)
				}
			}
    memset(chans,0,sizeof(chans));        // All outputs to 0


    uint8_t mixWarning = 0;
    //========== MIXER LOOP ===============

#if EXTRA_SKYMIXERS
    for(uint8_t i=0;i<MAX_SKYMIXERS+EXTRA_SKYMIXERS;i++)
#else
    for(uint8_t i=0;i<MAX_SKYMIXERS;i++)
#endif
		{
        SKYMixData *md = mixAddress( i ) ;

				int16_t lweight = md->weight ;
				if ( (lweight <= -126) || (lweight >= 126) )
				{
					lweight = REG100_100( lweight ) ;
				}
				else
				{
					if ( md->extWeight == 1 )
					{
						lweight += 125 ; 
					}
					else if ( md->extWeight == 3 )
					{
						lweight -= 125 ; 
					}
					else if ( md->extWeight == 2 )
					{
						if ( lweight < 0 )
						{
							lweight -= 250 ;
						}
						else
						{
							lweight += 250 ;
						}
					}
				}
				int16_t mixweight = lweight ;
				int16_t loffset = md->sOffset ;
				if ( (loffset <= -126) || (loffset >= 126) )
				{
					loffset = REG100_100( loffset ) ;
				}
				else
				{
					if ( md->extOffset == 1 )
					{
						loffset += 125 ; 
					}
					else if ( md->extOffset == 3 )
					{
						loffset -= 125 ; 
					}
					else if ( md->extOffset == 2 )
					{
						if ( lweight < 0 )
						{
							loffset -= 250 ;
						}
						else
						{
							loffset += 250 ;
						}
					}
				}
				int16_t mixoffset = loffset ;

        if((md->destCh==0) || (md->destCh>NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)) break;

        //Notice 0 = NC switch means not used -> always on line
        int16_t v  = 0;
        uint8_t swTog;
        uint8_t swon = swOn[i] ;
				
				bool t_switch = getSwitch(md->swtch,1) ;
        if (md->swtch && (md->srcRaw > PPM_BASE) && (md->srcRaw <= PPM_BASE+NUM_PPM) && (ppmInValid == 0) )
				{
					// then treat switch as false ???				
					t_switch = 0 ;
				}	
        if (md->swtch && (md->srcRaw > MIX_3POS+MAX_GVARS + NUM_SCALERS ) && (ppmInValid == 0) )
				{ // Extra PPM inputs (9-16)
					if (md->srcRaw <= MIX_3POS+MAX_GVARS + NUM_SCALERS + NUM_EXTRA_PPM )
					{
						// then treat switch as false ???				
						t_switch = 0 ;
					}
				}
      
        if ( t_switch )
				{
					if ( md->modeControl & ( 1 << CurrentPhase ) )
					{
						t_switch = 0 ;
					}
				}

        uint8_t k = md->srcRaw ;

#define DEL_MULT 256
        if(!t_switch)
        { // switch on?  if no switch selected => on
            swTog = swon ;
            swOn[i] = swon = false ;
            if (k == MIX_3POS+MAX_GVARS+1) act[i] = chans[md->destCh-1] * DEL_MULT / 100 ;	// "THIS"
            if( k !=MIX_MAX && k !=MIX_FULL) continue;// if not MAX or FULL - next loop
            if(md->mltpx==MLTPX_REP) continue; // if switch is off and REPLACE then off
            v = ( k == MIX_FULL ? -RESX : 0); // switch is off and it is either MAX=0 or FULL=-512
        }
        else {
            swTog = !swon ;
            swon = true;
            k -= 1 ;
						if ( k < NUM_SKYXCHNRAW+1+MAX_GVARS+1 )
						{
							v = anas[k]; //Switch is on. MAX=FULL=512 or value.
						}
						if ( k < 4 )
						{
							if ( md->disableExpoDr )
							{
     		      	v = rawSticks[k]; //Switch is on. MAX=FULL=512 or value.
							}
						}
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
						if ( k == MIX_3POS-1 )
						{
							if ( md->switchSource > 8 )	// Sort for X9E ********
							{
								v = getSwitch( CSW_INDEX+md->switchSource-8, 0, 0 ) ;
								v = v ? 1024 : -1024 ;
							}
							else
							{
								uint32_t /*EnumKeys*/ sw = switchIndex[md->switchSource] ;
								if ( ( md->switchSource == 5) || ( md->switchSource == 7) )
								{ // 2-POS switch
        					v = hwKeyState(sw) ? 1024 : -1024 ;
								}
								else if( md->switchSource == 8)
								{
									v = ((int32_t)switchPosition( HSW_Ele6pos0 ) * 2048 - 5120)/5 ;
								}
								else
								{ // 3-POS switch
        					v = hwKeyState(sw) ? -1024 : (hwKeyState(sw+1) ? 0 : 1024) ;
								}
							}
						}
#endif
#if defined(PCBSKY) || defined(PCB9XT)
						if ( k == MIX_3POS-1 )
						{
							if ( md->switchSource > 6 )
							{
								if ( md->switchSource == 31 )
								{
									v = ((int32_t)switchPosition( HSW_Ele6pos0 ) * 2048 - 5120)/5 ;
								}
								else
								{
									v = getSwitch( CSW_INDEX+md->switchSource-6, 0, 0 ) ;
									v = v ? 1024 : -1024 ;
								}
							}
							else
							{
								uint32_t sw = Sw3PosList[md->switchSource] ;
								if ( Sw3PosCount[md->switchSource] == 2 )
								{
        					v = hwKeyState(sw) ? 1024 : -1024 ;
								}
								else if ( Sw3PosCount[md->switchSource] == 6 )
								{
									v = ((int32_t)switchPosition( HSW_Ele6pos0 ) * 2048 - 5120)/5 ;
								}
								else
								{
        					v = hwKeyState(sw) ? -1024 : (hwKeyState(sw+1) ? 0 : 1024) ;
								}
							}
						}
#endif
//            if(k>=CHOUT_BASE && (k<i)) v = chans[k]; // if we've already calculated the value - take it instead // anas[i+CHOUT_BASE] = chans[i]
						if ( k >= CHOUT_BASE )
            {
							if(k<CHOUT_BASE+NUM_SKYCHNOUT)
							{
								if ( md->disableExpoDr )
								{
									v = g_chans512[k-CHOUT_BASE] ;
								}
								else
								{
            			if(k<CHOUT_BASE+md->destCh-1)
									{
										v = chans[k-CHOUT_BASE] / 100 ; // if we've already calculated the value - take it instead // anas[i+CHOUT_BASE] = chans[i]
									}
									else
									{
										v = ex_chans[k-CHOUT_BASE] ;
									}
								}
							}
#if EXTRA_SKYCHANNELS
							else if ( k > MIX_3POS+MAX_GVARS + NUM_SCALERS + NUM_EXTRA_PPM )
							{ // EXTRA_channels
								if(k <= MIX_3POS+MAX_GVARS + NUM_SCALERS + NUM_EXTRA_PPM + EXTRA_SKYCHANNELS )
								{
									uint32_t t = k - (MIX_3POS+MAX_GVARS + NUM_SCALERS + NUM_EXTRA_PPM) + 24 - 1 ;
									if ( md->disableExpoDr )
									{
										v = g_chans512[t] ;
									}
									else
									{
//            				if(k<CHOUT_BASE+md->destCh-1)
//										{
//											v = chans[k-CHOUT_BASE] / 100 ; // if we've already calculated the value - take it instead // anas[i+CHOUT_BASE] = chans[i]
//										}
//										else
										{
											v = ex_chans[t] ;
										}
									}
								}
								else
								{
									if(k <= MIX_TRIMS_START + 4 )
									{
										uint32_t t = k - MIX_TRIMS_START ;
										v = trimA[t] ;
										TrimInUse[t] |= 1 ;
									}
								}
							}
#endif
						}

						if (k == MIX_3POS+MAX_GVARS) v = chans[md->destCh-1] / 100 ;	// "THIS"
            if ( (k > MIX_3POS+MAX_GVARS) && ( k <= MIX_3POS+MAX_GVARS + NUM_SCALERS ) )
						{
							 v = calc_scaler( k - (MIX_3POS+MAX_GVARS+1), 0, 0 ) ;
            }
            if (k > MIX_3POS+MAX_GVARS + NUM_SCALERS)
						{
							if ( k <= MIX_3POS+MAX_GVARS + NUM_SCALERS + NUM_EXTRA_PPM )
							{
								v = g_ppmIns[k-(MIX_3POS+MAX_GVARS + NUM_SCALERS + 1) + 8]*2 ;
							}
							else if ( k >= EXTRA_POTS_START-1 )
							{
								// An extra pot
#ifdef PCBX7
								v = calibratedStick[k-EXTRA_POTS_START+7] ;
#else
 #ifdef PCBX9LITE
								v = calibratedStick[k-EXTRA_POTS_START+6] ;
 #else
								v = calibratedStick[k-EXTRA_POTS_START+8] ;
 #endif
#endif
							}
						}
						if(md->mixWarn) mixWarning |= 1<<(md->mixWarn-1); // Mix warning
        }
        swOn[i] = swon ;

        //========== INPUT OFFSET ===============
        if ( md->lateOffset == 0 )
        {
            if(mixoffset) v += calc100toRESX( mixoffset	) ;
        }

        //========== DELAY and PAUSE ===============
				if ( ( att & NO_DELAY_SLOW ) == 0 )
				{
        if (md->speedUp || md->speedDown || md->delayUp || md->delayDown)  // there are delay values
        {

					int16_t my_delay = sDelay[i] ;
					int32_t tact = act[i] ;
#if DEL_MULT == 256
						int16_t diff = v-(tact>>8) ;
#else
            int16_t diff = v-tact/DEL_MULT;
#endif
						if ( ( diff > 10 ) || ( diff < -10 ) )
						{
							if ( my_delay == 0 )
							{
        				if (md->delayUp || md->delayDown)  // there are delay values
								{								
									swTog = 1 ;
								}
							}
						}
						else
						{
							my_delay = 0 ;							
						}

            if(swTog) {
                //need to know which "v" will give "anas".
                //curves(v)*weight/100 -> anas
                // v * weight / 100 = anas => anas*100/weight = v
                if(md->mltpx==MLTPX_REP)
                {
                    tact = (int32_t)anas[md->destCh-1+CHOUT_BASE]*DEL_MULT * 100;
                    if(mixweight) tact /= mixweight ;
                }
                diff = v-tact/DEL_MULT;
                if(diff) my_delay = (diff<0 ? md->delayUp :  md->delayDown) * 10 ;
            }

            if(my_delay > 0)
						{ // perform delay
              if(tick10ms)
              {
                my_delay -= 1 ;
							}
              if ( my_delay != 0)
             	{ // At end of delay, use new V and diff
#if DEL_MULT == 256
               	v = tact >> 8 ;	   // Stay in old position until delay over
#else
                v = tact/DEL_MULT;   // Stay in old position until delay over
#endif
                diff = 0;
 	            }
							else
							{
								my_delay = -1 ;
							}
            }

					sDelay[i] = my_delay ;

            if(diff && (md->speedUp || md->speedDown)){
                //rate = steps/sec => 32*1024/100*md->speedUp/Down
                //act[i] += diff>0 ? (32768)/((int16_t)100*md->speedUp) : -(32768)/((int16_t)100*md->speedDown);
                //-100..100 => 32768 ->  100*83886/256 = 32768,   For MAX we divide by 2 since it's asymmetrical
                if(tick10ms) {
                    int32_t rate = (int32_t)DEL_MULT*2048*100;
                    if(mixweight) rate /= abs(mixweight);
										int16_t speed ;
                    if ( diff>0 )
										{
											speed = md->speedUp ;
										}
										else
										{
											rate = -rate ;											
											speed = md->speedDown ;
										}
										tact = (speed) ? tact+(rate)/((int16_t)10*speed) : (int32_t)v*DEL_MULT ;

                }
								{
#if DEL_MULT == 256
									int32_t tmp = tact>>8 ;
#else
									int32_t tmp = tact/DEL_MULT ;
#endif
                	if(((diff>0) && (v<tmp)) || ((diff<0) && (v>tmp))) tact=(int32_t)v*DEL_MULT; //deal with overflow
                }
#if DEL_MULT == 256
                v = tact >> 8 ;
#else
                v = tact/DEL_MULT;
#endif
            }
            else if (diff)
            {
              tact=(int32_t)v*DEL_MULT;
            }
					act[i] = tact ;
        }
				}
				else
				{
					act[i] = (int32_t)v*DEL_MULT ;
				}
        //========== CURVES ===============
				if ( md->differential )
				{
      		//========== DIFFERENTIAL =========
      		int8_t curveParam = REG100_100( md->curve ) ;
      		if (curveParam > 0 && v < 0)
      		  v = (v * (100 - curveParam)) / 100;
      		else if (curveParam < 0 && v > 0)
      		  v = (v * (100 + curveParam)) / 100;
				}
				else
				{
					if ( md->curve <= -28 )
					{
						// do expo using md->curve + 128
      			v = expo( v, md->curve + 128 ) ;
					}
					else
					{
        		switch(md->curve){
        		case 0:
        		    break;
        		case 1:
        		    if(md->srcRaw == MIX_FULL) //FUL
        		    {
        		        if( v<0 ) v=-RESX;   //x|x>0
        		        else      v=-RESX+2*v;
        		    }else{
        		        if( v<0 ) v=0;   //x|x>0
        		    }
        		    break;
        		case 2:
        		    if(md->srcRaw == MIX_FULL) //FUL
        		    {
        		        if( v>0 ) v=RESX;   //x|x<0
        		        else      v=RESX+2*v;
        		    }else{
        		        if( v>0 ) v=0;   //x|x<0
        		    }
        		    break;
        		case 3:       // x|abs(x)
        		    v = abs(v);
        		    break;
        		case 4:       //f|f>0
        		    v = v>0 ? RESX : 0;
        		    break;
        		case 5:       //f|f<0
        		    v = v<0 ? -RESX : 0;
        		    break;
        		case 6:       //f|abs(f)
        		    v = v>0 ? RESX : -RESX;
        		    break;
        		default: //c1..c16
								{
									int8_t idx = md->curve ;
									if ( idx < 0 )
									{
										v = -v ;
										idx = 6 - idx ;								
									}
        		    	v = intpol(v, idx - 7);
								}
        		}
					}
				}

        //========== TRIM ===============
        if((md->carryTrim==0) && (md->srcRaw>0) && (md->srcRaw<=4))
				{
					int32_t trim = trimA[md->srcRaw-1] ;
					v += trim ;  //  0 = Trim ON  =  Default
					TrimInUse[md->srcRaw-1] |= 1 ;
				}
        //========== MULTIPLEX ===============
        int32_t dv = (int32_t)v*mixweight ;
				
        //========== lateOffset ===============
				if ( md->lateOffset )
        {
            if(mixoffset) dv += calc100toRESX( mixoffset ) * 100 ;
        }
				
				int32_t *ptr ;			// Save calculating address several times
				ptr = &chans[md->destCh-1] ;
        switch((uint8_t)md->mltpx){
        case MLTPX_REP:
            *ptr = dv;
            break;
        case MLTPX_MUL:
						dv /= 100 ;
						dv *= *ptr ;
            dv /= RESXl;
            *ptr = dv ;
            break;
        default:  // MLTPX_ADD
            *ptr += dv; //Mixer output add up to the line (dv + (dv>0 ? 100/2 : -100/2))/(100);
            break;
        }
    }

    //========== MIXER WARNING ===============
    //1= 00,08
    //2= 24,32,40
    //3= 56,64,72,80
    {
        uint16_t tmr10ms ;
        tmr10ms = get_tmr10ms() ;

        if(mixWarning & 1) if(((tmr10ms&0xFF)==  0)) audioDefevent(AU_MIX_WARNING_1);
        if(mixWarning & 2) if(((tmr10ms&0xFF)== 64) || ((tmr10ms&0xFF)== 72)) audioDefevent(AU_MIX_WARNING_2);
        if(mixWarning & 4) if(((tmr10ms&0xFF)==128) || ((tmr10ms&0xFF)==136) || ((tmr10ms&0xFF)==144)) audioDefevent(AU_MIX_WARNING_3);        


    }

		ThrottleStickyOn = 0 ;
    //========== LIMITS ===============
    for(uint8_t i=0;i<NUM_SKYCHNOUT+EXTRA_SKYCHANNELS;i++)
		{
        // chans[i] holds data from mixer.   chans[i] = v*weight => 1024*100
        // later we multiply by the limit (up to 100) and then we need to normalize
        // at the end chans[i] = chans[i]/100 =>  -1024..1024
        // interpolate value with min/max so we get smooth motion from center to stop
        // this limits based on v original values and min=-1024, max=1024  RESX=1024

        int32_t q = chans[i];// + (int32_t)g_model.limitData[i].offset*100; // offset before limit

				if ( Fade.fadePhases )
				{
					int32_t l_fade = Fade.fade[i] ;
					if ( att & FADE_FIRST )
					{
						l_fade = 0 ;
					}
					l_fade += ( q / 100 ) * Fade.fadeScale[CurrentPhase] ;
					Fade.fade[i] = l_fade ;
			
					if ( ( att & FADE_LAST ) == 0 )
					{
						continue ;
					}
					l_fade /= Fade.fadeWeight ;
					q = l_fade * 100 ;
				}
    	  chans[i] = q / 100 ; // chans back to -1024..1024
        
				ex_chans[i] = chans[i]; //for getswitch

        LimitData *limit = &g_model.limitData[i] ;
#if EXTRA_SKYCHANNELS
				if ( i >= NUM_SKYCHNOUT )
				{
					limit = &g_model.elimitData[i-NUM_SKYCHNOUT] ;
				}
#endif			
				int16_t ofs = limit->offset;
				int16_t xofs = ofs ;
				if ( xofs > g_model.sub_trim_limit )
				{
					xofs = g_model.sub_trim_limit ;
				}
				else if ( xofs < -g_model.sub_trim_limit )
				{
					xofs = -g_model.sub_trim_limit ;
				}
        int16_t lim_p = 10*(limit->max+100) + xofs ;
        int16_t lim_n = 10*(limit->min-100) + xofs ; //multiply by 10 to get same range as ofs (-1000..1000)
				if ( lim_p > 1250 )
				{
					lim_p = 1250 ;
				}
				if ( lim_n < -1250 )
				{
					lim_n = -1250 ;
				}
        if(ofs>lim_p) ofs = lim_p;
        if(ofs<lim_n) ofs = lim_n;

        if(q) q = (q>0) ?
                    q*((int32_t)lim_p-ofs)/100000 :
                    -q*((int32_t)lim_n-ofs)/100000 ; //div by 100000 -> output = -1024..1024

        q += calc1000toRESX(ofs);
        lim_p = calc1000toRESX(lim_p);
        lim_n = calc1000toRESX(lim_n);
        if(q>lim_p) q = lim_p;
        if(q<lim_n) q = lim_n;
        if(limit->revert) q=-q;// finally do the reverse.

				{
					uint8_t numSafety = NUM_SKYCHNOUT - g_model.numVoice ;
					if ( i < numSafety )
					{
        		if(g_model.safetySw[i].opt.ss.swtch)  //if safety sw available for channel check and replace val if needed
						{
							if ( ( g_model.safetySw[i].opt.ss.mode != 1 ) && ( g_model.safetySw[i].opt.ss.mode != 2 ) )	// And not used as an alarm
							{
								static uint32_t sticky = 0 ;
								uint8_t applySafety = 0 ;
								int8_t sSwitch = g_model.safetySw[i].opt.ss.swtch ;
								
								if(getSwitch00( sSwitch))
								{
									applySafety = 1 ;
								}

								if ( g_model.safetySw[i].opt.ss.mode == 3 )
								{
									int8_t thr = g_model.safetySw[i].opt.ss.source ;
									uint32_t rev_thr = 0 ;
									if ( thr == 0 )
									{
										thr = 2 ;
									}
									else
									{
										if ( thr > 0 )
										{
											thr += 3 ;
										}
										else
										{
											rev_thr = 1 ;
											thr = -thr + 3 ;
										}
									}	
									// Special case, sticky throttle
									if( applySafety )
									{
										sticky &= ~(1<<i) ;
									}
									else
									{
										uint32_t throttleOK = 0 ;
										if ( g_model.throttleIdle )
										{
											if ( abs( calibratedStick[thr] ) < 20 )
											{
												throttleOK = 1 ;
											}
										}
										else
										{
											if ( rev_thr )
											{
  											if(calibratedStick[thr] > 1004)
  											{
													throttleOK = 1 ;
  											}
											}
											else
											{
  											if(calibratedStick[thr] < -1004)
  											{
													throttleOK = 1 ;
  											}
											}
										}
										
										if ( throttleOK )
										{
											if ( trainerThrottleValid )
											{
												if ( trainerThrottleValue < -1004 )
												{
													sticky |= (1<<i) ;
												}
											}	
											else
											{
												sticky |= (1<<i) ;
											}
										}
									}
									if ( ( sticky & (1<<i) ) == 0 )
									{
										applySafety = 1 ;
									}
									ThrottleStickyOn = applySafety ;
								}
								if ( applySafety )
								{
									q = calc100toRESX(g_model.safetySw[i].opt.ss.val) ;
									q += (q>=0) ? g_model.safetySw[i].opt.ss.tune : -g_model.safetySw[i].opt.ss.tune ;
								}
							}
						}
					}
				}
        chanOut[i] = q; //copy consistent word to int-level
		}
}



