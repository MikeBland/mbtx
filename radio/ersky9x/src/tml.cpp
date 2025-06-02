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
#include "ff.h"
#include "vars.h"

// Dump model as text

FIL g_dumpFile = {0} ;
SKYModelData *Pmodel ;

void dumpModel( uint8_t physicalRadioType ) ;
extern SKYMixData *mixAddress( uint32_t index ) ;

uint32_t IndentLevel ;

const char *Indent0 = "" ;
const char *Indent1 = " " ;
const char *Indent2 = "  " ;
const char *Indent3 = "   " ;

const char *indentText()
{
	switch ( IndentLevel )
	{
		case 0 :
		return Indent0 ;
		case 1 :
		return Indent1 ;
		case 2 :
		return Indent2 ;
		default :
		return Indent3 ;
	}
}

void dumpModelAsText()
{
  f_open( &g_dumpFile, (TCHAR *)"\\modelDump", FA_OPEN_ALWAYS | FA_WRITE) ;
	Pmodel = &g_model ;
	dumpModel( g_eeGeneral.physicalRadioType ) ;
	f_close( &g_dumpFile ) ;
}

void dumpNew( uint32_t value, const char *text = 0 ) ;


void dumpNew( uint32_t value, const char *text )
{
	if ( text )
	{
		f_printf(&g_dumpFile, "\n%d-%s,", value, text ) ;
	}
	else
	{
		f_printf(&g_dumpFile, "\n%d,", value ) ;
	}
}

void dumpValueLast( uint32_t value )
{
	f_printf(&g_dumpFile, "%d", value ) ;
}

void dumpTextLast( const char *text )
{
	f_printf(&g_dumpFile, "%10s", text ) ;
}

void dumpSingleValue( uint32_t index, int32_t value )
{
	f_printf(&g_dumpFile, "\n%s%d,%d", indentText(), index, value ) ;
}

void dumpSingleText_text( uint32_t index, const char *text, const char *text2 )
{
	f_printf(&g_dumpFile, "\n%s%d-%s,%d", indentText(), index, text, text2 ) ;
}

void dumpSingleValue_text( uint32_t index, const char *text, int32_t value )
{
	if ( text )
	{
		f_printf(&g_dumpFile, "\n%s%d-%s,%d", indentText(), index, text, value ) ;
	}
	else
	{
		f_printf(&g_dumpFile, "\n%s%d,%d", indentText(), index, value ) ;
	}
}

//void dumpSingleValueIndent( uint32_t index, int32_t value )
//{
//	f_printf(&g_dumpFile, "\n %d,%d", index, value ) ;
//}

//void dumpSingleValueIndent_text( uint32_t index, char *text, int32_t value )
//{
//	if ( text )
//	{
//		f_printf(&g_dumpFile, "\n %d-%s,%d", index, text, value ) ;
//	}
//	else
//	{
//		dumpSingleValueIndent( index, value ) ;
//	}
//}

void dumpDecimal( int32_t value, uint32_t dp )
{
	ldiv_t qr ;
	uint32_t sign = 0 ;
	int32_t divisor = (dp == 1 ) ? 10 : 100 ;
	if ( value < 0 )
	{
		sign = 1 ;
		value = -value ;
	}
	qr = div( value, divisor ) ;
	if ( sign )
	{
		f_printf(&g_dumpFile, "-%d.%d", qr.quot, qr.rem ) ;
		return ;
	}
	f_printf(&g_dumpFile, "%d.%d", qr.quot, qr.rem ) ;
}

void dumpSingleDecimal( uint32_t index, int32_t value, uint32_t dp )
{
	f_printf(&g_dumpFile, "\n%s%d,", indentText(), index ) ;
	dumpDecimal( value, dp ) ;
}

void dumpSingleDecimal_text( uint32_t index, const char *text, int32_t value, uint32_t dp )
{
	if ( text )
	{
		f_printf(&g_dumpFile, "\n%s%d-%s,", indentText(), index, text ) ;
		dumpDecimal( value, dp ) ;
	}
	else
	{
		dumpSingleDecimal( index, value, dp ) ;
	}
}

	

void dumpModel( uint8_t physicalRadioType )
{
	uint32_t i ;
	uint32_t j ;
	uint32_t k ;
	char ltext[12] ;
	f_printf(&g_dumpFile, "0, M, %d", physicalRadioType ) ;		// Data type model
	dumpNew( 1 ) ;
	ncpystr( (uint8_t *)ltext, (uint8_t *)Pmodel->name, 10 ) ;
	ltext[10] = 0 ;
	dumpTextLast( ltext ) ;
	dumpSingleValue_text( 2, "Voice", Pmodel->modelVoice ) ;
	dumpSingleValue_text( 3, "telemetryRxInvert", Pmodel->telemetryRxInvert ) ;
	dumpSingleValue_text( 4, "trainerOn", Pmodel->traineron ) ;
	dumpSingleValue_text( 5, "autiBtConnect", Pmodel->autoBtConnect ) ;
	dumpSingleValue_text( 6, "FrSkyUsrProto", Pmodel->FrSkyUsrProto ) ;
	dumpSingleValue_text( 7, "FrSkyGpsAlt", Pmodel->FrSkyGpsAlt ) ;
	dumpSingleValue_text( 8, "FrSkyImperial", Pmodel->FrSkyImperial ) ;
	dumpSingleValue_text( 9, "FrSkyAltAlarm", Pmodel->FrSkyAltAlarm ) ;
	dumpSingleValue_text( 10, "modelVersion", Pmodel->modelVersion ) ;
	dumpSingleValue_text( 11, "protocol", Pmodel->protocol ) ;
	dumpSingleValue_text( 12, "country", Pmodel->country ) ;
	dumpSingleValue_text( 13, "ppmNCH", Pmodel->ppmNCH ) ;
	dumpSingleValue_text( 14, "thrTrim", Pmodel->thrTrim ) ;
	dumpSingleValue_text( 15, "extendedTrims", Pmodel->extendedTrims ) ;
	dumpSingleValue( 16, Pmodel->thrExpo ) ;
	dumpSingleValue( 17, Pmodel->frskyComPort ) ;
	dumpSingleValue( 18, Pmodel->DsmTelemetry ) ;
	dumpSingleValue( 19, Pmodel->useCustomStickNames ) ;
	dumpSingleValue( 20, Pmodel->trimInc ) ;
	dumpSingleValue( 21, Pmodel->ppmDelay ) ;
	dumpSingleValue( 22, Pmodel->trimSw ) ;
	dumpSingleValue( 23, Pmodel->beepANACenter ) ;
	dumpSingleValue( 24, Pmodel->pulsePol ) ;
	dumpSingleValue( 25, Pmodel->extendedLimits ) ;
	dumpSingleValue( 26, Pmodel->swashInvertELE ) ;
	dumpSingleValue( 27, Pmodel->swashInvertAIL ) ;
	dumpSingleValue( 28, Pmodel->swashInvertCOL ) ;
	dumpSingleValue( 29, Pmodel->swashType ) ;
	dumpSingleValue( 30, Pmodel->swashCollectiveSource ) ;
	dumpSingleValue( 31, Pmodel->swashRingValue ) ;
	dumpSingleValue( 32, Pmodel->ppmFrameLength ) ;
	IndentLevel = 1 ;
	for ( i = 0 ; i < MAX_SKYMIXERS+EXTRA_SKYMIXERS+SPARE_SKYMIXERS ; i += 1 )
	{
	  SKYMixData *md = mixAddress( i ) ;
		if ( md->destCh )
		{
			dumpNew( 33, "Mix" ) ;
			dumpValueLast( i ) ;
			dumpSingleValue_text( 0, "destCh", md->destCh ) ;
			dumpSingleValue_text( 1, "srcRaw", md->srcRaw ) ;
			dumpSingleValue_text( 2, "weight", md->weight ) ;
			dumpSingleValue_text( 3, "curve", md->curve ) ;
			dumpSingleDecimal_text( 4, "delayUp", md->delayUp, 1 ) ;
			dumpSingleDecimal_text( 5, "delayDown", md->delayDown, 1 ) ;
			dumpSingleDecimal_text( 6, "speedUp", md->speedUp, 1 ) ;
			dumpSingleDecimal_text( 7, "speedDown", md->speedDown, 1 ) ;
			dumpSingleValue_text( 8, "carryTrim", md->carryTrim ) ;
			dumpSingleValue_text( 9, "mltpx", md->mltpx ) ;
			dumpSingleValue_text( 10, "lateOffset", md->lateOffset ) ;
			dumpSingleValue_text( 11, "mixWarn", md->mixWarn ) ;
			dumpSingleValue_text( 12, "disableExpoDr", md->disableExpoDr ) ;
			dumpSingleValue_text( 13, "differential", md->differential ) ;
			dumpSingleValue_text( 14, "sOffset", md->sOffset ) ;
//			char text[10] ;
//			k = 0x80 ;
//			for ( j = 0 ; j < 8 ; j += 1 )
//			{
//				text[j] = (md->modeControl & k ) ? '1' : '0' ;
//				k >>= 1 ;
//			}
//			text[8] = 0 ;
//			f_printf(&g_dumpFile, "\n %d-%s,%s", 15, "Modes", text ) ;
			f_printf(&g_dumpFile, "\n %d-%s,%08B", 15, "Modes", md->modeControl ) ;
			dumpSingleValue_text( 16, "switchSource", md->switchSource ) ;
			dumpSingleValue_text( 17, "extWeight", md->extWeight ) ;
			dumpSingleValue_text( 18, "extOffset", md->extOffset ) ;
			dumpSingleValue_text( 19, "extDiff", md->extDiff ) ;
			dumpSingleValue_text( 20, "varForWeight", md->varForWeight ) ;
			dumpSingleValue_text( 21, "varForOffset", md->varForOffset ) ;
			dumpSingleValue_text( 22, "varForExpo", md->varForExpo ) ;
		}
		else
		{
			break ;
		}
	}
	for ( i = 0 ; i < NUM_SKYCHNOUT+EXTRA_SKYCHANNELS ; i += 1 )
	{
    LimitData *ld = &Pmodel->limitData[i];
		if ( i >= NUM_SKYCHNOUT )
		{
			ld = &Pmodel->elimitData[i-NUM_SKYCHNOUT] ;
		}
		dumpNew( 34, "Limits" ) ;
		dumpValueLast( i+1 ) ;
		dumpSingleValue_text( 0, "min", ld->min ) ;
		dumpSingleValue_text( 1, "max", ld->max ) ;
		dumpSingleValue_text( 2, "reverse", ld->revert ) ;
		dumpSingleDecimal_text( 3, "subTrim", ld->offset, 2 ) ;
	}
	IndentLevel = 0 ;

	IndentLevel = 1 ;

	for ( i = 0 ; i < 4 ; i += 1 )
	{
		ExpoData *ed = &Pmodel->expoData[i] ;
		dumpNew( 35, "Expo" ) ;
		dumpValueLast( i ) ;
		for ( j = 0 ; j < 3 ; j += 1 )
		{
			for ( k = 0 ; k < 4 ; k += 1 )
			{
				dumpSingleValue( j*4+k, ed->expo[j][k/2][k&1] ) ;
			}
		}
		dumpSingleValue( 12, ed->drSw1 ) ;
		dumpSingleValue( 13, ed->drSw2 ) ;
	}
	IndentLevel = 0 ;

	dumpNew( 36 ) ;
	dumpTextLast( "Trims" ) ;
	IndentLevel = 1 ;
	for ( i = 0 ; i < 4 ; i += 1 )
	{
		dumpSingleValue( i, Pmodel->trim[i] ) ;
	}
	IndentLevel = 0 ;

	IndentLevel = 1 ;
	for ( i = 0 ; i < MAX_CURVE5 ; i += 1 )
	{
		dumpNew( 37, "Curve5" ) ;
		dumpValueLast( i ) ;
		for ( j = 0 ; j < 5 ; j += 1 )
		{
			dumpSingleValue( j, Pmodel->curves5[i][j] ) ;
		}
	}
	for ( i = 0 ; i < MAX_CURVE9 ; i += 1 )
	{
		dumpNew( 38, "Curve9" ) ;
		dumpValueLast( i ) ;
		for ( j = 0 ; j < 9 ; j += 1 )
		{
			dumpSingleValue( j, Pmodel->curves9[i][j] ) ;
		}
	}
	dumpNew( 39, "CurveXY" ) ;
	dumpValueLast( 0 ) ;
	for ( j = 0 ; j < 18 ; j += 1 )
	{
		dumpSingleValue( j, Pmodel->curvexy[j] ) ;
	}
	dumpNew( 39, "CurveXY" ) ;
	dumpValueLast( 1 ) ;
	for ( j = 0 ; j < 18 ; j += 1 )
	{
		dumpSingleValue( j, Pmodel->curve2xy[j] ) ;
	}
	dumpNew( 131, "Curve6" ) ;
	dumpValueLast( 0 ) ;
	for ( j = 0 ; j < 6 ; j += 1 )
	{
		dumpSingleValue( j, Pmodel->curve6[j] ) ;
	}


	for ( i = 0 ; i < NUM_SKYCSW ; i += 1 )
	{
		SKYCSwData *cd = &Pmodel->customSw[i] ;
		dumpNew( 40, "Cswitch" ) ;
		dumpValueLast( i ) ;
		dumpSingleValue_text( 0, "v1", cd->v1 ) ;
		dumpSingleValue_text( 1, "v2", cd->v2 ) ;
		dumpSingleValue_text( 2, "func", cd->func ) ;
		dumpSingleValue_text( 3, "exfunc", cd->exfunc ) ;
		dumpSingleValue_text( 4, "andSw", cd->andsw ) ;
		dumpSingleValue_text( 5, "bitAndV3", cd->bitAndV3 ) ;
		dumpSingleValue_text( 6, "delay", Pmodel->switchDelay[i] ) ;
	}

	IndentLevel = 0 ;

	dumpSingleValue_text( 41, "frSkyVoltThreshold", Pmodel->frSkyVoltThreshold ) ;
	dumpSingleValue_text( 42, "bt_telemetry", Pmodel->bt_telemetry ) ;
	dumpSingleValue_text( 43, "numVoice", Pmodel->numVoice ) ;

	IndentLevel = 1 ;


	for ( i = 0 ; i < 32 ; i += 1 )
  {
		SKYSafetySwData *sd = &Pmodel->safetySw[i] ;

		if ( i < (uint32_t)(NUM_SKYCHNOUT - Pmodel->numVoice) )
		{
			dumpNew( 44, "safetySw" ) ;
			dumpValueLast( i ) ;
			dumpSingleValue_text( 0, "switch", sd->opt.ss.swtch ) ;
			dumpSingleValue_text( 1, "mode", sd->opt.ss.mode ) ;
			dumpSingleValue_text( 2, "source", sd->opt.ss.source ) ;
			dumpSingleValue_text( 3, "val", sd->opt.ss.val ) ;
			dumpSingleValue_text( 4, "tune", sd->opt.ss.tune ) ;
		}
		else
		{
			dumpNew( 45, "voiceSw" ) ;
			dumpValueLast( i ) ;
			dumpSingleValue_text( 0, "switch", sd->opt.vs.vswtch ) ;
			dumpSingleValue_text( 1, "mode", sd->opt.vs.vmode ) ;
			dumpSingleValue_text( 2, "source", sd->opt.vs.vval ) ;
		}
	}

	for ( i = 0 ; i < 2 ; i += 1 )
  {
		SKYFrSkyChannelData *fd = &Pmodel->frsky.channels[i] ;
		dumpNew( 46, "voltScales" ) ;
		dumpValueLast( i ) ;
			
		dumpSingleValue_text( 0, "lratio", fd->lratio ) ;
		dumpSingleValue_text( 1, "gain", fd->gain ) ;
		dumpSingleValue_text( 2, "ratio3_4", fd->ratio3_4 ) ;
		dumpSingleValue_text( 3, "units3_4", fd->units3_4 ) ;
		dumpSingleValue_text( 4, "units", fd->units ) ;
	}
	
	for ( i = 0 ; i < 2 ; i += 1 )
	{
  	TimerMode *td = &Pmodel->timer[i] ;
		dumpNew( 47, "Timer" ) ;
		dumpValueLast( i+1 ) ;
			
		dumpSingleValue_text( 0, "tmrModeA", td->tmrModeA ) ;
		dumpSingleValue_text( 1, "tmrDir", td->tmrDir ) ;
		dumpSingleValue_text( 2, "tmrModeB", td->tmrModeB ) ;
		dumpSingleValue_text( 3, "tmrVal", td->tmrVal ) ;
    if ( i == 0 )
		{
			dumpSingleValue_text( 4, "AutoReset", Pmodel->t1AutoReset ) ;
			dumpSingleValue_text( 5, "RstSw", Pmodel->timer1RstSw ) ;
			dumpSingleValue_text( 6, "Cdown", Pmodel->timer1Cdown ) ;
			dumpSingleValue_text( 7, "Minbeep", Pmodel->timer1Mbeep ) ;
			dumpSingleValue_text( 8, "Haptic", Pmodel->timer1Haptic ) ;
		}
		else
		{
			dumpSingleValue_text( 4, "AutoReset", Pmodel->t2AutoReset ) ;
			dumpSingleValue_text( 5, "RstSw", Pmodel->timer2RstSw ) ;
			dumpSingleValue_text( 6, "Cdown", Pmodel->timer2Cdown ) ;
			dumpSingleValue_text( 7, "Minbeep", Pmodel->timer2Mbeep ) ;
			dumpSingleValue_text( 8, "Haptic", Pmodel->timer2Haptic ) ;
		}
	}              

	dumpNew( 150 ) ;
	dumpValueLast( 0 ) ;
	dumpSingleValue_text( 0, "CapacityAlarm", Pmodel->frskyAlarms.alarmData[0].frskyAlarmLimit ) ;
	dumpSingleValue_text( 1, "CapacitySound", Pmodel->frskyAlarms.alarmData[0].frskyAlarmSound ) ;

	dumpNew( 48, "CustomDisp" ) ;
	dumpValueLast( 0 ) ;
	for ( i = 0 ; i < 6 ; i += 1 )
	{
		dumpSingleValue( i, Pmodel->customDisplayIndex[i] ) ;
	}
	for ( i = 0 ; i < 7 ; i += 1 )
	{
		dumpSingleValue( i+6, Pmodel->customDisplay1Extra[i] ) ;
	}
	dumpNew( 48 ) ;
	dumpValueLast( 1 ) ;
	for ( i = 0 ; i < 6 ; i += 1 )
	{
		dumpSingleValue( i, Pmodel->customDisplay2Index[i] ) ;
	}
	for ( i = 0 ; i < 7 ; i += 1 )
	{
		dumpSingleValue( i+6, Pmodel->customDisplay2Extra[i] ) ;
	}
	 
//  FunctionData   functionData[NUM_FSW];			// Currently unused (16*10 = 160 bytes)
	
	IndentLevel = 1 ;

	for ( i = 0 ; i < MAX_MODES+1 ; i += 1 )
	{
		PhaseData *pd = (i < MAX_MODES ) ? &Pmodel->phaseData[i] : &Pmodel->xphaseData ;
		dumpNew( 49, "Mode" ) ;
		dumpValueLast( i ) ;
		for ( j = 0 ; j < 4 ; j += 1 )
		{
			IndentLevel = 1 ;
			dumpSingleValue_text( 0, "Trim", j ) ;
			IndentLevel = 2 ;
			dumpSingleValue_text( 0, "Tmode", pd->trim[j].mode ) ;
			dumpSingleValue_text( 1, "Tvalue", pd->trim[j].value ) ;
		}
		IndentLevel = 1 ;
		dumpSingleValue_text( 1, "Switch", pd->swtch ) ;
		dumpSingleValue_text( 2, "Switch2", pd->swtch2 ) ;
		dumpSingleValue_text( 3, "fadeIn", pd->fadeIn ) ;
		dumpSingleValue_text( 4, "fadeOut", pd->fadeOut ) ;

		ncpystr( (uint8_t *)ltext, (uint8_t *)pd->name, 6 ) ;
		ltext[6] = 0 ;
		f_printf(&g_dumpFile, "\n%s%d,%s", indentText(), 5, ltext ) ;
	}

	IndentLevel = 1 ;
	for ( i = 0 ; i < MAX_GVARS ; i += 1 )
	{
		dumpNew( 50,"Gvar" ) ;
		dumpValueLast( i ) ;
		dumpSingleValue_text( 0, "gvar", Pmodel->gvars[i].gvar ) ;
		dumpSingleValue_text( 1, "source", Pmodel->gvars[i].gvsource ) ;
		dumpSingleValue_text( 2, "switch", Pmodel->gvswitch[i] ) ;
	}
	 
	IndentLevel = 0 ;

	dumpSingleValue_text( 51, "numBlades", Pmodel->numBlades ) ;
	dumpSingleValue_text( 52, "startChannel", Pmodel->startChannel ) ;
	dumpSingleValue_text( 53, "startPPM2channel", Pmodel->startPPM2channel ) ;
	dumpSingleValue_text( 54, "ppm2NCH", Pmodel->ppm2NCH ) ;
	dumpSingleValue_text( 55, "sub_trim_limit", Pmodel->sub_trim_limit ) ;
	dumpSingleDecimal_text( 56, "FASoffset", Pmodel->FASoffset, 1 ) ;
	
	IndentLevel = 1 ;
		 
	dumpNew( 57, "Vario" ) ;
	dumpValueLast( 0 ) ;

	dumpSingleValue_text( 0, "source", Pmodel->varioData.varioSource ) ;
	dumpSingleValue_text( 1, "switch", Pmodel->varioData.swtch ) ;
	dumpSingleValue_text( 2, "sinkTones", Pmodel->varioData.sinkTones ) ;
	dumpSingleValue_text( 3, "param", Pmodel->varioData.param ) ;
	dumpSingleValue_text( 4, "baseFrequency", Pmodel->varioExtraData.baseFrequency ) ;
	dumpSingleValue_text( 5, "offsetFrequency", Pmodel->varioExtraData.offsetFrequency ) ;
	dumpSingleValue_text( 6, "volume", Pmodel->varioExtraData.volume ) ;
	 
	IndentLevel = 0 ;
	dumpSingleValue_text( 58, "anaVolume", Pmodel->anaVolume ) ;
	IndentLevel = 1 ;
	
	dumpNew( 59, "accessFailsafe" ) ;
	dumpValueLast( 0 ) ;
	for ( i = 0 ; i < 8 ; i += 1 )
	{
		dumpSingleValue( i, Pmodel->accessFailsafe[0][i] ) ;
	}
	dumpNew( 59, "accessFailsafe" ) ;
	dumpValueLast( 1 ) ;
	for ( i = 0 ; i < 8 ; i += 1 )
	{
		dumpSingleValue( i, Pmodel->accessFailsafe[1][i] ) ;
	}
	
	IndentLevel = 0 ;

	dumpSingleValue_text( 60, "logSwitch", Pmodel->logSwitch ) ;
	dumpSingleValue_text( 61, "logRate", Pmodel->logRate ) ;
	dumpSingleValue_text( 62, "logNew", Pmodel->logNew ) ;
	dumpSingleValue_text( 63, "beepANACenter3", Pmodel->beepANACenter3 ) ;
	dumpSingleValue_text( 64, "xprotocol", Pmodel->xprotocol ) ;
	dumpSingleValue_text( 65, "xcountry", Pmodel->xcountry ) ;
//	dumpSingleValue_text( 66, "t1AutoReset", Pmodel->t1AutoReset ) ;
//	dumpSingleValue_text( 67, "t2AutoReset", Pmodel->t2AutoReset ) ;
	dumpSingleValue_text( 68, "xppmNCH", Pmodel->xppmNCH ) ;
	dumpSingleValue_text( 69, "xppmDelay", Pmodel->xppmDelay ) ;

	dumpSingleValue_text( 70, "xpulsePol", Pmodel->xpulsePol ) ;
	dumpSingleValue_text( 71, "trainPulsePol", Pmodel->trainPulsePol ) ;
	dumpSingleValue_text( 72, "dsmAasRssi", Pmodel->dsmAasRssi ) ;
	dumpSingleValue_text( 73, "telemetry2RxInvert", Pmodel->telemetry2RxInvert ) ;
	dumpSingleValue_text( 74, "dsmVario", Pmodel->dsmVario ) ;
	dumpSingleValue_text( 75, "ForceTelemetryType", Pmodel->ForceTelemetryType ) ;
	dumpSingleValue_text( 76, "xppmFrameLength", Pmodel->xppmFrameLength ) ;
	dumpSingleValue_text( 77, "xstartChannel", Pmodel->xstartChannel ) ;
	dumpSingleValue_text( 78, "pxxRxNum", Pmodel->pxxRxNum ) ;
	dumpSingleValue_text( 79, "dsmMode", Pmodel->dsmMode ) ;

	dumpSingleValue_text( 80, "xPxxRxNum", Pmodel->xPxxRxNum ) ;

	f_printf(&g_dumpFile, "\n%d-%s,%016B", 81, "SwitchWarning", Pmodel->modelswitchWarningStates ) ;

	dumpSingleValue_text( 82, "enRSSIwarn", Pmodel->enRssiOrange ) ;
	dumpSingleValue_text( 83, "RSSIwarn", Pmodel->rssiOrange ) ;
	dumpSingleValue_text( 84, "enRSSIcritical", Pmodel->enRssiRed ) ;
	dumpSingleValue_text( 85, "RSSIcritical", Pmodel->rssiRed ) ;
	dumpSingleValue_text( 86, "rxVratio", Pmodel->rxVratio ) ;
	dumpSingleValue_text( 87, "currentSource", Pmodel->currentSource ) ;
	dumpSingleValue_text( 88, "altSource", Pmodel->altSource ) ;

	IndentLevel = 1 ;
	for ( i = 0 ; i < NUM_SCALERS ; i += 1 )
	{
		ScaleData *sd = &Pmodel->Scalers[i] ;
		ExtScaleData *ed = &Pmodel->eScalers[i] ;
		dumpNew( 89, "Scaler" ) ;
		dumpValueLast( i+1 ) ;
		dumpSingleValue_text( 0, "source", sd->source ) ;
		dumpSingleValue_text( 1, "offset", sd->offset ) ;
		dumpSingleValue_text( 2, "multx", sd->multx ) ;
		dumpSingleValue_text( 3, "mult", sd->mult ) ;
		dumpSingleValue_text( 4, "divx", sd->divx ) ;
		dumpSingleValue_text( 5, "div", sd->div ) ;
		dumpSingleValue_text( 6, "unit", sd->unit ) ;
		dumpSingleValue_text( 7, "neg", sd->neg ) ;
		dumpSingleValue_text( 8, "precision", sd->precision ) ;
		dumpSingleValue_text( 9, "offsetLast", sd->offsetLast ) ;
		dumpSingleValue_text( 10, "exFunction", sd->exFunction ) ;
		ncpystr( (uint8_t *)ltext, (uint8_t *)sd->name, 4 ) ;
		ltext[4] = 0 ;
		f_printf(&g_dumpFile, "\n%s%d,%s", indentText(), 11, ltext ) ;
		dumpSingleValue_text( 12, "mod", ed->mod ) ;
		dumpSingleValue_text( 13, "dest", ed->dest ) ;
		dumpSingleValue_text( 14, "exSource", ed->exSource ) ;
	}

	IndentLevel = 1 ;
	dumpNew( 90, "dsmLinkData" ) ;
	dumpValueLast( 0 ) ;
	dumpSingleValue_text( 0, "sourceWarn", Pmodel->dsmLinkData.sourceWarn ) ;
	dumpSingleValue_text( 1, "levelWarn", Pmodel->dsmLinkData.levelWarn ) ;
	dumpSingleValue_text( 2, "sourceCritical", Pmodel->dsmLinkData.sourceCritical ) ;
	dumpSingleValue_text( 3, "levelCritical", Pmodel->dsmLinkData.levelCritical ) ;

	IndentLevel = 0 ;
//	dumpSingleValue_text( 91, "timer1RstSw", Pmodel->timer1RstSw ) ;
//	dumpSingleValue_text( 92, "timer2RstSw", Pmodel->timer2RstSw ) ;
//	dumpSingleValue_text( 93, "timer1Cdown", Pmodel->timer1Cdown ) ;
//	dumpSingleValue_text( 94, "timer2Cdown", Pmodel->timer2Cdown ) ;
//	dumpSingleValue_text( 95, "timer1Mbeep", Pmodel->timer1Mbeep ) ;
//	dumpSingleValue_text( 96, "timer2Mbeep", Pmodel->timer2Mbeep ) ;
//	dumpSingleValue_text( 97, "timer1Haptic", Pmodel->timer1Haptic ) ;
//	dumpSingleValue_text( 98, "timer2Haptic", Pmodel->timer2Haptic ) ;
	dumpSingleValue_text( 99, "mlightSw", Pmodel->mlightSw ) ;
	dumpSingleValue_text( 100, "ppmOpenDrain", Pmodel->ppmOpenDrain ) ;
	 
	dumpNew( 101, "VoiceName" ) ;
	ncpystr( (uint8_t *)ltext, (uint8_t *)Pmodel->modelVname, VOICE_NAME_SIZE ) ;
	ltext[VOICE_NAME_SIZE] = 0 ;
	dumpTextLast( ltext ) ;

	IndentLevel = 1 ;
	for ( i = 0 ; i < NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS ; i += 1 )
	{
		VoiceAlarmData *vd = (i < NUM_VOICE_ALARMS) ? &Pmodel->vad[i] : &Pmodel->vadx[i-NUM_VOICE_ALARMS] ;
		dumpNew( 102, "VoiceAlert" ) ;
		dumpValueLast( i ) ;
		dumpSingleValue_text( 0, "source", vd->source ) ;
		dumpSingleValue_text( 1, "func", vd->func ) ;
		dumpSingleValue_text( 2, "switch", vd->swtch ) ;
		dumpSingleValue_text( 3, "rate", vd->rate ) ;
		dumpSingleValue_text( 4, "fnameType", vd->fnameType ) ;
		dumpSingleValue_text( 5, "haptic", vd->haptic ) ;
		dumpSingleValue_text( 6, "vsource", vd->vsource ) ;
		dumpSingleValue_text( 7, "mute", vd->mute ) ;
		dumpSingleValue_text( 8, "delay", vd->delay ) ;
		dumpSingleValue_text( 9, "offset", vd->offset ) ;
		uint16_t temp = (vd->fnameType == 0) ? 0 : vd->file.vfile ;
		if ( vd->fnameType == 1 || vd->fnameType == 4 )
		{
			ncpystr( (uint8_t *)ltext, (uint8_t *)Pmodel->modelVname, 8 ) ;
			ltext[8] = 0 ;
			f_printf(&g_dumpFile, "\n%s%d,%s", indentText(), 10, ltext ) ;
		}
		else
		{
			dumpSingleValue_text( 10, "index", temp ) ;
		}
	} 
	IndentLevel = 0 ;
	
	dumpSingleValue_text( 103, "mview", Pmodel->mview ) ;
	dumpSingleValue_text( 104, "telemetryProtocol", Pmodel->telemetryProtocol ) ;
	dumpSingleValue_text( 105, "com2Function", Pmodel->com2Function ) ;
	dumpSingleValue_text( 106, "com3Function", Pmodel->com3Function ) ;
	dumpSingleValue_text( 107, "telemetryBaudrate", Pmodel->telemetryBaudrate ) ;
	dumpSingleValue_text( 108, "com2Baudrate", Pmodel->com2Baudrate ) ;
	dumpSingleValue_text( 119, "throttleSource", Pmodel->throttleSource ) ;
	dumpSingleValue_text( 110, "throttleIdle", Pmodel->throttleIdle ) ;
	dumpSingleValue_text( 111, "throttleReversed", Pmodel->throttleReversed ) ;
	dumpSingleValue_text( 112, "disableThrottleCheck", Pmodel->disableThrottleCheck ) ;
	dumpSingleValue_text( 113, "basic_lua", Pmodel->basic_lua ) ;
	dumpSingleValue_text( 114, "instaTrimToTrims", Pmodel->instaTrimToTrims ) ;
	dumpSingleValue_text( 115, "BTfunction", Pmodel->BTfunction ) ;
	
	f_printf(&g_dumpFile, "\n116-totalTime,%lu", Pmodel->totalTime ) ;

	f_printf(&g_dumpFile, "\n%d-%s,%016B", 117, "xmodelswitchWarningStates", Pmodel->xmodelswitchWarningStates ) ;
	f_printf(&g_dumpFile, "\n%d-%s,%08B", 118, "ymodelswitchWarningStates", Pmodel->ymodelswitchWarningStates ) ;
	 
//	uint8_t customDisplay2Index[6] ;
	
	IndentLevel = 1 ;
	for ( i = 0 ; i < NUM_GVAR_ADJUST + EXTRA_GVAR_ADJUST ; i += 1 )
	{
		GvarAdjust *gd = (i < NUM_GVAR_ADJUST) ? &Pmodel->gvarAdjuster[i] : &Pmodel->egvarAdjuster[i-NUM_GVAR_ADJUST] ;
		dumpNew( 119, "GvarAdjuster" ) ;
		dumpValueLast( i ) ;
		dumpSingleValue_text( 0, "function", gd->function ) ;
		dumpSingleValue_text( 1, "gvarIndex", gd->gvarIndex ) ;
		dumpSingleValue_text( 2, "switch", gd->swtch ) ;
		dumpSingleValue_text( 3, "switch_value", gd->switch_value ) ;
	}

	f_printf(&g_dumpFile, "\n%d-%s,%016B", 120, "modelswitchWarningDisables", Pmodel->modelswitchWarningDisables ) ;
	f_printf(&g_dumpFile, "\n%d-%s,%016B", 121, "xmodelswitchWarningDisables", Pmodel->xmodelswitchWarningDisables ) ;
	f_printf(&g_dumpFile, "\n%d-%s,%08B", 122, "ymodelswitchWarningDisables", Pmodel->ymodelswitchWarningDisables ) ;

	dumpNew( 123, "imageName" ) ;
	ncpystr( (uint8_t *)ltext, (uint8_t *)Pmodel->modelImageName, sizeof(g_model.modelImageName) ) ;
	ltext[sizeof(g_model.modelImageName)] = 0 ;
	dumpTextLast( ltext ) ;

	IndentLevel = 0 ;

	dumpSingleValue_text( 124, "option_protocol", Pmodel->option_protocol ) ;
	dumpSingleValue_text( 125, "sub_protocol", Pmodel->sub_protocol ) ;
	dumpSingleValue_text( 126, "xsub_protocol", Pmodel->xsub_protocol ) ;

	IndentLevel = 1 ;
		 
	dumpNew( 127, "CustomCheckData" ) ;
	dumpValueLast( 0 ) ;
	dumpSingleValue_text( 0, "source", Pmodel->customCheck.source ) ;
	dumpSingleValue_text( 1, "min", Pmodel->customCheck.min ) ;
	dumpSingleValue_text( 2, "max", Pmodel->customCheck.max ) ;

	IndentLevel = 0 ;

	dumpSingleValue_text( 128, "btDefaultAddress", Pmodel->btDefaultAddress ) ;
	dumpSingleValue_text( 129, "xoption_protocol", Pmodel->xoption_protocol ) ;
	dumpSingleValue_text( 130, "trainerProfile", Pmodel->trainerProfile ) ;

	IndentLevel = 1 ;
	
	dumpNew( 132, "Music" ) ;
	dumpValueLast( 0 ) ;
	dumpSingleValue_text( 0, "musicStartSwitch", Pmodel->musicData.musicStartSwitch ) ;
	dumpSingleValue_text( 1, "musicPauseSwitch", Pmodel->musicData.musicPauseSwitch ) ;
	dumpSingleValue_text( 2, "musicPrevSwitch", Pmodel->musicData.musicPrevSwitch ) ;
	dumpSingleValue_text( 3, "musicNextSwitch", Pmodel->musicData.musicNextSwitch ) ;
	
	dumpNew( 133 ) ;
	dumpValueLast( 0 ) ;
	for ( i = 0 ; i < 4 ; i += 1 )
	{
		f_printf(&g_dumpFile, "\n %d-%s,%016B", i*2, "LogDisable", Pmodel->LogDisable[i] & 0x0000FFFF ) ;
		f_printf(&g_dumpFile, "\n %d-%s,%016B", i*2+1, "LogDisable", Pmodel->LogDisable[i] >> 16 ) ;
	}
	
	dumpNew( 134, "FailsafeMode" ) ;
	dumpValueLast( 0 ) ;
	dumpSingleValue_text( 0, "fmode", Pmodel->failsafeMode[0] ) ;
	dumpSingleValue_text( 1, "fmode", Pmodel->failsafeMode[1] ) ;

	for ( i = 0 ; i < 2 ; i += 1 )
	{
		struct t_module *pm = &Pmodel->Module[i] ;
		dumpNew( 135, "Module" ) ;
		dumpValueLast( i ) ;

		dumpSingleValue_text( 0, "protocol", pm->protocol ) ;
		dumpSingleValue_text( 1, "country", pm->country ) ;
		dumpSingleValue_text( 2, "ppmOpenDrain", pm->ppmOpenDrain ) ;
		dumpSingleValue_text( 3, "pulsePol", pm->pulsePol ) ;
		dumpSingleValue_text( 4, "channels", pm->channels ) ;
		dumpSingleValue_text( 5, "startChannel", pm->startChannel ) ;
		dumpSingleValue_text( 6, "sub_protocol", pm->sub_protocol ) ;
		dumpSingleValue_text( 7, "RxNum", pm->pxxRxNum ) ;
		dumpSingleValue_text( 8, "ppmDelay", pm->ppmDelay ) ;
		dumpSingleValue_text( 9, "ppmFrameLength", pm->ppmFrameLength ) ;
		dumpSingleValue_text( 10, "option_protocol", pm->option_protocol ) ;
		dumpSingleValue_text( 11, "failsafeMode", pm->failsafeMode ) ;
		dumpSingleValue_text( 12, "r9mPower", pm->r9mPower ) ;
		dumpSingleValue_text( 13, "pxxDoubleRate", pm->pxxDoubleRate ) ;
		dumpSingleValue_text( 14, "pxxHeartbeatPB14", pm->pxxHeartbeatPB14 ) ;
		
		dumpSingleValue_text( 15, "failsafe", 0 ) ;
		IndentLevel = 2 ;
		for ( j = 0 ; j < 16 ; j += 1 )
		{
			dumpSingleValue_text( j, "fs", pm->failsafe[j] ) ;
		}
		IndentLevel = 1 ;

		dumpSingleValue_text( 16, "externalAntenna", pm->externalAntenna ) ;
		dumpSingleValue_text( 17, "r9MflexMode", pm->r9MflexMode ) ;
		dumpSingleValue_text( 18, "highChannels", pm->highChannels ) ;
		dumpSingleValue_text( 19, "disableTelemetry", pm->disableTelemetry ) ;

		dumpSingleValue_text( 20, "exsub_protocol", pm->exsub_protocol ) ;
		dumpSingleValue_text( 21, "multiDisableTelemetry", pm->multiDisableTelemetry ) ;
		dumpSingleValue_text( 22, "extR9Mlite", pm->extR9Mlite ) ;
	}
	
	IndentLevel = 0 ;

	dumpSingleValue_text( 136, "telemetryTimeout", Pmodel->telemetryTimeout ) ;
	dumpSingleValue_text( 137, "throttleIdleScale", Pmodel->throttleIdleScale ) ;
	dumpNew( 138, "bgScript" ) ;
	ncpystr( (uint8_t *)ltext, (uint8_t *)Pmodel->backgroundScript, 8 ) ;
	ltext[8] = 0 ;
	dumpTextLast( ltext ) ;
	dumpSingleValue_text( 139, "voiceFlushSwitch", Pmodel->voiceFlushSwitch ) ;

	IndentLevel = 1 ;

	dumpNew( 140, "cellScalers" ) ;
	dumpValueLast( 0 ) ;
	for ( i = 0 ; i < 12 ; i += 1 )
	{
		dumpSingleValue_text( i, "cScale", Pmodel->cellScalers[i] ) ;
	}

	dumpNew( 141, "customTelNames" ) ;
	dumpValueLast( 0 ) ;
	for ( i = 0 ; i < 6 ; i += 1 )
	{
		ncpystr( (uint8_t *)ltext, (uint8_t *)&Pmodel->customTelemetryNames[i*4], 4 ) ;
		ltext[4] = 0 ;
		dumpSingleText_text( i, "name", ltext ) ;
	}
	for ( i = 0 ; i < 4 ; i += 1 )
	{
		ncpystr( (uint8_t *)ltext, (uint8_t *)&Pmodel->customTelemetryNames2[i*4], 4 ) ;
		ltext[4] = 0 ;
		dumpSingleText_text( i+6, "name", ltext ) ;
	}

	IndentLevel = 0 ;

	dumpSingleValue_text( 142, "extraSensors", Pmodel->extraSensors ) ;

	IndentLevel = 1 ;

	for ( i = 0 ; i < NUMBER_EXTRA_IDS ; i += 1 )
	{
		ExtraId *ed = &Pmodel->extraId[i] ;
		dumpNew( 143, "ExtraID" ) ;
		dumpValueLast( i ) ;
		dumpSingleValue_text( 0, "id", ed->id ) ;
		dumpSingleValue_text( 1, "dest", ed->dest ) ;
	}

	for ( i = 0 ; i < 2 ; i += 1 )
	{
		t_access *ad = &Pmodel->Access[i] ;
		dumpNew( 144, "Access" ) ;
		dumpValueLast( i ) ;
		dumpSingleValue_text( 0, "numChannels", ad->numChannels ) ;
		dumpSingleValue_text( 1, "type", ad->type ) ;
		for ( j = 0 ; j < 3 ; j += 1 )
		{
			IndentLevel = 1 ;
			dumpSingleValue_text( 0, "RxName", j ) ;
			IndentLevel = 2 ;
			ncpystr( (uint8_t *)ltext, (uint8_t *)ad->receiverName[j], 8 ) ;
			ltext[8] = 0 ;
			f_printf(&g_dumpFile, "\n%s%d,%s", indentText(), j, ltext ) ;
		}
		IndentLevel = 1 ;
	}
	IndentLevel = 0 ;

	dumpSingleValue_text( 146, "UseVars", Pmodel->vars ) ;

	IndentLevel = 1 ;
	for ( i = 0 ; i < 2 ; i += 1 )
	{
		t_hiResDisplay *ph = &Pmodel->hiresDisplay[i] ;
		dumpNew( 147, "HiResDisp" ) ;
		dumpValueLast( i ) ;
		IndentLevel = 1 ;
		dumpSingleValue_text( 0, "layout", ph->layout ) ;
		dumpSingleValue_text( 1, "options", ph->options ) ;
		for ( j = 0 ; j < 6 ; j += 1 )
		{
			IndentLevel = 1 ;
			dumpSingleValue_text( 2, "Box", j ) ;
			IndentLevel = 2 ;
			dumpSingleValue_text( 0, "item", ph->boxes[j].item ) ;
			dumpSingleValue_text( 1, "type", ph->boxes[j].type ) ;
			dumpSingleValue_text( 2, "colour", ph->boxes[j].colour ) ;
			dumpSingleValue_text( 3, "bgColour", ph->boxes[j].bgColour ) ;
		}
		IndentLevel = 1 ;
	}

	IndentLevel = 1 ;
	for ( i = 0 ; i < NUM_INPUT_LINES ; i += 1 )
	{
		te_InputsData *pi = &Pmodel->inputs[i] ;
		if ( pi->chn )
		{
			dumpNew( 148, "Input" ) ;
			dumpValueLast( i ) ;
			dumpSingleValue_text( 0, "srcRaw", pi->srcRaw ) ;
			dumpSingleValue_text( 1, "weight", pi->weight ) ;
			dumpSingleValue_text( 2, "switch", pi->swtch ) ;
			dumpSingleValue_text( 3, "carryTrim", pi->carryTrim ) ;
			dumpSingleValue_text( 4, "mode", pi->mode ) ;
			dumpSingleValue_text( 5, "side", pi->side ) ;
			dumpSingleValue_text( 6, "chn", pi->chn ) ;
			dumpSingleValue_text( 7, "varForWeight", pi->varForWeight ) ;
			dumpSingleValue_text( 8, "varForOffset", pi->varForOffset ) ;
			dumpSingleValue_text( 9, "varForCurve", pi->varForCurve ) ;
//			dumpSingleValue_text( 10, "flightModes", pi->flightModes ) ;
			f_printf(&g_dumpFile, "\n %d-%s,%08B", 10, "flightModes", pi->flightModes ) ;
			dumpSingleValue_text( 11, "offset", pi->offset ) ;
			dumpSingleValue_text( 12, "curve", pi->curve ) ;
		}
		else
		{
			break ;
		}
	}

	i = 0 ;
	t_varPack *pv = (t_varPack *) &Pmodel->varStore[0] ;
	while ( i < NUM_VARS )
	{
		dumpNew( 149, "Var" ) ;
		dumpValueLast( i ) ;
		dumpSingleValue_text( 0, "min", pv->min ) ;
		dumpSingleValue_text( 1, "max", pv->max ) ;
		dumpSingleValue_text( 2, "value", pv->value ) ;
		ncpystr( (uint8_t *)ltext, (uint8_t *)pv->name, 6 ) ;
		ltext[6] = 0 ;
		f_printf(&g_dumpFile, "\n%s%d,%s", indentText(), 3, ltext ) ;
		t_valVarPack *pVal = (struct t_valVarPack *)( pv+1) ;
		j = 0 ;
		while ( pVal->tag == 1 )
		{
			if ( (uint8_t *)pVal > (uint8_t *) &Pmodel->varStore[VAR_STORAGE_UINTS-1] )
			{
				break ;
			}
			IndentLevel = 1 ;
			dumpSingleValue_text( 4, "Opt", j ) ;
			IndentLevel = 2 ;
			dumpSingleValue_text( 0, "value", pVal->value ) ;
			dumpSingleValue_text( 1, "valIsSource", pVal->valIsSource ) ;
			dumpSingleValue_text( 2, "item", pVal->item ) ;
			dumpSingleValue_text( 3, "category", pVal->category ) ;
			j += 1 ;
			pVal += 1 ;
		}
		pv = (t_varPack *) pVal ;
		if ( (uint8_t *)pv > (uint8_t *) &Pmodel->varStore[VAR_STORAGE_UINTS-1] )
		{
			break ;
		}
		IndentLevel = 1 ;
		t_actVarPack *pAct = (struct t_actVarPack *)( pv ) ;
		j = 0 ;
		while ( pAct->tag == 2 )
		{
			if ( (uint8_t *)pAct > (uint8_t *) &Pmodel->varStore[VAR_STORAGE_UINTS-1] )
			{
				break ;
			}
			IndentLevel = 1 ;
			dumpSingleValue_text( 4, "Act", j ) ;
			IndentLevel = 2 ;
			dumpSingleValue_text( 0, "value", pAct->value ) ;
			dumpSingleValue_text( 1, "timer", pAct->timer ) ;
			dumpSingleValue_text( 2, "item", pAct->item ) ;
			dumpSingleValue_text( 3, "function", pAct->function ) ;
			dumpSingleValue_text( 4, "category", pAct->category ) ;
			j += 1 ;
			pAct += 1 ;
		}
		pv = (t_varPack *) pAct ;
		if ( (uint8_t *)pv > (uint8_t *) &Pmodel->varStore[VAR_STORAGE_UINTS-1] )
		{
			break ;
		}
		IndentLevel = 1 ;
		i += 1 ;
	}

	f_printf(&g_dumpFile, "\n" ) ;
}



