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
uint8_t mapPots( uint8_t value ) ;
uint8_t unmapPots( uint8_t value ) ;
void menuScaleOne(uint8_t event) ;
#ifdef TOUCH
extern uint8_t TlExitIcon ;
#endif
void menuModeOne(uint8_t event) ;
void putsTrimMode( coord_t x, coord_t y, uint8_t phase, uint8_t idx, uint8_t att ) ;
void menuOneAdjust(uint8_t event) ;
void editOneProtocol( uint8_t event ) ;
void displayModuleName( uint16_t x, uint16_t y, uint8_t index, uint8_t attr ) ;
void displayProtocol( uint16_t x, uint16_t y, uint8_t value, uint8_t attr ) ;
void menuRangeBind(uint8_t event) ;
extern PhaseData *getPhaseAddress( uint32_t phase ) ;
uint32_t fillPlaylist( TCHAR *dir, struct fileControl *fc, char *ext ) ;
extern uint8_t SuppressStatusLine ;
extern uint32_t BgSizePlayed ;
extern uint32_t BgTotalSize ;
extern const char StringAdjustFunctions[] ;
extern const uint8_t UnitsString[] ;
extern const uint8_t DestString[] ;
extern uint8_t AlphaEdited ;
extern char LastItem[] ;
extern const char HyphenString[] ;
void setStickCenter(uint32_t toSubTrims ) ; // copy state of 3 primary to subtrim
void displayLogicalSwitch( uint16_t x, uint16_t y, uint32_t index ) ;
extern struct t_clipboard Clipboard ;
void editOneSwitchItem( uint8_t event, uint32_t item, uint32_t index ) ;
extern uint8_t s_expoChan ;
extern uint8_t SingleExpoChan ;
extern uint8_t EditingModule ;
#ifdef TOUCH
extern uint8_t LastTselection ;
extern uint8_t LastTaction ;
#endif
extern uint8_t InputMap[] ;
extern uint8_t InputMapSize ;
void displayInputSource( coord_t x, coord_t y, uint8_t index, LcdFlags att ) ;
extern void createInputMap() ;
void drawSmallGVAR( uint16_t x, uint16_t y ) ;
void displayInputSource( uint16_t x, uint16_t y, uint8_t index, uint8_t att ) ;
int16_t gvarMenuItem(coord_t x, coord_t y, int16_t value, int16_t min, int16_t max, LcdFlags attr, uint8_t event ) ;
int16_t gvarDiffValue( uint16_t x, uint16_t y, int16_t value, uint32_t attr, uint8_t event ) ;


uint16_t scalerDecimal( coord_t y, uint16_t val, LcdFlags attr ) ;


#ifndef X20
#define STATUS_VERTICAL		242
#endif

void menuHeli( uint8_t event )
{
	touchMenuTitle( (char *)PSTR(STR_HELI_SETUP) ) ;

	static MState2 mstate2;
	uint32_t rows = 6 ;
	mstate2.check_columns(event, rows-1) ;

  uint32_t sub = mstate2.m_posVert ;
	uint16_t y ;
	uint8_t subN = 0 ;
	uint16_t colour = dimBackColour() ;
	LcdFlags attr ;
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif
	 
	y = TTOP ;
	 
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Swash Type"), y, attr ) ;
	drawIdxText( y, (char *)PSTR(SWASH_TYPE_STR)+2, g_model.swashType, attr|LUA_RIGHT ) ;
	if(attr) CHECK_INCDEC_H_GENVAR_0( g_model.swashType, 4 ) ;
  y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Collective"), y, attr ) ;
	if( attr )
	{
#ifdef X20		
		CHECK_INCDEC_H_MODELVAR( g_model.swashCollectiveSource, 0, NUM_SKYXCHNRAW ) ;
#else
		uint8_t x = mapPots( g_model.swashCollectiveSource ) ;
		CHECK_INCDEC_H_MODELVAR_0( x, NUM_SKYXCHNRAW+NumExtraPots ) ;
		g_model.swashCollectiveSource = unmapPots( x ) ;
#endif
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
  putsChnRaw( (TRIGHT-TRMARGIN)*TSCALE, (y+TVOFF)*TSCALE, g_model.swashCollectiveSource, LUA_RIGHT, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
  y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Swash Ring"), y, attr ) ;
	drawNumber( TRIGHT-5, y, g_model.swashRingValue, attr ) ;
	if( attr )
	{
		CHECK_INCDEC_H_MODELVAR_0( g_model.swashRingValue, 100) ;
	}
  y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("ELE Direction"), y, attr ) ;
	drawIdxText( y, (char *)PSTR( STR_HYPH_INV)+2, g_model.swashInvertELE, attr ) ;
  if(attr)
	{
		g_model.swashInvertELE = checkIncDec( g_model.swashInvertELE, 0, 1, EditType ) ;
	}
  y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("AIL Direction"), y, attr ) ;
	drawIdxText( y, (char *)PSTR( STR_HYPH_INV)+2, g_model.swashInvertAIL, attr ) ;
  if(attr)
	{
		g_model.swashInvertAIL = checkIncDec( g_model.swashInvertAIL, 0, 1, EditType ) ;
	}
  y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("COL Direction"), y, attr ) ;
	drawIdxText( y, (char *)PSTR( STR_HYPH_INV)+2, g_model.swashInvertCOL, attr ) ;
  if(attr)
	{
		g_model.swashInvertCOL = checkIncDec( g_model.swashInvertCOL, 0, 1, EditType ) ;
	}
}

void menuScalers(uint8_t event)
{
	touchMenuTitle( (char *)XPSTR("Scalers") ) ;
	EditType = EE_MODEL ;
	uint32_t rows = NUM_SCALERS ;
	static MState2 mstate2;
 	event = mstate2.check_columns(event, rows-1 ) ;
	

	uint8_t sub = mstate2.m_posVert ;
	uint16_t y = TTOP ;
  uint32_t k ;
#ifdef TOUCH
	uint32_t selected = 0 ;
#endif

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	selected = newVert & 0x0100 ;
#endif

#ifdef TOUCH
	if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() || selected )
#else
	if ( checkForMenuEncoderBreak( event ) )
#endif
	{
		s_currIdx = sub ;
		killEvents(event);
    s_editMode = false ;
		pushMenu(menuScaleOne) ;
	}

	uint16_t oldBcolour = LcdBackground ;
	uint16_t oldFcolour = LcdForeground ;
	
	for (uint32_t i = 0 ; i < 8 ; i += 1 )
	{
		uint8_t attr = sub == i ? INVERS : 0 ;
    y = (i)*TFH +TTOP ;
    k = i ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdBackground = ~LcdBackground ;
			LcdForeground = ~LcdForeground ;
		}
		y += TVOFF ;
#ifdef X20
		lcdDrawText( THOFF, y, XPSTR("SC") ) ;
  	lcdDrawChar( 2*FW+THOFF, y, k+'1' ) ;
  	lcdDrawChar( 9*FW, y, '+' ) ;
  	lcdDrawChar( 19*FW, y, '*' ) ;
  	lcdDrawChar( 26*FW, y, '/' ) ;
		putsChnRaw( 4*FW+THOFF, y, g_model.Scalers[k].source, 0 ) ;
		lcdDrawNumber( 15*FW+THOFF, y, g_model.Scalers[k].offset, 0 ) ;
		lcdDrawNumber( 24*FW+THOFF, y, g_model.Scalers[k].mult+1, 0 ) ;
		lcdDrawNumber( 31*FW+THOFF, y, g_model.Scalers[k].div+1, 0 ) ;
#else
		uint16_t t ;
		PUTS_P( THOFF, y, XPSTR("SC") ) ;
  	PUTC( 2*FW+THOFF, y, k+'1' ) ;
  	PUTC( 9*FW, y, '+' ) ;
  	PUTC( 19*FW, y, '*' ) ;
  	PUTC( 25*FW, y, '/' ) ;
		putsChnRaw( (4*FW+THOFF)*2, (y)*2, g_model.Scalers[k].source, 0 ) ;
		PUTS_NUM( 15*FW+THOFF, y, g_model.Scalers[k].offset, 0 ) ;
		t = g_model.Scalers[k].mult + ( g_model.Scalers[k].multx << 8 ) ;
		PUTS_NUM( 23*FW-1+THOFF, y, t+1, 0 ) ;
		t = g_model.Scalers[k].div + ( g_model.Scalers[k].divx << 8 ) ;
		PUTS_NUM( 29*FW+THOFF, y, t+1, 0 ) ;
#endif		 
		LcdBackground = oldBcolour ;
		LcdForeground = oldFcolour ;
	}
}

void menuModes(uint8_t event)
{
	uint32_t i ;
  uint8_t attr ;
#ifdef TOUCH	
	uint32_t selected = 0 ;
#endif
	 
	touchMenuTitle( (char *)PSTR(STR_MODES) ) ;
	TITLE(PSTR(STR_MODES)) ;
	static MState2 mstate2 ;
  
	uint32_t rows = 8 ;
	event = mstate2.check_columns( event, rows-1 ) ;
	
	uint16_t colour = LcdBackground ;
	
	uint8_t sub = mstate2.m_posVert ;

#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows+1, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	selected = newVert & 0x0100 ;
#endif

#ifdef TOUCH	
 #ifdef X20
	if ( handleSelectIcon() || ( event == EVT_KEY_BREAK(BTN_RE) ) || selected )
 #else
	if ( handleSelectIcon() || ( event == EVT_KEY_BREAK(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) || selected )
 #endif
#else
	if ( ( event == EVT_KEY_BREAK(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
#endif
	{
		s_currIdx = sub ;
		killEvents(event);
//#if MULTI_GVARS
//		if ( ( g_model.flightModeGvars ) || ( ( g_model.flightModeGvars == 0 ) && s_currIdx ) )
//#else
		if ( s_currIdx )
//#endif
		{
			if ( s_currIdx <= 7 )
			{
				pushMenu(menuModeOne) ;
			}
		}
  }
    
	uint16_t oldBcolour = LcdBackground ;
	uint16_t oldFcolour = LcdForeground ;

  for ( i=0 ; i<MAX_MODES+1+1 ; i += 1 )
	{
		uint32_t k ;
    uint16_t y = TTOP + i*TFH ;
    attr = (i == sub) ? INVERS : 0 ;
		if ( i == 0 )
		{
			
#ifdef X20
	  	lcdDrawChar( THOFF, TTOP+TVOFF, 'F' ) ;
  		lcdDrawChar( THOFF+FW, TTOP+TVOFF, '0' ) ;
			lcd_putsAtt( TRIGHT-TRMARGIN-10*FW, TTOP+TVOFF, XPSTR("R E T A"), attr ) ;
#else
	  	PUTC( THOFF, TTOP+TVOFF, 'F' ) ;
  		PUTC( THOFF+FW, TTOP+TVOFF, '0' ) ;
			PUTS_ATT( TRIGHT-TRMARGIN-10*FW, TTOP+TVOFF, XPSTR("R   E   T   A"), attr ) ;
#endif
			lcd_hline( 0, TTOP+TFH, TRIGHT ) ;
			s_editMode = 0 ;
			continue ;
		}
  	k = i - 1 ;
		
		PhaseData *p ;
#ifdef X20
		p = (k < MAX_MODES) ? &g_model.phaseData[k] : &g_model.xphaseData ;
#else
		p = getPhaseAddress( k ) ;
#endif
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdForeground = ~LcdForeground ;
		}
#ifdef X20
    lcd_putcAttColour( THOFF, y+TVOFF, 'F', 0, LcdForeground, attr ? ~colour : colour ) ;
    lcd_putcAttColour( THOFF+FW, y+TVOFF, '1'+k, 0, LcdForeground, attr ? ~colour : colour ) ;
		lcd_putsnAttColour( THOFF+3*FW, y+TVOFF/2, p->name, 6, 0, LcdForeground, attr ? ~LcdBackground : LcdBackground ) ;
#else  	
		lcdDrawChar( THOFF*HVSCALE, (y*HVSCALE+TVOFF), 'F', 0, LcdForeground ) ;
  	lcdDrawChar( (THOFF+FW)*HVSCALE, (y*HVSCALE+TVOFF), '1'+k, 0, LcdForeground ) ;
		PUTS_ATT_N_COLOUR( THOFF+3*FW, y+TVOFF/HVSCALE, p->name, 6, 0, LcdForeground ) ; // , attr ? ~LcdBackground : LcdBackground ) ;
#endif
		LcdForeground = oldFcolour ;
//		putsDrSwitchesColour( THOFF+9*FW, y+TVOFF/HVSCALE, p->swtch, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
//		putsDrSwitchesColour( THOFF+14*FW, y+TVOFF/HVSCALE, p->swtch2, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
		
#ifdef X20
		DrawDrSwitches( THOFF+12*FW, y, p->swtch, attr ) ;
		DrawDrSwitches( THOFF+17*FW, y, p->swtch2, attr ) ;
#else  	
		DrawDrSwitches( THOFF+9*FW, y+TVOFF/HVSCALE, p->swtch, attr ) ;
		DrawDrSwitches( THOFF+14*FW, y+TVOFF/HVSCALE, p->swtch2, attr ) ;
#endif

		LcdBackground = attr ? ~colour : colour ;
		if ( attr )
		{
			LcdForeground = ~LcdForeground ;
		}
    for ( uint32_t t = 0 ; t < NUM_STICKS ; t += 1 )
		{
			putsTrimMode( TRIGHT-3-10*FW+t*FW*2, y+TVOFF/HVSCALE, k+1, t, 0 ) ;
			int16_t v = p->trim[t].mode ;
 			if (v && ( v & 1 ) )
			{
#ifdef X20
        lcdDrawChar( TRIGHT-3-11*FW+t*FW*2, y+TVOFF/HVSCALE, '+' ) ;
#else
        PUTC( TRIGHT-3-11*FW+t*FW*2, y+TVOFF/HVSCALE, '+' ) ;
#endif
			}
		}
		LcdBackground = oldBcolour ;
		LcdForeground = attr ? ~oldFcolour : oldFcolour ;
		if ( p->fadeIn || p->fadeOut )
		{
#ifdef X20
	    lcd_putcAttColour( TRIGHT-3-FW, y+TVOFF, '*', 0, LcdForeground, attr ? ~colour : colour ) ;
#else
  		lcdDrawChar( (TRIGHT-3-FW)*2, (y+TVOFF)*2, '*', 0, LcdForeground ) ;
#endif
		}
		LcdForeground = oldFcolour ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
	}	 
}

void menuModeOne(uint8_t event)
{
  PhaseData *phase ;
	uint32_t t_pgOfs ;
#ifdef TOUCH
	uint32_t newVpos ;
#endif
	uint32_t index = s_currIdx ;
	if ( index )
	{
		index -= 1 ;
		phase = (index < MAX_MODES) ? &g_model.phaseData[index] : &g_model.xphaseData ;
	}
	else
	{
		phase = &g_model.xphaseData ;	// Keep compiler happy
	}
	static MState2 mstate2 ;
	touchMenuTitle( (char *)PSTR(STR_FL_MODE) ) ;
	uint32_t rows = s_currIdx ? 9 : 0 ;
	
	mstate2.check_columns(event, rows-1 ) ;
  PUTC( 12*FW, 0, '0'+s_currIdx ) ;

  uint32_t sub = mstate2.m_posVert ;
	uint16_t colour = dimBackColour() ;

	t_pgOfs = evalHresOffset( sub ) ;
#ifndef TOUCH
	(void) t_pgOfs ;
#endif	 
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;

	if ( rows > 9 )
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
	 
	uint16_t oldBcolour = LcdBackground ;
	uint16_t oldFcolour = LcdForeground ;

	uint32_t lines = rows > TLINES ? TLINES : rows ;
	for (uint8_t i = 0 ; i < lines ; i += 1 )
	{
    coord_t y = i * TFH + TTOP ;
    uint32_t k = i+t_pgOfs ;
    
		uint32_t attr = (sub==k) ? INVERS : 0 ;

    if ( s_currIdx == 0 )
		{
			k += 9 ;
		}

		switch(k)
		{
      case 0 : // switch
			{	
				drawItem( (char *)PSTR(STR_SWITCH), y, attr ) ;
				LcdBackground = attr ? ~colour : colour ;
				if ( attr )
				{
					LcdForeground = ~LcdForeground ;
				}
				phase->swtch = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, phase->swtch, LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				LcdBackground = oldBcolour ;
				LcdForeground = oldFcolour ;
			}
			break ;
      case 1 : // switch
			{	
				drawItem( (char *)PSTR(STR_SWITCH), y, attr ) ;
				LcdBackground = attr ? ~colour : colour ;
				if ( attr )
				{
					LcdForeground = ~LcdForeground ;
				}
				phase->swtch2 = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, phase->swtch2, LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				LcdBackground = oldBcolour ;
				LcdForeground = oldFcolour ;
			}
			break ;
      
// Trim values -500 to 500 = own Trim
// 501 to 507 from other mode
// 501 is mode 0, 502 is mode 1 unless his mode is 1 when it is mode 2
			
			case 2 : // trim
      case 3 : // trim
      case 4 : // trim
      case 5 : // trim
			{	
				switch(k)
				{
      		case 2 : // trim
						drawItem( (char *)XPSTR("Rudder Trim"), y, attr ) ;
					break ;
      		case 3 : // trim
						drawItem( (char *)XPSTR("Elevator Trim"), y, attr ) ;
					break ;
      		case 4 : // trim
						drawItem( (char *)XPSTR("Throttle Trim"), y, attr ) ;
					break ;
      		case 5 : // trim
						drawItem( (char *)XPSTR("Aileron Trim"), y, attr ) ;
					break ;
				}
				LcdBackground = attr ? ~colour : colour ;
				if ( attr )
				{
					if ( ! ( s_editMode && BLINK_ON_PHASE ) )
					{
//						fcolour = ~LcdForeground ;
						LcdForeground = ~LcdForeground ;
					}
				}
        putsTrimMode( TRIGHT-2*FW, y+TVOFF, s_currIdx, k-2, 0 ) ;
				t_trim v = phase->trim[k-2] ;
				int16_t w = v.mode ;
  			if (w > 0)
				{
	  			if (w & 1)
					{
	        	PUTC( TRIGHT-3*FW, y+TVOFF, '+' ) ;
					}
				}
				LcdBackground = oldBcolour ;
				LcdForeground = oldFcolour ;
        if (attr )
				{
extern t_trim rawTrimFix( uint8_t phase, t_trim v ) ;
					v = rawTrimFix( s_currIdx, v ) ;
					w = v.mode ;
					int16_t u = w ;
          w = checkIncDec16( w, 0, 15, EE_MODEL ) ;
					
					if (w != u)
					{
						if ( w & 1 )
						{
							if ( (w>>1) == s_currIdx )
							{
								if ( w > u )
								{
									w += 1 ;
									if ( s_currIdx == 7 )
									{
										if ( w > 14 )
										{
											w = 14 ;
										}
									}
								}
								else
								{
									w -= 1 ;
								}
							}
						}
						v.mode = w ;
						// reverse xyzzy!!
						if ( w & 1 )
						{
							// additive
							v.value = 0 ;
						}
						else
						{
							w >>= 1 ;
							if ( w == s_currIdx )
							{
								v.value = 0 ;
							}
							else
							{
								if ( w > s_currIdx )
								{
									w -= 1 ;
								}
								v.value = TRIM_EXTENDED_MAX + w + 1 ;
							}
						}
  					phase->trim[k-2] = v ;
          }
        }
			}
			break ;
			case 6 : // fadeIn
				drawItem( (char *)XPSTR("Fade In"), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, phase->fadeIn * 5, attr|PREC1) ; //, attr ? ~colour : colour ) ;
  			if( attr )
				{
					CHECK_INCDEC_H_MODELVAR( phase->fadeIn, 0, 15 ) ;
				}
			break ;
      
			case 7 : // fadeOut
				drawItem( (char *)XPSTR("Fade Out"), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, phase->fadeOut * 5, attr|PREC1) ; //, attr ? ~colour : colour ) ;
  			if( attr )
				{
					CHECK_INCDEC_H_MODELVAR( phase->fadeOut, 0, 15 ) ;
				}
			break ;
			
			case 8 : // Name
			{	
				drawItem( (char *)XPSTR("Mode Name"), y, attr ) ;
				LcdBackground = attr ? ~colour : colour ;
				alphaEditName( TRIGHT-sizeof(phase->name)*FW-FW, y+TVOFF, (uint8_t *)phase->name, sizeof(phase->name), attr|ALPHA_NO_NAME, (uint8_t *)0 ) ;
				LcdBackground = oldBcolour ;
			}
			break ;
			
		}
	}
}

void menuTemplates(uint8_t event)  //Issue 73
{
	touchMenuTitle( (char *)PSTR(STR_TEMPLATES) ) ;
	 
	static MState2 mstate2 ;
	event = mstate2.check_columns( event, NUM_TEMPLATES-1 ) ;
	EditType = EE_MODEL ;

  coord_t y = TTOP ;
  uint8_t k = 0 ;
  int8_t sub = mstate2.m_posVert ;
	
#ifdef TOUCH
	uint32_t selected = 0 ;
	int32_t newSelection = checkTouchSelect( NUM_TEMPLATES, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif

#ifdef TOUCH
	if ( checkForMenuEncoderLong( event ) || handleSelectIcon() || selected )
#else
	if ( checkForMenuEncoderLong( event ) )
#endif
	{
    //apply mixes or delete
    s_noHi = NO_HI_LEN ;
    applyTemplate(sub) ;
    audioDefevent(AU_WARNING2) ;
  }

	for(uint32_t i=0; i<(TLINES - 1); i++)
	{
    k = i ; // +t_pgOfs ;
    if( k == NUM_TEMPLATES)
		{
			break ;
		}
    
		if ( sub == k )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
		}
		saveEditColours( sub == k, LcdBackground ) ;
    PUTS_NUM_N( 3*FW+THOFF, y+TVOFF, k+1, 0 + LEADING0,2) ;
    PUTS_ATT( 6*FW, y+TVOFF, PSTR(n_Templates[k]), 0 ) ;
		restoreEditColours() ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
    y += TFH ;
  }
}

void menuModelMusic(uint8_t event)
{
	uint8_t subN = 0 ;
	touchMenuTitle( (char *)PSTR(STR_Music) ) ;
	uint8_t attr = 0 ;
	EditType = EE_MODEL ;
	uint16_t y = TTOP ;
	static MState2 mstate2;
	uint16_t colour = dimBackColour() ;
#ifdef X20
	int16_t temp ;
#endif
#ifdef TOUCH	
	static uint8_t touchControl = 0 ;
	uint32_t selected = 0 ;
#endif
#ifndef X20
	SuppressStatusLine = 1 ;
	lcdDrawSolidFilledRectDMA( 0, STATUS_VERTICAL, 480, 30, LcdBackground ) ;
#endif
	
	uint32_t rows = 8 ;
	if ( MusicPlaying == MUSIC_STOPPED )
	{
		rows = 8 ;
	}
	if ( ( MusicPlaying == MUSIC_PLAYING ) || ( MusicPlaying == MUSIC_PAUSED ) )
	{
		rows = 9 ;
	}
	
	mstate2.check_columns(event, rows-1) ;
  int8_t sub = mstate2.m_posVert ;

#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
	selected = newVert & 0x0100 ;
	
	if ( selected )
	{
		if ( ( sub == 7 ) || ( sub == 8 ) )
		{
			selected = 0 ;
			s_editMode = 0 ;
		}
	}
		
	if ( ( newSelection == 7 ) || ( newSelection == 8 ) )
	{
		if ( sub == 7 )
		{
			if ( touchControl == 0 )
			{
				if ( MusicPlaying == MUSIC_STOPPED )
				{
					MusicPlaying = MUSIC_STARTING ;
				}
				else
				{
					MusicPlaying = MUSIC_STOPPING ;
				}
				touchControl = 1 ;
			}
			selected = 0 ;
			s_editMode = 0 ;
		}
		else if ( sub == 8 )
		{
			if ( touchControl == 0 )
			{
				if ( MusicPlaying == MUSIC_PAUSED )
				{
					MusicPlaying = MUSIC_RESUMING ;
				}
				else
				{
					MusicPlaying = MUSIC_PAUSING ;
				}
				touchControl = 1 ;
			}
		}
	}
	else
	{
		if ( newSelection == -2 )
		{
			touchControl = 0 ;
		}
	}
#endif

	if ( event == EVT_ENTRY_UP )
	{ // From menuProcSelectUvoiceFile
		if ( FileSelectResult == 1 )
		{
			copyFileName( (char *)g_eeGeneral.musicVoiceFileName, SelectedVoiceFileName, MUSIC_NAME_LENGTH ) ;
			g_eeGeneral.musicVoiceFileName[MUSIC_NAME_LENGTH] = '\0' ;

			if ( g_eeGeneral.musicType )
			{
				// load playlist
				cpystr( cpystr( (uint8_t *)PlaylistDirectory, (uint8_t *)"\\music\\" ), g_eeGeneral.musicVoiceFileName ) ;
				fillPlaylist( PlaylistDirectory, &PlayFileControl, (char *)"WAV" ) ;
				g_eeGeneral.playListIndex = PlaylistIndex = 0 ;
			}
			STORE_GENERALVARS ;
		}
	}
			 
  if(sub==subN)
	{
	  attr = INVERS ;
	}
	drawItem( (char *)XPSTR("Start Switch"), y, ( attr ) ) ;
	uint8_t doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
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
#ifdef X20
	temp = g_model.musicData.musicStartSwitch ;
	if ( temp < -80 )
	{
		temp += 256 ;
	}
	temp = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, temp, LUA_RIGHT, doedit, event ) ;
	if ( temp > 127 )
	{
		temp -= 256 ;
	}
	g_model.musicData.musicStartSwitch = temp ;
#else
	g_model.musicData.musicStartSwitch = edit_dr_switch( TRIGHT-TRMARGIN-4*FW, y+TVOFF, g_model.musicData.musicStartSwitch, 0, doedit, event ) ;
#endif
	restoreEditColours() ;
	y += TFH ;
	subN += 1 ;
	
	attr = 0 ;
  if(sub==subN)
	{
	  attr = INVERS ;
	}
	drawItem( (char *)XPSTR("Pause Switch"), y, ( attr ) ) ;
	doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
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
#ifdef X20
	temp = g_model.musicData.musicPauseSwitch ;
	if ( temp < -80 )
	{
		temp += 256 ;
	}
	temp = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, temp, LUA_RIGHT, doedit, event ) ;
	if ( temp > 127 )
	{
		temp -= 256 ;
	}
	g_model.musicData.musicPauseSwitch = temp ;
#else
	g_model.musicData.musicPauseSwitch = edit_dr_switch( TRIGHT-TRMARGIN-4*FW, y+TVOFF, g_model.musicData.musicPauseSwitch, 0, doedit, event ) ;
#endif
	restoreEditColours() ;
	y += TFH ;
	subN += 1 ;
	
	attr = 0 ;
  if(sub==subN)
	{
	  attr = INVERS ;
	}
	drawItem( (char *)XPSTR("Previous Switch"), y, ( attr ) ) ;
	doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
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
#ifdef X20
	temp = g_model.musicData.musicPrevSwitch ;
	if ( temp < -80 )
	{
		temp += 256 ;
	}
	temp = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, temp, LUA_RIGHT, doedit, event ) ;
	if ( temp > 127 )
	{
		temp -= 256 ;
	}
	g_model.musicData.musicPrevSwitch = temp ;
#else
	g_model.musicData.musicPrevSwitch = edit_dr_switch( TRIGHT-TRMARGIN-4*FW, y+TVOFF, g_model.musicData.musicPrevSwitch, 0, doedit, event ) ;
#endif
	restoreEditColours() ;
	y += TFH ;
	subN += 1 ;
	
	attr = 0 ;
  if(sub==subN)
	{
	  attr = INVERS ;
	}
	drawItem( (char *)XPSTR("Next Switch"), y, ( attr ) ) ;
	doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
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
#ifdef X20
	temp = g_model.musicData.musicNextSwitch ;
	if ( temp < -80 )
	{
		temp += 256 ;
	}
	temp = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, temp, LUA_RIGHT, doedit, event ) ;
	if ( temp > 127 )
	{
		temp -= 256 ;
	}
	g_model.musicData.musicNextSwitch = temp ;
#else
	g_model.musicData.musicNextSwitch = edit_dr_switch( TRIGHT-TRMARGIN-4*FW, y+TVOFF, g_model.musicData.musicNextSwitch, 0, doedit, event ) ;
#endif
	restoreEditColours() ;
	
	EditType = EE_GENERAL ;

#ifdef X20
  if ( event == EVT_KEY_BREAK(BTN_RE) )
#else
  if ( ( event == EVT_KEY_FIRST(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
#endif
	{
		if ( sub > 3 )
		{
			killEvents(event) ;
			s_editMode = 0 ;
			event = 0 ;
		}
		switch ( sub )
		{
			case 5 :
				VoiceFileType = VOICE_FILE_TYPE_MUSIC ;
      	pushMenu( menuSelectVoiceFile ) ;
			break ;
			case 7 :
				if ( MusicPlaying == MUSIC_STOPPED )
				{
					MusicPlaying = MUSIC_STARTING ;
				}
				else
				{
					MusicPlaying = MUSIC_STOPPING ;
				}
			break ;

			case 8 :
				if ( MusicPlaying == MUSIC_PAUSED )
				{
					MusicPlaying = MUSIC_RESUMING ;
				}
				else
				{
					MusicPlaying = MUSIC_PAUSING ;
				}
			break ;
		}
	}
	if ( g_eeGeneral.musicType )
	{
		if ( MusicPlaying == MUSIC_PLAYING )
		{
			if ( sub == 8 )
			{
			  if ( event == EVT_KEY_FIRST(KEY_LEFT) )
				{
					killEvents(event) ;
					MusicPrevNext = MUSIC_NP_PREV ;
				}
			  if ( event == EVT_KEY_FIRST(KEY_RIGHT) )
				{
					killEvents(event) ;
					MusicPrevNext = MUSIC_NP_NEXT ;
				}
			}
		}
	}

	y += TFH ;
	subN += 1 ;

	attr = 0 ;
  if(sub==subN)
	{
	  attr = INVERS ;
	}
	drawItem( (char *)XPSTR("Type"), y, ( attr ) ) ;
	uint32_t b = g_eeGeneral.musicType ;
	drawIdxText( y, XPSTR("\004NameList"), b, attr|LUA_RIGHT ) ; //0, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
  if ( attr )
	{
		g_eeGeneral.musicType = checkIncDec16( b, 0, 1, EditType ) ;
	}
	if ( g_eeGeneral.musicType != b )
	{
		g_eeGeneral.musicVoiceFileName[0] = '\0' ;
	}
	y += TFH ;
	subN += 1 ;

	attr = 0 ;
  if ( sub==subN )
	{
	  attr = INVERS ;
	}
	drawItem( (char *)XPSTR("File"), y, ( attr ) ) ;
#ifdef X20
	lcd_putsAttColour( TRIGHT-TRMARGIN-MUSIC_NAME_LENGTH*FW, y+TVOFF, (char *)g_eeGeneral.musicVoiceFileName, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
#else
	lcdDrawSolidFilledRectDMA( (TMID-4*FW)*TSCALE, y*TSCALE+2, 4*FW*TSCALE, TFH*TSCALE-2, ( attr ) ? ~colour : colour ) ;

	PUTS_ATT_COLOUR( TRIGHT-TRMARGIN-MUSIC_NAME_LENGTH*FW, y+TVOFF, (char *)g_eeGeneral.musicVoiceFileName, 0, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
#endif
#ifdef TOUCH	
	if (attr)
	{
//		lcdHiresRect( (TRIGHT-TRMARGIN-MUSIC_NAME_LENGTH*FW)*TSCALE-1, (y+TVOFF)*TSCALE-1, MUSIC_NAME_LENGTH*FW*TSCALE+2, 17, LCD_BLACK ) ;
		if ( handleSelectIcon() || selected )
		{
			VoiceFileType = VOICE_FILE_TYPE_MUSIC ;
     	pushMenu( menuSelectVoiceFile ) ;
		}
	}
#endif
	y += TFH ;
	subN += 1 ;

  g_eeGeneral.musicLoop = touchOnOffItem( g_eeGeneral.musicLoop, y, XPSTR("Loop"), (sub == subN ), colour ) ;

	y += TFH ;
	subN += 1 ;
	
	attr = 0 ;
  if(sub==subN)
	{
	  attr = INVERS ;
	}
	drawItem( (char *)XPSTR("Play"), y, ( attr ) ) ;
	if ( MusicPlaying == MUSIC_STOPPED )
	{
#ifdef X20
		lcd_putsAttColour( TRIGHT-TRMARGIN-5*FW, y+TVOFF, (char *)XPSTR("Start"), 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
#else
		PUTS_ATT_COLOUR( TRIGHT-TRMARGIN-5*FW, y+TVOFF, (char *)XPSTR("Start"), 0, attr ? ~LcdForeground : LcdForeground) ; //, attr ? ~colour : colour ) ;
#endif
	}
	else
	{
#ifdef X20
		lcd_putsAttColour( TRIGHT-TRMARGIN-4*FW, y+TVOFF, (char *)XPSTR("Stop"), 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
#else
		PUTS_ATT_COLOUR( TRIGHT-TRMARGIN-4*FW, y+TVOFF, (char *)XPSTR("Stop"), 0, attr ? ~LcdForeground : LcdForeground) ; //, attr ? ~colour : colour ) ;
#endif
		y += TFH ;
		subN += 1 ;

		attr = 0 ;
  	if(sub==subN)
		{
		  attr = INVERS ;
		}
		drawItem( (char *)XPSTR(""), y, ( attr ) ) ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;	// Blanks line
#ifdef X20
		lcd_putsAttColour( TRIGHT-TRMARGIN-6*FW, y+TVOFF, (char *)(MusicPlaying == MUSIC_PAUSED ? XPSTR("Resume") : XPSTR(" Pause")), 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
#else
		PUTS_ATT_COLOUR( TRIGHT-TRMARGIN, y+TVOFF, (char *)(MusicPlaying == MUSIC_PAUSED ? XPSTR("Resume") : XPSTR(" Pause")), LUA_RIGHT, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
#endif
		if ( g_eeGeneral.musicType )
		{
			if ( MusicPlaying == MUSIC_PLAYING )
			{
#ifdef X20
				lcdDrawText( TRIGHT+FW, y+TVOFF, XPSTR("Next")) ;
				lcdDrawText( TRIGHT+FW, y-TFH+TVOFF, XPSTR("Prev")) ;
#else
				PUTS_P( TRIGHT+FW, y+TVOFF, XPSTR("Next")) ;
				PUTS_P( TRIGHT+FW, y-TFH+TVOFF, XPSTR("Prev")) ;
#endif
#ifdef TOUCH	
				if ( checkTouchposition( (TRIGHT+FW)*TSCALE, (y-TFH)*TSCALE, LCD_W-1, y*TSCALE ) )
				{
					MusicPrevNext = MUSIC_NP_PREV ;
				}
				if ( checkTouchposition( (TRIGHT+FW)*TSCALE, y*TSCALE, LCD_W-1, (y+TFH)*TSCALE ) )
				{
					MusicPrevNext = MUSIC_NP_NEXT ;
				}
#endif
			}
		}
#ifdef X20
		lcdDrawText( FW, 13*FH, CurrentPlayName ) ;
#else
		PUTS_P( FW, 14*FH, CurrentPlayName ) ;
#endif
	}

	uint32_t a ;
	a = BgTotalSize ;
	b = BgSizePlayed ;
	if ( a > 14000000 )
	{
		a /= 64 ;
		b /= 64 ;
	}
	uint16_t percent = ( a - b) * 300 / a ;
	uint16_t x = 20*TSCALE ;
#ifdef X20
	uint16_t yb = 14*FH ;
	uint16_t w = 701 ;
#else
	uint16_t yb = 15*FH*TSCALE+6 ;
	uint16_t w = 401 ;
#endif
	uint16_t h = 28/TSCALE ;
	uint16_t solid ;
	if ( percent > 300 )
	{
		percent = 300 ;
	}
	solid = (w-2) * percent / 300 ;
//	lcd_rect( x, yb, w, h ) ;
	lcdHiresRect( x, yb, w, h, LCD_BLACK ) ;

	if ( MusicPlaying != MUSIC_STOPPED )
	{
		if ( solid )
		{
			pushPlotType( PLOT_BLACK ) ;
			w = yb + h - 1 ;
			yb += 1 ;
			x += 1 ;
			while ( yb < w )
			{
 				lcd_hline(x/TSCALE, yb/TSCALE, solid/TSCALE ) ;
				yb += 1 ;			
			}
			popPlotType() ;
		}
	}
}

void menuRadioVars(uint8_t event)
{
	TITLE(XPSTR("Radio Vars"));
	static MState2 mstate2 ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	lcd_hline( 0, TTOP, TRIGHT ) ;
	uint32_t rows = NUM_RADIO_VARS ;
	mstate2.check_columns( event, rows-1 ) ;
	
	uint32_t i ;
	uint8_t sub = mstate2.m_posVert ;
	
#ifdef TOUCH
	sub = mstate2.m_posVert = handleTouchSelect( rows, 0, sub ) ;
#endif

 	for( i = 0 ; i < rows ; i += 1 )
	{
    uint16_t y = i * TFH + TTOP ;
    uint8_t attr = (sub==i) ? INVERS : 0 ;

		drawItem( (char *)"Radio Var", y, attr ) ;
#ifdef X20
	  lcdDrawChar( 10*FW+THOFF, y+TVOFF, i+'1' ) ;
#else
	  PUTC( 10*FW+THOFF, y+TVOFF, i+'1' ) ;
#endif

		drawNumber( TRIGHT-TRMARGIN, y, g_eeGeneral.radioVar[i], attr ) ; //, attr ? ~colour : colour ) ;
		if(attr)
		{
			g_eeGeneral.radioVar[i] = checkIncDec16( g_eeGeneral.radioVar[i], -1024, 1024, EE_GENERAL ) ;
   	}
	}
}

#ifdef X20
#define varioExtraData varioData
#endif

void menuVario(uint8_t event)
{
	touchMenuTitle( (char *)XPSTR("Vario") ) ;
	static MState2 mstate2 ;
	uint32_t rows = 7 ;
	mstate2.check_columns( event, rows-1 ) ;
	uint8_t sub = mstate2.m_posVert ;
	uint8_t subN = 0 ;
	uint16_t colour = dimBackColour() ;
 	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif
	 
	for( uint32_t j = 0 ; j < 7 ; j += 1 )
	{
		LcdFlags attr = (sub==subN) ? INVERS : 0 ;
		coord_t y = TTOP + j * TFH ;

		switch ( j )
		{
			case 0 :
				drawItem( (char *)PSTR(STR_VARIO_SRC), y, attr ) ;
				drawIdxText( y, (CHAR *)PSTR(STR_VSPD_A2), g_model.varioData.varioSource, attr|LUA_RIGHT ) ; // , attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
   		  if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( g_model.varioData.varioSource, 2+NUM_SCALERS ) ;
   		  }
			break ;
				
			case 1 :
				drawItem( (char *)PSTR(STR_2SWITCH)+1, y, attr ) ;
				saveEditColours( attr, colour ) ;
				g_model.varioData.swtch = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, g_model.varioData.swtch, LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				restoreEditColours() ;
			break ;

			case 2 :
				drawItem( (char *)PSTR(STR_2SENSITIVITY)+1, y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, g_model.varioData.param, attr ) ;
   			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( g_model.varioData.param, 50 ) ;
	   		}
			break ;

			case 3 :
				drawItem( (char *)XPSTR("Base Freq."), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, g_model.varioExtraData.baseFrequency, attr ) ;
   			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR( g_model.varioExtraData.baseFrequency, -50, 50 ) ;
	   		}
			break ;

			case 4 :
				drawItem( (char *)XPSTR("Offset Freq."), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, g_model.varioExtraData.offsetFrequency, attr ) ;
   			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR( g_model.varioExtraData.offsetFrequency, -20, 20 ) ;
	   		}
			break ;

			case 5 :
				drawItem( (char *)XPSTR("Volume"), y, attr ) ;
				if ( g_model.varioExtraData.volume )
				{
					drawNumber( TRIGHT-TRMARGIN, y, g_model.varioExtraData.volume, attr ) ;
				}
				else
				{
					drawText( TRIGHT-TRMARGIN, y, (char *)XPSTR("Vol"), attr|LUA_RIGHT ) ;
				}
   			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( g_model.varioExtraData.volume, NUM_VOL_LEVELS-1 ) ;
	   		}
			break ;

			case 6 :
				g_model.varioData.sinkTones = touchOnOffItem( g_model.varioData.sinkTones, y, PSTR(STR_SINK_TONES), attr, colour ) ;
			break ;
		}
		subN += 1 ;
	}
}

#ifdef X20
#undef varioExtraData
#endif

void menuLimitsOne(uint8_t event)
{
	touchMenuTitle( (char *)PSTR(STR_LIMITS) ) ;
	static MState2 mstate2 ;
	uint8_t attr ;
	uint32_t index ;
	uint32_t rows = 4 ;
	index = s_currIdx ;
  int8_t limit = (g_model.extendedLimits ? 125 : 100) ;
	uint16_t y = TTOP ;
	int16_t value ;
	int16_t t = 0 ;
	
	mstate2.check_columns(event, rows-1 ) ;
	
	uint8_t sub = mstate2.m_posVert ;

	putsChn( 14*FW, 0,index+1,0 ) ;
	lcdDrawNumber( 23*FW*TSCALE, 0, g_chans512[index]/2 + 1500 ) ;
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif

  LimitData *ld = &g_model.limitData[index];
#ifndef X20
 #if EXTRA_SKYCHANNELS
	if ( index >= NUM_SKYCHNOUT )
	{
		ld = &g_model.elimitData[index-NUM_SKYCHNOUT] ;
	}
 #endif			
#endif			
	attr = sub==0 ? INVERS : 0 ;
	
	drawItem( XPSTR("Sub Trim"), y, (sub == 0 ) ) ;
	drawNumber( TRIGHT-TRMARGIN, y, ld->offset, attr|PREC1 ) ; //, attr ? ~colour : colour ) ;
	if ( attr )
	{
		StepSize = 50 ;
		ld->offset = checkIncDec16( ld->offset, -1000, 1000, EE_MODEL ) ;
	}
	y += TFH ;
	attr = sub==1 ? INVERS : 0 ;
	if ( g_model.sub_trim_limit )
	{
		if ( ( t = ld->offset ) )
		{
			if ( t > g_model.sub_trim_limit )
			{
				t = g_model.sub_trim_limit ;
			}
			else if ( t < -g_model.sub_trim_limit )
			{
				t = -g_model.sub_trim_limit ;
			}
		}
	}
	value = t / 10 ;
	value += (int8_t)(ld->min-100) ;
	if ( value < -125 )
	{
		value = -125 ;						
	}
	drawItem( XPSTR("Min"), y, (sub == 1 ) ) ;
	drawNumber( TRIGHT-TRMARGIN, y, value, attr) ; //, attr ? ~colour : colour ) ;
  if(attr)
	{
    ld->min -=  100;
		CHECK_INCDEC_H_MODELVAR( ld->min, -limit,25);
    ld->min +=  100;
  }
	y += TFH ;
	attr = sub==2 ? INVERS : 0 ;
	value = t / 10 ;
	value += (int8_t)(ld->max+100) ;
	if ( value > 125 )
	{
		value = 125 ;						
	}

	drawItem( XPSTR("Max"), y, (sub == 2 ) ) ;
	drawNumber( TRIGHT-TRMARGIN, y, value, attr) ; //, attr ? ~colour : colour ) ;
  if(attr)
	{
    ld->max +=  100;
    CHECK_INCDEC_H_MODELVAR( ld->max, -25,limit);
    ld->max -=  100;
  }
	y += TFH ;
	attr = sub==3 ? INVERS : 0 ;

	drawItem( XPSTR("Reverse"), y, (sub == 3 ) ) ;
//	lcd_putsAttIdxColour( TRIGHT-TRMARGIN-3*FW, y+TVOFF, PSTR( STR_HYPH_INV)+2, ld->revert, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
	drawIdxText( y, (char *)PSTR( STR_HYPH_INV)+2, ld->revert, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
  if(attr)
	{
		ld->revert = checkIncDec( ld->revert, 0, 1, EditType ) ;
	}
}

extern const char StringAdjustFunctions[] = "\005-----Add  Set CSet V+/-  Inc/0Dec/0+/Lim-/Lim" ;

void menuOneAdjust(uint8_t event)
{
	touchMenuTitle( (char *)XPSTR("GVAR Adjust") ) ;
	EditType = EE_MODEL ;
	static MState2 mstate2;
	uint32_t rows = 4 ;
 	event = mstate2.check_columns(event, rows - 1 ) ;

	uint8_t sub = mstate2.m_posVert ;
	GvarAdjust *pgvaradj ;
#ifdef X20
	pgvaradj = &g_model.gvarAdjuster[s_currIdx] ;
 	lcdDrawNumber(21*FW, 0, 1+ s_currIdx, 0 ) ;
#else
	pgvaradj = ( s_currIdx >= NUM_GVAR_ADJUST ) ? &g_model.egvarAdjuster[s_currIdx - NUM_GVAR_ADJUST] : &g_model.gvarAdjuster[s_currIdx] ;
 	PUTS_NUM(21*FW, 0, 1+ s_currIdx, 0 ) ;
#endif

	uint16_t colour = dimBackColour() ;

#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif

	for (uint32_t i = 0 ; i < rows ; i += 1 )
	{
    uint16_t y = i * TFH + TTOP ;
    uint8_t attr = (sub==i) ? INVERS : 0 ;

		switch(i)
		{
			case 0 : // GV
				drawItem( (char *)XPSTR("GVAR"), y, attr ) ;
				if ( pgvaradj->gvarIndex > 6 )
				{
					drawIdxText( y, (char *)PSTR(STR_GV_SOURCE), pgvaradj->gvarIndex-6, attr ) ;
				}
				else
				{
					drawNumber( TRIGHT-TRMARGIN, y, pgvaradj->gvarIndex+1, attr ) ;
				}
  			if(attr)
				{
 	        CHECK_INCDEC_H_MODELVAR( pgvaradj->gvarIndex, 0, 10 ) ;
				}
			break ;
			case 1 : // Fn
			{	
				uint32_t old = pgvaradj->function ;
				drawItem( (char *)XPSTR("Function"), y, attr ) ;
				drawIdxText( y, (char *)StringAdjustFunctions, pgvaradj->function, attr ) ;
 				if(attr)
				{ 
					CHECK_INCDEC_H_MODELVAR( pgvaradj->function, 0, 8 ) ;
					if ( pgvaradj->function != old )
					{
						if ( ( old < 4 ) || ( old > 6 ) )
						{
							if ( pgvaradj->function >= 4 )
							{
								pgvaradj->switch_value = 0 ;
							}
						}
						else
						{
							if ( ( pgvaradj->function < 4 ) || ( pgvaradj->function > 6 ) )
							{
								pgvaradj->switch_value = 0 ;
							}
						}
					}
				}
			}
			break ;
			case 2 : // switch
				drawItem( (char *)PSTR(STR_SWITCH), y, attr ) ;
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
				pgvaradj->swtch = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, pgvaradj->swtch, LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				restoreEditColours() ;
			break ;
			case 3 : // Param
				if ( ( pgvaradj->function < 4 ) || ( pgvaradj->function > 6 ) )
				{
					if ( pgvaradj->function == 3 )
					{
						drawItem( (char *)XPSTR("Source"), y, attr ) ;
						drawIdxText( y, (char *)PSTR(STR_GV_SOURCE), pgvaradj->switch_value, attr ) ;
		  			if(attr)
						{
							CHECK_INCDEC_H_MODELVAR( pgvaradj->switch_value, 0, 69 ) ;
						}
					}
					else
					{
						drawItem( (char *)XPSTR("Value"), y, attr ) ;
						drawNumber( TRIGHT-TRMARGIN, y, pgvaradj->switch_value, attr ) ;
						if ( attr )
						{
 	      	  	CHECK_INCDEC_H_MODELVAR( pgvaradj->switch_value, -125, 125 ) ;
						}
					}
				}
				else
				{
					drawItem( (char *)PSTR(STR_SWITCH), y, attr ) ;
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
					pgvaradj->switch_value = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, pgvaradj->switch_value, LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
					restoreEditColours() ;
				}
			break ;
		}
	}
}




void menuAdjust(uint8_t event)
{
	touchMenuTitle( (char *)XPSTR("GVAR Adjust") ) ;
	EditType = EE_MODEL ;
#ifdef X20
	uint32_t rows = NUM_GVAR_ADJUST ;
#else
	uint32_t rows = NUM_GVAR_ADJUST + EXTRA_GVAR_ADJUST ;
#endif
	static MState2 mstate2;
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
 	event = mstate2.check_columns(event, rows - 1 ) ;
	
	uint32_t sub = mstate2.m_posVert ;
	uint16_t y ;
	
#ifdef TOUCH	
	uint32_t selected = 0 ;
	uint32_t newVpos ;
#endif
	uint8_t k = 0 ;
  uint32_t t_pgOfs ;
	t_pgOfs = evalHresOffset( sub ) ;

#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
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
		pushMenu(menuOneAdjust) ;
  }
	
#ifdef TOUCH	
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
#endif


	for (uint32_t i=0; i<TLINES; i++ )
	{
		GvarAdjust *pgvaradj ;
		uint8_t idx ;
    y=(i)*TFH +TTOP+TVOFF ;
    k=i+t_pgOfs;
#ifdef X20
		pgvaradj = &g_model.gvarAdjuster[k] ;
#else
		pgvaradj = ( k >= NUM_GVAR_ADJUST ) ? &g_model.egvarAdjuster[k - NUM_GVAR_ADJUST] : &g_model.gvarAdjuster[k] ;
#endif
		idx = pgvaradj->gvarIndex ;
  	PUTC( 0, y, 'A' ) ;
		if ( k < 9 )
		{
  		PUTC( 1*FW, y, k+'1' ) ;
		}
		else
		{
  		PUTC( 1*FW, y, k > 18 ? '2' : '1' ) ;
  		PUTC( 2*FW, y, ( (k+1) % 10) +'0' ) ;
		}

		if ( sub==k )
		{
			int16_t value ;
	 		PUTC( 20*FW, 0, '=' ) ;
			value = 7 ;
			if ( idx >= value )
			{
				value = getTrimValueAdd( CurrentPhase, idx - value  ) ;
#ifdef X20
				lcdDrawTextAtIndex( 17*FW, 0, PSTR(STR_GV_SOURCE), idx-(value-1), 0 ) ;
#else
				PUTS_AT_IDX( 17*FW, 0, PSTR(STR_GV_SOURCE), idx-(value-1), 0 ) ;
#endif
			}
			else
			{
				value = getGvar(idx) ;
				dispGvar( 17*FW, 0, idx+1, 0 ) ;
			}
			PUTS_NUMX( 25*FW, 0, value ) ;
		} 
		uint8_t attr = ((sub==k) ? INVERS : 0);
		
		uint16_t oldFcolour = LcdForeground ;
		
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 3*FW*TSCALE, (y-TVOFF)*TSCALE+2, (TRIGHT-3*FW)*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdForeground = ~LcdForeground ;
		}

		if ( idx > 6 )
		{
#ifdef X20
			lcdDrawTextAtIndex( 3*FW+2, y, PSTR(STR_GV_SOURCE), idx-6, attr ) ;
		}
		else
		{
  		lcdDrawText( 3*FW+2, y, XPSTR("GV") ) ;
			lcdDrawChar( 5*FW+2, y, idx+'1', 0 ) ;
		}
		lcdDrawTextAtIndex( 7*FW, y, StringAdjustFunctions, pgvaradj->function, 0);
    putsDrSwitches(13*FW-1, y, pgvaradj->swtch, 0);
		if ( ( pgvaradj->function < 4 ) || ( pgvaradj->function > 6 ) )
		{
			if ( pgvaradj->function == 3 )
			{
				lcdDrawTextAtIndex( 18*FW, y, PSTR(STR_GV_SOURCE), pgvaradj->switch_value, attr ) ;
			}
			else
			{
				lcdDrawNumber( 21*FW, y, pgvaradj->switch_value, 0 ) ;
#else
			PUTS_AT_IDX( 3*FW+2, y, PSTR(STR_GV_SOURCE), idx-6, attr ) ;
		}
		else
		{
  		PUTS_P( 3*FW+2, y, XPSTR("GV") ) ;
			PUTC_ATT( 5*FW+2, y, idx+'1', 0 ) ;
		}
		PUTS_AT_IDX( 7*FW, y, StringAdjustFunctions, pgvaradj->function, 0);
    putsDrSwitches(13*FW-1, y, pgvaradj->swtch, 0);
		if ( ( pgvaradj->function < 4 ) || ( pgvaradj->function > 6 ) )
		{
			if ( pgvaradj->function == 3 )
			{
				PUTS_AT_IDX( 18*FW, y+TVOFF, PSTR(STR_GV_SOURCE), pgvaradj->switch_value, attr ) ;
			}
			else
			{
				PUTS_NUM( 21*FW, y+TVOFF, pgvaradj->switch_value, 0 ) ;
#endif
			}
		}
		else
		{
      putsDrSwitches(17*FW, y, pgvaradj->switch_value, 0);
		}
		LcdForeground = oldFcolour ;
		lcd_hline( 0, y+TFH-TVOFF, TRIGHT ) ;
	}
}

static uint8_t s_scalerSource ;

void menuScaleOne(uint8_t event)
{
	touchMenuTitle( (char *)XPSTR("SC     =") ) ;
	uint8_t index = s_currIdx ;
  PUTC_ATT( 6*FW, 0, index+'1', INVERS ) ;

	static MState2 mstate2 ;
	uint32_t rows = 13 ;
	mstate2.check_columns(event, rows-1 ) ;
#ifdef TOUCH
	uint32_t selected = 0 ;
	uint32_t newVpos ;
#endif
	
  uint8_t sub = mstate2.m_posVert;
	uint16_t t_pgOfs ;
	t_pgOfs = evalHresOffset( sub ) ;
#ifndef TOUCH
	(void) t_pgOfs ;
#endif	 
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
	selected = newVert & 0x0100 ;
//	if ( newSelection >= 0 )
//	{
//		sub = mstate2.m_posVert = newSelection ;
//	}

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
#endif

	uint16_t colour = dimBackColour() ;

	putsTelemetryChannel( 15*FW, 0, index+TEL_ITEM_SC1, 0, 0, TELEM_UNIT ) ;

	ScaleData *pscaler ;
	pscaler = &g_model.Scalers[index] ;
#ifndef X20
	ExtScaleData *epscaler ;
	epscaler = &g_model.eScalers[index] ;
#endif
	if ( event == EVT_ENTRY )
	{
		RotaryState = ROTARY_MENU_UD ;
	}
	else if ( event == EVT_ENTRY_UP )
	{
		// Returned from editing
		if ( TextResult )
		{
			if ( s_scalerSource )
			{
#ifndef X20
				epscaler->exSource = unmapPots( TextIndex ) ;
#else
				pscaler->exSource = TextIndex ;
#endif
			}
			else
			{
#ifndef X20
				pscaler->source = unmapPots( TextIndex ) ;
#else
				pscaler->source = TextIndex ;
#endif
			}
	    eeDirty(EE_MODEL) ;
		}
	}
  
  for( uint32_t k = 0 ; k < TLINES ; k += 1 )
	{
#ifndef X20
		uint16_t t ;
#endif
    uint16_t y = k * TFH + TTOP ;
    uint8_t i = k + s_pgOfs ;
		uint8_t attr = (sub==i ? INVERS : 0) ;
		uint16_t oldBcolour = LcdBackground ;
		switch(i)
		{
      case 0 :	// Source
			case 7 :	// exSource
			{	
				uint8_t x ;
				
				if ( i == 0 )
				{				
					drawItem( (char *)XPSTR("Source"), y, attr ) ;
					x = pscaler->source ;
				}
				else
				{
					drawItem( (char *)XPSTR("ex Source"), y, attr ) ;
#ifdef X20
					x = pscaler->exSource ;
#else
					x = epscaler->exSource ;
#endif
				}
				saveEditColours( attr, colour ) ;
				putsChnRaw( (TRIGHT-TRMARGIN)*TSCALE, (y+TVOFF)*TSCALE, x, LUA_RIGHT ) ;
				restoreEditColours() ;
				if( attr )
				{
#ifdef X20
					x = offsetCheckOutOfOrder( x, TelemMap, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS, NUM_SKYXCHNRAW+1, EE_MODEL ) ;
					if ( i == 0 )
					{
						pscaler->source = x ;
					}
					else
					{
						pscaler->exSource = x ;
					}
#else
					x = mapPots( x ) ;
					x = checkIncDec16( x,0,NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots-1+1,EE_MODEL);
					if ( i == 0 )
					{
						pscaler->source = unmapPots( x ) ;
					}
					else
					{
						epscaler->exSource = unmapPots( x ) ;
					}
#endif					 
#ifdef TOUCH
					if ( checkForMenuEncoderBreak( event ) || selected )
#else
					if ( checkForMenuEncoderBreak( event ) )
#endif
					{
						// Long MENU pressed
						if ( i == 0 )
						{				
							x = pscaler->source ;
							s_scalerSource = 0 ;
						}
						else
						{
#ifdef X20
							x = pscaler->exSource ;
#else
							x = epscaler->exSource ;
#endif
							s_scalerSource = 1 ;
						}
#ifdef X20
						TextIndex = x ;
#else
						TextIndex = mapPots( x ) ;
#endif
  				  TextType = TEXT_TYPE_SW_SOURCE ;
  				  killEvents(event) ;
						pushMenu(menuTextHelp) ;
					}
//					else
//					{
//#ifdef X20
//						x = offsetCheckOutOfOrder( x, TelemMap, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS, NUM_SKYXCHNRAW+1, EE_MODEL ) ;
//#else
//						x = checkIncDec16( x,0,NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots-1+1,EE_MODEL) ;
//#endif
//					}
				}
			}
			break ;
			case 1 :	// name
				drawItem( (char *)XPSTR("Scaler Name"), y, attr ) ;
				LcdBackground = attr ? ~colour : colour ;
				alphaEditName( TRIGHT-sizeof(pscaler->name)*FW-FW, y+TVOFF, (uint8_t *)pscaler->name, sizeof(pscaler->name), attr|ALPHA_NO_NAME, (uint8_t *)0 ) ;
	 			if ( AlphaEdited )
				{
					sortTelemText() ;				
				}
				LcdBackground = oldBcolour ;
			break ;
      case 2 :	// offset
				drawItem( (char *)PSTR(STR_OFFSET), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, pscaler->offset, attr) ; //, attr ? ~colour : colour ) ;
				if ( attr )
				{
					StepSize = 100 ;
					pscaler->offset = checkIncDec16( pscaler->offset, -32000, 32000, EE_MODEL ) ;
				}
			break ;
      case 3 :	// mult
				drawItem( (char *)XPSTR("Multiplier"), y, attr ) ;
				saveEditColours( attr, colour ) ;
#ifdef X20
				pscaler->mult = scalerDecimal( y, pscaler->mult, attr ) ;
#else
				t = pscaler->mult + ( pscaler->multx << 8 ) ;
				t = scalerDecimal( y, t, attr ) ;
				pscaler->mult = t ;
				pscaler->multx = t >> 8 ;
#endif
				restoreEditColours() ;
			break ;
      case 4 :	// div
				drawItem( (char *)XPSTR("Divisor"), y, attr ) ;
				saveEditColours( attr, colour ) ;
#ifdef X20
				pscaler->div = scalerDecimal( y, pscaler->div, attr ) ;
#else
				t = pscaler->div + ( pscaler->divx << 8 ) ;
				t = scalerDecimal( y, t, attr ) ;
				pscaler->div = t ;
				pscaler->divx = t >> 8 ;
#endif
				restoreEditColours() ;
			break ;
      case 5 :	// mod
				drawItem( (char *)XPSTR("Mod Value"), y, attr ) ;
				saveEditColours( attr, colour ) ;
#ifdef X20
				pscaler->mod = scalerDecimal( y, pscaler->mod, attr ) ;
#else
				epscaler->mod = scalerDecimal( y, epscaler->mod, attr ) ;
#endif
				restoreEditColours() ;
			break ;
      case 6 :	// offsetLast
				drawItem( (char *)XPSTR("Offset At"), y, attr ) ;
				drawIdxText( y, XPSTR("\005FirstLast "), pscaler->offsetLast, attr|LUA_RIGHT ) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->offsetLast, 1 ) ;
			break ;
			case 8 :
				drawItem( (char *)XPSTR("Function"), y, attr ) ;
			  if(attr)
				{
					CHECK_INCDEC_H_MODELVAR( pscaler->exFunction, 0, 6 ) ;
				}
				drawIdxText( y, (char *)XPSTR("\010--------     AddSubtractMultiply  Divide     Mod     Min"), pscaler->exFunction, attr ) ;
			break ;
			case 9 :	// unit
				drawItem( (char *)XPSTR("Unit"), y, attr ) ;
				drawIdxText( y, (char *)XPSTR(UnitsString), pscaler->unit, attr|LUA_RIGHT ) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->unit, 10 ) ;
			break ;
      case 10 :	// sign
				drawItem( (char *)XPSTR("Sign"), y, attr ) ;
				saveEditColours( attr, colour ) ;
  			PUTC_ATT( TRIGHT-TRMARGIN-FW, y+TVOFF, pscaler->neg ? '-' : '+', 0 ) ;
				restoreEditColours() ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->neg, 1 ) ;
			break ;
      case 11 :	// precision
				drawItem( (char *)XPSTR("Decimals"), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, pscaler->precision, attr ) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->precision, 2 ) ;
			break ;
      case 12 :	// Dest
				drawItem( (char *)XPSTR("Dest"), y, attr ) ;
#define NUM_SCALE_DESTS		17
				if( attr )
				{
#ifdef X20
					CHECK_INCDEC_H_MODELVAR( pscaler->dest, 0, NUM_SCALE_DESTS ) ;
#else
					CHECK_INCDEC_H_MODELVAR( epscaler->dest, 0, NUM_SCALE_DESTS ) ;
#endif
				}
#ifdef X20
				drawIdxText( y, (char *)XPSTR(DestString), pscaler->dest, attr ) ;
#else
				drawIdxText( y, XPSTR(DestString), epscaler->dest, attr|LUA_RIGHT ) ;
#endif
			break ;
		}
	}
}

void menuCellScaling(uint8_t event)
{
	touchMenuTitle( (char *)XPSTR("Cell Scaling") ) ;
	static MState2 mstate2 ;
	uint32_t rows = 12 ;
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	mstate2.check_columns(event, rows-1 ) ;	
	
	coord_t y = 0 ;
	uint32_t k ;
	uint32_t t_pgOfs ;
	uint32_t index ;
  uint32_t sub = mstate2.m_posVert ;
#ifdef TOUCH
	uint32_t newVpos ;
#endif
	t_pgOfs = evalHresOffset( sub ) ;
	
#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	uint32_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
	
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
#endif

	for( uint32_t i = 0 ; i < TLINES ; i += 1 )
	{
    y = i * TFH + TTOP ;
    k=i+t_pgOfs;
    LcdFlags attr = (sub==k) ? INVERS : 0 ;
		drawNumberedItem( (char *)XPSTR("Cell"), y, attr, k+1 ) ;
//		drawItem( (char *)XPSTR("Cell"), y, attr ) ;
//		PUTS_ATT_LEFT( y, XPSTR( "Cell\023v" ) ) ;
// 		PUTS_NUM( LcdNextPos, y+TVOFF/2, k+1, LEFT ) ;
//		PUTS_NUMX(  6*FW, y, k+1 ) ;
		
//    attr = ((sub==k) ? InverseBlink : 0);
//    attr = (sub==i) ? INVERS : 0 ;
		uint32_t scaling = 1000 + g_model.cellScalers[k] ;
// 		PUTS_NUM( TRIGHT-FW-TRMARGIN, y+TVOFF/2, scaling/10, attr | PREC2 ) ;
// 		PUTS_NUM( TRIGHT-FW-TRMARGIN, y+TVOFF/2, scaling%10, attr | LEFT ) ;
		drawNumber( TRIGHT-TRMARGIN-FW, y, scaling/10, attr|PREC2 ) ;
		drawNumber( TRIGHT-TRMARGIN-FW, y, scaling%10, attr|LEFT ) ;
		
		if( attr )
		{
    	CHECK_INCDEC_H_MODELVAR( g_model.cellScalers[k], -50, 50 ) ;
		}
	
		index = k + FR_CELL1 ;
		attr = TelemetryDataValid[index] ? 0 : BLINK ;
 		PUTS_NUM(  12*FW, y+TVOFF/TSCALE, TelemetryData[index], attr | PREC2 ) ;
		lcdDrawChar( 12*FW*TSCALE, y*TSCALE+TVOFF, 'v' ) ;
	}
}

#ifdef X20
#define TOTAL_CHANNELS NUM_SKYCHNOUT
#else
#define TOTAL_CHANNELS (NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)
#endif

void menuLimits(uint8_t event)
{
	static MState2 mstate2 ;
	touchMenuTitle( (char *)XPSTR("Limits") ) ;
	uint32_t rows = TOTAL_CHANNELS + 1 ;
	
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif

	event = mstate2.check_columns(event, rows - 1) ;

#ifdef TOUCH
	uint32_t selected = 0 ;
	uint32_t newVpos ;
#endif
	coord_t y = 0 ;
	uint32_t k = 0 ;
	uint32_t t_pgOfs ;

#ifdef TOUCH
	if ( ( event == EVT_ENTRY ) || ( event == EVT_ENTRY_UP ) )
	{
		LastTaction = 2 ;
		LastTselection = mstate2.m_posVert ;
	}
#endif
	 
	uint32_t sub = mstate2.m_posVert ;

	t_pgOfs = evalHresOffset( sub ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint32_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	selected = newVert & 0x0100 ;
#endif

	if ( sub < TOTAL_CHANNELS )
	{
		lcdDrawNumber( 15*FW*2, 0, g_chans512[sub]/2 + 1500 ) ;
	}

#ifdef TOUCH
	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, NUM_SKYCHNOUT+EXTRA_SKYCHANNELS+1-(TLINES)+1, t_pgOfs ) ;
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

	if ( handleSelectIcon() || selected )
	{
		s_currIdx = sub ;
		if ( sub < TOTAL_CHANNELS )
		{
			pushMenu(menuLimitsOne) ;
  	  s_editMode = false ;
		}
		else
		{
			if(sub==TOTAL_CHANNELS)
			{
				if ( event == 0 )
				{
					event = EVT_KEY_BREAK(BTN_RE) ;					
				}
			}
		}
	}
#endif

	switch(event)
	{
    case EVT_KEY_BREAK(BTN_RE):
			if(sub==TOTAL_CHANNELS)
			{
  			//last line available - add the "copy trim menu" line
  			s_noHi = NO_HI_LEN;
  			killEvents(event);
  			s_editMode = 0 ;
  			setStickCenter(1); //if highlighted and menu pressed - copy trims
			}
			else
			{
				s_currIdx = sub ;
				pushMenu(menuLimitsOne) ;
  		  s_editMode = false ;
			}
	  break ;
	}

	for( uint32_t i = 0 ; i < TLINES ; i++ )
	{
    y=(i)*TFH +TTOP ;
    k=i+t_pgOfs ;
    
    uint8_t attr = (sub==k) ? INVERS : 0 ;
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
		}
		if(k==TOTAL_CHANNELS)
		{
			break ;
		}
    
		LimitData *ld = &g_model.limitData[k];
#ifndef X20
		if ( k >= NUM_SKYCHNOUT )
		{
			ld = &g_model.elimitData[k-NUM_SKYCHNOUT] ;
		}
#endif
    
		drawNumberedItemWide( (char *)PSTR(STR_CH), y, attr, k+1 ) ;
		 
		drawNumber( 12*FW, y, ld->offset, attr|PREC1) ; //, attr ? ~LcdBackground : LcdBackground ) ;
		int16_t value ;
		int16_t t = 0 ;
		if ( g_model.sub_trim_limit )
		{
			if ( ( t = ld->offset ) )
			{
				if ( t > g_model.sub_trim_limit )
				{
					t = g_model.sub_trim_limit ;
				}
				else if ( t < -g_model.sub_trim_limit )
				{
					t = -g_model.sub_trim_limit ;
				}
			}
		}
		value = t / 10 ;
		value += (int8_t)(ld->min-100) ;
		if ( value < -125 )
		{
			value = -125 ;						
		}
		drawNumber( 18*FW, y, value, attr) ; //, attr ? ~LcdBackground : LcdBackground ) ;
		value = t / 10 ;
		value += (int8_t)(ld->max+100) ;
		if ( value > 125 )
		{
			value = 125 ;						
		}
		drawNumber( 24*FW, y, value, attr) ; //, attr ? ~LcdBackground : LcdBackground ) ;
		drawIdxText( y, (char *)PSTR( STR_HYPH_INV)+2, ld->revert, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
//		lcd_putsAttIdxColour( TRIGHT-TRMARGIN-3*FW, y+TVOFF, PSTR( STR_HYPH_INV)+2, ld->revert, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~LcdBackground : LcdBackground ) ;
	}
	if( k == TOTAL_CHANNELS )
	{
  	//last line available - add the "copy trim menu" line
  	uint8_t attr = (sub==TOTAL_CHANNELS) ? INVERS : 0 ;
//		if ( attr )
//		{
//			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
//		}
#ifdef X20
  	lcd_putsAttColour( 3*FW, y+TVOFF, PSTR(STR_COPY_TRIM), 0, attr ? ~LcdForeground : LcdForeground, attr ? ~LcdBackground : LcdBackground ) ;
#else
  	PUTS_ATT_COLOUR( 3*FW, y+TVOFF, PSTR(STR_COPY_TRIM), 0, attr ? ~LcdForeground : LcdForeground ) ; // , attr ? ~LcdBackground : LcdBackground ) ;
#endif
		lcd_hline( 0, y+TFH, TRIGHT ) ;
	}
}

void menuSwitchOne(uint8_t event)
{
	static MState2 mstate2 ;
	uint8_t attr ;
	uint32_t index ;
	uint32_t rows = 7 ;
	if ( Clipboard.content == CLIP_SWITCH )
	{
		rows += 1 ;
	}
	index = s_currIdx ;
#ifdef X20
	X20CSwData &cs = g_model.customSw[index] ;
#else
	SKYCSwData &cs = g_model.customSw[index] ;
#endif
	uint8_t cstate = CS_STATE(cs.func) ;
	uint32_t extra = 0 ;
	uint32_t Vextra = 0 ;
  if( cstate == CS_2VAL )
	{
		rows += 1 ;
		extra = 1 ;
		Vextra = TFH ;
	}

	touchMenuTitle( (char *)XPSTR("SWITCH") ) ;

	mstate2.check_columns(event, rows-1 ) ;

	uint8_t sub = mstate2.m_posVert ;

	if ( event == EVT_ENTRY_UP )
	{
		// Returned from editing
		sub = mstate2.m_posVert = saveHpos ;
		
		if ( TextResult )
		{
			if ( sub == 0 )
			{
				cs.func = SwitchFunctionMap[TextIndex] ;
			}
			else if ( sub == 1 )
			{
#ifdef X20
				cs.v1u =( TextIndex ) ;
#else
				cs.v1u = unmapPots( TextIndex ) ;
#endif
			}
			else
			{
#ifdef X20
				cs.v2u = ( TextIndex ) ;
#else
				cs.v2u = unmapPots( TextIndex ) ;
#endif
			}
	 	  eeDirty(EE_MODEL) ;
		}
	}


	//write SW name here
	displayLogicalSwitch( 11*FW, 0, index ) ;

	coord_t y ;
	uint16_t colour = dimBackColour() ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif
	 
	y = TTOP ;

	attr = 0 ;
	if ( sub == 0 )
	{
		attr = INVERS ;
	}
	drawItem( (char *)XPSTR("Function"), y, attr ) ;
	drawIdxText( y, (char *)PSTR(CSWITCH_STR), cs.func, attr|LUA_RIGHT ) ; // 0, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;

	y += TFH ;
	
	if (cstate != CS_TIMER)
	{
		drawItem( (char *)XPSTR("V1"), y, sub == 1 ) ;
	}
	else
	{
		drawItem( (char *)XPSTR(""), y, sub == 1 ) ;
	}

	y += TFH ;
	
	drawItem( (char *)XPSTR(""), y, sub == 2 ) ;
  if( (cstate == CS_VOFS) || (cstate == CS_2VAL) )
	{
#ifdef X20
		lcdDrawText( THOFF, TTOP+2*TFH+TVOFF, XPSTR("val") ) ;
#else
		PUTS_P( THOFF, TTOP+2*TFH+TVOFF, XPSTR("val") ) ;
#endif
	}
	else if( (cstate != CS_TIMER) && (cstate != CS_TMONO) )
	{
#ifdef X20
		lcdDrawText( THOFF, TTOP+2*TFH+TVOFF, XPSTR("V2") ) ;
#else
		PUTS_P( THOFF, TTOP+2*TFH+TVOFF, XPSTR("V2") ) ;
#endif
	}
	 
	y += TFH ;

	if ( extra )
	{
		drawItem( (char *)XPSTR("V3"), y, sub == 3 ) ;
	 
		y += TFH ;
	}
	 
	drawItem( (char *)XPSTR(""), y, sub == 3+extra ) ;
	y += TFH ;
	
	drawItem( (char *)XPSTR("Delay"), y, sub == 4+extra ) ;
	y += TFH ;
	
	drawItem( (char *)XPSTR("Ex.Func"), y, sub == 5+extra ) ;
	y += TFH ;
	
	drawItem( (char *)XPSTR("Copy"), y, sub == 6+extra ) ;

	if ( Clipboard.content == CLIP_SWITCH )
	{
		y += TFH ;
		drawItem( (char *)XPSTR("Paste"), y, sub == 7+extra ) ;
	}
	 
  if( (cstate == CS_VOFS) || (cstate == CS_2VAL) )
  {
		saveEditColours( (sub == 1), colour ) ;
		putsChnRaw( (TRIGHT-TRMARGIN)*TSCALE, (TTOP+TFH+TVOFF)*TSCALE, cs.v1u, LUA_RIGHT ) ;
		restoreEditColours() ;

#ifdef X20
    if ( (cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) )
#else
    if ( ( (cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1u < EXTRA_POTS_START ) ) || ( cs.v1u >= EXTRA_POTS_START + 8) )
#endif
		{
			int16_t value = convertTelemConstant( cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, cs.v2 ) ;
			saveEditColours( (sub == 2), colour ) ;
			putsTelemetryChannel( TRIGHT-TRMARGIN, TTOP+2*TFH+TVOFF, cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, value, LUA_RIGHT, TELEM_NOTIME_UNIT | TELEM_UNIT| TELEM_CONSTANT ) ;
			restoreEditColours() ;
		}
    else
		{
			drawNumber( TRIGHT-TRMARGIN, TTOP+2*TFH, cs.v2, 0) ; //, sub == 2 ? ~colour : colour ) ;
		}
		if ( extra )
		{
#ifdef X20
    	if ( ( (cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) ) )
#else
    	if ( ( (cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1u < EXTRA_POTS_START ) ) || ( cs.v1u >= EXTRA_POTS_START + 8) )
#endif
			{
				int16_t value = convertTelemConstant( cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, (int8_t)cs.bitAndV3 ) ;
				saveEditColours( (sub == 3), colour ) ;
				putsTelemetryChannel( TRIGHT-TRMARGIN, TTOP+3*TFH+TVOFF, cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, value, LUA_RIGHT, TELEM_NOTIME_UNIT | TELEM_UNIT| TELEM_CONSTANT ) ;
				restoreEditColours() ;
			}
    	else
			{
				drawNumber( TRIGHT-TRMARGIN, TTOP+3*TFH, (int8_t)cs.bitAndV3, 0) ; //, sub == 3 ? ~colour : colour ) ;
			}
		}
  }
  else if(cstate == CS_VBOOL)
  {
		saveEditColours( sub==1, colour ) ;
   	putsDrSwitches(TRIGHT-TRMARGIN, TTOP+TFH+TVOFF, cs.v1, LUA_RIGHT ) ;
		restoreEditColours() ;
		saveEditColours( sub==2, colour ) ;
   	putsDrSwitches(TRIGHT-TRMARGIN, TTOP+2*TFH+TVOFF, cs.v2, LUA_RIGHT ) ;
		restoreEditColours() ;
  }
  else if(cstate == CS_VCOMP)
  {
		saveEditColours( sub==1, colour ) ;
		putsChnRaw( (TRIGHT-TRMARGIN)*TSCALE, (TTOP+TFH+TVOFF)*TSCALE, cs.v1u, LUA_RIGHT ) ;
		restoreEditColours() ;
		saveEditColours( sub==2, colour ) ;
		putsChnRaw( (TRIGHT-TRMARGIN)*TSCALE, (TTOP+2*TFH+TVOFF)*TSCALE, cs.v2u, LUA_RIGHT ) ;
		restoreEditColours() ;
  }
	else if(cstate == CS_TIMER)
	{
		int8_t x ;
		uint8_t att = 0 ;
		x = cs.v1 ;
		if ( x < 0 )
		{
			x = -x-1 ;
			att = PREC1 ;
		}
#ifdef X20
	  lcdDrawText( THOFF,TTOP+TFH+TVOFF, XPSTR("Off") ) ;
#else
	  PUTS_P( THOFF,TTOP+TFH+TVOFF, XPSTR("Off") ) ;
#endif
		drawNumber( TRIGHT-TRMARGIN, TTOP+TFH, x+1, att) ; //, (sub == 1) ? ~colour : colour ) ;
		att = 0 ;
		x = cs.v2 ;
		if ( x < 0 )
		{
			x = -x-1 ;
			att = PREC1 ;
		}
#ifdef X20
	  lcdDrawText( THOFF,TTOP+2*TFH+TVOFF, XPSTR("On ") ) ;
#else
	  PUTS_P( THOFF,TTOP+2*TFH+TVOFF, XPSTR("On ") ) ;
#endif
		drawNumber( TRIGHT-TRMARGIN, TTOP+2*TFH, x+1, att) ; //, (sub == 2) ? ~colour : colour ) ;
		cs.exfunc = 0 ;
	}
	else if(cstate == CS_TMONO)
	{
		saveEditColours( sub==1, colour ) ;
   	putsDrSwitches(TRIGHT-TRMARGIN, TTOP+TFH+TVOFF, cs.v1, LUA_RIGHT ) ;
		restoreEditColours() ;
		uint8_t att = 0 ;
		int8_t x ;
		x = cs.v2 ;
		if ( x < 0 )
		{
			x = -x-1 ;
			att = PREC1 ;
		}
#ifdef X20
		lcdDrawText( THOFF, TTOP+2*TFH+TVOFF, XPSTR("Time") ) ;
#else
		PUTS_P( THOFF, TTOP+2*TFH+TVOFF, XPSTR("Time") ) ;
#endif
		drawNumber( TRIGHT-TRMARGIN, TTOP+2*TFH, x+1, att) ; //, (sub == 2) ? ~colour : colour ) ;
		cs.exfunc = 0 ;
	}
	else// cstate == U16
	{
		uint16_t x ;
		x = (uint8_t) cs.v2 ;
		x |= cs.bitAndV3 << 8 ;
		saveEditColours( sub==1, colour ) ;
		putsChnRaw( (TRIGHT-TRMARGIN)*TSCALE, (TTOP+TFH+TVOFF)*TSCALE, cs.v1u, LUA_RIGHT ) ;
		restoreEditColours() ;
		drawNumber( TRIGHT-TRMARGIN, TTOP+2*TFH, x, 0) ; //, (sub == 2) ? ~colour : colour ) ;
	}

	saveEditColours( sub==3+extra, colour ) ;
#ifdef X20
 	putsDrSwitches(TRIGHT-TRMARGIN-4*FW, TTOP+3*TFH+TVOFF+Vextra, cs.andsw, 0 ) ;
#else
 	putsDrSwitches(TRIGHT-TRMARGIN, TTOP+3*TFH+TVOFF+Vextra, getAndSwitch( cs ), LUA_RIGHT ) ;
#endif
	restoreEditColours() ;

#ifdef X20
	lcdDrawTextAtIndex( THOFF, TTOP+3*TFH+TVOFF+Vextra, XPSTR("\003ANDOR XOR"), cs.exfunc, 0 ) ;
	lcd_putsAttIdxColour( TRIGHT-TRMARGIN-3*FW, TTOP+5*TFH+TVOFF+Vextra, XPSTR("\003ANDOR XOR"), cs.exfunc, 0, ( sub == 5+extra ) ? ~LcdForeground : LcdForeground, ( sub == 5+extra ) ? ~colour : colour ) ;

	drawNumber( TRIGHT-TRMARGIN, TTOP+4*TFH+Vextra, cs.switchDelay, PREC1 ) ;
#else
	PUTS_AT_IDX( THOFF, TTOP+3*TFH+TVOFF+Vextra, XPSTR("\003ANDOR XOR"), cs.exfunc, 0 ) ;
	PUTS_AT_IDX_COLOUR( TRIGHT-TRMARGIN, TTOP+5*TFH+TVOFF+Vextra, XPSTR("\003ANDOR XOR"), cs.exfunc, LUA_RIGHT, ( sub == 5+extra ) ? ~LcdForeground : LcdForeground ) ; //, ( sub == 5+extra ) ? ~colour : colour ) ;

	drawNumber( TRIGHT-TRMARGIN, TTOP+4*TFH+Vextra, g_model.switchDelay[index], PREC1) ; //, (sub == 4+extra) ? ~colour : colour ) ;
#endif

	if ( sub <= 5+extra )
	{
		if ( extra )
		{
			if ( sub == 3 )
			{
				g_posHorz = 1 ;
				sub = 2 ;
			}
			else
			{
				if ( sub > 3 )
				{
					sub -= 1 ;
				}
			}
		}
		editOneSwitchItem( event, sub, index ) ;
		if ( cstate == CS_2VAL )
		{
			if ( g_posHorz )
			{
				g_posHorz = 0 ;
				sub = 3 ;
			}
			else
			{
				if ( sub >= 3 )
				{
					sub += 1 ;
				}
			}
		}
	}
	TextResult = 0 ;

	if ( sub == 6+extra )
	{
#ifdef TOUCH	
			if ( checkForMenuEncoderLong( event ) || handleSelectIcon() )
#else
			if ( checkForMenuEncoderLong( event ) )
#endif
		{
			Clipboard.clipswitch = cs ;
			Clipboard.content = CLIP_SWITCH ;
		}
	}

	if ( sub == 7+extra )
	{
#ifdef TOUCH	
		if ( checkForMenuEncoderLong( event ) || handleSelectIcon() )
#else
		if ( checkForMenuEncoderLong( event ) )
#endif
		{
			cs = Clipboard.clipswitch ;
		}
	}
}

void menuSwitches(uint8_t event)
{
	uint32_t rows = NUM_SKYCSW ;
	touchMenuTitle( (char *)PSTR(STR_CUST_SWITCH) ) ;
	EditType = EE_MODEL ;
	static MState2 mstate2;
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	event = mstate2.check_columns(event, rows-1 ) ;

#ifdef TOUCH
	uint32_t selected = 0 ;
	uint32_t newVpos ;
#endif

	coord_t y ;
	uint8_t k = 0 ;
	uint32_t sub = mstate2.m_posVert ;
	uint32_t t_pgOfs ;

	t_pgOfs = evalHresOffset( sub ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	selected = newVert & 0x0100 ;

	if ( handleSelectIcon() )
	{
		selected = 1 ;
	}

	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub >= t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif
		
	for(uint32_t i = 0 ; i < TLINES ; i += 1 )
	{
    y=(i)*TFH +TTOP+TVOFF ;
    k = i + t_pgOfs ;
    uint8_t attr = sub == k ? INVERS : 0 ;
		uint8_t m = k ;
#ifdef X20
    X20CSwData &cs = g_model.customSw[m];
#else
    SKYCSwData &cs = g_model.customSw[m] ;
#endif

		//write SW names here
		displayLogicalSwitch( 0, y, m ) ;

		uint16_t oldBcolour = LcdBackground ;
		uint16_t oldFcolour = LcdForeground ;
    
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 2*FW*TSCALE, (y-TVOFF)*TSCALE+2, (TRIGHT-2*FW)*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdBackground = ~LcdBackground ;
			LcdForeground = ~LcdForeground ;
		}

#ifdef X20
		lcdDrawTextAtIndex( 2*FW+4, y, PSTR(CSWITCH_STR),cs.func, 0);
#else
		PUTS_AT_IDX( 2*FW+4, y, PSTR(CSWITCH_STR), cs.func, 0 ) ;
#endif

 	  uint8_t cstate = CS_STATE(cs.func);

	  if( (cstate == CS_VOFS) || (cstate == CS_2VAL) )
    {
			putsChnRaw( (10*FW-6)*TSCALE, y*TSCALE, cs.v1u  , 0);
#ifdef X20
	    if ( cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT )
#else
	  	if ( ( (cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1u < EXTRA_POTS_START ) ) || ( cs.v1u >= EXTRA_POTS_START + 8) )
#endif
 			{
				int16_t value = convertTelemConstant( cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, cs.v2 ) ;
				putsTelemetryChannel( 18*FW-8, y, cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, value, 0, TELEM_NOTIME_UNIT | TELEM_UNIT| TELEM_CONSTANT);
			}
    	else
			{
#ifdef X20
        lcdDrawNumber( 18*FW-9, y, cs.v2  ,0);
#else
    	  drawNumber( 18*FW-9, y, cs.v2  ,0);
#endif
			}
    }
    else if(cstate == CS_VBOOL)
    {
    	putsDrSwitches(10*FW-6, y, cs.v1  , 0);
    	putsDrSwitches(14*FW-7, y, cs.v2  , 0);
    }
    else if(cstate == CS_VCOMP)
    {
    	putsChnRaw( (10*FW-6)*TSCALE, y*TSCALE, cs.v1u  , 0);
    	putsChnRaw( (14*FW-4)*TSCALE, y*TSCALE, cs.v2u  , 0);
    }
		else if(cstate == CS_TIMER)
		{
			int8_t x ;
			uint8_t att = 0 ;
			x = cs.v1 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att = PREC1 ;
			}
#ifdef X20
	    lcdDrawText( 13*FW, y, PSTR("On") ) ;
      lcdDrawNumber( 13*FW-5, y, x+1  ,att ) ;
#else
	  	PUTS_ATT_LEFT( y+TVOFF/2, PSTR(STR_15_ON) ) ;
    	PUTS_NUM( 13*FW-5, y, x+1  ,att ) ;
#endif
			att = 0 ;
			x = cs.v2 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att = PREC1 ;
			}
#ifdef X20
      lcdDrawNumber( 18*FW-3, y, x+1 , att ) ;
#else
    	PUTS_NUM( 18*FW-3, y, x+1 , att ) ;
#endif
		}
		else if(cstate == CS_TMONO)
		{
    	putsDrSwitches(10*FW-6, y, cs.v1  , 0);
			uint8_t att = 0 ;
			int8_t x ;
			x = cs.v2 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att = PREC1 ;
			}
#ifdef X20
      lcdDrawNumber( 17*FW-2, y, x+1 , att ) ;
#else
    	PUTS_NUM( 17*FW-2, y, x+1 , att ) ;
#endif
		}
		else// cstate == U16
		{
			uint16_t x ;
			x = cs.v2u ;
			x |= cs.bitAndV3 << 8 ;
    	putsChnRaw( (10*FW-6-FW)*TSCALE, y*TSCALE, cs.v1  , 0);
#ifdef X20
      lcdDrawNumber( 18*FW-9, y, x  , 0,5);
#else
    	PUTS_NUM_N( 18*FW-9, y, x  , 0,5);
#endif
		}
#ifdef X20
		putsDrSwitches( 18*FW-3, y, cs.andsw, 0 ) ;
		lcdDrawText( 22*FW, y, XPSTR("Delay" ) ) ;
		lcdDrawNumber( TRIGHT-TRMARGIN, y, cs.switchDelay, PREC1 ) ;
#else
		putsDrSwitches( 18*FW-3, y, getAndSwitch( cs ),0 ) ;
		PUTS_P( 22*FW, y, XPSTR("Delay" ) ) ;
		PUTS_NUM( TRIGHT-TRMARGIN, y, g_model.switchDelay[m], PREC1 ) ;
#endif

		if ( attr )
		{
#ifdef TOUCH	
			if ( ( checkForMenuEncoderBreak( event ) ) || selected )
#else
			if ( checkForMenuEncoderBreak( event ) )
#endif
			{
				// Long MENU pressed
			  s_currIdx = sub ;
//  				TextType = 0 ;
				pushMenu(menuSwitchOne) ;
  		  s_editMode = false ;
			}
		}
		LcdBackground = oldBcolour ;
		LcdForeground = oldFcolour ;
		lcd_hline( 0, y+TFH-TVOFF, TRIGHT ) ;
	}
}

void menuSensors(uint8_t event)
{
	touchMenuTitle( (char *)XPSTR("Sensors") ) ;
	static MState2 mstate2 ;
	uint32_t rows = NUMBER_EXTRA_IDS + 6 + 4 + 1 ;
	
#ifndef TOUCH
	Tevent = event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	Tevent = event = mstate2.check_columns(event, rows-1 ) ;
	
#ifdef TOUCH
//	uint32_t selected = 0 ;
	uint32_t newVpos ;
#endif

	coord_t y ;
	uint32_t k = 0 ;
	uint32_t sub = mstate2.m_posVert ;
	uint32_t t_pgOfs ;

	t_pgOfs = evalHresOffset( sub ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
//	selected = newVert & 0x0100 ;

//	if ( handleSelectIcon() )
//	{
//		selected = 1 ;
//	}

	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub >= t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif
	 
//	uint32_t subN = 0 ;

//	uint16_t oldBcolour = LcdBackground ;
	uint16_t colour = dimBackColour() ;

	for(uint32_t i = 0 ; i < TLINES ; i += 1 )
	{
    y=(i)*TFH +TTOP ;
    k = i + t_pgOfs ;
    LcdFlags attr = sub == k ? INVERS : 0 ;

		if ( k < 10 )
		{
			uint32_t idx = 69 ;
			if ( k > 5 )
			{
				idx = 77 ;
			}
			setLastIdx( (char *) PSTR(STR_TELEM_ITEMS), idx + k ) ;
			drawItem( (char *)LastItem, y, attr ) ;
			saveEditColours( attr, colour ) ;
#ifdef X20
			alphaEditName( TRIGHT-TRMARGIN, y+TVOFF/2, (uint8_t *)&g_model.customTelemetryNames[k*4], 4, attr|ALPHA_NO_NAME|LUA_RIGHT, (uint8_t *)XPSTR( "Custom Name") ) ;
#else
			if ( k < 6)
			{
				alphaEditName( TRIGHT-TRMARGIN, y+TVOFF/2, (uint8_t *)&g_model.customTelemetryNames[k*4], 4, attr|ALPHA_NO_NAME|LUA_RIGHT, (uint8_t *)XPSTR( "Custom Name") ) ;
			}
			else
			{
				alphaEditName( TRIGHT-TRMARGIN, y+TVOFF/2, (uint8_t *)&g_model.customTelemetryNames2[(k-6)*4], 4, attr|ALPHA_NO_NAME|LUA_RIGHT, (uint8_t *)XPSTR( "Custom Name") ) ;
			}
#endif
			restoreEditColours() ;
 			if ( AlphaEdited )
			{
				sortTelemText() ;				
			}
		}
		else
		{
			uint32_t j ;
			j = k - 10 ;
			if ( j < g_model.extraSensors )
			{
				drawItem( (char *)"", y, attr ) ;
#ifdef X20
 			  lcd_outhex4( THOFF, y+TVOFF, g_model.extraId[j].id ) ;
#else
				PUT_HEX4( THOFF*2, y+TVOFF, g_model.extraId[j].id ) ;
#endif
				drawIdxText( y, (char *) XPSTR(DestString), g_model.extraId[j].dest, attr ) ;
 			  if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( g_model.extraId[j].dest, NUM_SCALE_DESTS ) ;
 			  }
			}	
			else
			{
				if ( j < NUMBER_EXTRA_IDS )
				{
					drawItem( (char *)HyphenString, y, attr ) ;
				}
				else
				{
					drawItem( (char *)XPSTR("Clear All"), y, attr ) ;
#ifdef X20
					lcd_putsAtt( TRIGHT-TRMARGIN-9*FW, y+TVOFF, XPSTR("MENU LONG"), 0 ) ;
#else
					PUTS_ATT( TRIGHT-TRMARGIN-9*FW, y+TVOFF, XPSTR("MENU LONG"), 0 ) ;
#endif
 			  	if(attr)
					{				
						if ( checkForMenuEncoderLong( event ) )
						{
							uint32_t i ;
							for ( i = 0 ; i < NUMBER_EXTRA_IDS ; i += 1 )
							{
								g_model.extraId[i].dest = 0 ;
							}
							g_model.extraSensors = 0 ;
						}
					}
				}
			}
		}
	}
}

void menuTrainerProtocol(uint8_t event)
{
	touchMenuTitle( (char *)PSTR(STR_Trainer) ) ;
	
	static MState2 mstate2;
	uint32_t rows = 6 ;
	mstate2.check_columns(event, rows-1) ;

	uint32_t sub = mstate2.m_posVert ;
	coord_t y ;
	uint32_t subN = 0 ;
	LcdFlags attr ;
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif
	 
	y = TTOP ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Trainer Polarity"), y, attr ) ;
	drawIdxText( y, (char *)PSTR(STR_POS_NEG), g_model.trainPulsePol, attr|LUA_RIGHT ) ; // 0, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
	if(attr) CHECK_INCDEC_H_GENVAR_0( g_model.trainPulsePol, 1 ) ;
	y += TFH ;
 	subN += 1 ;
		
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Channels"), y, attr ) ;
	if (attr)
	{
#ifdef X20
 		CHECK_INCDEC_H_MODELVAR( g_model.trainerPpmNCH, 0, 12 ) ;
	}
	drawNumber( TRIGHT-TRMARGIN, y, g_model.trainerPpmNCH + 4, attr ) ;
#else
 		CHECK_INCDEC_H_MODELVAR_0( g_model.ppmNCH, 12 ) ;
	}
	drawNumber( TRIGHT-TRMARGIN, y, g_model.ppmNCH + 4, attr) ; //, attr ? ~colour : colour ) ;
#endif
	y += TFH ;
 	subN += 1 ;
		
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_PPMFRAME_MSEC), y, attr ) ;
	if (attr)
	{
#ifdef X20
 		CHECK_INCDEC_H_MODELVAR( g_model.trainerPpmFrameLength, -12, 12 ) ;
	}
	drawNumber( TRIGHT-TRMARGIN, y, g_model.trainerPpmFrameLength*5 + 225, attr|PREC1 ) ;
#else
 		CHECK_INCDEC_H_MODELVAR( g_model.ppmFrameLength, -12, 12 ) ;
	}
	drawNumber( TRIGHT-TRMARGIN, y, g_model.ppmFrameLength*5 + 225, attr|PREC1) ; //, attr ? ~colour : colour ) ;
#endif
	y += TFH ;
 	subN += 1 ;
		
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Delay (uS)"), y, attr ) ;
	if (attr)
	{
#ifdef X20
		CHECK_INCDEC_H_MODELVAR( g_model.trainerPpmDelay,-4,10);
	}
	drawNumber( TRIGHT-TRMARGIN, y, (g_model.trainerPpmDelay*50)+300, attr ) ;
#else
		CHECK_INCDEC_H_MODELVAR( g_model.ppmDelay,-4,10);
	}
	drawNumber( TRIGHT-TRMARGIN, y, (g_model.ppmDelay*50)+300, attr) ; //, attr ? ~colour : colour ) ;
#endif
	y += TFH ;
 	subN += 1 ;
		
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_PPM_1ST_CHAN), y, attr ) ;
#ifdef X20
	if(attr) { CHECK_INCDEC_H_MODELVAR( g_model.trainerStartChannel, 0, 16 ) ; }
	drawNumber( TRIGHT-TRMARGIN, y, g_model.trainerStartChannel + 1, attr ) ;
#else
	if(attr) { CHECK_INCDEC_H_MODELVAR_0( g_model.startChannel, 16 ) ; }
	drawNumber( TRIGHT-TRMARGIN, y, g_model.startChannel + 1, attr) ; //, attr ? ~colour : colour ) ;
#endif
	y += TFH ;
 	subN += 1 ;
		
	attr = (sub==subN) ? INVERS : 0 ;
	uint8_t value = g_model.trainerProfile + 1 ;
	if ( g_model.traineron == 0 )
	{
		value = 0 ;
	}
	drawItem( (char *)PSTR(STR_Trainer), y, attr ) ;
  if(value)
	{
		drawNumber( TRIGHT-TRMARGIN, y, value-1, attr) ; //, attr ? ~colour : colour ) ;
#ifdef X20
		lcdDrawChar( THOFF+9*FW, y+TVOFF, '(' ) ;
    lcdDrawChar( THOFF+16*FW, y+TVOFF, ')' ) ;
		validateText( g_eeGeneral.trainerProfile[value-1].profileName, 6 ) ;
		lcdDrawText( THOFF+10*FW, y+TVOFF, (const char *)g_eeGeneral.trainerProfile[value-1].profileName, 6, 0 ) ;
#else
		PUTC( THOFF+9*FW, y+TVOFF, '(' ) ;
    PUTC( THOFF+16*FW, y+TVOFF, ')' ) ;
		validateText( g_eeGeneral.trainerProfile[value-1].profileName, 6 ) ;
		PUTS_ATT_N( THOFF+10*FW, y+TVOFF, (const char *)g_eeGeneral.trainerProfile[value-1].profileName, 6, 0 ) ;
#endif
	}
  else
	{
#ifdef X20
  	lcd_putsAttColour( TRIGHT-TRMARGIN-3*FW, y+TVOFF, PSTR(STR_OFF), 0, attr ? ~LcdForeground : LcdForeground ) ;
#else
  	PUTS_ATT_COLOUR( TRIGHT-TRMARGIN-3*FW, y+TVOFF, PSTR(STR_OFF), 0, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~LcdBackground : LcdBackground ) ;
#endif
	}
  if(sub==subN)
	{
		CHECK_INCDEC_H_MODELVAR_0( value, 4 ) ;
		MaskRotaryLong = 1 ;
#ifndef X20 // Add this in later
		if ( ( value > 0) && ( checkForMenuEncoderLong( event ) ) )
		{
			SingleExpoChan = 1 ;
			s_expoChan = value-1 ;
			s_editMode = 0 ;
      killEvents(event);
      pushMenu(menuProcTrainer) ; 
		}
#endif
	}

	g_model.traineron = value > 0 ;
	if ( value )
	{
		g_model.trainerProfile = value - 1 ;
	}
}

void menuOneGvar(uint8_t event)
{
	touchMenuTitle( (char *)PSTR(STR_GLOBAL_VARS) ) ;
	EditType = EE_MODEL ;
	static MState2 mstate2;
	uint32_t rows = 3 ;	
	GvarData *pgvar ;
//#if MULTI_GVARS
//	if ( g_model.flightModeGvars )
//	{
//		rows = 6 ;
//	}
//#endif
	event = mstate2.check_columns(event, rows - 1 ) ;

	uint8_t sub = mstate2.m_posVert ;

#ifdef X20
	lcdDrawChar( 20*FW, 0, s_currIdx+ ((s_currIdx > 8) ? '8' : '1') ) ;
#else
	PUTC( 20*FW, 0, s_currIdx+ ((s_currIdx > 8) ? '8' : '1') ) ;
#endif
	uint16_t colour = dimBackColour() ;
//#if MULTI_GVARS
//	uint16_t oldBcolour = LcdBackground ;
//#endif

#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif

	pgvar = &g_model.gvars[s_currIdx] ;
//	GVarXData *pgxvar = &g_model.xgvars[s_currIdx] ;

	for ( uint32_t i = 0 ; i < rows ; i += 1 )
	{
    coord_t y = i * TFH + TTOP ;
    LcdFlags attr = (sub==i) ? INVERS : 0 ;
		
		switch(i)
		{
			case 0 : // switch
//#if MULTI_GVARS
//				if ( g_model.flightModeGvars )
//				{
//					uint8_t *psource ;
//					drawItem( (char *)XPSTR("Source"), y, attr ) ;
//					psource = ( ( (uint8_t*)&g_model.gvars) + s_currIdx ) ;
//					drawIdxText( y, (char *)PSTR(STR_GV_SOURCE), *psource, attr ) ;
//					// STR_GV_SOURCE
// 					if(attr)
//					{ 
//						CHECK_INCDEC_H_MODELVAR( *psource, 0, 69+NUM_RADIO_VARS ) ;
//					}
//				}
//				else
//#endif
				{
					drawItem( (char *)PSTR(STR_SWITCH), y, attr ) ;
					LcdFlags doedit = attr ;
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
#ifdef X20
					pgvar->gvswitch = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, pgvar->gvswitch, LUA_RIGHT, doedit ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
#else
					g_model.gvswitch[s_currIdx] = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, g_model.gvswitch[s_currIdx], LUA_RIGHT, doedit ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
#endif
					restoreEditColours() ;
				}
			break ;
			case 1 :
//#if MULTI_GVARS
//				if ( g_model.flightModeGvars )
//				{
//					drawItem( (char *)XPSTR("Unit"), y, attr ) ;
//					drawChar( TRIGHT-TRMARGIN-FW, y, pgxvar->unit ? '%' : '-', attr, attr ? ~colour : colour ) ;
//					if ( attr )
//					{
//						CHECK_INCDEC_H_MODELVAR_0( pgxvar->unit, 1 ) ;
//					}
//				}
//				else
//#endif
			{
				drawItem( (char *)XPSTR("Source"), y, attr ) ;
#ifdef X20
				uint32_t t = pgvar->gvsource ;
				if ( t > 4 )
				{
					if ( t == 90 )
					{
						t = 5 ;
					}
					else
					{
						t += 1 ;
					}
				}
				drawIdxText( y, (char *)PSTR(STR_GV_SOURCE), t, attr ) ;
				// STR_GV_SOURCE
 				if(attr)
				{
					CHECK_INCDEC_H_MODELVAR( t, 0, 86+NUM_RADIO_VARS ) ;
					if ( t > 4 )
					{
						if ( t == 5 )
						{
							t = 90 ;
						}
						else
						{
							t -= 1 ;
						}
					}
					pgvar->gvsource = t ;
				}
#else
				drawIdxText( y, (char *)PSTR(STR_GV_SOURCE), pgvar->gvsource, attr ) ;
				// STR_GV_SOURCE
 				if(attr)
				{ 
					CHECK_INCDEC_H_MODELVAR( pgvar->gvsource, 0, 69+NUM_RADIO_VARS ) ;
				}
#endif
			}
			break ;
			case 2 :
//#if MULTI_GVARS
//				if ( g_model.flightModeGvars )
//				{
//					drawItem( (char *)XPSTR("Precision"), y, attr ) ;
//					drawIdxText( y, (char *)XPSTR("\0030.-0.0"), pgxvar->prec, attr ) ;
//		    	if(attr)
//					{
//      		  CHECK_INCDEC_H_MODELVAR_0( pgxvar->prec, 1 ) ;
//					}	
//				}
//				else
//#endif
				{
					drawItem( (char *)XPSTR("Value"), y, attr ) ;
					drawNumber( TRIGHT-TRMARGIN, y, pgvar->gvar, attr ) ;
  				if(attr)
					{
						CHECK_INCDEC_H_MODELVAR( pgvar->gvar, -125, 125 ) ;
					}
				}
			break ;
//#if MULTI_GVARS
//			case 3 :
//				drawItem( (char *)XPSTR("Minimum"), y, attr ) ;
//				drawNumber( TRIGHT-TRMARGIN, y, GVAR_MIN+pgxvar->min, attr ) ;
//				if ( attr )
//				{
//					if ( GVAR_MIN+pgxvar->min > GVAR_MAX-pgxvar->max )
//					{
//						pgxvar->min = pgxvar->max = 0 ;
//					}
//					pgxvar->min = checkIncDec16( GVAR_MIN+pgxvar->min, GVAR_MIN, GVAR_MAX-pgxvar->max, EE_MODEL ) - GVAR_MIN ;
//				}
//			break ;
//			case 4 :
//				drawItem( (char *)XPSTR("Maximum"), y, attr ) ;
//				drawNumber( TRIGHT-TRMARGIN, y, GVAR_MAX-pgxvar->max, attr ) ;
//	    	if(attr)
//				{
//      	  pgxvar->max = GVAR_MAX-checkIncDec16( GVAR_MAX-pgxvar->max, GVAR_MIN+pgxvar->min, GVAR_MAX, EE_MODEL ) ;
//    		}
//			break ;
//			case 5 :
//				drawItem( (char *)XPSTR("Name"), y, attr ) ;
//				LcdBackground = attr ? ~colour : colour ;
//				alphaEditName( TRIGHT-3*FW, y+TVOFF, &g_model.gvarNames[s_currIdx*3], 3, attr|ALPHA_NO_NAME, (uint8_t *)0 ) ;
//				LcdBackground = oldBcolour ;
////				pgxvar->popup = touchOnOffItem( pgxvar->popup, y, XPSTR("Popup"), attr, DimBackColour ) ;
//			break ;
//#endif
//			case 6 :
//			{	
//				drawItem( (char *)XPSTR("Value(      )"), y, attr ) ;
//				displayFlightMode( 6*FW, y+TVOFF, CurrentPhase, 0 ) ;
//				int16_t v = getGvar( s_currIdx ) ;
//				drawNumber( TRIGHT-TRMARGIN, y, v, attr ) ;
//				if (attr)
//				{
//					v = checkIncDec16( v, -125, 125, EE_MODEL ) ;
//					setGVar( s_currIdx, v ) ;
//				}
//			}
		}
	}
}

void menuGlobals(uint8_t event)
{
	touchMenuTitle( (char *)PSTR(STR_GLOBAL_VARS) ) ;
	EditType = EE_MODEL ;
	static MState2 mstate2;
	uint32_t t_pgOfs ;
	uint32_t rows = MAX_GVARS ;
	uint32_t k = 0 ;
#ifdef TOUCH	
	uint32_t selected = 0 ;
	uint32_t newVpos ;
#endif
	 
//#if MULTI_GVARS
//	if ( g_model.flightModeGvars )
//	{
//		rows = 12 ;
//	}
//#endif	
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	event = mstate2.check_columns(event, rows - 1 ) ;

	uint8_t sub = mstate2.m_posVert ;
	coord_t y ;

	t_pgOfs = evalHresOffset( sub ) ;

#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	selected = newVert & 0x0100 ;
#endif

#ifdef TOUCH	
	if ( rows > 9 )
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

#ifdef TOUCH	
	if ( handleSelectIcon() || selected || ( event == EVT_KEY_BREAK(BTN_RE) ) )
#else
	if ( event == EVT_KEY_BREAK(BTN_RE) )
#endif
//	if ( handleSelectIcon() || ( event == EVT_KEY_BREAK(BTN_RE) ) )
	{
		s_currIdx = sub ;
		killEvents(event);
		s_editMode = false ;
		pushMenu(menuOneGvar) ;
  }

//	if ( s_editMode == 0 )
//	{
//		if ( subSub < 2 )
//		{
//			if ( handleSelectIcon() )
//			{
//				s_editMode = 1 ;
//			}
//		}
//	}

	uint32_t lines = rows > TLINES ? TLINES : rows ;
	for (uint32_t i=0; i<lines ; i += 1 )
	{
    y=(i)*TFH +TTOP ;
    k=i+t_pgOfs ;
  	uint8_t attr = ((sub==k) ? INVERS : 0);
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
		}
		saveEditColours( attr, LcdBackground ) ;
#ifdef X20
  	lcdDrawText( THOFF, y+TVOFF, PSTR(STR_GV));
		lcdDrawChar( THOFF+2*FW, y+TVOFF, k+ ((k > 8) ? '8' : '1') ) ;
#else
  	PUTS_P( THOFF, y+TVOFF, PSTR(STR_GV));
		PUTC( THOFF+2*FW, y+TVOFF, k+ ((k > 8) ? '8' : '1') ) ;
#endif
		GvarData *pgvar ;
//#if MULTI_GVARS
//		if ( g_model.flightModeGvars )
//		{
//			attr = 0 ;
//			pgvar = &g_model.gvars[0] ;
//		}
//		else
//		{
//			pgvar = &g_model.gvars[k] ;
//		}
//#else
		pgvar = &g_model.gvars[k] ;
//#endif
//#if MULTI_GVARS
//		if ( g_model.flightModeGvars == 0 )
//#endif
		{
#ifdef X20
	    putsDrSwitches( 7*FW, y+TVOFF, g_model.gvars[k].gvswitch, 0 );
#else
	    putsDrSwitches( 7*FW, y+TVOFF, g_model.gvswitch[k], 0 );
#endif
			uint32_t t ;
			uint8_t *psource ;
//			pgvar = &g_model.gvars[k] ;
			psource = &pgvar->gvsource ;
			t = *psource ;
#ifdef X20
			lcdDrawTextAtIndex( 14*FW-4, y+TVOFF, PSTR(STR_GV_SOURCE), t, 0 ) ;
			lcdDrawNumber( 22*FW, y+TVOFF, pgvar->gvar, 0) ;
#else
			PUTS_AT_IDX( 14*FW-4, y+TVOFF, PSTR(STR_GV_SOURCE), t, 0 ) ;
			PUTS_NUM( TRIGHT-TRMARGIN, y+TVOFF, pgvar->gvar, 0 ) ;
#endif
		}
//#if MULTI_GVARS
//		else
//		{
//			uint8_t *psource ;
//			uint32_t t ;
//			psource = ( ( (uint8_t*)&g_model.gvars) + k ) ;
//			t = *psource ;
//			PUTS_AT_IDX( 14*FW-4, y+TVOFF, PSTR(STR_GV_SOURCE), t, 0 ) ;
//			PUTS_NUM( TRIGHT-TRMARGIN, y+TVOFF, readMgvar( 0, k ), 0 ) ;
//		}
//#endif
		
		restoreEditColours() ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
	}
}

void menuProtocol(uint8_t event)
{
	uint32_t selected = 0 ;
	uint8_t dataItems = 3 ;
	uint8_t need_range = 0 ;
	uint32_t i ;

	switch ( g_model.Module[0].protocol )
	{
		case PROTO_PXX :
		case PROTO_MULTI :
		case PROTO_DSM2 :
			need_range = 1 ;
			dataItems += 1 ;
		break ;
	}

	switch ( g_model.Module[1].protocol )
	{
		case PROTO_PXX :
		case PROTO_MULTI :
		case PROTO_DSM2 :
			need_range |= 2 ;
			dataItems += 1 ;
		break ;
	}
	
	touchMenuTitle( (char *)XPSTR("Protocol") ) ;
	static MState2 mstate2 ;
	 
	event = mstate2.check_columns( event, dataItems-1 ) ;

	uint32_t sub = mstate2.m_posVert ;
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( dataItems, 0, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
	selected = newVert & 0x0100 ;

	if ( handleSelectIcon() || selected )
	{
		selected = 1 ;
		if ( event == 0 )
		{
			event = EVT_KEY_BREAK(BTN_RE) ;
		}
	}
#endif
	 
	coord_t y = TTOP ;
	uint32_t subN = 0 ;

	uint16_t oldBcolour = LcdBackground ;
	uint16_t oldFcolour = LcdForeground ;
	if ( sub == subN )
	{
		lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
		LcdForeground = ~LcdForeground ;
		LcdBackground = ~LcdBackground ;
	}
#ifdef X20
	lcd_putsAtt( THOFF, y+TVOFF, PSTR(STR_Trainer), 0 ) ;
#else
	PUTS_ATT( THOFF, y+TVOFF, PSTR(STR_Trainer), 0 ) ;
#endif
	LcdForeground = oldFcolour ;
	LcdBackground = oldBcolour ;
	
	lcd_hline( 0, y+TFH, TRIGHT ) ;
  y += TFH ;
	subN += 1 ;

	for ( i = 0 ; i < 2 ; i += 1 )
	{
		if ( sub == subN )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdForeground = ~LcdForeground ;
			LcdBackground = ~LcdBackground ;
		}
		displayModuleName( THOFF, y+TVOFF, i, 0 ) ;
		displayProtocol( THOFF+10*FW, y+TVOFF, g_model.Module[i].protocol, 0 ) ;
		LcdForeground = oldFcolour ;
		LcdBackground = oldBcolour ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
  	y += TFH ;
		subN += 1 ;
	}
		
	if ( sub <= 2 )
	{
  	switch (event)
		{
#ifndef X20
  		case EVT_KEY_FIRST(KEY_MENU) :
#endif
  		case EVT_KEY_BREAK(BTN_RE) :
				killEvents(event) ;
				switch ( sub )
				{
					case 0 :
						pushMenu(menuTrainerProtocol) ;
					break ;
					case 1 :
						EditingModule = 0 ;
						pushMenu( editOneProtocol ) ;
					break ;
					case 2 :
						EditingModule = 1 ;
						pushMenu( editOneProtocol ) ;
					break ;
				}

			break ;
		}
	}

	for ( i = 1 ; i < 4 ; i <<= 1 )
	{
		if ( need_range & i )
		{
  		if(sub==subN)
			{
				lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
				LcdForeground = ~LcdForeground ;
				LcdBackground = ~LcdBackground ;
			}	
#ifdef X20
			lcdDrawText( THOFF, y+TVOFF, PSTR(STR_RANGE) ) ;
#else
			PUTS_P( THOFF, y+TVOFF, PSTR(STR_RANGE) ) ;
#endif
			displayModuleName( 12*FW+THOFF, y+TVOFF, i > 1, 0 ) ;
			LcdForeground = oldFcolour ;
			LcdBackground = oldBcolour ;
  		if(sub==subN)
			{
				if ( ( checkForMenuEncoderLong( event ) ) || selected )
				{
					uint8_t module = 0 ;
					if ( i & 2 )
					{
						module = 1 ;
					}
  		  	BindRangeFlag[module] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
					s_currIdx = module ;
					pushMenu(menuRangeBind) ;
				}
			}
			lcd_hline( 0, y+TFH, TRIGHT ) ;
			y += TFH ;
			subN += 1 ;
		}
	}

//			if ( need_range & 1 )
//			{
////				if(t_pgOfs<=subN)
////				{
//				PUTS_ATT_LEFT( y, PSTR(STR_RANGE) ) ;
//				PUTS_ATT_LEFT( y, "\014Internal") ;
//  		  if(sub==subN)
//				{
//					lcd_char_inverse( 0, y, 11*FW, 0 ) ;
//					if ( checkForMenuEncoderLong( event ) )
//					{
//  		  	  BindRangeFlag[0] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
//						s_currIdx = 0 ;
//						pushMenu(menuRangeBind) ;
//					}
//				}
//				y+=FH ;
//				subN += 1 ;
//			}
//			if ( need_range & 2 )
//			{
////				if(t_pgOfs<=subN)
////				{
//				PUTS_ATT_LEFT( y, PSTR(STR_RANGE) ) ;
//				PUTS_ATT_LEFT( y, "\014External") ;
//  		  if(sub==subN)
//				{
//					lcd_char_inverse( 0, y, 11*FW, 0 ) ;
//					if ( checkForMenuEncoderLong( event ) )
//					{
//  		  	  BindRangeFlag[1] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
//						s_currIdx = 1 ;
//						pushMenu(menuRangeBind) ;
//					}
//				}
////				y+=FH ;
////				subN += 1 ;
//			}

//		}
//	} // module for loop
}

#ifdef X20
const uint8_t LogLookup[] =
{
 1, 2, 3, 4, 7, 8, 9,10,11,12,
13,16,17,18,19,20,21,22,23,24,
25,26,27,28,29,30,31,32,33,34,
35,36,37,38,39,40,41,42,43,44,
45,48,49,50,51,52,53,54,55,56,
57,58,59,60,61,62,63,64,65,66,
67,68,69,70,71,72,73,74,78,79,
80,81,82,83,84,85,86,89,90,91,92
} ;

uint8_t AlphaLogLookup[sizeof(LogLookup)] ;

#else
const uint8_t LogLookup[] =
{
 1, 2, 3, 4, 7, 8, 9,10,11,12,
13,16,17,18,19,20,21,22,23,24,
25,26,27,28,29,30,31,32,33,34,
35,36,37,38,39,40,41,42,43,44,
45,48,49,50,51,52,53,54,55,56,
57,58,59,60,61,62,63,64,65,66,
67,68,69,70,71,72,73,74,78,79,
80,81,82,83,84,85,86,89//,90
} ;
#define LOG_LOOKUP_SIZE	78
#endif

const uint8_t LogRateMap[] = { 3, 2, 0, 1 } ;
extern const uint8_t LogLookup[] ;

extern const uint8_t TelemSize  ;
extern uint8_t TelemMap[] ;
#ifndef X20
extern union t_sharedMemory SharedMemory ;
#endif

void menuLogging(uint8_t event)
{
	touchMenuTitle( (char *)XPSTR("Logging") ) ;
	static MState2 mstate2;
	uint32_t rows = 1+sizeof(LogLookup)+5+4+3+1 ;
#ifndef TOUCH
	Tevent = event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	Tevent = event = mstate2.check_columns(event, rows-1 ) ;

#ifdef TOUCH
//	uint32_t selected = 0 ;
	uint32_t newVpos ;
#endif

	coord_t y ;
	uint32_t sub = mstate2.m_posVert ;
//	uint32_t blink = InverseBlink ;
	uint32_t t_pgOfs ;
  uint32_t k ;

	t_pgOfs = evalHresOffset( sub ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
//	selected = newVert & 0x0100 ;

//	if ( handleSelectIcon() )
//	{
//		selected = 1 ;
//	}

	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
//	newVpos = scrollBar( 620, 32, 70, 480-32, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub >= t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif

	if ( event == EVT_ENTRY )
	{
		uint32_t i ;
		uint32_t j ;
		uint32_t k ;
		uint32_t m ;
		k = 0 ;
		for ( i = 0 ; i < TelemSize ; i += 1 )
		{
			m = 0 ;
			for ( j = 0 ; j < sizeof(LogLookup) ; j += 1 )
			{
				if ( TelemMap[i] == LogLookup[j]-1 )
				{
#ifdef X20
					AlphaLogLookup[k] = TelemMap[i] ;
#else
					SharedMemory.AlphaLogLookup[k] = TelemMap[i] ;
#endif
					m = 1 ;
					break ;
				}
			}
			if ( m )
			{
				k += 1 ;
			}
		}
	}

	uint16_t colour = dimBackColour() ;

  for( uint32_t j = 0 ; j < TLINES ; j += 1 )
	{
    y = j*TFH + TTOP ;
    k = j + t_pgOfs ;
    LcdFlags attr = (sub==k) ? INVERS : 0 ;

		if ( k == 0 )
		{
			drawItem( (char *)PSTR(STR_LOG_SWITCH), y, attr ) ;
			LcdFlags doedit = attr ;
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
			g_model.logSwitch = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, g_model.logSwitch, LUA_RIGHT, doedit ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
//			g_model.logSwitch = edit_dr_switch( TRIGHT-TRMARGIN, y, g_model.logSwitch, LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
			restoreEditColours() ;
		}
		else if ( k == 1 )
		{
			drawItem( (char *)PSTR(STR_LOG_RATE), y, attr ) ;
			drawIdxText( y, (char *)XPSTR("\0041.0s2.0s0.5s0.2s"), g_model.logRate, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
 			if(attr)
			{
				g_model.logRate = checkOutOfOrder( g_model.logRate, (uint8_t *)LogRateMap, 4 ) ;
   		}
		}
		else if ( k == 2 )
		{
			g_model.logNew = touchOnOffItem( g_model.logNew, y, (char *)XPSTR("New File"), attr, colour ) ;
		}
		else if ( k == 3 )
		{
			drawItem( (char *)XPSTR("Data Timeout(s)"), y, attr ) ;
			drawNumber( TRIGHT-TRMARGIN, y, g_model.telemetryTimeout + 25, attr|PREC1) ; //, attr ? ~hcolour : hcolour ) ;
			if(attr)
			{
				CHECK_INCDEC_H_MODELVAR_0( g_model.telemetryTimeout, 75 ) ;
   		}
		}
		else if ( k == 4 )
		{
			drawItem( (char *)XPSTR("Select None"), y, attr ) ;
			if ( attr )
			{
#ifdef TOUCH
				uint32_t activated = handleSelectIcon() ;
 #ifdef X20
    		if ( (event == EVT_KEY_BREAK(BTN_RE)) || activated )
 #else
    		if ( (event == EVT_KEY_BREAK(BTN_RE)) || (event == EVT_KEY_BREAK(KEY_MENU)) || activated )
 #endif
#else				
    		if ( (event == EVT_KEY_BREAK(BTN_RE)) || (event == EVT_KEY_BREAK(KEY_MENU)) )
#endif
				{
  				s_editMode = 0 ;
					g_model.LogDisable[0] = 0xFFFFFFFF ;
					g_model.LogDisable[1] = 0xFFFFFFFF ;
					g_model.LogDisable[2] = 0xFFFFFFFF ;
					g_model.LogDisable[3] = 0xFFFFFFFF ;
				}
			}
		}
		else if ( k == 5 )
		{
			drawItem( (char *)XPSTR("Select All"), y, attr ) ;
			if ( attr )
			{
#ifdef TOUCH
				uint32_t activated = handleSelectIcon() ;
 #ifdef X20
    		if ( (event == EVT_KEY_BREAK(BTN_RE)) || activated )
 #else
    		if ( (event == EVT_KEY_BREAK(BTN_RE)) || (event == EVT_KEY_BREAK(KEY_MENU)) || activated )
 #endif
#else				
    		if ( (event == EVT_KEY_BREAK(BTN_RE)) || (event == EVT_KEY_BREAK(KEY_MENU)) )
#endif
				{
  				s_editMode = 0 ;
					g_model.LogDisable[0] = 0 ;
					g_model.LogDisable[1] = 0 ;
					g_model.LogDisable[2] = 0 ;
					g_model.LogDisable[3] = 0 ;
				}
			}
		}
		else if ( k == 6 )
		{
			drawItem( (char *)XPSTR("Select Active"), y, attr ) ;
			if ( attr )
			{
#ifdef TOUCH
				uint32_t activated = handleSelectIcon() ;
 #ifdef X20
    		if ( (event == EVT_KEY_BREAK(BTN_RE)) || activated )
 #else
    		if ( (event == EVT_KEY_BREAK(BTN_RE)) || (event == EVT_KEY_BREAK(KEY_MENU)) || activated )
 #endif
#else				
    		if ( (event == EVT_KEY_BREAK(BTN_RE)) || (event == EVT_KEY_BREAK(KEY_MENU)) )
#endif
				{
  				s_editMode = 0 ;
					uint32_t i ;
					uint32_t idx ;
					for ( i = 0 ;  i < sizeof(LogLookup) ; i += 1 )
					{
						idx = LogLookup[i]-1 ;
#ifndef X20
 						if ( idx > TELEM_GAP_START )
						{
							idx += 8 ;
						}
#endif
#ifdef BITBAND_SRAM_REF
						uint32_t *p ;
						uint32_t index = LogLookup[i]-1 ;
						p = (uint32_t *) (BITBAND_SRAM_BASE + ((uint32_t)&g_model.LogDisable - BITBAND_SRAM_REF) * 32) ;
						p[index] = !telemItemValid( idx ) ;
#else
						uint32_t offset ;
						uint32_t bit ;
						uint32_t index = LogLookup[i]-1 ;
						offset = index >> 5 ;
						bit = 1 << (index & 0x0000001F) ;
						if ( telemItemValid( idx ) )
						{
							g_model.LogDisable[offset]	&= ~bit ;
						}
						else
						{
							g_model.LogDisable[offset]	|= bit ;
						}
#endif
					}
				}
			}
		}
		else
		{
			uint32_t index = LogLookup[k-7]-1 ;
#ifndef BITBAND_SRAM_REF
			uint32_t bit ;
			uint32_t offset ;
			uint32_t value ;
#endif
			if ( k == 7+sizeof(LogLookup) )
			{
				index = LOG_LAT ;
				drawItem( (char *)XPSTR("Lat"), y, attr ) ;
				if ( TelemetryDataValid[FR_GPS_LAT] )
				{
#ifdef X20
   				putTick( 11*FW, y+TVOFF, LcdForeground );
#else
					putTick( 60, y + TVOFF ) ;
#endif
				}
			}
			else if ( k == 8+sizeof(LogLookup) )
			{
				index = LOG_LONG ;
				drawItem( (char *)XPSTR("Long"), y, attr ) ;
				if ( TelemetryDataValid[FR_GPS_LONG] )
				{
#ifdef X20
   				putTick( 11*FW, y+TVOFF, LcdForeground );
#else
					putTick( 60, y + TVOFF ) ;
#endif
				}
			}
			else if ( k == 9+sizeof(LogLookup) )
			{
				index = LOG_BTRX ;
				drawItem( (char *)XPSTR("BtRx"), y, attr ) ;
			}
			else if ( k > 9+sizeof(LogLookup) )
			{
				index = LOG_STK_THR + k - (10+sizeof(LogLookup)) ;
				char *p ;
				
				p = XPSTR("Stk-THR\0Stk-AIL\0Stk-ELE\0Stk-RUD") ;
				p += 8 * (index - LOG_STK_THR) ;
				drawItem( p, y, attr ) ;

#ifdef X20
   			putTick( 11*FW, y+TVOFF, LcdForeground );
#else
				putTick( 60, y + TVOFF ) ;
#endif
			}
			else
			{
				uint32_t idx ;
#ifdef X20
				idx = AlphaLogLookup[k-7] ;
#else
				idx = SharedMemory.AlphaLogLookup[k-7] ;
#endif
				attr &= ~TSSI_TEXT ;
				setLastTelemIdx( idx+1 ) ;
				drawItem( (char *)LastItem, y, attr ) ;

#ifndef X20
				if ( idx > TELEM_GAP_START )
				{
					idx += 8 ;
				}
#endif				
#ifdef X20
				if ( index < TELEMETRYDATALENGTH )
#else
				if ( index < HUBDATALENGTH )
#endif
				{
					if ( telemItemValid( idx ) )
					{
#ifdef X20
	   				putTick( 11*FW, y+TVOFF, LcdForeground );
#else
						putTick( 60, y + TVOFF ) ;
#endif
					}
					if ( attr )
					{
						putsTelemetryChannel( 14*FW, 0, idx, get_telemetry_value(idx), 0, TELEM_UNIT ) ;
					}
				}
				index = idx ;
#ifndef X20
				if ( index > TELEM_GAP_START )
				{
					index -= 8 ;
				}
#endif
			}
#ifdef BITBAND_SRAM_REF
			uint32_t *p ;
			p = (uint32_t *) (BITBAND_SRAM_BASE + ((uint32_t)&g_model.LogDisable - BITBAND_SRAM_REF) * 32) ;
			p[index] = offonItem( p[index], y, attr ) ;
#else
			offset = index >> 5 ;
			bit = 1 << (index & 0x0000001F) ;
			value = 1 ;
			if ( g_model.LogDisable[offset] & bit )
			{
				value = 0 ;
			}
			value = touchOnOffItemMultiple( value, y, attr, colour ) ;
			if ( value == 0 )
			{
				g_model.LogDisable[offset]	|= bit ;
			}
			else
			{
				g_model.LogDisable[offset]	&= ~bit ;
			}
#endif
		}
	}
}

void editOneInput(uint8_t event)
{
	uint8_t rows = 9 ;
	struct te_InputsData *pinput = &g_model.inputs[s_curItemIdx] ;
//	uint32_t newVpos ;

#ifdef TOUCH
	TlExitIcon = 1 ;
	uint32_t selected = 0 ;
#endif

	if ( event == EVT_ENTRY )
	{
		createInputMap() ;
	}

	static MState2 mstate2 ;
	mstate2.check_columns(event, rows - 1 ) ;

	TITLE( XPSTR("Edit Input"));

	if ( event == EVT_ENTRY )
	{
		RotaryState = ROTARY_MENU_UD ;
	}
	if ( event == EVT_ENTRY_UP )
	{
		if ( TextResult )
		{
			uint8_t value ;
			value = InputMap[ TextIndex ] ;
			pinput->srcRaw = value ;
			eeDirty(EE_MODEL) ;
		}
	}

  uint32_t  sub    = mstate2.m_posVert;

	uint32_t t_pgOfs ;
	t_pgOfs = evalHresOffset( sub ) ;
#ifndef TOUCH
	(void) t_pgOfs ;
#endif

	lcd_hline( 0, TTOP, TRIGHT ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
	selected = newVert & 0x0100 ;
#endif

//	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
//	if ( newVpos != t_pgOfs )
//	{
//		s_pgOfs = t_pgOfs = newVpos ;
//		if ( sub < t_pgOfs )
//		{
//			mstate2.m_posVert = sub = t_pgOfs ;
//		}
//		else if ( sub > t_pgOfs + TLINES - 1 )
//		{
//			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
//		}
//	}

  for ( uint32_t k = 0 ; k<TLINES ; k += 1 )
  {
		uint16_t y = TTOP + k*TFH ;
    uint8_t i = k + s_pgOfs ;
    uint8_t attr = sub==i ? INVERS : 0;
  	uint8_t b ;
	
    switch(i)
		{
      case 0:
			{	
				drawItem( (char *)PSTR(STR_2SOURCE)+1, y, attr ) ;
				uint32_t value = pinput->srcRaw ;
				saveEditColours( attr, DimBackColour ) ;
#ifdef X20
				putsChnOpRaw( (TRIGHT-TRMARGIN)*TSCALE, (y+TVOFF)*TSCALE, value, 0, LUA_RIGHT ) ;
#else
				putsChnOpRaw( (TRIGHT-TRMARGIN)*TSCALE, (y+TVOFF)*TSCALE, value, 0, 0, LUA_RIGHT ) ;
#endif
				restoreEditColours() ;
				
				if ( attr )
				{
					uint8_t x = value ;
#ifdef TOUCH
					if ( handleSelectIcon() || ( event == EVT_KEY_BREAK(BTN_RE) ) || selected )
#else
					if ( ( event == EVT_KEY_BREAK(BTN_RE) ) )
#endif
					{
						// Long MENU pressed
						TextIndex = x ;
  					TextType = TEXT_TYPE_CUSTOM ;
#ifdef X20
						TextDataControl.TextOption = 0 ;
						TextDataControl.TextWidth = 6 ;
						TextDataControl.TextMax = InputMapSize-1 ;
						TextDataControl.TextFunction = displayInputSource ;
						TextDataControl.TextMap = InputMap ;
#else
						TextControl.TextOption = 0 ;
						TextControl.TextWidth = 6 ;
						TextControl.TextMax = InputMapSize-1 ;
						TextControl.TextFunction = displayInputSource ;
						TextControl.TextMap = InputMap ;
#endif
  					killEvents(event) ;
						pushMenu(menuTextHelp) ;
    				s_editMode = false ;
					}
					pinput->srcRaw = x ;
			  }
			}
			break ;
	    case 1 :
			{	
				drawItem( (char *)PSTR(STR_2WEIGHT)+1, y, attr ) ;
				saveEditColours( attr, DimBackColour ) ;
				drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+2 ) ;
#ifdef USE_VARS
				if ( g_model.vars )
				{
					int16_t value = pinput->weight ;
					if ( pinput->varForWeight )
					{
						value += 1000 ;
					}
					value = editVarCapableValue( TRIGHT-TRMARGIN+FW, y+TVOFF, value, -100, 100, NUM_VARS, attr, event ) ;
					if ( value > 900 )
					{
						value -= 1000 ;
						pinput->varForWeight = 1 ;
					}
					else
					{
						pinput->varForWeight = 0 ;
					}
					pinput->weight = value ;
				}
				else
#endif
				{
					pinput->weight = gvarMenuItem( TRIGHT-TRMARGIN, y+TVOFF, pinput->weight, -100, 100, GVAR_100|attr, event ) ;
				}
				restoreEditColours() ;
 	  		
//				int16_t value = pinput->weight ;
//				if ( value > 100 )
//				{
//					value += 400 ;
//				}	
//				value = gvarDiffValue( 15*FW, y, value, attr, event ) ;
//				restoreEditColours() ;
//				if ( value > 500 )
//				{
//					value -= 400 ;
//				}
//				pinput->weight = value ;
			}
			break ;
	    case 2 :
				drawItem( (char *)PSTR(STR_2SWITCH)+1, y, attr ) ;
				saveEditColours( attr, DimBackColour ) ;
				putsDrSwitches(TRIGHT-TRMARGIN, y+TVOFF/TSCALE, pinput->swtch, LUA_RIGHT) ;
				restoreEditColours() ;
        if(attr) CHECK_INCDEC_MODELSWITCH( pinput->swtch, -MaxSwitchIndex, MaxSwitchIndex);
			break ;
	    case 3 :
				drawItem( (char *)XPSTR("TRIM"), y, attr ) ;
				drawIdxText( y, (char *)XPSTR("\003 OnOffRudEleThrAil"), pinput->carryTrim, attr ) ;
   			if (attr)
				{
					CHECK_INCDEC_H_MODELVAR( pinput->carryTrim, 0, 5) ;
				}
			break ;
	    case 4 :
			{	
				b = 1 ;
				drawItem( (char *)PSTR(STR_MODES), y, attr ) ;
						
				if ( attr )
				{
					Columns = 7 ;
				}
  					
				for ( uint32_t p = 0 ; p<MAX_MODES+2 ; p++ )
				{
					uint8_t z = pinput->flightModes ;
#ifdef X20
   				lcdDrawChar( TRIGHT-TRMARGIN-8*(FW+4)+p*(FW+4), y+TVOFF, '0'+p, ( z & b ) ? 0 : INVERS ) ;
#else
   				PUTC_ATT( TRIGHT-TRMARGIN-7*(FW+3)+p*(FW+2), y+TVOFF, '0'+p, ( z & b ) ? 0 : INVERS ) ;
#endif
					if( attr && ( g_posHorz == p ) )
					{
#ifdef X20
						lcdHiresRect( TRIGHT-TRMARGIN-8*(FW+4)+p*(FW+4)-1, y+TVOFF-1, FW+2, 34, DimBackColour ) ;
						lcdHiresRect( TRIGHT-TRMARGIN-8*(FW+4)+p*(FW+4)-2, y+TVOFF-2, FW+4, 36, DimBackColour ) ;
						lcdHiresRect( TRIGHT-TRMARGIN-8*(FW+4)+p*(FW+4)-3, y+TVOFF-3, FW+6, 38, DimBackColour ) ;
#else
						lcd_rectColour( TRIGHT-TRMARGIN-7*(FW+3)+p*(FW+2)-1, y+TVOFF-1, FW+2, 9, DimBackColour ) ;
//						lcdHiresRect( TRIGHT-TRMARGIN-8*(FW+4)+p*(FW+4)-1, y+TVOFF-1, FW+2, 34, DimBackColour ) ;
//						lcdHiresRect( TRIGHT-TRMARGIN-8*(FW+4)+p*(FW+4)-2, y+TVOFF-2, FW+4, 36, DimBackColour ) ;
//						lcdHiresRect( TRIGHT-TRMARGIN-8*(FW+4)+p*(FW+4)-3, y+TVOFF-3, FW+6, 38, DimBackColour ) ;
#endif
						if ( event==EVT_KEY_BREAK(BTN_RE) ) 
						{
							pinput->flightModes ^= b ;
     					eeDirty(EE_MODEL) ;
   						s_editMode = false ;
						}
					}
					b <<= 1 ;
				}
			}
			break ;
	    case 5 :
			{	
				drawItem( (char *)PSTR(STR_OFFSET), y, attr ) ;
				saveEditColours( attr, DimBackColour ) ;
				drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+2 ) ;
				
				int16_t value = pinput->offset ;

#ifdef USE_VARS
				if ( g_model.vars )
				{
					if ( pinput->varForOffset )
					{
						value += 1000 ;
					}
					value = editVarCapableValue( TRIGHT-TRMARGIN+FW, y+TVOFF, value, -100, 100, NUM_VARS, attr, event ) ;
					if ( value > 900 )
					{
						value -= 1000 ;
						pinput->varForOffset = 1 ;
					}
					else
					{
						pinput->varForOffset = 0 ;
					}
				}
				else
#endif
				{
					value = gvarMenuItem( TRIGHT-TRMARGIN, y+TVOFF, value, -100, 100, GVAR_100|attr, event ) ;
				}
				pinput->offset = value ;
				restoreEditColours() ;

//				int16_t value = pinput->offset ;
//				if ( value > 100 )
//				{
//					value += 400 ;
//				}	
//				value = gvarDiffValue( 15*FW, y, value, attr, event ) ;
//				restoreEditColours() ;
//				if ( value > 500 )
//				{
//					value -= 400 ;
//				}
//				pinput->offset = value ;
			}	
			break ;
	    case 6 :
			{	
				drawItem( (char *)XPSTR("Curve"), y, attr ) ;
				drawIdxText( y, (char *)XPSTR("\005 None FuncCurve Diff Expo"), pinput->mode, attr ) ;
				uint8_t oldvalue = pinput->mode ;
   		  if(attr) CHECK_INCDEC_H_MODELVAR( pinput->mode, 0, 4 ) ;
				if ( pinput->mode != oldvalue )
				{
					if ( ( pinput->mode == 3 ) || ( pinput->mode == 4 ) )		// diff/expo
					{
						pinput->curve = 0 ;
					}
					else if ( pinput->mode == 2 )		// curve
					{
						pinput->curve = 0 ;
					}
					else if ( pinput->mode == 1 )		// Function
					{
						pinput->curve = 1 ;
					}
					else
					{
						pinput->curve = 0 ;
					}
				}
			}
			break ;
	    case 7 :
				drawItem( (char *)XPSTR("Value"), y, attr ) ;
				saveEditColours( attr, DimBackColour ) ;
				if ( pinput->mode == 3 )	// Diff
				{
					int16_t value = pinput->curve ;
					drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+2 ) ;
#ifdef USE_VARS
					if ( g_model.vars )
					{
						if ( pinput->varForCurve )
						{
							value += 1000 ;
						}
						value = editVarCapableValue( TRIGHT-TRMARGIN, y+TVOFF, value, -100, 100, NUM_VARS, attr, event ) ;
						if ( value > 900 )
						{
							value -= 1000 ;
							pinput->varForCurve = 1 ;
						}
						else
						{
							pinput->varForCurve = 0 ;
						}
					}
					else
#endif
					{
						if ( value > 100 )
						{
							value += 400 ;
						}	
						value = gvarDiffValue( TRIGHT-TRMARGIN, y+TVOFF, value, attr|0x80000000, event ) ;
						if ( value > 500 )
						{
							value -= 400 ;
						}
					}
					pinput->curve = value ;
				}
				else
				{
					if ( pinput->mode == 4 )	// Expo
					{
						drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+2 ) ;
						int16_t value = pinput->curve ;
#ifdef USE_VARS
						if ( g_model.vars )
						{
							if ( pinput->varForCurve )
							{
								value += 1000 ;
							}
							value = editVarCapableValue( TRIGHT-TRMARGIN, y+TVOFF, value, -100, 100, NUM_VARS, attr, event ) ;
							if ( value > 900 )
							{
								value -= 1000 ;
								pinput->varForCurve = 1 ;
							}
							else
							{
								pinput->varForCurve = 0 ;
							}
						}
						else
#endif
						{
							if ( value > 100 )
							{
								value += 400 ;
							}	
							value = gvarDiffValue( TRIGHT-TRMARGIN, y+TVOFF, value, attr|0x80000000, event ) ;
							if ( value > 500 )
							{
								value -= 400 ;
							}
						}
						pinput->curve = value ;
					}
					else
					{
						int32_t temp ;
						temp = pinput->curve ;
						if ( pinput->mode == 2 )
						{
							if ( temp >= 0 )
							{
								temp += 7 ;
							}
						}
						put_curve( TRIGHT-TRMARGIN, y+TVOFF/2, temp, LUA_RIGHT ) ;
						if ( pinput->mode )
						{
          	  if(attr)
							{
								if ( pinput->mode == 1 ) // Function
								{
        		  		CHECK_INCDEC_H_MODELVAR( pinput->curve, 1, 6 ) ;
								}
								else
								{
        		  		CHECK_INCDEC_H_MODELVAR( pinput->curve, -MAX_CURVE5-MAX_CURVE9-3, MAX_CURVE5+MAX_CURVE9-1+3 ) ;
								}
							}
						}
					}
				}
				restoreEditColours() ;
			break ;
	    case 8 :
				drawItem( (char *)XPSTR("Side"), y, attr ) ;
				drawIdxText( y, (char *)XPSTR("\003---x>0x<0"), pinput->side, attr ) ;
   			if (attr)
				{
					CHECK_INCDEC_H_MODELVAR( pinput->side, 0, 2) ;
				}
			break ;
	  }
	}
}

