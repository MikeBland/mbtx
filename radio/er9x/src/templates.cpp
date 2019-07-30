/*
 * Author - Erez Raviv <erezraviv@gmail.com>
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
 * ============================================================
 * Templates file
 *
 * eccpm
 * crow
 * throttle cut
 * flaperon
 * elevon
 * v-tail
 * throttle hold
 * Aileron Differential
 * Spoilers
 * Snap Roll
 * ELE->Flap
 * Flap->ELE
 *
 *
 *
 * =============================================================
 * Assumptions:
 * All primary channels are per modi12x3
 * Each template added to the end of each channel
 *
 *
 *
 */

#include "er9x.h"
#include "templates.h"
#include "language.h"

void clearMixes()
{
	uint16_t i ;
  uint8_t *md = (uint8_t *) &g_model.mixData[0] ;
	for ( i = sizeof(g_model.mixData) ; i ; i -= 1 )
	{
		*md++ = 0 ;
	}
//	memset(g_model.mixData,0,sizeof(g_model.mixData)); //clear all mixes
}

#ifdef NO_TEMPLATES
void setMix( uint8_t mix, uint8_t order )
{
  MixData *md = &g_model.mixData[mix];
//  memset( md, 0, sizeof(MixData) ) ; // Not needed, all mixes already cleared
  md->destCh = mix + 1 ;
	md->weight = 100 ;
#ifndef V2
	md->lateOffset = 1 ;
#endif
	md->srcRaw = ( order & 3 ) + 1 ;
}

void applyTemplate()
{
	uint8_t bch = pgm_read_byte(bchout_ar + g_eeGeneral.templateSetup) ;
  clearMixes();
  setMix( 3, bch ) ;
	bch /= 4 ;
  setMix( 2, bch ) ;
	bch /= 4 ;
  setMix( 1, bch ) ;
	bch /= 4 ;
  setMix( 0, bch ) ;
}

#else

#ifndef NO_TEMPLATES

const static prog_char APM string_1[] = STR_T_S_4CHAN   ;
const static prog_char APM string_2[] = STR_T_TCUT      ;
const static prog_char APM string_3[] = STR_T_STICK_TCUT;
const static prog_char APM string_4[] = STR_T_V_TAIL    ;
const static prog_char APM string_5[] = STR_T_ELEVON    ;
const static prog_char APM string_6[] = STR_T_HELI_SETUP;
const static prog_char APM string_7[] = STR_T_GYRO      ;
const static prog_char APM string_8[] = STR_T_SERVO_TEST16;
const static prog_char APM string_9[] = STR_T_SERVO_TEST8;

const prog_char *const n_Templates[9] PROGMEM = {
    string_1,
    string_2,
    string_3,
    string_4,
    string_5,
    string_6,
    string_7,
    string_8,
    string_9
};

#endif

static MixData* setDest(uint8_t dch)
{
    uint8_t i = 0;
    MixData *md = &g_model.mixData[0];

    while ((md->destCh<=dch) && (md->destCh) && (i<MAX_MIXERS)) i++, md++;
    if(i==MAX_MIXERS) return &g_model.mixData[0];

    memmove(md+1, md, (MAX_MIXERS-(i+1))*sizeof(MixData) );
    memset( md, 0, sizeof(MixData) ) ;
    md->destCh = dch;
		md->weight = 100 ;
#ifndef V2
		md->lateOffset = 1 ;
#endif
    return md ;
}

//#ifdef NO_TEMPLATES
//inline
//#endif 
#ifndef NO_TEMPLATES
void clearCurves()
{
    memset(g_model.curves5,0,sizeof(g_model.curves5)); //clear all curves
    memset(g_model.curves9,0,sizeof(g_model.curves9)); //clear all curves
}

static void setCurve(uint8_t c, const prog_int8_t *ar )
{
	int8_t *p ;
	
	p = g_model.curves5[c] ;
  for(uint8_t i=0; i<5; i++)
	{
		*p++ = pgm_read_byte(ar++) ;
	}
}

void setSwitch(uint8_t idx, uint8_t func, int8_t v1, int8_t v2)
{
#ifdef V2
	CxSwData *cs = &g_model.customSw[idx-1] ;
#else
	CSwData *cs = &g_model.customSw[idx-1] ;
#endif // V2
  cs->func = func ;
  cs->andsw = 0 ;
  cs->v1   = v1 ;
  cs->v2   = v2 ;
}

#endif


#ifndef NO_TEMPLATES
const prog_int8_t heli_ar1[] PROGMEM = {-100, 20, 30, 70, 90};
const prog_int8_t heli_ar2[] PROGMEM = {80, 70, 60, 70, 100};
const prog_int8_t heli_ar3[] PROGMEM = {100, 90, 80, 90, 100};
const prog_int8_t heli_ar4[] PROGMEM = {-30, -15, 0, 50, 100};
const prog_int8_t heli_ar5[] PROGMEM = {-100, -50, 0, 50, 100};
#endif


MixData *setMix( uint8_t dch, uint8_t stick )
{
  MixData *md ;
	md=setDest( dch ) ;
	md->srcRaw=CM( stick ) ;
	return md ;
}




#ifdef NO_TEMPLATES
void applyTemplate()
#else
void applyTemplate(uint8_t idx)
#endif
{
#ifndef NO_TEMPLATES
    MixData *md = &g_model.mixData[0];
#endif

    //CC(STK)   -> vSTK
    //ICC(vSTK) -> STK
#define ICC(x) icc[(x)-1]
    uint8_t icc[4] ;
//    for(uint8_t i=1; i<=4; i++) //generate inverse array
//		{
////        for(uint8_t j=1; j<=4; j++) if(CC(i)==j) icc[j-1]=i;
//			icc[CC(i)-1] = i ;
//		}

		uint8_t bch = pgm_read_byte(bchout_ar + g_eeGeneral.templateSetup) ;
    for ( uint8_t i = 4 ; i > 0 ; i -= 1 )
		{
			icc[bch & 3] = i ;
			bch >>= 2 ;
		}

#ifndef NO_TEMPLATES
    uint8_t j = 0;

    //Simple 4-Ch
    if(idx==j++) 
    {
#endif
        clearMixes();
        setMix(ICC(STK_RUD), STK_RUD ) ;
        setMix(ICC(STK_ELE), STK_ELE ) ;
        setMix(ICC(STK_THR), STK_THR ) ;
        setMix(ICC(STK_AIL), STK_AIL ) ;

#ifndef NO_TEMPLATES
    }

    //T-Cut
    if(idx==j++)
    {
//        md=setDest(ICC(STK_THR));  md->srcRaw=MIX_MAX;  md->weight=-100;  md->swtch=DSW_THR;  md->mltpx=MLTPX_REP;
#ifdef V2
			V2SafetySwData *sd = &g_model.safetySw[ICC(STK_THR)-1] ;
			sd->mode = 0 ;
			sd->swtch = DSW_THR ;
			sd->val = g_model.throttleIdle ? 0 : -100 ;
#else
    	SafetySwData *sd = &g_model.safetySw[ICC(STK_THR)-1] ;
			sd->opt.ss.mode = 0 ;
			sd->opt.ss.swtch = DSW_THR ;
			sd->opt.ss.val = g_model.throttleIdle ? 0 : -100 ;
#endif // V2
    }

    //sticky t-cut
    if(idx==j++)
    {
//        md=setDest(ICC(STK_THR));  md->srcRaw=MIX_MAX;  md->weight=-100;  md->swtch=DSW_SWC;  md->mltpx=MLTPX_REP;
//        md=setDest(14);            md->srcRaw=CH(14);
//        md=setDest(14);            md->srcRaw=MIX_MAX;  md->weight=-100;  md->swtch=DSW_SWB;  md->mltpx=MLTPX_REP;
//        md=setDest(14);            md->srcRaw=MIX_MAX;  md->swtch=DSW_THR;  md->mltpx=MLTPX_REP;

//        setSwitch(0xB,CS_VNEG, CM(STK_THR), -99);
//        setSwitch(0xC,CS_VPOS, CH(14), 0);

#ifdef V2
			V2SafetySwData *sd = &g_model.safetySw[ICC(STK_THR)-1] ;
			sd->mode = 1 ;
			sd->swtch = DSW_THR ;
			sd->val = g_model.throttleIdle ? 0 : -100 ;
#else
    	SafetySwData *sd = &g_model.safetySw[ICC(STK_THR)-1] ;
 #ifdef SAFETY_ONLY
 			sd->opt.ss.mode = 1 ;
 #else
 			sd->opt.ss.mode = 3 ;
 #endif
 			sd->opt.ss.swtch = DSW_THR ;
			sd->opt.ss.val = g_model.throttleIdle ? 0 : -100 ;
#endif // V2
    }

    //V-Tail
    if(idx==j++) 
    {
        clearMixes();
        setMix(ICC(STK_RUD), STK_RUD )->weight=-50 ;
        md=setDest(ICC(STK_RUD));  md->srcRaw=CM(STK_ELE);  md->weight=-50 ;
        setMix(ICC(STK_ELE), STK_RUD )->weight=-50 ;
        setMix(ICC(STK_ELE), STK_ELE )->weight=-50 ;
        setMix(ICC(STK_THR), STK_THR ) ;
        setMix(ICC(STK_AIL), STK_AIL ) ;

    }

    //Elevon\\Delta
    if(idx==j++)
    {
        clearMixes();
        setMix(ICC(STK_ELE), STK_ELE )->weight=-50 ;
        setMix(ICC(STK_ELE), STK_AIL )->weight=-50 ;
        setMix(ICC(STK_AIL), STK_ELE )->weight=-50 ;
        md=setDest(ICC(STK_AIL));  md->srcRaw=CM(STK_AIL);  md->weight=-50 ;
        setMix(ICC(STK_RUD), STK_RUD ) ;
        setMix(ICC(STK_THR), STK_THR ) ;
    }

    //Heli Setup
    if(idx==j++)
    {
        clearMixes();  //This time we want a clean slate
        clearCurves();

        //Set up Mixes
        //3 cyclic channels
        md=setDest(1);  md->srcRaw=MIX_CYC1;
        md=setDest(2);  md->srcRaw=MIX_CYC2;
        md=setDest(3);  md->srcRaw=MIX_CYC3;

        //rudder
        setMix(4, STK_RUD ) ;

        //Throttle
        md=setDest(5);  md->srcRaw=CM(STK_THR); md->swtch= DSW_ID0; md->curve=CV(1); md->carryTrim=TRIM_OFF;
        md=setDest(5);  md->srcRaw=CM(STK_THR); md->swtch= DSW_ID1; md->curve=CV(2); md->carryTrim=TRIM_OFF;
        md=setDest(5);  md->srcRaw=CM(STK_THR); md->swtch= DSW_ID2; md->curve=CV(3); md->carryTrim=TRIM_OFF;
        md=setDest(5);  md->srcRaw=MIX_MAX;      md->weight=-100; md->swtch= DSW_THR; md->mltpx=MLTPX_REP;

        //gyro gain
        md=setDest(6);  md->srcRaw=MIX_FULL; md->weight=30; md->swtch=-DSW_GEA;

        //collective
        md=setDest(11); md->srcRaw=CM(STK_THR);  md->weight=70; md->swtch= DSW_ID0; md->curve=CV(4); md->carryTrim=TRIM_OFF;
        md=setDest(11); md->srcRaw=CM(STK_THR);  md->weight=70; md->swtch= DSW_ID1; md->curve=CV(5); md->carryTrim=TRIM_OFF;
        md=setDest(11); md->srcRaw=CM(STK_THR);  md->weight=70; md->swtch= DSW_ID2; md->curve=CV(6); md->carryTrim=TRIM_OFF;

        g_model.swashType = SWASH_TYPE_120;
        g_model.swashCollectiveSource = CH(11);

        //Set up Curves
        setCurve(CURVE5(1),heli_ar1);
        setCurve(CURVE5(2),heli_ar2);
        setCurve(CURVE5(3),heli_ar3);
        setCurve(CURVE5(4),heli_ar4);
        setCurve(CURVE5(5),heli_ar5);
        setCurve(CURVE5(6),heli_ar5);
    }

    //Gyro Gain
    if(idx==j++)
    {
        md=setDest(6);  md->srcRaw=STK_P2; md->weight= 50; md->swtch=-DSW_GEA; md->sOffset=50;
        md=setDest(6);  md->srcRaw=STK_P2; md->weight=-50; md->swtch= DSW_GEA; md->sOffset=-50;
    }

    //Servo Test
    if( (idx==j) || ( idx == j+1) )
    {
        md=setDest( (idx==j) ? 16 : 8 ) ;
				md->srcRaw=MIX_FULL; md->weight= 100; md->swtch=DSW_SWB;
        md->speedUp = 7; md->speedDown = 7 ;

        setSwitch(11,CS_TIME, 8, 8) ;

    }
//		j += 2 ;
    STORE_MODELVARS;
    eeWaitComplete() ;

#endif

}

#endif


