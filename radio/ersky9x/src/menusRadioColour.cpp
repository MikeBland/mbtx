/****************************************************************************
*  Copyright (c) 2025 by Michael Blandford. All rights reserved.
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

// Externals that may need placing somewhere else



void menuColour( uint8_t event, uint8_t mode )
{
	uint32_t red ;
	uint32_t green ;
	uint32_t blue ;
	uint16_t colour ;

	if ( mode )
	{
		touchMenuTitle( (char *)XPSTR("Text Colour") ) ;
		colour = g_eeGeneral.textColour ;
	}
	else
	{
		touchMenuTitle( (char *)XPSTR("Background Colour") ) ;
		colour = g_eeGeneral.backgroundColour ;
	}
	
	uint32_t rows = 3 ;

	static MState2 mstate2 ;
	mstate2.check_columns(event, rows-1) ;

  int8_t  sub    = mstate2.m_posVert ;
		
	red = colour >> 11 ;
	green = ( colour >> 6 ) & 0x1F ;
	blue = colour & 0x1F ;

	uint8_t subN = 0 ;
	LcdFlags attr ;

#ifdef TOUCH
	if ( TouchUpdated )
	{
		if ( TouchControl.event == TEVT_DOWN )
		{
				
		}
		else if ( TouchControl.event == TEVT_UP )
		{
			if ( ( TouchControl.x <= TRIGHT*TSCALE ) && (TouchControl.x >= TMID*TSCALE) && !s_editMode )
			{
				uint32_t vert = TouchControl.y ;
				vert -= TTOP*TSCALE ;
				vert /= TFH*TSCALE ;
				if ( vert >= TLINES - 1 )
				{
					vert = TLINES - 2 ;
				}
				if ( vert < rows )
				{
					TouchControl.itemSelected = vert+1 ;
					TouchUpdated = 0 ;
					sub = mstate2.m_posVert = vert ;
				}
			}
		}
	}
#endif
	 
	coord_t y = TTOP ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Red"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, red, attr) ; //, attr ? ~hcolour : hcolour ) ;
  if ( attr )
	{
		CHECK_INCDEC_H_GENVAR_0( red, 31 ) ;
	}
	y += TFH ;
	subN += 1 ;

	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Green"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, green, attr) ; //, attr ? ~hcolour : hcolour ) ;
  if ( attr )
	{
		CHECK_INCDEC_H_GENVAR_0( green, 31 ) ;
	}
	y += TFH ;
	subN += 1 ;
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Blue"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, blue, attr) ; //, attr ? ~hcolour : hcolour ) ;
  if ( attr )
	{
		CHECK_INCDEC_H_GENVAR_0( blue, 31 ) ;
	}
	
	if ( mode == 0 )
	{
		if ( (red < 3) && (green < 3) && (blue < 3) )
		{
			red = 3 ;
			green = 3 ;
			blue = 3 ;
		}
	}

	colour = ( red << 11 ) | ( green << 6 ) | ( blue ) ;
	
	if ( mode )
	{
		g_eeGeneral.textColour = LcdForeground = colour ;
	}
	else
	{
		g_eeGeneral.backgroundColour = LcdBackground = colour ;
		dimBackColour() ;
	}
}

void checkTheme( themeData *t )
{
	if ( t->backColour == 0 )
	{
		if ( t->textColour == 0 )
		{
			if ( t->brightness <= 1 )
			{
				t->backColour = g_eeGeneral.backgroundColour ;
				t->textColour = g_eeGeneral.textColour ;
				t->brightness = g_eeGeneral.bright ;
			}
		}
	}
}

void menuBackground( uint8_t event )
{
	menuColour( event, 0 ) ;	
}

void menuForeground( uint8_t event )
{
	menuColour( event, 1 ) ;	
}

void menuDisplay( uint8_t event )
{
	touchMenuTitle( (char *)PSTR(STR_Display) ) ;
#ifdef TOUCH
	uint32_t selected = 0 ;
#endif

	static MState2 mstate2;
#ifdef X20
	uint32_t rows = 7 ;
#else
	uint32_t rows = 9 ;
#endif
	mstate2.check_columns(event, rows-1) ;
	 
  uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;
	uint8_t subN = 0 ;
	uint16_t colour = dimBackColour() ;
	uint8_t attr ;
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
	selected = newVert & 0x0100 ;
#endif
	 
	y = TTOP ;
	 
	attr = (sub==subN) ? INVERS : 0 ;
#ifdef X20
	drawItem( (char *)XPSTR("Brightness"), y, attr ) ;
#else
	drawItem( (char *)PSTR(STR_BRIGHTNESS), y, attr ) ;
#endif
	drawNumber( TRIGHT-TRMARGIN, y, 100 - g_eeGeneral.bright, attr) ; //, attr ? ~colour : colour ) ;
  if(attr)
	{
		uint8_t oldBrightness = 100 - g_eeGeneral.bright ;
#ifdef X20
		CHECK_INCDEC_H_GENVAR( oldBrightness, 1, 100 ) ; //5-10V
#else
		CHECK_INCDEC_H_GENVAR( oldBrightness, 0, 100) ; //5-10V
#endif
		if ( oldBrightness != 100 - g_eeGeneral.bright )
		{
#ifdef X20
			g_eeGeneral.bright = oldBrightness ;
			backlightOn(oldBrightness) ;
#else
			backlight_set( g_eeGeneral.bright = 100 - oldBrightness ) ;
#endif
		}
	}
  y += TFH ;
	subN += 1 ;

#ifndef X20
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Off brightness"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, 100 - g_eeGeneral.bright_white, attr) ; //, attr ? ~colour : colour ) ;
  if(attr)
	{
		uint8_t oldBrightness = 100 - g_eeGeneral.bright_white ;
		CHECK_INCDEC_H_GENVAR( oldBrightness, 0, 100) ; //5-10V
		g_eeGeneral.bright_white = 100 - oldBrightness ;
	}
  y += TFH ;
	subN += 1 ;
#endif
	 
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_LIGHT_SWITCH), y, attr ) ;
#ifdef X20
	DrawDrSwitches( TRIGHT-TRMARGIN, y, g_eeGeneral.lightSw, attr|LUA_RIGHT ) ;
#else
	DrawDrSwitches( TRIGHT-TRMARGIN, y+TVOFF/HVSCALE, g_eeGeneral.lightSw, attr|LUA_RIGHT ) ;
#endif
	if(attr)
	{
		CHECK_INCDEC_GENERALSWITCH( event, g_eeGeneral.lightSw, -MaxSwitchIndex, MaxSwitchIndex) ;
	}
  y += TFH ;
	subN += 1 ;
	
	for ( uint32_t i = 0 ; i < 2 ; i += 1 )
	{
		attr = (sub==subN) ? INVERS : 0 ;
		uint8_t b ;
		drawItem( (char *)(( i == 0) ? PSTR(STR_LIGHT_AFTER) : PSTR(STR_LIGHT_STICK)), y, attr ) ;
		b = ( i == 0 ) ? g_eeGeneral.lightAutoOff : g_eeGeneral.lightOnStickMove ;

    if( b )
		{
			drawNumber( TRIGHT-5-FW, y, b * 5, attr) ; //, attr ? ~colour : colour ) ;
			drawChar( TRIGHT-3-FW, y, 's', attr ) ; //, attr ? ~colour : colour ) ;
    }
    else
		{
#ifndef TOUCH
			if ( attr & INVERS )
			{
				if ( s_editMode && BLINK_ON_PHASE )
				{
					attr = 0 ;
				}
			}
#endif
#ifdef X20
  		lcd_putsAttColour( TRIGHT-5-3*FW, y+TVOFF, PSTR(STR_OFF), 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
#else
  		PUTS_ATT_COLOUR( TRIGHT-TRMARGIN, y+TVOFF, PSTR(STR_OFF), LUA_RIGHT, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
#endif
		}
    if ( attr )
		{
			CHECK_INCDEC_H_GENVAR_0( b, 600/5) ;
		}
		if ( i == 0 )
		{
			g_eeGeneral.lightAutoOff = b ;
		}
		else
		{
			g_eeGeneral.lightOnStickMove = b ;
		}
  	y += TFH ;
		subN += 1 ;
	}
	
#ifndef X20
	attr = (sub==subN) ? INVERS : 0 ;
	g_eeGeneral.flashBeep = touchOnOffItem( g_eeGeneral.flashBeep, y, PSTR(STR_FLASH_ON_BEEP), attr, colour ) ;
  y += TFH ;
	subN += 1 ;
#endif

	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Background Colour"), y, attr ) ;
	if( attr )
	{
#ifdef TOUCH
		if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() || selected )
#else
		if ( checkForMenuEncoderBreak( event ) )
#endif
		{
			pushMenu( menuBackground ) ;
		}
	}
 	y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Text Colour"), y, attr ) ;
	if( attr )
	{
#ifdef TOUCH
		if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() || selected )
#else
		if ( checkForMenuEncoderBreak( event ) )
#endif
		{
			pushMenu( menuForeground ) ;
		}
	}
 	y += TFH ;
	subN += 1 ;

	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Theme"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, g_eeGeneral.selectedTheme, attr ) ;
	if( attr )
	{
		uint8_t oldTheme = g_eeGeneral.selectedTheme ;
		CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.selectedTheme, 3 ) ;
		if ( oldTheme != g_eeGeneral.selectedTheme )
		{
			checkTheme( &g_eeGeneral.theme[g_eeGeneral.selectedTheme] ) ;
			themeData *t = &g_eeGeneral.theme[oldTheme] ;
			t->backColour = g_eeGeneral.backgroundColour ;
			t->textColour = g_eeGeneral.textColour ;
			t->brightness = g_eeGeneral.bright ;
			t = &g_eeGeneral.theme[g_eeGeneral.selectedTheme] ;
			LcdBackground = g_eeGeneral.backgroundColour = t->backColour ;
			LcdForeground = g_eeGeneral.textColour = t->textColour ;
			g_eeGeneral.bright = t->brightness ;
			dimBackColour() ;
		}
	}
}

void menuAlarms( uint8_t event )
{
	touchMenuTitle( (char *)PSTR(STR_Alarms) ) ;
	static MState2 mstate2;
	uint32_t rows = 6 ;
	event = mstate2.check_columns(event, rows-1) ;

  uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;
	uint16_t colour = dimBackColour() ;
	uint8_t attr ;
	uint8_t subN = 0 ;

#ifdef TOUCH
	sub = mstate2.m_posVert = handleTouchSelect( rows, 0, sub ) ;
#endif
//	int32_t newSelection = checkTouchSelect( rows, 0 ) ;
//	uint16_t newVert = processSelection( sub , newSelection ) ;
//	sub = mstate2.m_posVert = newVert & 0x00FF ;
//	checkTouchEnterEdit( newVert ) ;

	y = TTOP ;
	 
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_BATT_WARN), y, attr ) ;
  if(attr)
	{
		CHECK_INCDEC_H_GENVAR( g_eeGeneral.vBatWarn, 40, 120) ; //5-10V
	}
	drawNumber( TRIGHT-TRMARGIN-FW, y, g_eeGeneral.vBatWarn, attr|PREC1) ; //, attr ? ~colour : colour ) ;
	drawChar( TRIGHT-TRMARGIN-FW, y, 'V', attr ) ; //, attr ? ~colour : colour ) ;
  y += TFH ;
	subN += 1 ;

	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_INACT_ALARM), y, attr ) ;
  if(attr)
	{
		attr = INVERS ;
		CHECK_INCDEC_H_GENVAR( g_eeGeneral.inactivityTimer, -10, 110) ; //0..120minutes
	}
	drawNumber( TRIGHT-TRMARGIN-FW, y, g_eeGeneral.inactivityTimer+10, attr) ; //, attr ? ~colour : colour ) ;
	drawChar( TRIGHT-TRMARGIN-FW, y, 'm', attr ) ; //, attr ? ~colour : colour ) ;
  y += TFH ;
	subN += 1 ;

	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_VOLUME), y, attr ) ;
	int8_t value = g_eeGeneral.inactivityVolume + ( NUM_VOL_LEVELS-3 ) ;
	drawNumber( TRIGHT-TRMARGIN, y, value, attr) ; //, attr ? ~colour : colour ) ;
  if(attr)
	{
		CHECK_INCDEC_H_GENVAR_0( value,NUM_VOL_LEVELS-1) ;
		g_eeGeneral.inactivityVolume = value - ( NUM_VOL_LEVELS-3 ) ;
	} 	
  y += TFH ;
	subN += 1 ;
      
	uint8_t b = g_eeGeneral.disableThrottleWarning ;
	g_eeGeneral.disableThrottleWarning = touchOffOnItem( b, y, PSTR(STR_SPLASH_SCREEN), sub == subN, colour ) ;
  y += TFH ;
	subN += 1 ;

	b = g_eeGeneral.disableAlarmWarning ;
	g_eeGeneral.disableAlarmWarning = touchOffOnItem( b, y, PSTR(STR_ALARM_WARN), sub == subN, colour ) ;
  y += TFH ;
	subN += 1 ;

	b = g_eeGeneral.disableRxCheck ;
	g_eeGeneral.disableRxCheck = touchOffOnItem( b, y, XPSTR("Receiver Warning"), sub == subN, colour ) ;
}

void menuAudio( uint8_t event )
{
	touchMenuTitle( (char *)PSTR(STR_AudioHaptic) ) ;

	static MState2 mstate2;
	uint32_t rows = 10 ;
	if ( g_eeGeneral.welcomeType == 2 )
	{
		rows += 1 ;
	}
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	mstate2.check_columns(event, rows-1) ;
	 
#ifdef TOUCH
	uint32_t newVpos ;
#endif
  uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;
	uint32_t k = 0 ;
	uint16_t t_pgOfs ;
	uint16_t colour = dimBackColour() ;
	
	t_pgOfs = evalHresOffset( sub ) ;
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;

	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1)+1, t_pgOfs ) ;
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
#endif

	for( uint32_t i = 0 ; i<TLINES ; i += 1 )
	{
    y = i * TFH + TTOP ;
    k = i + t_pgOfs ;
    LcdFlags attr = (sub==k) ? INVERS : 0 ;
		switch ( k )
		{
			case 0 :
			{	
				uint8_t current_volume ;
				current_volume = g_eeGeneral.volume ;
				drawItem( (char *)PSTR(STR_VOLUME), y, attr ) ;
				drawNumber( TRIGHT-5, y, current_volume, attr) ; //, attr ? ~colour : colour ) ;
  			if ( attr )
				{
					CHECK_INCDEC_H_GENVAR_0( current_volume,NUM_VOL_LEVELS-1) ;
					if ( current_volume != g_eeGeneral.volume )
					{
						setVolume( g_eeGeneral.volume = current_volume ) ;
					}
				}
			}
      break ;
			case 1 :
			{	
				uint8_t b ;
  			b = g_eeGeneral.beeperVal ;
				drawItem( (char *)PSTR(STR_BEEPER), y, attr ) ;
				drawIdxText( y, (char *)PSTR(STR_BEEP_MODES), b, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
				if ( attr )
				{
					CHECK_INCDEC_H_GENVAR_0( b, 6 ) ;
					g_eeGeneral.beeperVal = b ;
				}
			}
      break ;
			case 2 :
				drawItem( (char *)PSTR(STR_SPEAKER_PITCH), y, attr ) ;
				drawNumber( TRIGHT-5, y, g_eeGeneral.speakerPitch, attr) ; //, attr ? ~colour : colour ) ;
  			if ( attr )
				{
					CHECK_INCDEC_H_GENVAR( g_eeGeneral.speakerPitch, 1, 100) ;
				}	
      break ;
			case 3 :
				drawItem( (char *)PSTR(STR_HAPTICSTRENGTH), y, attr ) ;
				drawNumber( TRIGHT-5, y, g_eeGeneral.hapticStrength, attr) ; //, attr ? ~colour : colour ) ;
  			if ( attr )
				{
					CHECK_INCDEC_H_GENVAR( g_eeGeneral.hapticStrength, 0, 5 ) ;
				}	
      break ;
			case 4 :
				drawItem( (char *)XPSTR("Haptic Min Run"), y, attr ) ;
				drawNumber( TRIGHT-5, y, g_eeGeneral.hapticMinRun+20, attr) ; //, attr ? ~colour : colour ) ;
  			if ( attr )
				{
					CHECK_INCDEC_H_GENVAR( g_eeGeneral.hapticMinRun, 0, 20 ) ;
				}	
      break ;
			case 5 :
				drawItem( (char *)XPSTR(GvaString), y, attr ) ;
  			if( attr )
				{
#ifdef TOUCH
					if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() )
#else
					if ( checkForMenuEncoderBreak( event ) )
#endif
					{
//						SubMenuCall = 0x85 ;
						pushMenu(menuGlobalVoiceAlarm) ;
					}
				}
      break ;
			case 6 :
  			g_eeGeneral.preBeep = touchOnOffItem( g_eeGeneral.preBeep, y, PSTR(STR_BEEP_COUNTDOWN), attr, colour ) ;
      break ;
			case 7 :
  			g_eeGeneral.minuteBeep = touchOnOffItem( g_eeGeneral.minuteBeep, y, PSTR(STR_MINUTE_BEEP), attr, colour ) ;
      break ;
			case 8 :
				drawItem( (char *)XPSTR( "Welcome Type"), y, attr ) ;
				drawIdxText( y, XPSTR("\006System  NoneCustom"),g_eeGeneral.welcomeType, attr|LUA_RIGHT ) ; // 0, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
				if(attr) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.welcomeType, 2 ) ;
      break ;
			case 9 :
			{
				uint16_t oldBcolour = LcdBackground ;
				if ( g_eeGeneral.welcomeType == 2 )
				{
					Tevent = 0 ;
					drawItem( (char *)XPSTR( "FileName"), y, attr ) ;
  				if( attr )
					{
#ifdef TOUCH
						if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() )
#else
						if ( checkForMenuEncoderBreak( event ) )
#endif
						{
							VoiceFileType = VOICE_FILE_TYPE_USER ;
    				 	pushMenu( menuSelectVoiceFile ) ;
    					s_editMode = false ;
						}
						if ( event == EVT_ENTRY_UP )
						{
							if ( FileSelectResult == 1 )
							{
						 		copyFileName( (char *)g_eeGeneral.welcomeFileName, SelectedVoiceFileName, 8 ) ;
	  				 		eeDirty(EE_GENERAL) ;		// Save it
							}
						}
					} 
			 		LcdBackground = attr ? ~colour : colour ;
					alphaEditName( TRIGHT-8*FW-5, y+TVOFF, (uint8_t *)g_eeGeneral.welcomeFileName, sizeof(g_eeGeneral.welcomeFileName), attr|ALPHA_NO_NAME, (uint8_t *)0 ) ;
					LcdBackground = oldBcolour ;
					validateName( g_eeGeneral.welcomeFileName, sizeof(g_eeGeneral.welcomeFileName) ) ;
				}
			}
 #ifndef X20
			case 10 :
			if ( ( ( k == 10 ) && ( g_eeGeneral.welcomeType == 2 ) ) || ( ( k == 9 ) && ( g_eeGeneral.welcomeType < 2 ) ) )
			{	
				uint8_t b ;
				drawItem( (char *)XPSTR( "Hardware Volume"), y, attr ) ;
				b = g_eeGeneral.unused_PPM_Multiplier ;
				drawNumber( TRIGHT-5, y, g_eeGeneral.unused_PPM_Multiplier, attr) ; //, attr ? ~colour : colour ) ;
		    if ( attr )
				{
					CHECK_INCDEC_H_GENVAR( g_eeGeneral.unused_PPM_Multiplier, 0, 127) ;
				}
				if ( b != g_eeGeneral.unused_PPM_Multiplier )
				{
					b = g_eeGeneral.unused_PPM_Multiplier ;
void I2C_set_volume( register uint8_t volume ) ;
					I2C_set_volume( b ) ;
				}
			}	
      break ;
 #endif		
		}
	}
}

void menuControls(uint8_t event)
{
	touchMenuTitle( (char *)PSTR(STR_Controls) ) ;
	static MState2 mstate2 ;
	uint32_t rows = 8 ;
	event = mstate2.check_columns( event, rows-1 ) ;
	uint16_t colour = dimBackColour() ;
		
	uint8_t subN = 0 ;
	uint8_t sub = mstate2.m_posVert ;
	uint16_t y = TTOP ;

#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows+1, 0, 0 ) ;
	if ( newSelection >= 0 )
	{
		if ( newSelection > 3 )
		{
			newSelection -= 1 ;
		}
	}
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
//	selected = newVert & 0x0100 ;
#endif

	uint8_t attr = sub==subN ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_CHAN_ORDER), y, (sub == subN ) ) ;
	uint8_t bch = bchout_ar[g_eeGeneral.templateSetup] ;
	if(attr) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.templateSetup, 23 ) ;
#ifndef TOUCH
	if ( attr & INVERS )
	{
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
	}
#endif
	saveEditColours( attr, colour ) ;
  for ( uint32_t i = 4 ; i > 0 ; i -= 1 )
	{
		uint8_t letter ;
		letter = *(PSTR(STR_SP_RETA) +(bch & 3) + 1 ) ;
#ifdef X20
  	lcdDrawChar( TRIGHT-TRMARGIN-5*FW+i*FW, y+TVOFF, letter, 0 ) ;
#else
  	PUTC_ATT( TRIGHT-TRMARGIN-5*FW+i*FW, y+TVOFF, letter, 0 ) ;
#endif
		bch >>= 2 ;
	}
	restoreEditColours() ;
 	y += TFH ;
	subN += 1 ;
      
	attr = sub==subN ? INVERS : 0 ;
	uint8_t ct = g_eeGeneral.crosstrim + ( g_eeGeneral.xcrosstrim << 1 ) ;
	drawItem( (char *)PSTR(STR_CROSSTRIM), y, attr ) ;
  if(attr)
	{
		CHECK_INCDEC_H_GENVAR_0( ct, 2 ) ;
	}
	saveEditColours( attr, colour ) ;
  drawIdxText( y, XPSTR("\003OFFON Vtg"), ct, attr|LUA_RIGHT ) ;
	restoreEditColours() ;
	g_eeGeneral.crosstrim = ct ;
	g_eeGeneral.xcrosstrim = ct >> 1 ;
 	y += TFH ;
	subN += 1 ;

#ifndef X20	
	uint8_t oldValue = g_eeGeneral.throttleReversed ;
	g_eeGeneral.throttleReversed = touchOnOffItem( g_eeGeneral.throttleReversed, y, PSTR(STR_THR_REVERSE), (sub == subN ), colour ) ;
	if ( g_eeGeneral.throttleReversed != oldValue )
	{
  	checkTHR() ;
	}
 	y += TFH ;
	subN += 1 ;
#endif

 	attr = 0 ;
	uint8_t mode = g_eeGeneral.stickMode ;
  if(sub==subN)
	{
		attr = INVERS ;
		if ( s_editMode )
		{
			CHECK_INCDEC_H_GENVAR_0( mode,3 ) ;
			if ( mode != g_eeGeneral.stickMode )
			{
//				g_eeGeneral.stickScroll = 0 ;
				g_eeGeneral.stickMode = mode ;							
			}
		}
	}
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
	}
	saveEditColours( attr, LcdBackground ) ;
#ifdef X20
	lcdDrawText( THOFF, y+TVOFF, PSTR(STR_MODE) ) ;
#else
	PUTS_P( THOFF, y+TVOFF, PSTR(STR_MODE) ) ;
#endif
	lcd_hline( 0, y+TFH, TRIGHT ) ;
	for ( uint32_t i = 0 ; i < 4 ; i += 1 )
	{
#ifndef X20	
 		lcd_HiResimg( (6+4*i)*FW*2, y*2+TVOFF*2+1, sticksHiRes, i, 0, LcdForeground, attr&INVERS ? ~LcdBackground : LcdBackground ) ;
#endif
	}
	restoreEditColours() ;
 	y += TFH ;
    
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
	}
#ifndef TOUCH
	if ( attr & INVERS )
	{
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
	}
#endif
	saveEditColours( attr, LcdBackground ) ;
#ifdef X20
  lcdDrawChar( 3*FW, y+TVOFF, '1'+g_eeGeneral.stickMode, 0 ) ;
#else
  PUTC_ATT( 3*FW, y+TVOFF, '1'+g_eeGeneral.stickMode, 0 ) ;
#endif
  for(uint32_t i=0; i<4; i++)
	{
		putsChnRaw( ((6+4*i)*FW)*TSCALE, (y+TVOFF)*TSCALE, modeFixValue( i ), 0 ) ;//sub==3?INVERS:0);
	}
	restoreEditColours() ;
	lcd_hline( 0, y+TFH, TRIGHT ) ;
 	y += TFH ;
	subN += 1 ;
	
	if ( sub >= rows )
	{
		sub = mstate2.m_posVert = rows-1 ;
	}
	// Edit custom stick names
  for ( uint32_t i = 0 ; i < 4 ; i += 1 )
	{
		attr = sub==subN ? INVERS : 0 ;
#ifdef X20
    lcdDrawTextAtIndex( THOFF, y+TVOFF, PSTR(STR_STICK_NAMES), i, 0 ) ;
    lcdDrawText( THOFF+4*FW, y+TVOFF, (char *)"Stick Name" ) ;
#else
    PUTS_AT_IDX( THOFF/2, y+TVOFF, PSTR(STR_STICK_NAMES), i, 0 ) ;
    PUTS_P( THOFF+4*FW, y+TVOFF, (char *)"Stick Name" ) ;
#endif
		uint16_t oldBcolour = LcdBackground ;
		LcdBackground = attr ? ~colour : colour ;
		lcdDrawSolidFilledRectDMA( TMID*TSCALE, y*TSCALE+2, (TRIGHT-TMID)*TSCALE, TFH*TSCALE-2, LcdBackground ) ;
		alphaEditName( TRIGHT-TRMARGIN, y+TVOFF, &g_eeGeneral.customStickNames[i*4], 4, attr|ALPHA_NO_NAME|LUA_RIGHT, (uint8_t *)&PSTR(STR_STICK_NAMES)[i*5+1] ) ;
		LcdBackground = oldBcolour ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
	 	y += TFH ;
		subN += 1 ;
	}
}
