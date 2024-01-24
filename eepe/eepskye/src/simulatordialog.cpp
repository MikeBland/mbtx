#include "simulatordialog.h"
#include "ui_simulatordialog.h"
#include "../../common/node.h"
#include <QtGui>
#include <stdint.h>
#include "pers.h"
#include "helpers.h"
#include "qextserialenumerator.h"
#include "qextserialport.h"
#include <QMessageBox>
//#include <QSound>


#define GBALL_SIZE  20
#define GVARS	1

#define RESX    1024
#define RESXu   1024u
#define RESXul  1024ul
#define RESXl   1024l
#define RESKul  100ul
#define RESX_PLUS_TRIM (RESX+128)

//#define IS_THROTTLE(x)  (((2-(g_eeGeneral.stickMode&1)) == x) && (x<4))

const uint8_t switchIndex[8] = { HSW_SA0, HSW_SB0, HSW_SC0, HSW_SD0, HSW_SE0, HSW_SF2, HSW_SG0, HSW_SH2 } ;

int32_t WelcomePlayed = 0 ;

//uint16_t DebugOutputCount ;

uint8_t simulatorDialog::IS_THROTTLE( uint8_t x)
{
	if ( g_model.modelVersion >= 2 )
	{
		return ((x) == 2) ;
	}
	return (((2-(g_eeGeneral.stickMode&1)) == x) && (x<4)) ;
}

#define GET_DR_STATE(x) (!getSwitchDr(g_model.expoData[x].drSw1) ?   \
    DR_HIGH :                                  \
    !getSwitchDr(g_model.expoData[x].drSw2)?   \
    DR_MID : DR_LOW);

extern int GlobalModified ;
extern EEGeneral Sim_g ;
extern int GeneralDataValid ;
extern ModelData Sim_m ;
extern int ModelDataValid ;

uint8_t Last_switch[NUM_SKYCSW] ;

simulatorDialog::simulatorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::simulatorDialog)
{
    ui->setupUi(this);
		
		current_limits = 2 ;
    beepVal = 0;
    beepShow = 0;

    bpanaCenter = 0;
    g_tmr10ms = 0;
		one_sec_precount = 0 ;
		serialSending = 0 ;
		gvar_or_scalers = 0 ;
		port = NULL ;

    memset(&chanOut,0,sizeof(chanOut));
    memset(&calibratedStick,0,sizeof(calibratedStick));
    memset(&g_ppmIns,0,sizeof(g_ppmIns));
    memset(&ex_chans,0,sizeof(ex_chans));
    memset(&fade,0,sizeof(ex_chans));
    memset(&trim,0,sizeof(trim));

    memset(&sDelay,0,sizeof(sDelay));
    memset(&act,0,sizeof(act));

    memset(&anas,0,sizeof(anas));
    memset(&chans,0,sizeof(chans));

    memset(&swOn,0,sizeof(swOn));
    memset(&CsTimer_lastVal,0,sizeof(CsTimer_lastVal));

		CalcScaleNest = 0 ;

    trimptr[0] = &trim[0] ;
    trimptr[1] = &trim[1] ;
    trimptr[2] = &trim[2] ;
    trimptr[3] = &trim[3] ;

		fadeRate = 0 ;
		fadePhases = 0 ;

		for ( int i = 1 ; i < MAX_PHASES+1 ; i += 1 )
		{
			fadeScale[i] = 0 ;
		}
    fadeScale[0] = 25600 ;

    setupSticks();
		ticktimer = 0 ;
		txType = 0 ;
		txBitType = 0 ;
		ui->SAslider->setValue(0) ;
		ui->SBslider->setValue(0) ;
		ui->SEslider->setValue(0) ;
		ui->SFslider->setValue(0) ;
		ui->SCslider->setValue(0) ;
		ui->SDslider->setValue(0) ;
		ui->SGslider->setValue(0) ;
//		ui->SHslider->setValue(0) ;
		QList<QextPortInfo> ports = QextSerialEnumerator::getPorts() ;
    ui->serialPortCB->clear() ;
	  foreach (QextPortInfo info, ports)
		{
			if ( info.portName.length() )
			{
	  		ui->serialPortCB->addItem(info.portName) ;
			}
		}
		VoiceCheckFlag100mS = 2 ;
//    setupTimer();
}

simulatorDialog::~simulatorDialog()
{
		if ( port )
		{
			if (port->isOpen())
			{
		 	  port->close();
			}
		 	delete port ;
			port = NULL ;
		}
    delete ui;
}

void simulatorDialog::closeEvent(QCloseEvent *event)
{
	if ( ticktimer )
	{
    ticktimer->stop() ;
    delete ticktimer ;
	}
	ticktimer = 0 ;
	if ( port )
	{
		if (port->isOpen())
		{
	 	  port->close();
		}
	 	delete port ;
		port = NULL ;
	}
	event->accept() ;
}

void simulatorDialog::setupTimer()
{
  if (ticktimer == 0)
  {
    ticktimer = new QTimer(this);
    connect(ticktimer,SIGNAL(timeout()),this,SLOT(timerEvent()));
	}
  getValues();
	CurrentPhase = getFlightPhase() ;
  perOut(true,0);
  ticktimer->start(10);
}

int8_t getAndSwitch( SKYCSwData &cs, uint32_t txType )
{
  int8_t x = 0 ;
  if ( ( txType == RADIO_TYPE_SKY ) || ( txType == RADIO_TYPE_9XTREME ) )
	{
		if ( cs.andsw )	// Code repeated later, could be a function
		{
			x = cs.andsw ;
			if ( ( x > 8 ) && ( x <= 9+NUM_SKYCSW ) )
			{
				x += 1 ;
			}
			if ( ( x < -8 ) && ( x >= -(9+NUM_SKYCSW) ) )
			{
				x -= 1 ;
			}
			if ( x == 9+NUM_SKYCSW+1 )
			{
				x = 9 ;			// Tag TRN on the end, keep EEPROM values
			}
			if ( x == -(9+NUM_SKYCSW+1) )
			{
				x = -9 ;			// Tag TRN on the end, keep EEPROM values
			}
		}
	}
	else
	{
		x = cs.andsw ;
	}
	return x ;
}

// every 20mS
void simulatorDialog::processSwitchTimer( uint32_t i )
{
  SKYCSwData &cs = g_model.customSw[i];
//  uint8_t cstate = CS_STATE(cs.func);

//  if(cstate == CS_TIMER)
//	{
		int16_t y ;
		y = CsTimer_lastVal[i] ;
		if ( y == 0 )
		{
			int8_t z ;
			z = cs.v1 ;
			if ( z >= 0 )
			{
				z = -z-1 ;
				y = z * 50 ;
			}
			else
			{
				y = z * 5 ;
			}
			g_model.gvars[5].gvar = y ;
		}
		else if ( y < 0 )
		{
			if ( ++y == 0 )
			{
				int8_t z ;
				z = cs.v2 ;
				if ( z >= 0 )
				{
					z += 1 ;
					y = z * 50 - 1 ;
				}
				else
				{
					y = -(z*5)-1 ;
				}
				g_model.gvars[6].gvar = y ;
			}
		}
		else  // if ( CsTimer_lastVal[i] > 0 )
		{
			y -= 1 ;
		}

		int8_t x = getAndSwitch( cs, txType ) ;
		if ( x )
		{
      if (getSwitch( x,0,0) == 0 )
			{
				Last_switch[i] = 0 ;
				if ( cs.func == CS_NTIME )
				{
					int8_t z ;
					z = cs.v1 ;
					if ( z >= 0 )
					{
						z = -z-1 ;
						y = z * 50 ;					
					}
					else
					{
						y = z * 5 ;
					}
				}
				else
				{
					y = -1 ;
				}
			}
			else
			{
				Last_switch[i] = 2 ;
			}
		}
		CsTimer_lastVal[i] = y ;
//	}
}

inline qint16 calc100toRESX(qint8 x)
{
    return (qint16)x*10 + x/4 - x/64;
}

inline qint16 calc1000toRESX(qint16 x)
{
    return x + x/32 - x/128 + x/512;
}


void simulatorDialog::processSwitches()
{
	uint32_t cs_index ;
	for ( cs_index = 0 ; cs_index < NUM_SKYCSW ; cs_index += 1 )
	{
  	SKYCSwData &cs = g_model.customSw[cs_index] ;
  	uint8_t ret_value = false ;

  	if( cs.func )
		{
  		int8_t a = cs.v1 ;
  		int8_t b = cs.v2 ;
  		int16_t x = 0 ;
  		int16_t y = 0 ;
  		uint8_t s = CS_STATE( cs.func, g_model.modelVersion ) ;

  		if ( (s == CS_VOFS) || (s == CS_2VAL) )
  		{
  		  x = getValue(cs.v1u-1);
    		if ( ( ( cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1u < EXTRA_POTS_START ) ) || (cs.v1u >= EXTRA_POTS_START + 8) )
				{
  		    y = convertTelemConstant( cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, cs.v2, &g_model ) ;
				}
  		  else
				{
  		  	y = calc100toRESX(cs.v2);
				}
  		}
  		else if(s == CS_VCOMP)
  		{
 		    x = getValue(cs.v1u-1);
 		    y = getValue(cs.v2u-1);
  		}

  		switch (cs.func)
			{
	  		case (CS_VPOS):
  		    ret_value = (x>y);
  	    break;
  			case (CS_VNEG):
  		    ret_value = (x<y) ;
  	    break;
  			case (CS_APOS):
	  	    ret_value = (abs(x)>y) ;
  		  break;
	  		case (CS_ANEG):
  		    ret_value = (abs(x)<y) ;
  	    break;
				case CS_VEQUAL :
  		    ret_value = (x == y) ;
  	    break;
				case CS_EXEQUAL:
					if ( isAgvar( cs.v1 ) )
					{
						x *= 10 ;
						y *= 10 ;
					}
  		  	ret_value = abs(x-y) < 32 ;
  			break;
	
				case CS_VXEQUAL:
					if ( isAgvar( cs.v1 ) || isAgvar( cs.v2 ) )
					{
						x *= 10 ;
						y *= 10 ;
					}
  			  ret_value = abs(x-y) < 32 ;
  			break;
		
  			case (CS_AND):
  			case (CS_OR):
  			case (CS_XOR):
  			{
  			  bool res1 = getSwitch(a,0,0) ;
  			  bool res2 = getSwitch(b,0,0) ;
  			  if ( cs.func == CS_AND )
  			  {
  			    ret_value = res1 && res2 ;
  			  }
  			  else if ( cs.func == CS_OR )
  			  {
  			    ret_value = res1 || res2 ;
  			  }
  			  else  // CS_XOR
  			  {
  			    ret_value = res1 ^ res2 ;
  			  }
  			}
  			break;

	  		case (CS_EQUAL):
  		    ret_value = (x==y);
  	    break;
  			case (CS_NEQUAL):
  		    ret_value = (x!=y);
  	    break;
  			case (CS_GREATER):
  		    ret_value = (x>y);
  		   break;
	  		case (CS_LESS):
  		    ret_value = (x<y);
  	    break;
	  		case (CS_NTIME):
					processSwitchTimer( cs_index ) ;
					ret_value = CsTimer_lastVal[cs_index] >= 0 ;
  			break ;
				case (CS_TIME):
				{	
					processSwitchTimer( cs_index ) ;
  			  ret_value = CsTimer_lastVal[cs_index] >= 0 ;
					int8_t x = getAndSwitch( cs, txType ) ;
					if ( x )
					{
					  if (getSwitch( x, 0, 0 ) )
						{
							if ( ( Last_switch[cs_index] & 2 ) == 0 )
							{ // Triggering
								ret_value = 1 ;
							}	
						}
					}
				}
  			break ;
  			case (CS_MONO):
  			case (CS_RMONO):
				{
					if ( VoiceCheckFlag100mS & 2 )
					{
						// Resetting, retrigger any monostables
						Last_switch[cs_index] &= ~2 ;
					}
					int8_t andSwOn = 1 ;
					if ( ( cs.func == CS_RMONO ) )
					{
						andSwOn = getAndSwitch( cs, txType ) ;
						if ( andSwOn )
						{
							andSwOn = getSwitch( andSwOn,0,0) ;
						}
						else
						{
							andSwOn = 1 ;
						}
					}
					
				  if (getSwitch( cs.v1,0,0) )
					{
						if ( ( Last_switch[cs_index] & 2 ) == 0 )
						{
							// Trigger monostable
							uint8_t trigger = 1 ;
							if ( ( cs.func == CS_RMONO ) )
							{
								if ( ! andSwOn )
								{
									trigger = 0 ;
								}
							}
							if ( trigger )
							{
								Last_switch[cs_index] = 3 ;
								int16_t x ;
								x = cs.v2 * 5 ;
								if ( x < 0 )
								{
									x = -x ;
								}
								else
								{
									x += 5 ;
									x *= 10 ;
								}
								CsTimer_lastVal[cs_index] = x ;							
							}
						}
					}
					else
					{
						Last_switch[cs_index] &= ~2 ;
					}
					int16_t y ;
					y = CsTimer_lastVal[cs_index] ;
					if ( Now_switch[cs_index] < 2 )	// not delayed
					{
						if ( y )
						{
							if ( ( cs.func == CS_RMONO ) )
							{
								if ( ! andSwOn )
								{
									y = 1 ;
								}	
							}
							if ( --y == 0 )
							{
								Last_switch[cs_index] &= ~1 ;
							}
							CsTimer_lastVal[cs_index] = y ;
						}
					}
 			  	ret_value = CsTimer_lastVal[cs_index] > 0 ;
				}
  			break ;
  
				case (CS_LATCH) :
		  		if (getSwitch( cs.v1,0,0) )
					{
						Last_switch[cs_index] = 1 ;
					}
					else
					{
					  if (getSwitch( cs.v2,0,0) )
						{
							Last_switch[cs_index] = 0 ;
						}
					}
  			  ret_value = Last_switch[cs_index] & 1 ;
  			break ;
  			
				case (CS_FLIP) :
		  		if (getSwitch( cs.v1,0,0) )
					{
						if ( ( Last_switch[cs_index] & 2 ) == 0 )
						{
							// Clock it!
					    if (getSwitch( cs.v2,0,0) )
							{
								Last_switch[cs_index] = 3 ;
							}
							else
							{
								Last_switch[cs_index] = 2 ;
							}
						}
					}
					else
					{
						Last_switch[cs_index] &= ~2 ;
					}
  			  ret_value = Last_switch[cs_index] & 1 ;
  			break ;
  			case (CS_BIT_AND) :
				{	
  			  x = getValue(cs.v1u-1);
					y = (uint8_t) cs.v2u ;
					y |= cs.bitAndV3 << 8 ;
  			  ret_value = ( x & y ) != 0 ;
				}
  			break ;
				case CS_RANGE :
				{
					int16_t z ;
    			if ( ( ( cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1u < EXTRA_POTS_START ) ) || (cs.v1u >= EXTRA_POTS_START + 8) )
					{
            z = convertTelemConstant( cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, (int8_t)cs.bitAndV3, &g_model ) ;
					}
  		  	else
					{
  		  		z = calc100toRESX((int8_t)cs.bitAndV3) ;
					}
  		    ret_value = (x >= y) && (x <= z) ;
				}			 
				case CS_DELTAGE :
				case CS_MOD_D_GE :
				{
					uint8_t update = 0 ;
//          if (LS_LAST_VALUE(mixerCurrentFlightMode, idx) == CS_LAST_VALUE_INIT) {
//            LS_LAST_VALUE(mixerCurrentFlightMode, idx) = x;
//          }
//          int16_t diff = x - LS_LAST_VALUE(mixerCurrentFlightMode, idx);
          int16_t diff = x - CsTimer_lastVal[cs_index] ;
//          if (ls->func == LS_FUNC_DIFFEGREATER)
          if (cs.func == CS_DELTAGE )
					{
            if (y >= 0)
						{
              ret_value = (diff >= y);
              if (diff < 0)
                update = true;
            }
            else
						{
              ret_value = (diff <= y);
              if (diff > 0)
                update = true;
            }
          }
          else
					{
            ret_value = (abs(diff) >= y);
          }
          if (ret_value || update)
					{
//            LS_LAST_VALUE(mixerCurrentFlightMode, idx) = x ;
            CsTimer_lastVal[cs_index] = x ;
          }
				}			 
  			break ;
				default:
  		    ret_value = false;
 		    break;
  		}

			int8_t z = getAndSwitch( cs, txType ) ;
			if ( z )
			{
				switch ( cs.exfunc )
				{
					case 0 :
  		    	ret_value &= getSwitch( z, 0, 0 ) ;
					break ;
					case 1 :
  		    	ret_value |= getSwitch( z, 0, 0 ) ;
					break ;
					case 2 :
  		    	ret_value ^= getSwitch( z, 0, 0 ) ;
					break ;
				}
			}
			
//			if ( ret_value )
//			{
//				int8_t x = getAndSwitch( cs ) ;
//				if ( x )
//				{
//  		    ret_value = getSwitch( x, 0, 0 ) ;
//				}
//			}
			if ( ( cs.func < CS_LATCH ) || ( cs.func > CS_RMONO ) )
			{
				Last_switch[cs_index] = ret_value ;
			}
			if ( Now_switch[cs_index] == 0 )	// was off
			{
				if ( ret_value )
				{
					if ( g_model.switchDelay[cs_index] )
					{
						ret_value = g_model.switchDelay[cs_index] * 10 ;
					}
				}
			}
			else
			{
				if ( Now_switch[cs_index] > 1 )	// delayed
				{
					if ( ret_value )
					{
						uint8_t temp = Now_switch[cs_index] - 2 ;
						if ( temp )
						{
							ret_value = temp ;
						}
					}
				}
			}
			Now_switch[cs_index] = ret_value ;
		}
		else // no function
		{
			if ( VoiceCheckFlag100mS & 2 )
			{
				Now_switch[cs_index] = 0 ;
			}
		}
	}
	
}

void simulatorDialog::timerEvent()
{
		uint8_t i ;
		g_tmr10ms++;

		if ( GlobalModified )
		{
			if ( GeneralDataValid )
			{
	    	memcpy(&g_eeGeneral,&Sim_g,sizeof(EEGeneral));
				GeneralDataValid = 0 ;
				configSwitches() ;
			}
			if ( ModelDataValid )
			{
        memcpy(&g_model,&Sim_m,sizeof(SKYModelData));
				ModelDataValid = 0 ;
				VoiceCheckFlag100mS = 2 ;
			}
    	
			char buf[sizeof(g_model.name)+1];
    	memcpy(&buf,&g_model.name,sizeof(g_model.name));
    	buf[sizeof(g_model.name)] = 0;
    	modelName = tr("Simulating ") + QString(buf);
    	setWindowTitle(modelName);

    	if(g_eeGeneral.stickMode & 1)
    	{
    		  nodeLeft->setCenteringY(false);   //mode 1,3 -> THR on left
    		  ui->holdLeftY->setChecked(true);
    	}
    	else
    	{
    		  nodeRight->setCenteringY(false);   //mode 1,3 -> THR on right
    		  ui->holdRightY->setChecked(true);
    	}

//			CurrentPhase = getFlightPhase() ;

			trim[0] = g_model.trim[0] ;//(g_eeGeneral.stickMode>1)   ? 3 : 
			trim[1] = g_model.trim[1] ;//(g_eeGeneral.stickMode & 1) ? 2 : 
			trim[2] = g_model.trim[2] ;//(g_eeGeneral.stickMode & 1) ? 1 : 
			trim[3] = g_model.trim[3] ;//(g_eeGeneral.stickMode>1)   ? 0 : 
      ui->trimHLeft->setValue( getTrimValue( CurrentPhase, (g_eeGeneral.stickMode>1)   ? 3 : 0 ));  // mode=(0 || 1) -> rud trim else -> ail trim
      ui->trimVLeft->setValue( getTrimValue( CurrentPhase, (g_eeGeneral.stickMode & 1) ? 2 : 1 ));  // mode=(0 || 2) -> thr trim else -> ele trim
      ui->trimVRight->setValue(getTrimValue( CurrentPhase, (g_eeGeneral.stickMode & 1) ? 1 : 2 ));  // mode=(0 || 2) -> ele trim else -> thr trim
      ui->trimHRight->setValue(getTrimValue( CurrentPhase, (g_eeGeneral.stickMode>1)   ? 0 : 3 ));  // mode=(0 || 1) -> ail trim else -> rud trim
//    	ui->trimHLeft->setValue( g_model.trim[(g_eeGeneral.stickMode>1)   ? 3 : 0]);  // mode=(0 || 1) -> rud trim else -> ail trim
//    	ui->trimVLeft->setValue( g_model.trim[(g_eeGeneral.stickMode & 1) ? 2 : 1]);  // mode=(0 || 2) -> thr trim else -> ele trim
//    	ui->trimVRight->setValue(g_model.trim[(g_eeGeneral.stickMode & 1) ? 1 : 2]);  // mode=(0 || 2) -> ele trim else -> thr trim
//    	ui->trimHRight->setValue(g_model.trim[(g_eeGeneral.stickMode>1)   ? 0 : 3]);  // mode=(0 || 1) -> ail trim else -> rud trim
			GlobalModified = 0 ;
		}
    
		getValues();

    perOutPhase(false,0);

    setValues();
    centerSticks();

    timerTick();
    //    if(s_timerState != TMR_OFF)
    setWindowTitle(modelName + QString(" - Timer: (%3, %4) %1:%2")
                   .arg(abs(-s_timer[0].s_timerVal)/60, 2, 10, QChar('0'))
                   .arg(abs(-s_timer[0].s_timerVal)%60, 2, 10, QChar('0'))
                   .arg(getTimerMode(g_model.timer[0].tmrModeA))
                   .arg(g_model.timer[0].tmrDir ? "Count Up" : "Count Down"));

		ui->Timer1->setText(QString("%1:%2").arg(abs(-s_timer[0].s_timerVal)/60, 2, 10, QChar('0'))
                   .arg(abs(-s_timer[0].s_timerVal)%60, 2, 10, QChar('0'))) ;
		ui->Timer2->setText(QString("%1:%2").arg(abs(-s_timer[1].s_timerVal)/60, 2, 10, QChar('0'))
                   .arg(abs(-s_timer[1].s_timerVal)%60, 2, 10, QChar('0'))) ;

		if(beepVal)
    {
        beepVal = 0;
        QApplication::beep();
    }


#define CBEEP_ON  "QLabel { background-color: #FF364E }"
#define CBEEP_OFF "QLabel { }"

    ui->label_beep->setStyleSheet(beepShow ? CBEEP_ON : CBEEP_OFF);
    if(beepShow)
		{
			if ( --beepShow == 0 )
			{
			  ui->label_beep->setText( "BEEP" ) ;
			}
		}

		if ( one_sec_precount & 1 )	// Every 20mS
		{
			processSwitches() ;
			VoiceCheckFlag100mS = 0 ;

			
//			for ( i = 0 ; i < NUM_SKYCSW ; i += 1 )
//			{
//        SKYCSwData &cs = g_model.customSw[i];
////        uint8_t cstate = CS_STATE(cs.func, g_model.modelVersion);
//  			if ( g_model.modelVersion >= 3 )
//				{
//					if ( cs.func == CS_LATCH )
//					{
//		    	  if (getSwitch( cs.v1, 0, 0) )
//						{
//							Last_switch[i] = 1 ;
//						}
//						else
//						{
//			  	    if (getSwitch( cs.v2, 0, 0) )
//							{
//								Last_switch[i] = 0 ;
//							}
//						}
//					}
//					if ( cs.func == CS_FLIP )
//					{
//		    	  if (getSwitch( cs.v1, 0, 0) )
//						{
//							if ( ( Last_switch[i] & 2 ) == 0 )
//							{
//								// Clock it!
//			  	    	if (getSwitch( cs.v2, 0, 0) )
//								{
//									Last_switch[i] = 3 ;
//								}
//								else
//								{
//									Last_switch[i] = 2 ;
//								}
//							}
//						}
//						else
//						{
//							Last_switch[i] &= ~2 ;
//						}
//					}
//			  }
//			}
		}


		if ( ++one_sec_precount >= 10 )
		{
			one_sec_precount -= 10 ;
			// One tenth second has elapsed			
//			for ( i = 0 ; i < NUM_SKYCSW ; i += 1 )
//			{
//        SKYCSwData &cs = g_model.customSw[i];
//        uint8_t cstate = CS_STATE(cs.func, g_model.modelVersion);

//    		if(cstate == CS_TIMER)
//				{
//					int16_t y ;
//					y = CsTimer[i] ;
//					if ( y == 0 )
//					{
//						int8_t z ;
//						z = cs.v1 ;
//						if ( z >= 0 )
//						{
//							z = -z-1 ;
//							y = z * 10 ;					
//						}
//						else
//						{
//							y = z ;
//						}
//					}
//					else if ( y < 0 )
//					{
//						if ( ++y == 0 )
//						{
//							int8_t z ;
//							z = cs.v2 ;
//							if ( z >= 0 )
//							{
//								z += 1 ;
//								y = z * 10 - 1 ;
//							}
//							else
//							{
//								y = -z-1 ;
//							}
//						}
//					}
//					else  // if ( CsTimer[i] > 0 )
//					{
//						y -= 1 ;
//					}
//					int8_t x ;
//					if ( ( txType == 0 ) || ( txType == 3 ) )
//					{
//						x = getAndSwitch( cs, txType ) ;
//					}
//					else
//					{
//						x = cs.andsw ;
//					}
//					if ( x )
//					{
//	      	  if (getSwitch( x, 0, 0) == 0 )
//					  {
//							Last_switch[i] = 0 ;
//							if ( cs.func == CS_NTIME )
//							{
//								int8_t z ;
//								z = cs.v1 ;
//								if ( z >= 0 )
//								{
//									z = -z-1 ;
//									y = z * 10 ;					
//								}
//								else
//								{
//									y = z ;
//								}
//							}
//							else
//							{
//								y = -1 ;
//							}
//						}	
//						else
//						{
//							Last_switch[i] = 2 ;
//						}
//					}
//					CsTimer[i] = y ;
//				}
//  			if ( g_model.modelVersion >= 3 )
//				{
//					if ( ( cs.func == CS_MONO ) || ( cs.func == CS_RMONO ) )
//					{
//						int8_t andSwOn = 1 ;
//						if ( ( cs.func == CS_RMONO ) )
//						{
//							andSwOn = ((txType==1) || (txType == 2) || (txType == 9)) ? cs.andsw : getAndSwitch( cs, txType ) ;
//							if ( andSwOn )
//							{
//								andSwOn = getSwitch( andSwOn, 0, 0) ;
//							}
//							else
//							{
//								andSwOn = 1 ;
//							}
//						}
		    	  
//						if (getSwitch( cs.v1, 0, 0) )
//						{
//							if ( ( Last_switch[i] & 2 ) == 0 )
//							{
//								// Trigger monostable
//								uint8_t trigger = 1 ;
//								if ( ( cs.func == CS_RMONO ) )
//								{
//									if ( ! andSwOn )
//									{
//										trigger = 0 ;
//									}
//								}
//								if ( trigger )
//								{
//									Last_switch[i] = 3 ;
//									int16_t x ;
//									x = cs.v2 ;
//									if ( x < 0 )
//									{
//										x = -x ;
//									}
//									else
//									{
//										x += 1 ;
//										x *= 10 ;
//									}
//									CsTimer[i] = x ;							
//								}
//						  }
//						}
//						else
//						{
//							Last_switch[i] &= ~2 ;
//						}
//						int16_t y ;
//						y = CsTimer[i] ;
//						if ( y )
//						{
//							if ( ( cs.func == CS_RMONO ) )
//							{
//								if ( ! andSwOn )
//								{
//									y = 1 ;
//								}	
//							}
//							if ( --y == 0 )
//							{
//								Last_switch[i] &= ~1 ;
//							}
//							CsTimer[i] = y ;
//						}
//					}
//				}
//			}
		}
			
    processVoiceAlarms() ;
		
		// Now send serial data
		if ( serialSending )
		{
      if ( ++serialTimer > 1 )
			{
  			uint8_t serialCmd[28] = {0} ;
  			uint8_t *p = serialCmd ;
				uint32_t outputbitsavailable = 0 ;
				uint32_t outputbits = 0 ;
				
				serialTimer = 0 ;
				*p++ = 0x0F ;
				for ( i = 0 ; i < 16 ; i += 1 )
			 	{
					int16_t x = chanOut[i] ;
					x *= 4 ;
					x /= 5 ;
					x += 0x3E0 ;
					if ( x < 0 )
					{
						x = 0 ;
					}
					if ( x > 2047 )
					{
						x = 2047 ;
					}
					outputbits |= x << outputbitsavailable ;
					outputbitsavailable += 11 ;
					while ( outputbitsavailable >= 8 )
					{
            *p++ = outputbits ;
						outputbits >>= 8 ;
						outputbitsavailable -= 8 ;
					}
				}
				*p++ = 0 ;
				*p = 0 ;
//  			serialCmd[23] = 0 ;
//  			serialCmd[24] = 0 ;
	  		port->write( QByteArray::fromRawData ( ( char *)serialCmd, 25 ), 25 );

			}
			
//      static uint8_t first_last = 0 ;
//			if ( ++serialTimer > 2 )
//			{
//				serialTimer = 0 ;
//  			uint8_t serialCmd[24] = {0,0,0};
//  			uint8_t *p = serialCmd ;

//  			// Send current values to serial
//  			if ( port )
//  			{
//  			  if (port->isOpen())
//  			  {
//            for (int i=first_last; i<=first_last+7; i++)
//						{
//							int16_t x = chanOut[i] / 2 ;
//  				  	uint chval = x + 1500 ;
//  			  		*p++ = i; // Channel
//				    	*p++ = (chval >> 8) & 0xFF; // 2nd byte of value
//  				  	*p++ = chval & 0xFF; // 1st byte of value
//  			  	}
// 			  		port->write( QByteArray::fromRawData ( ( char *)serialCmd, 24 ), 24 );
//						if (ui->Send16chkB->isChecked() )
//						{
//            	if ( first_last )
//							{
//            	  first_last = 0 ;
//							}
//							else
//							{
//            	  first_last = 8 ;
//							}
//						}
//						else
//						{
//            	first_last = 0 ;
//						}
//  			  }
//  			}
//			}
		} 

}

void simulatorDialog::centerSticks()
{
    if(ui->leftStick->scene()) nodeLeft->stepToCenter();
    if(ui->rightStick->scene()) nodeRight->stepToCenter();
}

void simulatorDialog::configSwitches()
{
		if ( txBitType & ( RADIO_BITTYPE_X12 | RADIO_BITTYPE_X10 | RADIO_BITTYPE_T16 | RADIO_BITTYPE_TX16S | RADIO_BITTYPE_X10E | RADIO_BITTYPE_TX18S )  )
		{
		  ui->Sixslider->setMaximum( 5 ) ;
			ui->Sixwidget->show() ;
		}
		else
		{
			ui->Sixwidget->hide() ;
		}

		if ( txBitType & ( RADIO_BITTYPE_TARANIS | RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E | RADIO_BITTYPE_QX7 | RADIO_BITTYPE_XLITE | RADIO_BITTYPE_T12 | RADIO_BITTYPE_X9L | RADIO_BITTYPE_X12 | RADIO_BITTYPE_X10 | RADIO_BITTYPE_T16 | RADIO_BITTYPE_TX16S | RADIO_BITTYPE_X10E | RADIO_BITTYPE_TX18S )  )
		{
			ui->SAslider->setMaximum( 2 ) ;
			ui->SAwidget->show() ;
			ui->SBwidget->show() ;
			ui->labelSA->setText("SA") ;
			ui->labelSB->setText("SB") ;
			ui->labelSC->setText("SC") ;
			ui->labelSD->setText("SD") ;
			ui->labelSE->setText("SE") ;
			ui->labelSF->setText("SF") ;

			if ( txBitType & ( RADIO_BITTYPE_QX7 | RADIO_BITTYPE_XLITE | RADIO_BITTYPE_T12 | RADIO_BITTYPE_X9L ) )
			{
				ui->SEwidget->hide() ;
			}
			else
			{
				ui->SEwidget->show() ;
			}
			ui->SFwidget->show() ;
			ui->SFslider->setMaximum(1) ;
			ui->SCwidget->show() ;
			if ( txType == RADIO_TYPE_X9L )
			{
				ui->SDwidget->hide() ;
				ui->dialP_2->hide() ;
			}
			else
			{
				ui->SDwidget->show() ;
			}
			if ( txBitType & ( RADIO_BITTYPE_QX7 | RADIO_BITTYPE_XLITE | RADIO_BITTYPE_T12 | RADIO_BITTYPE_X9L ) )
			{
				ui->SGwidget->hide() ;
			}
			else
			{
				ui->SGwidget->show() ;
			}	 
//			ui->SHwidget->show() ;
			if ( txBitType & ( RADIO_BITTYPE_QX7 | RADIO_BITTYPE_XLITE | RADIO_BITTYPE_T12 | RADIO_BITTYPE_X9L ) )
			{
				ui->SliderL->hide() ;
				ui->SliderR->hide() ;
			}
			else
			{
				ui->SliderL->show() ;
				ui->SliderR->show() ;
			}
			if ( txType == RADIO_TYPE_TPLUS )
			{
				ui->dialP_3->show() ;
			}
			else
			{
				ui->dialP_3->hide() ;
			}
			ui->switchTRN->setText("SH") ;
			ui->switchTRN->setVisible( true ) ;
//			ui->SHwidget->hide() ;
			
//			ui->switchTHR->setVisible( false ) ;
//			ui->switchRUD->setVisible( false ) ;
//			ui->switchELE->setVisible( false ) ;
//			ui->switchAIL->setVisible( false ) ;
//			ui->switchGEA->setVisible( false ) ;
//			ui->switchID0->setVisible( false ) ;
//			ui->switchID1->setVisible( false ) ;
//			ui->switchID2->setVisible( false ) ;
			if ( g_eeGeneral.switchMapping & USE_PB1 )
			{
				ui->switchPB1->show() ;
			}
			else
			{
				ui->switchPB1->hide() ;
			}
			if ( g_eeGeneral.switchMapping & USE_PB2 )
			{
				ui->switchPB2->show() ;
			}
			else
			{
				ui->switchPB2->hide() ;
			}
			ui->switchPB3->hide() ;
			ui->switchPB4->hide() ;
		}
		else
		{
			ui->SAwidget->hide() ;
			ui->SBwidget->hide() ;
			ui->SEwidget->hide() ;
			ui->SFwidget->hide() ;
			ui->SCwidget->hide() ;
			ui->SDwidget->hide() ;
			ui->SGwidget->hide() ;
//			ui->SHwidget->hide() ;
			ui->switchTRN->setText("TRN") ;

      if ( g_eeGeneral.extraPotsSource[0] )
			{
				ui->SliderL->show() ;
			}
			else
			{
				ui->SliderL->hide() ;
			}
      if ( g_eeGeneral.extraPotsSource[1] )
			{
				ui->SliderR->show() ;
			}
			else
			{
				ui->SliderR->hide() ;
			}
			ui->dialP_3->show() ;
			if ( g_eeGeneral.switchMapping & USE_THR_3POS )
			{
				ui->SFwidget->show() ;
				ui->SFslider->setMaximum(2) ;
//				ui->switchTHR->hide() ;
				ui->labelSF->setText("THR") ;
			}
			else
			{
				ui->SFwidget->show() ;
				ui->SFslider->setMaximum(1) ;
				ui->labelSF->setText("THR") ;
//				ui->switchTHR->show() ;
			}
			if ( g_eeGeneral.switchMapping & USE_RUD_3POS )
			{
				ui->SEslider->setMaximum( 2 ) ;
				ui->SEwidget->show() ;
//				ui->switchRUD->hide() ;
				ui->labelSE->setText("RUD") ;
			}
			else
			{
				ui->SEslider->setMaximum( 1 ) ;
				ui->SEwidget->show() ;
//				ui->switchRUD->show() ;
				ui->labelSE->setText("RUD") ;
			}
			if ( g_eeGeneral.switchMapping & USE_ELE_3POS )
			{
				ui->SAslider->setMaximum( 2 ) ;
				ui->SAwidget->show() ;
//				ui->switchELE->hide() ;
				ui->labelSA->setText("ELE") ;
			}
			else if ( g_eeGeneral.switchMapping & USE_ELE_6POS )
			{
				ui->SAslider->setMaximum( 5 ) ;
				ui->SAwidget->show() ;
//				ui->switchELE->hide() ;
				ui->labelSA->setText("ELE") ;
			}
			else
			{
				ui->SAslider->setMaximum( 1 ) ;
				ui->SAwidget->show() ;
//				ui->switchELE->hide() ;
				ui->labelSA->setText("ELE") ;
//				ui->switchELE->show() ;
			}
			
      if ( g_eeGeneral.analogMapping & MASK_6POS )
			{
				ui->SAslider->setMaximum( 5 ) ;
				ui->SAwidget->show() ;
				ui->labelSA->setText("6P") ;
			}
			
			if ( g_eeGeneral.switchMapping & USE_AIL_3POS )
			{
				ui->SBslider->setMaximum( 2 ) ;
				ui->SBwidget->show() ;
//				ui->switchAIL->hide() ;
				ui->labelSB->setText("AIL") ;
			}
			else
			{
				ui->SBslider->setMaximum( 1 ) ;
				ui->SBwidget->show() ;
//				ui->switchAIL->hide() ;
				ui->labelSB->setText("AIL") ;
//				ui->switchAIL->show() ;
			}
			if ( g_eeGeneral.switchMapping & USE_GEA_3POS )
			{
				ui->SDslider->setMaximum( 2 ) ;
				ui->SDwidget->show() ;
//				ui->switchGEA->hide() ;
				ui->labelSD->setText("GEA") ;
			}
			else
			{
				ui->SDslider->setMaximum( 1 ) ;
				ui->SDwidget->show() ;
//				ui->switchGEA->hide() ;
				ui->labelSD->setText("GEA") ;
			}
			if ( g_eeGeneral.switchMapping & USE_PB1 )
			{
				ui->switchPB1->show() ;
			}
			else
			{
				ui->switchPB1->hide() ;
			}
			if ( g_eeGeneral.switchMapping & USE_PB2 )
			{
				ui->switchPB2->show() ;
			}
			else
			{
				ui->switchPB2->hide() ;
			}
			if ( g_eeGeneral.switchMapping & USE_PB3 )
			{
				ui->switchPB3->show() ;
			}
			else
			{
				ui->switchPB3->hide() ;
			}
			if ( g_eeGeneral.switchMapping & USE_PB4 )
			{
				ui->switchPB4->show() ;
			}
			else
			{
				ui->switchPB4->hide() ;
			}
			ui->switchTRN->setVisible( true ) ;
			ui->switchTRN->setText("TRN") ;
			
			ui->SCslider->setMaximum( 2 ) ;
			ui->SCwidget->show() ;
//				ui->switchAIL->hide() ;
			ui->labelSC->setText("IDx") ;
			
			
//			ui->switchID0->setVisible( true ) ;
//			ui->switchID1->setVisible( true ) ;
//			ui->switchID2->setVisible( true ) ;
		}
    createSwitchMapping( &g_eeGeneral, MAX_DRSWITCH, txType ) ;
}


void simulatorDialog::loadParams(const EEGeneral gg, const SKYModelData gm, struct t_radioData *radioData)
{
		if ( ticktimer )
		{
    	ticktimer->stop() ;
		}
	
    memcpy(&g_eeGeneral,&gg,sizeof(EEGeneral));
    memcpy(&g_model,&gm,sizeof(SKYModelData));
		txType = radioData->type ;
    txBitType = radioData->bitType ;

    char buf[sizeof(g_model.name)+1];
    memcpy(&buf,&g_model.name,sizeof(g_model.name));
    buf[sizeof(g_model.name)] = 0;
    modelName = tr("Simulating ") + QString(buf);
    setWindowTitle(modelName);

    if(g_eeGeneral.stickMode & 1)
    {
        nodeLeft->setCenteringY(false);   //mode 1,3 -> THR on left
        ui->holdLeftY->setChecked(true);
 		  nodeLeft->stepToBottom();   //mode 1,3 -> THR on left
    }
    else
    {
        nodeRight->setCenteringY(false);   //mode 1,3 -> THR on right
        ui->holdRightY->setChecked(true);
		  nodeRight->stepToBottom();   //mode 1,3 -> THR on right
    }

		CurrentPhase = getFlightPhase() ;

		trim[0] = g_model.trim[0] ;//(g_eeGeneral.stickMode>1)   ? 3 : 
		trim[1] = g_model.trim[1] ;//(g_eeGeneral.stickMode & 1) ? 2 : 
		trim[2] = g_model.trim[2] ;//(g_eeGeneral.stickMode & 1) ? 1 : 
		trim[3] = g_model.trim[3] ;//(g_eeGeneral.stickMode>1)   ? 0 : 
    ui->trimHLeft->setValue( getTrimValue( CurrentPhase, (g_eeGeneral.stickMode>1)   ? 3 : 0 ));  // mode=(0 || 1) -> rud trim else -> ail trim
    ui->trimVLeft->setValue( getTrimValue( CurrentPhase, (g_eeGeneral.stickMode & 1) ? 2 : 1 ));  // mode=(0 || 2) -> thr trim else -> ele trim
    ui->trimVRight->setValue(getTrimValue( CurrentPhase, (g_eeGeneral.stickMode & 1) ? 1 : 2 ));  // mode=(0 || 2) -> ele trim else -> thr trim
    ui->trimHRight->setValue(getTrimValue( CurrentPhase, (g_eeGeneral.stickMode>1)   ? 0 : 3 ));  // mode=(0 || 1) -> ail trim else -> rud trim

    beepVal = 0;
    beepShow = 0;
    bpanaCenter = 0;
    g_tmr10ms = 0 ;

		int i ;
    for ( i = 0 ; i < 2 ; i += 1 )
		{
			s_timer[i].s_sum = 0 ;
			s_timer[i].lastSwPos = 0 ;
			s_timer[i].sw_toggled = 0 ;
			s_timer[i].s_timeCumSw = 0 ;
			s_timer[i].s_timerState = 0 ;
			s_timer[i].lastResetSwPos= 0 ;
			s_timer[i].s_timeCumThr = 0 ;
			s_timer[i].s_timeCum16ThrP = 0 ;
			s_timer[i].s_timerVal = 0 ;
			s_timer[i].last_tmr = 0 ;
		}

    s_timeCumTot = 0;
    s_timeCumAbs = 0;
    s_timeCumSw = 0;
    s_timeCumThr = 0;
    s_timeCum16ThrP = 0;
    s_timerState = 0;
    beepAgain = 0;
    g_LightOffCounter = 0;
    s_timerVal[0] = 0;
    s_timerVal[1] = 0;
    s_time = 0;
    s_cnt = 0;
    s_sum = 0;
    sw_toggled = 0;

		GlobalModified = 0 ;

		configSwitches() ;
    setupTimer();
		if ( WelcomePlayed == 0 )
		{
			voiceDisplay( "WELCOME" ) ;
//			QSound::play( "C:/Progs/eepe/voice/WELCOME.wav" ) ;
			WelcomePlayed = 1 ;
		}
}


uint32_t simulatorDialog::getFlightPhase()
{
	uint32_t i ;
  for ( i = 0 ; i < MAX_PHASES+1 ; i += 1 )
	{
    PhaseData *phase ;
		phase = (i < MAX_MODES) ? &g_model.phaseData[i] : &g_model.xphaseData ;
    if ( phase->swtch )
		{
    	if ( getSwitch( phase->swtch, 0, 0 ) )
			{
    		if ( phase->swtch2 )
				{
					if ( getSwitch( phase->swtch2, 0, 0 ) )
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
    	if ( phase->swtch2 && getSwitch( phase->swtch2, 0, 0 ) )
			{
    	  return i + 1 ;
    	}
		}
  }
  return 0 ;
}

int16_t simulatorDialog::getRawTrimValue( uint8_t phase, uint8_t idx )
{
	if ( phase )
	{
	  PhaseData *p ;
		phase -= 1 ;
		p = (phase < MAX_MODES) ? &g_model.phaseData[phase] : &g_model.xphaseData ;
		return p->trim[idx] ;
	}	
	else
	{
//		return *trimptr[idx] ;
		return g_model.trim[idx] ;
	}
}

uint32_t simulatorDialog::getTrimFlightPhase( uint8_t phase, uint8_t idx )
{
  for ( uint32_t i=0 ; i<MAX_PHASES+1 ; i += 1 )
	{
    if (phase == 0) return 0;
    int16_t trim = getRawTrimValue( phase, idx ) ;
    if ( trim <= TRIM_EXTENDED_MAX )
		{
			return phase ;
		}
    uint32_t result = trim-TRIM_EXTENDED_MAX-1 ;
    if (result >= phase)
		{
			result += 1 ;
		}
    phase = result;
  }
  return 0;
}


int16_t simulatorDialog::getTrimValue( uint8_t phase, uint8_t idx )
{
  return getRawTrimValue( getTrimFlightPhase( phase, idx ), idx ) ;
}


void simulatorDialog::setTrimValue(uint8_t phase, uint8_t idx, int16_t trim)
{
	if ( phase )
	{
		phase = getTrimFlightPhase( phase, idx ) ;
	}
	if ( phase )
	{
    if(trim < -125 || trim > 125)
		{
			trim = ( trim > 0 ) ? 125 : -125 ;
		}	
	  PhaseData *p ;
		phase -= 1 ;
		p = (phase < MAX_MODES) ? &g_model.phaseData[phase] : &g_model.xphaseData ;
  	p->trim[idx] = trim ;
	}
	else
	{
    if(trim < -125 || trim > 125)
		{
			trim = ( trim > 0 ) ? 125 : -125 ;
		}	
//   	*trimptr[idx] = trim ;
		g_model.trim[idx] = trim ;
	}
}

uint32_t simulatorDialog::adjustMode( uint32_t x )
{
  if ( g_model.modelVersion >= 2 )
	{
		switch (g_eeGeneral.stickMode )
		{
			case 0 :
				if ( x == 2 )
				{
					x = 1 ;
				}
				else if ( x == 1 )
				{
					x = 2 ;
				}
			break ;
			case 3 :
				if ( x == 3 )
				{
					x = 0 ;
				}
				else if ( x == 0 )
				{
					x = 3 ;
				}
			break ;
			case 2 :
				x = 3 - x ;
			break ;
		}
		return x ;
	}
	
	if ( x == 2 )
	{
		x = 1 ;
	}
	else if ( x == 1 )
	{
		x = 2 ;
	}
	return x ;
}

const uint8_t stickScramble[] =
{
    0, 1, 2, 3,
    0, 2, 1, 3,
    3, 1, 2, 0,
    3, 2, 1, 0 } ;


void simulatorDialog::processAdjusters()
{
static uint8_t GvAdjLastSw[NUM_GVAR_ADJUST_SKY][2] ;
  for ( uint32_t i = 0 ; i < NUM_GVAR_ADJUST_SKY ; i += 1 )
	{
		GvarAdjust *pgvaradj ;
		pgvaradj = &g_model.gvarAdjuster[i] ;
		uint32_t idx = pgvaradj->gvarIndex ;
	
		int8_t sw0 = pgvaradj->swtch ;
		int8_t sw1 = 0 ;
		uint32_t switchedON = 0 ;
		int32_t value = g_model.gvars[idx].gvar ;
		if ( sw0 )
		{
			sw0 = getSwitch(sw0,0,0) ;
			if ( !GvAdjLastSw[i][0] && sw0 )
			{
    		switchedON = 1 ;
			}
			GvAdjLastSw[i][0] = sw0 ;
		}
		if ( pgvaradj->function > 3 )
		{
			sw1 = pgvaradj->switch_value ;
			if ( sw1 )
			{
				sw1 = getSwitch(sw1,0,0) ;
				if ( !GvAdjLastSw[i][1] && sw1 )
				{
    			switchedON |= 2 ;
				}
				GvAdjLastSw[i][1] = sw1 ;
			}
		}

		switch ( pgvaradj->function )
		{
			case 1 :	// Add
				if ( switchedON & 1 )
				{
     			value += pgvaradj->switch_value ;
				}
			break ;

			case 2 :
				if ( switchedON & 1 )
				{
     			value = pgvaradj->switch_value ;
				}
			break ;

			case 3 :
				if ( switchedON & 1 )
				{
					if ( pgvaradj->switch_value == 5 )	// REN
					{
						value = 0 ; // RotaryControl ; can't handle
					}
					else
					{
						value = getGvarSourceValue( pgvaradj->switch_value ) ;
					}
				}
			break ;
				
			case 4 :
				if ( switchedON & 1 )
				{
     			value += 1 ;
				}
				if ( switchedON & 2 )
				{
     			value -= 1 ;
				}
			break ;
			
			case 5 :
				if ( switchedON & 1 )
				{
     			value += 1 ;
				}
				if ( switchedON & 2 )
				{
     			value = 0 ;
				}
			break ;

			case 6 :
				if ( switchedON & 1 )
				{
     			value -= 1 ;
				}
				if ( switchedON & 2 )
				{
     			value = 0 ;
				}
			break ;
			
			case 7 :
				if ( switchedON & 1 )
				{
     			value += 1 ;
					if ( value > pgvaradj->switch_value )
					{
						value = pgvaradj->switch_value ;
					}
				}
			break ;
			
			case 8 :
				if ( switchedON & 1 )
				{
     			value -= 1 ;
					if ( value < pgvaradj->switch_value )
					{
						value = pgvaradj->switch_value ;
					}
				}
			break ;
		}
  	if(value > 125)
		{
			value = 125 ;
		}	
  	if(value < -125 )
		{
			value = -125 ;
		}	
		g_model.gvars[idx].gvar = value ;
	}
}

int8_t simulatorDialog::getGvarSourceValue( uint8_t src )
{
  int16_t value = 0 ;
	
	if ( src <= 4 )
	{
		uint32_t y ;
		y = src - 1 ;

//		y = adjustMode( y ) ;
		value = getTrimValue( CurrentPhase, y ) ;
				 
	}
	else if ( src == 5 )	// REN
	{
		value = 0 ;
	}
	else if ( src <= 9 )	// Stick
	{
    value = calibratedStick[CONVERT_MODE(src-5,g_model.modelVersion,g_eeGeneral.stickMode)-1] / 8 ;
	}
	else if ( src <= ( ((txType==1) || (txType == 2)) ? 13 : 12 ) )	// Pot
	{
		uint32_t y ;
    y = src - 6 ;

		y = adjustMode( y ) ;
				
		value = calibratedStick[ y ] / 8 ;
	}
	else if ( src <= ( ((txType==1) || (txType == 2)) ? 37 : 36 ) )	// Chans
	{
    value = ex_chans[src-( ((txType==1) || (txType == 2)) ? 14 : 13)] / 10 ;
	}
  else if ( src <= ( ((txType==1) || (txType == 2)) ? 45 : 44 ) )	// Scalers
	{
    value = calc_scaler( src - ( ((txType==1) || (txType == 2)) ? 38 : 37 ) ) ;
	}
  else// if ( src <= ( ((txType==1) || (txType == 2)) ? 45+24 : 44+24 ) )	// Scalers
	{ // Outputs
		int32_t x ;
    x = chanOut[src-( ((txType==1) || (txType == 2)) ? 46 : 45 )] ;
		x *= 100 ;
		value = x / 1024 ;
	}
	if ( value < -125 )
	{
		value = -125 ;					
	}
	if ( value > 125 )
	{
		value = 125 ;
	}
	return value ;
}

void simulatorDialog::getValues()
{
		int8_t trims[4] ;

  StickValues[0] = 1024*nodeLeft->getX(); //RUD
  StickValues[1] = -1024*nodeLeft->getY(); //ELE
  StickValues[2] = -1024*nodeRight->getY(); //THR
  StickValues[3] = 1024*nodeRight->getX(); //AIL
  if ( g_model.modelVersion >= 2 )
	{
		uint8_t stickIndex = g_eeGeneral.stickMode*4 ;
		
//    calibratedStick[stickScramble[stickIndex+0]] = 1024*nodeLeft->getX(); //RUD
//    calibratedStick[stickScramble[stickIndex+1]] = -1024*nodeLeft->getY(); //ELE
//    calibratedStick[stickScramble[stickIndex+2]] = -1024*nodeRight->getY(); //THR
//    calibratedStick[stickScramble[stickIndex+3]] = 1024*nodeRight->getX(); //AIL
    
		uint8_t index ;
		index =g_eeGeneral.crosstrim ? 3 : 0 ;
		index =  stickScramble[stickIndex+index] ;
		trims[index] = ui->trimHLeft->value();
		index =g_eeGeneral.crosstrim ? 2 : 1 ;
		index =  stickScramble[stickIndex+index] ;
		trims[index] = ui->trimVLeft->value();
		index =g_eeGeneral.crosstrim ? 1 : 2 ;
		index =  stickScramble[stickIndex+index] ;
		trims[index] = ui->trimVRight->value();
		index =g_eeGeneral.crosstrim ? 0 : 3 ;
		index =  stickScramble[stickIndex+index] ;
		trims[index] = ui->trimHRight->value();
	}
	else
	{
//    calibratedStick[0] = 1024*nodeLeft->getX(); //RUD
//    calibratedStick[1] = -1024*nodeLeft->getY(); //ELE
//    calibratedStick[2] = -1024*nodeRight->getY(); //THR
//    calibratedStick[3] = 1024*nodeRight->getX(); //AIL
    trims[g_eeGeneral.crosstrim ? 3 : 0] = ui->trimHLeft->value();
    trims[g_eeGeneral.crosstrim ? 2 : 1] = ui->trimVLeft->value();
    trims[g_eeGeneral.crosstrim ? 1 : 2] = ui->trimVRight->value();
    trims[g_eeGeneral.crosstrim ? 0 : 3] = ui->trimHRight->value();
	}
		uint32_t phase ;

		phase = getTrimFlightPhase( CurrentPhase, 0 ) ;
    setTrimValue( phase, 0, trims[0] ) ;
		phase = getTrimFlightPhase( CurrentPhase, 1 ) ;
    setTrimValue( phase, 1, trims[1] ) ;
		phase = getTrimFlightPhase( CurrentPhase, 2 ) ;
    setTrimValue( phase, 2, trims[2] ) ;
		phase = getTrimFlightPhase( CurrentPhase, 3 ) ;
    setTrimValue( phase, 3, trims[3] ) ;
    
    calibratedStick[4] = ui->dialP_1->value();
    calibratedStick[5] = ui->dialP_2->value();
		if ( ((txType==1) || (txType == 2) || (txType == RADIO_TYPE_X10)) )
		{
	    calibratedStick[6] = ui->SliderL->value();
    	calibratedStick[7] = ui->SliderR->value(); // For X9D
			if ( txType == 2 )
			{
    		calibratedStick[8] = ui->dialP_3->value();
			}
		}
		else
		{
    	calibratedStick[6] = ui->dialP_3->value();
		}

// May be for none X9D??
		if ( g_eeGeneral.extraPotsSource[0] )
		{
    	calibratedStick[7] = ui->SliderL->value(); // For X9D
		}

		if ( throttleReversed( &g_eeGeneral, &g_model ) )
    {
      StickValues[THR_STICK] *= -1;
      if( !g_model.thrTrim)
      {
        *trimptr[THR_STICK] *= -1;
      }
    }


	for( uint8_t i = 0 ; i < MAX_GVARS ; i += 1 )
	{
//		int x ;
		// ToDo, test for trim inputs here
		if ( g_model.gvars[i].gvsource )
		{
      int16_t value = 0 ;
			if ( g_model.gvswitch[i] )
			{
				if ( !getSwitch( g_model.gvswitch[i], 0, 0 ) )
				{
					continue ;
				}
			}
			
			uint8_t src = g_model.gvars[i].gvsource ;
			if ( src == 5 )	// REN
			{
			}
			else
			{
				value = getGvarSourceValue( src ) ;
			}
			if ( value > 125 )
			{
				value = 125 ;
			}
			if ( value < -125 )
			{
				value = -125 ;
			}
			g_model.gvars[i].gvar = value ;
		}
	}
	processAdjusters() ;

}

int simulatorDialog::chVal(int val)
{
	if ( current_limits == 1 )
	{
    return qMin(1280, qMax(-1280, val));
	}
	else
	{
    return qMin(1024, qMax(-1024, val));
	}
}

void simulatorDialog::setValues()
{
  if ( current_limits != g_model.extendedLimits )
	{
		int limit ;
    current_limits = g_model.extendedLimits ;
		if ( current_limits )
		{
      limit = 1280 ;
		}
		else
		{
      limit = 1024 ;
		}
    ui->chnout_1->setMinimum( - limit ) ;
    ui->chnout_1->setMaximum( limit ) ;
    ui->chnout_2->setMinimum( - limit ) ;
    ui->chnout_2->setMaximum( limit ) ;
    ui->chnout_3->setMinimum( - limit ) ;
    ui->chnout_3->setMaximum( limit ) ;
    ui->chnout_4->setMinimum( - limit ) ;
    ui->chnout_4->setMaximum( limit ) ;
    ui->chnout_5->setMinimum( - limit ) ;
    ui->chnout_5->setMaximum( limit ) ;
    ui->chnout_6->setMinimum( - limit ) ;
    ui->chnout_6->setMaximum( limit ) ;
    ui->chnout_7->setMinimum( - limit ) ;
    ui->chnout_7->setMaximum( limit ) ;
    ui->chnout_8->setMinimum( - limit ) ;
    ui->chnout_8->setMaximum( limit ) ;
    ui->chnout_9->setMinimum( - limit ) ;
    ui->chnout_9->setMaximum( limit ) ;
    ui->chnout_10->setMinimum( - limit ) ;
    ui->chnout_10->setMaximum( limit ) ;
    ui->chnout_11->setMinimum( - limit ) ;
    ui->chnout_11->setMaximum( limit ) ;
    ui->chnout_12->setMinimum( - limit ) ;
    ui->chnout_12->setMaximum( limit ) ;
    ui->chnout_13->setMinimum( - limit ) ;
    ui->chnout_13->setMaximum( limit ) ;
    ui->chnout_14->setMinimum( - limit ) ;
    ui->chnout_14->setMaximum( limit ) ;
    ui->chnout_15->setMinimum( - limit ) ;
    ui->chnout_15->setMaximum( limit ) ;
    ui->chnout_16->setMinimum( - limit ) ;
    ui->chnout_16->setMaximum( limit ) ;
		
	}
    ui->chnout_1->setValue(chVal(chanOut[0]));
    ui->chnout_2->setValue(chVal(chanOut[1]));
    ui->chnout_3->setValue(chVal(chanOut[2]));
    ui->chnout_4->setValue(chVal(chanOut[3]));
    ui->chnout_5->setValue(chVal(chanOut[4]));
    ui->chnout_6->setValue(chVal(chanOut[5]));
    ui->chnout_7->setValue(chVal(chanOut[6]));
    ui->chnout_8->setValue(chVal(chanOut[7]));
    ui->chnout_9->setValue(chVal(chanOut[8]));
    ui->chnout_10->setValue(chVal(chanOut[9]));
    ui->chnout_11->setValue(chVal(chanOut[10]));
    ui->chnout_12->setValue(chVal(chanOut[11]));
    ui->chnout_13->setValue(chVal(chanOut[12]));
    ui->chnout_14->setValue(chVal(chanOut[13]));
    ui->chnout_15->setValue(chVal(chanOut[14]));
    ui->chnout_16->setValue(chVal(chanOut[15]));

    ui->chnoutV_1->setText(QString("%1").arg((qreal)chanOut[0]*100/1024, 0, 'f', 1));
    ui->chnoutV_2->setText(QString("%1").arg((qreal)chanOut[1]*100/1024, 0, 'f', 1));
    ui->chnoutV_3->setText(QString("%1").arg((qreal)chanOut[2]*100/1024, 0, 'f', 1));
    ui->chnoutV_4->setText(QString("%1").arg((qreal)chanOut[3]*100/1024, 0, 'f', 1));
    ui->chnoutV_5->setText(QString("%1").arg((qreal)chanOut[4]*100/1024, 0, 'f', 1));
    ui->chnoutV_6->setText(QString("%1").arg((qreal)chanOut[5]*100/1024, 0, 'f', 1));
    ui->chnoutV_7->setText(QString("%1").arg((qreal)chanOut[6]*100/1024, 0, 'f', 1));
    ui->chnoutV_8->setText(QString("%1").arg((qreal)chanOut[7]*100/1024, 0, 'f', 1));
    ui->chnoutV_9->setText(QString("%1").arg((qreal)chanOut[8]*100/1024, 0, 'f', 1));
    ui->chnoutV_10->setText(QString("%1").arg((qreal)chanOut[9]*100/1024, 0, 'f', 1));
    ui->chnoutV_11->setText(QString("%1").arg((qreal)chanOut[10]*100/1024, 0, 'f', 1));
    ui->chnoutV_12->setText(QString("%1").arg((qreal)chanOut[11]*100/1024, 0, 'f', 1));
    ui->chnoutV_13->setText(QString("%1").arg((qreal)chanOut[12]*100/1024, 0, 'f', 1));
    ui->chnoutV_14->setText(QString("%1").arg((qreal)chanOut[13]*100/1024, 0, 'f', 1));
    ui->chnoutV_15->setText(QString("%1").arg((qreal)chanOut[14]*100/1024, 0, 'f', 1));
    ui->chnoutV_16->setText(QString("%1").arg((qreal)chanOut[15]*100/1024, 0, 'f', 1));

    ui->leftXPerc->setText(QString("X %1\%").arg((qreal)nodeLeft->getX()*100, 2, 'f', 0));
    ui->leftYPerc->setText(QString("Y %1\%").arg((qreal)nodeLeft->getY()*-100, 2, 'f', 0));

    ui->rightXPerc->setText(QString("X %1\%").arg((qreal)nodeRight->getX()*100, 2, 'f', 0));
    ui->rightYPerc->setText(QString("Y %1\%").arg((qreal)nodeRight->getY()*-100, 2, 'f', 0));

#define CSWITCH_ON  "QLabel { background-color: #4CC417 }"
#define CSWITCH_OFF "QLabel { }"

		int i = 0 ;
		if ( ((txType==1) || (txType == 2)) )
		{
			i = MAX_XDRSWITCH - MAX_DRSWITCH ;
		}

    ui->labelCSW_1->setStyleSheet(getSwitch(DSW_SW1+i,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_2->setStyleSheet(getSwitch(DSW_SW2+i,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_3->setStyleSheet(getSwitch(DSW_SW3,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_4->setStyleSheet(getSwitch(DSW_SW4+i,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_5->setStyleSheet(getSwitch(DSW_SW5+i,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_6->setStyleSheet(getSwitch(DSW_SW6+i,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_7->setStyleSheet(getSwitch(DSW_SW7+i,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_8->setStyleSheet(getSwitch(DSW_SW8+i,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_9->setStyleSheet(getSwitch(DSW_SW9+i,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_10->setStyleSheet(getSwitch(DSW_SWA+i,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_11->setStyleSheet(getSwitch(DSW_SWB+i,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_12->setStyleSheet(getSwitch(DSW_SWC+i,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_13->setStyleSheet(getSwitch(DSW_SWD+i,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_14->setStyleSheet(getSwitch(DSW_SWE+i,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_15->setStyleSheet(getSwitch(DSW_SWF+i,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_16->setStyleSheet(getSwitch(DSW_SWG+i,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_17->setStyleSheet(getSwitch(DSW_SWH+i,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_18->setStyleSheet(getSwitch(DSW_SWI+i,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_19->setStyleSheet(getSwitch(DSW_SWJ+i,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_20->setStyleSheet(getSwitch(DSW_SWK+i,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_21->setStyleSheet(getSwitch(DSW_SWL+i,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_22->setStyleSheet(getSwitch(DSW_SWM+i,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_23->setStyleSheet(getSwitch(DSW_SWN+i,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_24->setStyleSheet(getSwitch(DSW_SWO+i,0)  ? CSWITCH_ON : CSWITCH_OFF);

#define CBLUE "QSlider::handle:horizontal:disabled { background: #0000CC;border: 1px solid #aaa;border-radius: 4px; }"
#define CRED  "QSlider::handle:horizontal:disabled { background: #CC0000;border: 1px solid #aaa;border-radius: 4px; }"
#define CGREEN "QSlider::handle:horizontal:disabled { background: #00AA00;border: 1px solid #aaa;border-radius: 4px; }"
#define CCYAN "QSlider::handle:horizontal:disabled { background: #CCCC00;border: 1px solid #aaa;border-radius: 4px; }"

  const char *color[4] = { CBLUE, CRED, CGREEN, CCYAN } ;

	int onoff[16] ;
	int j ;
	int k ;

	j = g_model.startPPM2channel ;

	if ( j == 0)
	{
		uint8_t chans = g_model.ppmNCH + 4 ;
		chans *= 2 ;
		if ( chans > 16 )
		{
			chans -= 13 ;
		}
		j = g_model.startChannel + chans ;
	}
	else
	{
		j -= 1 ;
	}
	{
		uint8_t chans = g_model.ppm2NCH + 4 ;
		chans *= 2 ;
		if ( chans > 16 )
		{
			chans -= 13 ;
		}
		k = j + chans ;
	}
	for ( i = 0 ; i < 16 ; i += 1 )
	{
		onoff[i] = 0 ;
		if ( i >= g_model.startChannel )
		{
			uint8_t chans = g_model.ppmNCH + 4 ;
			chans *= 2 ;
			if ( chans > 16 )
			{
				chans -= 13 ;
			}
			if ( i < g_model.startChannel + chans )
			{
				onoff[i] |= 1 ;
			}
		}
		if ( i >= j )
		{
			if ( i < k )
			{
				onoff[i] |= 2 ;
			}
		}
	}
  ui->chnout_1->setStyleSheet( color[onoff[0]] ) ;
  ui->chnout_2->setStyleSheet( color[onoff[1]] ) ;
  ui->chnout_3->setStyleSheet( color[onoff[2]] ) ;
  ui->chnout_4->setStyleSheet( color[onoff[3]] ) ;
  ui->chnout_5->setStyleSheet( color[onoff[4]] ) ;
  ui->chnout_6->setStyleSheet( color[onoff[5]] ) ;
  ui->chnout_7->setStyleSheet( color[onoff[6]] ) ;
  ui->chnout_8->setStyleSheet( color[onoff[7]] ) ;
  ui->chnout_9->setStyleSheet( color[onoff[8]] ) ;
  ui->chnout_10->setStyleSheet( color[onoff[9]]  ) ;
  ui->chnout_11->setStyleSheet( color[onoff[10]] ) ;
  ui->chnout_12->setStyleSheet( color[onoff[11]] ) ;
  ui->chnout_13->setStyleSheet( color[onoff[12]] ) ;
  ui->chnout_14->setStyleSheet( color[onoff[13]] ) ;
  ui->chnout_15->setStyleSheet( color[onoff[14]] ) ;
  ui->chnout_16->setStyleSheet( color[onoff[14]] ) ;

	if ( gvar_or_scalers )
	{
		ui->Gvar1->setText( tr("%1").arg( calc_scaler(0) ) ) ;
		ui->Gvar2->setText( tr("%1").arg( calc_scaler(1) ) ) ;
		ui->Gvar3->setText( tr("%1").arg( calc_scaler(2) ) ) ;
		ui->Gvar4->setText( tr("%1").arg( calc_scaler(3) ) ) ;
		ui->Gvar5->setText( tr("%1").arg( calc_scaler(4) ) ) ;
		ui->Gvar6->setText( tr("%1").arg( calc_scaler(5) ) ) ;
		ui->Gvar7->setText( tr("%1").arg( calc_scaler(6) ) ) ;
	}
	else
	{
		ui->Gvar1->setText( tr("%1").arg(g_model.gvars[0].gvar) ) ;
		ui->Gvar2->setText( tr("%1").arg(g_model.gvars[1].gvar) ) ;
		ui->Gvar3->setText( tr("%1").arg(g_model.gvars[2].gvar) ) ;
		ui->Gvar4->setText( tr("%1").arg(g_model.gvars[3].gvar) ) ;
		ui->Gvar5->setText( tr("%1").arg(g_model.gvars[4].gvar) ) ;
		ui->Gvar6->setText( tr("%1").arg(g_model.gvars[5].gvar) ) ;
		ui->Gvar7->setText( tr("%1").arg(g_model.gvars[6].gvar) ) ;
	}	

	i = getFlightPhase() ;
	if ( i )
	{
	  PhaseData *p ;
		uint8_t phase ;
		phase = i - 1 ;
		p = (phase < MAX_MODES) ? &g_model.phaseData[phase] : &g_model.xphaseData ;
		if ( p->name[0] )
		{
			QString n = p->name ;
			while ( n.endsWith(" ") )
			{
				n = n.left(n.size()-1) ;			
			}
			if ( n.length() )
			{
				ui->FlightMode->setText( n ) ;
				i = -1 ;
			}
		}
	}
	if ( i >= 0 )
	{
		ui->FlightMode->setText( tr("FM%1").arg(i) ) ;
	}




}

void simulatorDialog::voiceDisplay( QString name )
{
  ui->label_beep->setText( name ) ;
	beepShow = 150 ;
}


void simulatorDialog::beepWarn1()
{
    beepVal = 1;
    beepShow = 20;
}

void simulatorDialog::beepWarn2()
{
    beepVal = 1;
    beepShow = 20;
}

void simulatorDialog::beepWarn()
{
    beepVal = 1;
    beepShow = 20;
}

void simulatorDialog::setupSticks()
{
    QGraphicsScene *leftScene = new QGraphicsScene(ui->leftStick);
    leftScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    ui->leftStick->setScene(leftScene);

    // ui->leftStick->scene()->addLine(0,10,20,30);

    QGraphicsScene *rightScene = new QGraphicsScene(ui->rightStick);
    rightScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    ui->rightStick->setScene(rightScene);

    // ui->rightStick->scene()->addLine(0,10,20,30);

    nodeLeft = new Node();
    nodeLeft->setPos(-GBALL_SIZE/2,-GBALL_SIZE/2);
    nodeLeft->setBallSize(GBALL_SIZE);
    leftScene->addItem(nodeLeft);

    nodeRight = new Node();
    nodeRight->setPos(-GBALL_SIZE/2,-GBALL_SIZE/2);
    nodeRight->setBallSize(GBALL_SIZE);
    rightScene->addItem(nodeRight);
}

void simulatorDialog::resizeEvent(QResizeEvent *event)
{

    if(ui->leftStick->scene())
    {
        QRect qr = ui->leftStick->contentsRect();
        qreal w  = (qreal)qr.width()  - GBALL_SIZE;
        qreal h  = (qreal)qr.height() - GBALL_SIZE;
        qreal cx = (qreal)qr.width()/2;
        qreal cy = (qreal)qr.height()/2;
        ui->leftStick->scene()->setSceneRect(-cx,-cy,w,h);

        QPointF p = nodeLeft->pos();
        p.setX(qMin(cx, qMax(p.x(), -cx)));
        p.setY(qMin(cy, qMax(p.y(), -cy)));
        nodeLeft->setPos(p);
    }

    if(ui->rightStick->scene())
    {
        QRect qr = ui->rightStick->contentsRect();
        qreal w  = (qreal)qr.width()  - GBALL_SIZE;
        qreal h  = (qreal)qr.height() - GBALL_SIZE;
        qreal cx = (qreal)qr.width()/2;
        qreal cy = (qreal)qr.height()/2;
        ui->rightStick->scene()->setSceneRect(-cx,-cy,w,h);

        QPointF p = nodeRight->pos();
        p.setX(qMin(cx, qMax(p.x(), -cx)));
        p.setY(qMin(cy, qMax(p.y(), -cy)));
        nodeRight->setPos(p);
    }
    QDialog::resizeEvent(event);
}


bool simulatorDialog::hwKeyState(int key)
{
	if ( txBitType & ( RADIO_BITTYPE_TARANIS | RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E | RADIO_BITTYPE_QX7 | RADIO_BITTYPE_XLITE | RADIO_BITTYPE_T12 | RADIO_BITTYPE_X9L | RADIO_BITTYPE_X12 | RADIO_BITTYPE_X10 | RADIO_BITTYPE_T16 | RADIO_BITTYPE_TX16S | RADIO_BITTYPE_X10E | RADIO_BITTYPE_TX18S )  )
  {
    switch (key)
    {
			case HSW_SA0 :	return ui->SAslider->value() == 0 ; break ;
			case HSW_SA1 : return ui->SAslider->value() == 1 ; break ;
			case HSW_SA2 : return ui->SAslider->value() == 2 ; break ;
			case HSW_SB0 : return ui->SBslider->value() == 0 ; break ;
			case HSW_SB1 : return ui->SBslider->value() == 1 ; break ;
			case HSW_SB2 : return ui->SBslider->value() == 2 ; break ;
			case HSW_SC0 : return ui->SCslider->value() == 0 ; break ;
			case HSW_SC1 : return ui->SCslider->value() == 1 ; break ;
			case HSW_SC2 : return ui->SCslider->value() == 2 ; break ;
			case HSW_SD0 : return ui->SDslider->value() == 0 ; break ;
			case HSW_SD1 : return ui->SDslider->value() == 1 ; break ;
			case HSW_SD2 : return ui->SDslider->value() == 2 ; break ;
			case HSW_SE0 : return ui->SEslider->value() == 0 ; break ;
			case HSW_SE1 : return ui->SEslider->value() == 1 ; break ;
			case HSW_SE2 : return ui->SEslider->value() == 2 ; break ;
//			case HSW_SF0 : return ui->SFslider->value() == 0 ; break ;
			case HSW_SF2 : return ui->SFslider->value() == 1 ; break ;
			case HSW_SG0 : return ui->SGslider->value() == 0 ; break ;
			case HSW_SG1 : return ui->SGslider->value() == 1 ; break ;
			case HSW_SG2 : return ui->SGslider->value() == 2 ; break ;
//			case HSW_SH0 : return ui->SHslider->value() == 0 ; break ;
//			case HSW_SH2 : return ui->SHslider->value() == 1 ; break ;
			case HSW_SH2 : return ui->switchTRN->isDown() ; break ;
			case HSW_Ele6pos0 :	return ui->Sixslider->value() == 0 ; break ;
			case HSW_Ele6pos1 :	return ui->Sixslider->value() == 1 ; break ;
			case HSW_Ele6pos2 :	return ui->Sixslider->value() == 2 ; break ;
			case HSW_Ele6pos3 :	return ui->Sixslider->value() == 3 ; break ;
			case HSW_Ele6pos4 :	return ui->Sixslider->value() == 4 ; break ;
			case HSW_Ele6pos5 :	return ui->Sixslider->value() == 5 ; break ;
			default:
        return keyState( (EnumKeys) key ) ;
      break;
		}
	}
	else
	{
    switch (key)
    {
    	case (HSW_ThrCt):   return ui->SFslider->value() == 1; break;
    	case (HSW_RuddDR):  return ui->SEslider->value() == 1; break;
    	case (HSW_ElevDR):  return ui->SAslider->value() == 1; break;
    	case (HSW_ID0):     return ui->SCslider->value() == 0; break;
    	case (HSW_ID1):     return ui->SCslider->value() == 1; break;
    	case (HSW_ID2):     return ui->SCslider->value() == 2; break;
    	case (HSW_AileDR):  return ui->SBslider->value() == 1; break;
    	case (HSW_Gear):    return ui->SDslider->value() == 1; break;
    	case (HSW_Trainer): return ui->switchTRN->isDown(); break;
			
			case HSW_Thr3pos0	:	return ui->SFslider->value() == 0 ; break ;
			case HSW_Thr3pos1	:	return ui->SFslider->value() == 1 ; break ;
			case HSW_Thr3pos2	:	return ui->SFslider->value() == 2 ; break ;
			case HSW_Rud3pos0	:	return ui->SEslider->value() == 0 ; break ;
			case HSW_Rud3pos1	:	return ui->SEslider->value() == 1 ; break ;
			case HSW_Rud3pos2	:	return ui->SEslider->value() == 2 ; break ;
			case HSW_Ele3pos0	:	return ui->SAslider->value() == 0 ; break ;
			case HSW_Ele3pos1	:	return ui->SAslider->value() == 1 ; break ;
			case HSW_Ele3pos2	:	return ui->SAslider->value() == 2 ; break ;
			case HSW_Ail3pos0	:	return ui->SBslider->value() == 0 ; break ;
			case HSW_Ail3pos1	:	return ui->SBslider->value() == 1 ; break ;
			case HSW_Ail3pos2	:	return ui->SBslider->value() == 2 ; break ;
			case HSW_Gear3pos0 :	return ui->SDslider->value() == 0 ; break ;
			case HSW_Gear3pos1 :	return ui->SDslider->value() == 1 ; break ;
			case HSW_Gear3pos2 :	return ui->SDslider->value() == 2 ; break ;
			case HSW_Ele6pos0 :	return ui->SAslider->value() == 0 ; break ;
			case HSW_Ele6pos1 :	return ui->SAslider->value() == 1 ; break ;
			case HSW_Ele6pos2 :	return ui->SAslider->value() == 2 ; break ;
			case HSW_Ele6pos3 :	return ui->SAslider->value() == 3 ; break ;
			case HSW_Ele6pos4 :	return ui->SAslider->value() == 4 ; break ;
			case HSW_Ele6pos5 :	return ui->SAslider->value() == 5 ; break ;
	    case HSW_Pb1	:	return ui->switchPB1->isDown() ; break ;
  	  case HSW_Pb2	:	return ui->switchPB2->isDown() ; break ;
    	default:
        return keyState( (EnumKeys) key ) ;
      break;
		}
	}
	
}

bool simulatorDialog::keyState(EnumKeys key)
{
  if ( ( txType == 0 ) || ( txType == 3 ) )
	{
    switch (key)
    {
    case (SW_ThrCt):   return ui->SFslider->value() == 1; break;
    case (SW_RuddDR):  return ui->SEslider->value() == 1; break;
    case (SW_ElevDR):  return ui->SAslider->value() == 1; break;
    case (SW_ID0):     return ui->SCslider->value() == 0; break;
    case (SW_ID1):     return ui->SCslider->value() == 1; break;
    case (SW_ID2):     return ui->SCslider->value() == 2; break;
    case (SW_AileDR):  return ui->SBslider->value() == 1; break;
    case (SW_Gear):    return ui->SDslider->value() == 1; break;
    case (SW_Trainer): return ui->switchTRN->isDown(); break;
    default:
        return false;
        break;
    }
	}
	else
	{
    switch (key)
		{
//			case SW_SA0 :	return ui->SAslider->value() == 0 ; break ;
//			case SW_SA1 : return ui->SAslider->value() == 1 ; break ;
//			case SW_SA2 : return ui->SAslider->value() == 2 ; break ;
//			case SW_SB0 : return ui->SBslider->value() == 0 ; break ;
//			case SW_SB1 : return ui->SBslider->value() == 1 ; break ;
//			case SW_SB2 : return ui->SBslider->value() == 2 ; break ;
			case SW_SC0 : return ui->SCslider->value() == 0 ; break ;
			case SW_SC1 : return ui->SCslider->value() == 1 ; break ;
			case SW_SC2 : return ui->SCslider->value() == 2 ; break ;
//			case SW_SD0 : return ui->SDslider->value() == 0 ; break ;
//			case SW_SD1 : return ui->SDslider->value() == 1 ; break ;
//			case SW_SD2 : return ui->SDslider->value() == 2 ; break ;
//			case SW_SE0 : return ui->SEslider->value() == 0 ; break ;
//			case SW_SE1 : return ui->SEslider->value() == 1 ; break ;
//			case SW_SE2 : return ui->SEslider->value() == 2 ; break ;
//			case SW_SF0 : return ui->SFslider->value() == 0 ; break ;
			case SW_SF2 : return ui->SFslider->value() == 1 ; break ;
//			case SW_SG0 : return ui->SGslider->value() == 0 ; break ;
//			case SW_SG1 : return ui->SGslider->value() == 1 ; break ;
//			case SW_SG2 : return ui->SGslider->value() == 2 ; break ;
//			case SW_SH0 : return ui->SHslider->value() == 0 ; break ;
//			case SW_SH2 : return ui->SHslider->value() == 1 ; break ;
			case SW_SH2 : return ui->switchTRN->isDown() ; break ;
    	default:
        return false;
      break;
		}
	}
}

qint16 simulatorDialog::getValue(qint8 i)
{
	uint8_t offset = ((txType==1) || (txType == 2)) ? 1 : 0 ;

    if(i<(PPM_BASE)) return calibratedStick[i];//-512..512
		else if(i >= EXTRA_POTS_START-1) return calibratedStick[i-EXTRA_POTS_START+8] ;
    else if(i<(CHOUT_BASE+offset)) return g_ppmIns[i-PPM_BASE];// - g_eeGeneral.ppmInCalib[i-PPM_BASE];
    else if(i<(CHOUT_BASE+NUM_SKYCHNOUT+offset)) return ex_chans[i-CHOUT_BASE];
		else
		{
			if ( i == 118 )
			{
				return getFlightPhase() ;
			}
      int j ;
			j = i-CHOUT_BASE-NUM_SKYCHNOUT - 25 ;
			if ( ( j >= 0 ) && ( j < 7 ) )
			{
        return g_model.gvars[j].gvar ;
			}
			if ( ( j >= 12 ) && ( j < 12+NUM_SCALERS ) )
			{
        return calc_scaler( j-12 ) ;
			}
			j = i-CHOUT_BASE-NUM_SKYCHNOUT - 4 ;
			if ( ( j == 0 ) || ( j == 1 ) )
			{
    		return s_timer[j].s_timerVal ;
			}
		}
    return 0;
}

int32_t simulatorDialog::isAgvar(uint8_t value)
{
	if ( value >= 70 )
	{
		if ( value <= 76 )
		{
			return 1 ;
		}
	}
	return 0 ;
}


#define SW_STACK_SIZE	6
int8_t SwitchStack[SW_STACK_SIZE] ;

bool simulatorDialog::getSwitchDr( int swtch )
{
	uint8_t aswitch = abs(swtch) ;
	if ( ( aswitch <= HSW_FM7 ) && ( aswitch >= HSW_FM0 ) )
	{
		aswitch -= HSW_FM0 ;
		aswitch = getFlightPhase() == aswitch ;
		return (swtch < 0) ? !aswitch : aswitch ;
	}
	else
	{
		return getSwitch( swtch, 0, 0 ) ;
	}
}

bool simulatorDialog::getSwitch(int swtch, bool nc, qint8 level)
{
  bool ret_value ;
  uint8_t cs_index ;
  uint8_t aswitch ;

	aswitch = abs(swtch) ;
 	SwitchStack[level] = aswitch ;

	int limit = ((txType==1) || (txType == 2) || (txType == 9) || (txType == 10)) ? MAX_XDRSWITCH : MAX_DRSWITCH ;
	cs_index = abs(swtch)-(limit-NUM_SKYCSW);

	{
		int32_t index ;
		for ( index = level - 1 ; index >= 0 ; index -= 1 )
		{
			if ( SwitchStack[index] == aswitch )
			{ // Recursion on this switch taking place
    		ret_value = Last_switch[cs_index] & 1 ;
		    return swtch>0 ? ret_value : !ret_value ;
			}
		}
	}
	if ( level > SW_STACK_SIZE - 1 )
  {
  	ret_value = Last_switch[cs_index] & 1 ;
  	return swtch>0 ? ret_value : !ret_value ;
  }

	if ( swtch == 0 )
	{
		return nc ;
	}
	if ( swtch == limit )
	{
		return true ;
	}
	if ( swtch == -limit )
	{
		return false ;
	}

	if ( ( txType == 0 ) || ( txType == 3 ) )
	{
		if ( abs(swtch) > MAX_DRSWITCH )
		{
			uint8_t value = hwKeyState( abs(swtch) ) ;
			if ( swtch > 0 )
			{
				return value ;
			}
			else
			{
				return ! value ;
			}
		}
	}
	else
	{
    if ( abs(swtch) > limit )
		{
			uint8_t value = hwKeyState( abs(swtch) ) ;
			if ( swtch > 0 )
			{
				return value ;
			}
			else
			{
				return ! value ;
			}
		}
	}

  uint8_t dir = swtch>0;
  if(abs(swtch)<(limit-NUM_SKYCSW))
	{
    if(!dir) return ! keyState((EnumKeys)(SW_BASE-swtch-1));
    return            keyState((EnumKeys)(SW_BASE+swtch-1));
  }

    //custom switch, Issue 78
    //use putsChnRaw
    //input -> 1..4 -> sticks,  5..8 pots
    //MAX,FULL - disregard
    //ppm
	ret_value = Now_switch[cs_index] & 1 ;
		
//		SKYCSwData &cs = g_model.customSw[cs_index];
//    if(!cs.func) return false;
		
//    int8_t a = cs.v1;
//    int8_t b = cs.v2;
//    int16_t x = 0;
//    int16_t y = 0;

//    // init values only if needed
//    uint8_t s = CS_STATE(cs.func, g_model.modelVersion);
    
//		if(s == CS_VOFS)
//    {
//        x = getValue(a-1);
//      if (cs.v1 > CHOUT_BASE+NUM_SKYCHNOUT)
//			{
//        uint8_t idx = cs.v1-CHOUT_BASE-NUM_SKYCHNOUT-1 ;
//        y = convertTelemConstant( idx, cs.v2, &g_model ) ;
////        y = convertTelemConstant( cs.v1-CHOUT_BASE-NUM_SKYCHNOUT-1, cs.v2 ) ;
////				y = b ;
//			}
//			else
//			{
//        y = calc100toRESX(b);
//			}
//    }
//    else if(s == CS_VCOMP)
//    {
//        x = getValue(a-1);
//        y = getValue(b-1);
//    }

//    switch (cs.func) {
//    case (CS_VPOS):
//        ret_value = (x>y);
//        break;
//    case (CS_VNEG):
//        ret_value = (x<y) ;
//        break;
//    case (CS_APOS):
//        ret_value = (abs(x)>y) ;
//        break;
//    case (CS_ANEG):
//        ret_value = (abs(x)<y) ;
//        break;
//    case (CS_VEQUAL):
//        ret_value = (x==y);
//        break;
//		case CS_EXEQUAL:
//			if ( isAgvar( a ) )
//			{
//				x *= 10 ;
//				y *= 10 ;
//			}
//    	ret_value = abs(x-y) < 32 ;
//	  break ;
//		case CS_VXEQUAL:
//			if ( isAgvar( a ) || isAgvar( b ) )
//			{
//				x *= 10 ;
//				y *= 10 ;
//			}
//    	ret_value = abs(x-y) < 32 ;
//	  break ;

//    case (CS_AND):
//        ret_value = (getSwitch(a,0,level+1) && getSwitch(b,0,level+1));
//        break;
//    case (CS_OR):
//        ret_value = (getSwitch(a,0,level+1) || getSwitch(b,0,level+1));
//        break;
//    case (CS_XOR):
//        ret_value = (getSwitch(a,0,level+1) ^ getSwitch(b,0,level+1));
//        break;
//  	case (CS_BIT_AND) :
//  	  x = getValue(a-1);
//			y = (uint8_t) cs.v2 ;
//      y |= cs.bitAndV3 << 8 ;
//  	  ret_value = ( x & y ) != 0 ;
//    break;

//    case (CS_EQUAL):
//        ret_value = (x==y);
//        break;
//    case (CS_NEQUAL):
//        ret_value = (x!=y);
//        break;
//    case (CS_GREATER):
//        ret_value = (x>y);
//        break;
//    case (CS_LESS):
//        ret_value = (x<y);
//        break;
//    		case (CS_EGREATER):	// CS_LATCH
//    		    if ( g_model.modelVersion < 3 )
//						{
//							ret_value = (x>=y);
//						}
//						else
//						{
//							ret_value = Last_switch[cs_index] & 1 ;
//						}
//    		    break;
//    		case (CS_ELESS):		// CS_FLIP
//    		    if ( g_model.modelVersion < 3 )
//						{
//    		    	ret_value = (x<=y);
//						}
//						else
//						{
//							ret_value = Last_switch[cs_index] & 1 ;
//						}
//    		    break;
//    case (CS_NTIME):
//        ret_value = CsTimer[cs_index] >= 0 ;
//    break ;
//    case (CS_TIME):
//		{	
//  	  ret_value = CsTimer[cs_index] >= 0 ;
//			int8_t x ;
//			if ( ( txType == 0 ) || ( txType == 3 ) )
//			{
//				x = getAndSwitch( cs, txType ) ;
//			}
//			else
//			{
//				x = cs.andsw ;
//			}
//			if ( x )
//			{
//			  if (getSwitch( x, 0, level+1) )
//				{
//          if ( ( Last_switch[cs_index] & 2 ) == 0 )
//					{ // Triggering
//						ret_value = 1 ;
//					}	
//				}
//			}
//		}
//    break ;
//  	case (CS_MONO):
//  	case (CS_RMONO):
//    	ret_value = CsTimer[cs_index] > 0 ;
//	  break ;
//    default:
//        return false;
//        break;
//    }
//		if ( ret_value )
//		{
//			if ( cs.andsw )
//			{
//				int8_t x ;
//				x = cs.andsw ;
//				if ( ( txType == 0 ) || ( txType == 3 ) )
//				{
//					if ( ( x > 8 ) && ( x <= 9+NUM_SKYCSW ) )
//					{
//						x += 1 ;
//					}
//					if ( ( x < -8 ) && ( x >= -(9+NUM_SKYCSW) ) )
//					{
//						x -= 1 ;
//					}
//					if ( x == 9+NUM_SKYCSW+1 )
//					{
//						x = 9 ;			// Tag TRN on the end, keep EEPROM values
//					}
//					if ( x == -(9+NUM_SKYCSW+1) )
//					{
//						x = -9 ;			// Tag TRN on the end, keep EEPROM values
//					}
//				}
//        ret_value = getSwitch( x, 0, level+1) ;
//			}
//		}
//    if ( g_model.modelVersion >= 3 )
//		{
//      if ( cs.func < CS_LATCH )
//			{
//				Last_switch[cs_index] = ret_value ;
//			}
//		}
//		else
//		{
//			Last_switch[cs_index] = ret_value ;
//		}
	return swtch>0 ? ret_value : !ret_value ;
}


uint16_t expou(uint16_t x, uint16_t k)
{
    // k*x*x*x + (1-k)*x
    return ((unsigned long)x*x*x/0x10000*k/(RESXul*RESXul/0x10000) + (RESKul-k)*x+RESKul/2)/RESKul;
}
// expo-funktion:
// ---------------
// kmplot
// f(x,k)=exp(ln(x)*k/10) ;P[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20]
// f(x,k)=x*x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
// f(x,k)=x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
// f(x,k)=1+(x-1)*(x-1)*(x-1)*k/10 + (x-1)*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]

int16_t expo(int16_t x, int16_t k)
{
    if(k == 0) return x;
    int16_t   y;
    bool    neg =  x < 0;
    if(neg)   x = -x;
    if(k<0){
        y = RESXu-expou(RESXu-x,-k);
    }else{
        y = expou(x,k);
    }
    return neg? -y:y;
}

uint16_t isqrt32(uint32_t n)
{
    uint16_t c = 0x8000;
    uint16_t g = 0x8000;

    for(;;) {
        if((uint32_t)g*g > n)
            g ^= c;
        c >>= 1;
        if(c == 0)
            return g;
        g |= c;
    }
}

int16_t simulatorDialog::intpol(int16_t x, uint8_t idx) // -100, -75, -50, -25, 0 ,25 ,50, 75, 100
{
#define D9 (RESX * 2 / 8)
#define D5 (RESX * 2 / 4)
#define D6 (RESX * 2 / 5)
    uint32_t    cv9 = idx >= MAX_CURVE5;
		int8_t *crv ;
		if ( idx == MAX_CURVE5 + MAX_CURVE9 )
		{ // The xy curve
			crv = g_model.curvexy ;
			cv9 = 2 ;
		}
		else if ( idx == MAX_CURVE5 + MAX_CURVE9 + 1)
		{ // The xy curve
			crv = g_model.curve2xy ;
			cv9 = 2 ;
		}
		else if ( idx == MAX_CURVE5 + MAX_CURVE9 + 2 )
		{
			crv = g_model.curve6 ;
			cv9 = 3 ;
		}
		else
		{
    	crv = cv9 ? g_model.curves9[idx-MAX_CURVE5] : g_model.curves5[idx];
		}
    int16_t erg;

    x+=RESXu;
    if(x < 0)
		{
        erg = (int16_t)crv[0] * (RESX/4);
    }
		else if(x >= (RESX*2))
		{
			if ( cv9 == 3 )
			{
      	erg = (int16_t)crv[5] * (RESX/4);
			}
			else
			{
        erg = (int16_t)crv[(cv9 ? 8 : 4)] * (RESX/4);
			}
    }
		else
		{
			int16_t deltax ;
			div_t qr ;
			if ( cv9 == 2 ) // xy curve
			{
		    int16_t a = 0 ;
				int16_t b ;
				int16_t c ;
				uint32_t i ;

				// handle end points
  	    c = RESX + calc100toRESX(crv[17]) ;
				if ((uint16_t)x>c)
				{
					return calc100toRESX(crv[8]) ;
				}
  	    b = RESX + calc100toRESX(crv[9]) ;
				if ((uint16_t)x<b)
				{
					return calc100toRESX(crv[0]) ;
				}

				for ( i = 0 ; i < 8 ; i += 1 )
				{
	        a = b ;
  	      b = (i==7 ? c : RESX + calc100toRESX(crv[i+10]));
    	    if ((uint16_t)x<=b) break;
				}
				qr.quot = i ;
				qr.rem = x - a ;
				deltax = b - a ;
			}
			else
			{
        if(cv9 == 3)
				{
					qr = div( x, D6 ) ;
					deltax = D6 ;
				}
				else if ( cv9 )
				{
					qr = div( x, D9 ) ;
					deltax = D9 ;
        }
				else
				{
					qr = div( x, D5 ) ;
					deltax = D5 ;
        }
			}
			int32_t y1 = (int16_t)crv[qr.quot] * (RESX/4) ;
			int32_t deltay = (int16_t)crv[qr.quot+1] * (RESX/4) - y1 ;
			erg = y1 + ( qr.rem ) * deltay / deltax ;
    }
    return erg / 25; // 100*D5/RESX;
}

void simulatorDialog::resetTimern( uint32_t timer )
{
  struct t_timer *tptr = &s_timer[timer] ;
	tptr->s_timerState = TMR_OFF; //is changed to RUNNING dep from mode
  tptr->s_timeCumThr=0;
  tptr->s_timeCumSw=0;
  tptr->s_timeCum16ThrP=0;
	tptr->s_sum = 0 ;
	tptr->last_tmr = g_model.timer[timer].tmrVal ;
	tptr->s_timerVal = ( g_model.timer[timer].tmrDir ) ? 0 : tptr->last_tmr ;
}

void simulatorDialog::resetTimer1()
{
  s_timeCumAbs=0;
	resetTimern( 0 ) ;
}

void simulatorDialog::resetTimer2()
{
	resetTimern( 1 ) ;
}

void simulatorDialog::resetTimer()
{
	resetTimer1() ;
	resetTimer2() ;
}


void simulatorDialog::timerTick()
{
	uint8_t timer ;
	int8_t tma ;
  int16_t tmb ;
  uint16_t tv ;
    
		int16_t val = 0;
//    if((abs(g_model.timer[0].tmrModeA)>1) && (abs(g_model.timer[0].tmrModeA)<TMR_VAROFS)) {
//        val = calibratedStick[CONVERT_MODE(abs(g_model.timer[0].tmrModeA)/2,g_model.modelVersion,g_eeGeneral.stickMode)-1];
//        val = (g_model.timer[0].tmrModeA<0 ? RESX-val : val+RESX ) / (RESX/16);  // only used for %
//    }

  s_cnt++;			// Number of times val added in

	int hsw_max = ((txType==1) || (txType == 2) || (txType == 9)) ? HSW_MAX_X9D : HSW_MAX ;
	for( timer = 0 ; timer < 2 ; timer += 1 )
	{
		struct t_timer *ptimer = &s_timer[timer] ;
		uint8_t resetting = 0 ;
		if ( timer == 0 )
		{
			tmb = g_model.timer1RstSw ;
		}
		else
		{
			tmb = g_model.timer2RstSw ;
		}
		if ( tmb )
		{
			if ( tmb < -hsw_max )
			{
				tmb += 256 ;
			}

    	if(tmb>=(hsw_max))	 // toggeled switch
			{
    	  uint8_t swPos = getSwitch( tmb-(hsw_max), 0 ) ;
				if ( swPos != ptimer->lastResetSwPos )
				{
					ptimer->lastResetSwPos = swPos ;
					if ( swPos )	// Now on
					{
						resetting = 1 ;
					}
				}
			}
			else
			{
				if ( getSwitch( tmb, 0 ) )
				{
					resetting = 1 ;
				}
			}
		}
		if ( resetting )
		{
			if ( timer == 0 )
			{
				resetTimer1() ;
			}
			else
			{
				resetTimer2() ;
			}
		}
	
		tma = g_model.timer[timer].tmrModeA ;
    tmb = g_model.timer[timer].tmrModeB ;
		if ( tmb < -hsw_max )
		{
			tmb += 256 ;
		}

// code for cx%
//		val = throttle_val ;
//    val = calibratedStick[CONVERT_MODE(abs(g_model.timer[0].tmrModeA)/2,g_model.modelVersion,g_eeGeneral.stickMode)-1];
  	val = calibratedStick[3-1];
   	if(tma>=TMR_VAROFS) // Cxx%
		{
 	    val = chanOut[tma-TMR_VAROFS] ;
		}		

		val = ( val + RESX ) / (RESX/16) ;

		if ( tma != TMRMODE_NONE )		// Timer is not off
		{ // We have a triggerA so timer is running 
    	if(tmb>=(hsw_max))	 // toggeled switch
			{
    	  if(!(ptimer->sw_toggled | ptimer->s_sum | s_cnt | s_time | ptimer->lastSwPos)) ptimer->lastSwPos = 0 ;  // if initializing then init the lastSwPos
        uint8_t swPos = getSwitch( tmb-(hsw_max), 0 ) ;
    	  if(swPos && !ptimer->lastSwPos)  ptimer->sw_toggled = !ptimer->sw_toggled;  //if switch is flipped first time -> change counter state
    	  ptimer->lastSwPos = swPos;
    	}
    	else
			{
				if ( tmb )
				{
          ptimer->sw_toggled = getSwitch( tmb, 0 ); //normal switch
				}
				else
				{
					ptimer->sw_toggled = 1 ;	// No trigger B so use as active
				}
			}
		}

		if ( ( ptimer->sw_toggled == 0 ) || resetting )
		{
			val = 0 ;
		}

    ptimer->s_sum += val ;   // Add val in
    if( ( (uint16_t)( g_tmr10ms-s_time) ) < 100 )		// BEWARE of 32 bit processor extending 16 bit values
		{
			if ( timer == 0 )
			{
				continue ; //1 sec
			}
			else
			{
				return ;
			}
		}
    val     = ptimer->s_sum/s_cnt;   // Average of val over last 100mS
    ptimer->s_sum  -= val*s_cnt;     //rest (remainder not added in)

		if ( timer == 0 )
		{
    	s_timeCumTot += 1;
	    s_timeCumAbs += 1;
			g_eeGeneral.totalElapsedTime += 1 ;
		}
		else
		{
	    s_cnt   = 0;    // ready for next 100mS
			s_time += 100;  // 100*10mS passed
		}
    if(val) ptimer->s_timeCumThr       += 1;
		if ( !resetting )
		{
    	if(ptimer->sw_toggled) ptimer->s_timeCumSw += 1;
		}
    ptimer->s_timeCum16ThrP            += val>>1;	// val/2

    tv = ptimer->s_timerVal = g_model.timer[timer].tmrVal ;
    if(tma == TMRMODE_NONE)
		{
			ptimer->s_timerState = TMR_OFF;
		}
    else
		{
			if ( tma==TMRMODE_ABS )
			{
				if ( tmb == 0 ) ptimer->s_timerVal -= s_timeCumAbs ;
	    	else ptimer->s_timerVal -= ptimer->s_timeCumSw ; //switch
			}
	    else if(tma<TMR_VAROFS-1) ptimer->s_timerVal -= ptimer->s_timeCumThr;	// stick
		  else ptimer->s_timerVal -= ptimer->s_timeCum16ThrP/16 ; // stick% or Cx%
		}   
    
		switch(ptimer->s_timerState)
    {
    case TMR_OFF:
        if(tma != TMRMODE_NONE) ptimer->s_timerState=TMR_RUNNING;
        break;
    case TMR_RUNNING:
        if(ptimer->s_timerVal<0 && tv) ptimer->s_timerState=TMR_BEEPING;
        break;
    case TMR_BEEPING:
        if(ptimer->s_timerVal <= -MAX_ALERT_TIME)   ptimer->s_timerState=TMR_STOPPED;
        if(tv == 0)       ptimer->s_timerState=TMR_RUNNING;
        break;
    case TMR_STOPPED:
        break;
    }

  	  if(ptimer->last_tmr != ptimer->s_timerVal)  //beep only if seconds advance
    	{
    		ptimer->last_tmr = ptimer->s_timerVal;
        if(ptimer->s_timerState==TMR_RUNNING)
        {
					uint8_t audioControl ;
					if ( timer == 0 )
					{
						audioControl = g_eeGeneral.preBeep | g_model.timer1Cdown ;
					}
					else
					{
						audioControl = g_model.timer2Cdown ;
					}
            if(audioControl && g_model.timer[timer].tmrVal) // beep when 30, 15, 10, 5,4,3,2,1 seconds remaining
            {
              	if(ptimer->s_timerVal==30) {beepAgain=2; beepWarn2();} //beep three times
              	if(ptimer->s_timerVal==20) {beepAgain=1; beepWarn2();} //beep two times
                if(ptimer->s_timerVal==10)  beepWarn2();
                if(ptimer->s_timerVal<= 5)
								{
									if(ptimer->s_timerVal>= 0)
									{
										beepWarn2();
//										audioVoiceDefevent(AU_TIMER_LT3, ptimer->s_timerVal) ;
									}
									else
									{
										if ( ( timer == 0 ) && g_eeGeneral.preBeep )
										{
											beepWarn2();
//											audioDefevent(AU_TIMER_LT3);
										}
									}
								}
								if(g_eeGeneral.flashBeep && (ptimer->s_timerVal==30 || ptimer->s_timerVal==20 || ptimer->s_timerVal==10 || ptimer->s_timerVal<=3))
                    g_LightOffCounter = FLASH_DURATION;
            }
						div_t mins ;
						mins = div( g_model.timer[timer].tmrDir ? g_model.timer[timer].tmrVal- ptimer->s_timerVal : ptimer->s_timerVal, 60 ) ;
					if ( timer == 0 )
					{
						audioControl = g_eeGeneral.minuteBeep | g_model.timer1Mbeep ;
					}
					else
					{
						audioControl = g_model.timer2Mbeep ;

					}
            if( audioControl && ((mins.rem)==0)) //short beep every minute
            {
//								if ( g_eeGeneral.speakerMode & 2 )
//								{
                beepWarn2();
//									if ( mins.quot ) {voice_numeric( mins.quot, 0, V_MINUTES ) ;}
//								}
//								else
//								{
//                	audioDefevent(AU_WARNING1);
//								}
                if(g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
            }
        }
        else if(ptimer->s_timerState==TMR_BEEPING)
        {
					if ( ( timer == 0 ) && g_eeGeneral.preBeep )
					{
            beepWarn();
//            audioDefevent(AU_TIMER_LT3);
            if(g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
					}
        }
    	}
    
		if( g_model.timer[timer].tmrDir) ptimer->s_timerVal = tv-ptimer->s_timerVal; //if counting backwards - display backwards
	}
}

// GVARS helpers

int8_t simulatorDialog::REG100_100(int8_t x)
{
	return REG( x, -100, 100 ) ;
}

int8_t simulatorDialog::REG(int8_t x, int8_t min, int8_t max)
{
  int8_t result = x;
  if (x >= 126 || x <= -126) {
    x = (uint8_t)x - 126;
    result = g_model.gvars[x].gvar ;
    if (result < min) {
      g_model.gvars[x].gvar = result = min;
//      eeDirty( EE_MODEL | EE_TRIM ) ;
    }
    if (result > max) {
      g_model.gvars[x].gvar = result = max;
//      eeDirty( EE_MODEL | EE_TRIM ) ;
    }
  }
  return result;
}

uint8_t simulatorDialog::REGisGvar( int8_t x )
{
  return (x >= 126 || x <= -126) ;	
}

PhaseData *simulatorDialog::getPhaseAddress( uint32_t phase )
{
	return (phase < MAX_MODES) ? &g_model.phaseData[phase] : &g_model.xphaseData ;
}

void simulatorDialog::perOutPhase( bool init, uint8_t att ) 
{
	static uint8_t lastPhase = 0 ;
	uint8_t thisPhase ;
	thisPhase = getFlightPhase() ;
	if ( thisPhase != lastPhase )
	{
		uint8_t time1 = 0 ;
		uint8_t time2 ;
		
		if ( lastPhase )
		{
      time1 = getPhaseAddress(lastPhase-1)->fadeOut ;
		}
		if ( thisPhase )
		{
      time2= getPhaseAddress(thisPhase-1)->fadeIn ;
			if ( time2 > time1 )
			{
        time1 = time2 ;
			}
		}
		if ( time1 )
		{
			fadeRate = ( 25600 / 50 ) / time1 ;
    	fadePhases |= ( 1 << lastPhase ) | ( 1 << thisPhase ) ;
		}
		lastPhase = thisPhase ;
	}
	att |= FADE_FIRST ;
	if ( fadePhases )
	{
		fadeWeight = 0 ;
		uint8_t fadeMask = 1 ;
    for (uint8_t p=0; p<MAX_PHASES+1; p++)
		{
			if ( fadePhases & fadeMask )
			{
				if ( p != thisPhase )
				{
					CurrentPhase = p ;
					fadeWeight += fadeScale[p] ;
					perOut( false, att ) ;
					att &= ~FADE_FIRST ;				
				}
			}
			fadeMask <<= 1 ;
		}	
	}
	else
	{
		fadeScale[thisPhase] = 25600 ;
	}
	fadeWeight += fadeScale[thisPhase] ;
	CurrentPhase = thisPhase ;
	perOut( false, att | FADE_LAST ) ;

	if ( fadePhases )
	{
		uint8_t fadeMask = 1 ;
    for (uint8_t p=0; p<MAX_PHASES+1; p+=1)
		{
			quint16	l_fadeScale = fadeScale[p] ;
			
			if ( fadePhases & fadeMask )
			{
				if ( p != thisPhase )
				{
          if ( l_fadeScale > fadeRate )
					{
						l_fadeScale -= fadeRate ;
					}
					else
					{
						l_fadeScale = 0 ;
						fadePhases &= ~fadeMask ;						
					}
				}
				else
				{
          if ( 25600 - l_fadeScale > fadeRate)
					{
						l_fadeScale += fadeRate ;
					}
					else
					{
						l_fadeScale = 25600 ;
						fadePhases &= ~fadeMask ;						
					}
				}
			}
			else
			{
				l_fadeScale = 0 ;
			}
			fadeScale[p] = l_fadeScale ;
			fadeMask <<= 1 ;
		}
	}
}

void simulatorDialog::perOut(bool init, uint8_t att)
{
    int16_t trimA[4];
    uint8_t  anaCenter = 0;
//    uint16_t d = 0;

//		CurrentPhase = getFlightPhase() ;

    uint8_t ele_stick, ail_stick ;
	  if ( g_model.modelVersion >= 2 )
		{
			ele_stick = 1 ; //ELE_STICK ;
  	  ail_stick = 3 ; //AIL_STICK ;
		}
		else
		{
			ele_stick = ELE_STICK ;
  	  ail_stick = AIL_STICK ;
		}
    //===========Swash Ring================
//    if(g_model.swashRingValue)
//    {
//        uint32_t v = (calibratedStick[ELE_STICK]*calibratedStick[ELE_STICK] +
//                      calibratedStick[AIL_STICK]*calibratedStick[AIL_STICK]);
//        uint32_t q = RESX*g_model.swashRingValue/100;
//        q *= q;
//        if(v>q)
//            d = isqrt32(v);
//    }
    //===========Swash Ring================


		uint8_t num_analog = 7 ;
		if ( ((txType==1) || (txType == 2) || (txType == 9)) )
		{
			num_analog = 8 ;
			if ( txType == 2 )
			{
				num_analog = 9 ;
			}
		}
    for(uint8_t i=0;i<num_analog;i++)
		{        // calc Sticks

        //Normalization  [0..2048] ->   [-1024..1024]
      int16_t v ;
			uint8_t index = i ;

			if ( i < 4 )
			{
        v = StickValues[i];
			  if ( g_model.modelVersion >= 2 )
				{
					uint8_t stickIndex = g_eeGeneral.stickMode*4 ;
					index = stickScramble[stickIndex+i] ;
				}
        calibratedStick[index] = v; //for show in expo
			}
			else
			{
        v = calibratedStick[i] ;
			}
        //    v -= g_eeGeneral.calibMid[i];
        //    v  =  v * (int32_t)RESX /  (max((int16_t)100,(v>0 ?
        //                                     g_eeGeneral.calibSpanPos[i] :
        //                                     g_eeGeneral.calibSpanNeg[i])));
        //    if(v <= -RESX) v = -RESX;
        //    if(v >=  RESX) v =  RESX;
        //    calibratedStick[i] = v; //for show in expo

        if(!(v/16)) anaCenter |= 1<<(CONVERT_MODE((i+1),g_model.modelVersion,g_eeGeneral.stickMode)-1);

        //===========Swash Ring================
//        if(d && (index==ele_stick || index==ail_stick))
//            v = (int32_t)v*g_model.swashRingValue*RESX/(d*100);
        //===========Swash Ring================


//				if ( g_model.modelVersion >= 2 )
//				{
//					if ( i < 4 )
//					{
//						uint8_t stickIndex = g_eeGeneral.stickMode*4 ;
//  	        index = stickScramble[stickIndex+i] ;
//					}
//				}
        if(i<4)
				{ //only do this for sticks
        	rawSticks[index] = v ; //set values for mixer
            uint8_t expoDrOn = GET_DR_STATE(index);
            uint8_t stkDir = v>0 ? DR_RIGHT : DR_LEFT;

            if(IS_THROTTLE(index) && g_model.thrExpo){
#if GVARS
                v  = 2*expo((v+RESX)/2,REG100_100(g_model.expoData[index].expo[expoDrOn][DR_EXPO][DR_RIGHT]));
#else
                v  = 2*expo((v+RESX)/2,g_model.expoData[index].expo[expoDrOn][DR_EXPO][DR_RIGHT]);
#endif                    
                stkDir = DR_RIGHT;
            }
            else
#if GVARS
                v  = expo(v,REG100_100(g_model.expoData[index].expo[expoDrOn][DR_EXPO][stkDir]));
#else
                v  = expo(v,g_model.expoData[index].expo[expoDrOn][DR_EXPO][stkDir]);
#endif                    

#if GVARS
            int32_t x = (int32_t)v * (REG(g_model.expoData[index].expo[expoDrOn][DR_WEIGHT][stkDir]+100, 0, 100))/100;
#else
            int32_t x = (int32_t)v * (g_model.expoData[index].expo[expoDrOn][DR_WEIGHT][stkDir]+100)/100;
#endif                    
            v = (int16_t)x;
            if (IS_THROTTLE(index) && g_model.thrExpo) v -= RESX;

				  if ( g_model.modelVersion >= 2 )
					{
          	trimA[i] = getTrimValue( CurrentPhase, i )*2 ;
					}	
					else
					{
            //do trim -> throttle trim if applicable
            int32_t vv = 2*RESX;
            if(IS_THROTTLE(i) && g_model.thrTrim)
						{
							int8_t ttrim ;
							ttrim = getTrimValue( CurrentPhase, i ) ;
//							ttrim = *trimptr[i] ;
							if(g_eeGeneral.throttleReversed)
							{
								ttrim = -ttrim ;
							}
							vv = ((int32_t)ttrim+125)*(RESX-v)/(2*RESX);
						}

            //trim
            trimA[i] = (vv==2*RESX) ? getTrimValue( CurrentPhase, i )*2 : (int16_t)vv*2; //    if throttle trim -> trim low end
//            trimA[i] = (vv==2*RESX) ? *trimptr[i]*2 : (int16_t)vv*2; //    if throttle trim -> trim low end
					}
				}
				if ( att & FADE_FIRST )
				{
					if ( g_model.modelVersion >= 2 )
					{
       			anas[index] = v; //set values for mixer
					}
					else
					{
       			anas[i] = v; //set values for mixer
					}
				}
    }
	  if ( g_model.modelVersion >= 2 )
		{
      if(g_model.thrTrim)
			{
				int8_t ttrim ;
				ttrim = getTrimValue( CurrentPhase, 2 ) ;
				if(g_eeGeneral.throttleReversed)
				{
					ttrim = -ttrim ;
				}
       	trimA[2] = ((int32_t)ttrim+125)*(RESX-anas[2])/(RESX) ;
			}
		}

  uint8_t Mix_3pos ;
  uint8_t Mix_max ;
  uint8_t Mix_full ;
  uint8_t Chout_base ;
//  if ( ((txType==1) || (txType == 2)) )
//  {
//    Mix_3pos = MIX_3POS+1 ;
//    Mix_max = MIX_MAX + 1 ;
//    Mix_full = MIX_FULL + 1 ;
//		Chout_base = CHOUT_BASE + 1 ;
//  }
//  else
//  {
    Mix_3pos = MIX_3POS ;
    Mix_max = MIX_MAX ;
    Mix_full = MIX_FULL ;
		Chout_base = CHOUT_BASE ;
//  }
  
	if ( att & FADE_FIRST )
	{
    //===========BEEP CENTER================
    anaCenter &= g_model.beepANACenter;
    if(((bpanaCenter ^ anaCenter) & anaCenter)) beepWarn1();
    bpanaCenter = anaCenter;


//    calibratedStick[Mix_max-1]=calibratedStick[Mix_full-1]=1024;
    anas[Mix_max-1]  = RESX;     // MAX
    anas[Mix_full-1] = RESX;     // FULL
	  if ( ((txType==1) || (txType == 2) || (txType == 9)) )
		{
			anas[Mix_3pos-1] = keyState(SW_SC0) ? -1024 : (keyState(SW_SC1) ? 0 : 1024) ;
		}
		else
		{
			anas[Mix_3pos-1] = keyState(SW_ID0) ? -1024 : (keyState(SW_ID1) ? 0 : 1024) ;
    }
		
		for(uint8_t i=0;i<NUM_PPM;i++)    anas[i+PPM_BASE]   = g_ppmIns[i];// - g_eeGeneral.ppmInCalib[i]; //add ppm channels
    for(uint8_t i=0;i<NUM_SKYCHNOUT;i++) anas[i+Chout_base] = chans[i]; //other mixes previous outputs
#if GVARS
        for(uint8_t i=0;i<MAX_GVARS;i++) anas[i+Mix_3pos] = g_model.gvars[i].gvar * 1024 / 100 ;
#endif

		int16_t heliEle = anas[ele_stick] ;
		int16_t heliAil = anas[ail_stick] ;

    //===========Swash Ring================
    if(g_model.swashRingValue)
    {
      uint32_t v = ((int32_t)heliEle*heliEle + (int32_t)heliAil*heliAil);
		  int16_t tmp = calc100toRESX(g_model.swashRingValue) ;
      uint32_t q ;
      q =(int32_t)tmp * tmp ;
      if(v>q)
      {
        uint16_t d = isqrt32(v);
        heliEle = (int32_t)heliEle*tmp/((int32_t)d) ;
        heliAil = (int32_t)heliAil*tmp/((int32_t)d) ;
      }
    }

    //===========Swash Mix================
#define REZ_SWASH_X(x)  ((x) - (x)/8 - (x)/128 - (x)/512)   //  1024*sin(60) ~= 886
#define REZ_SWASH_Y(x)  ((x))   //  1024 => 1024

    if(g_model.swashType)
    {
        int16_t vp = 0 ;
        int16_t vr = 0 ;
        
	      vp = heliEle+trimA[ele_stick];
  	    vr = heliAil+trimA[ail_stick];
        int16_t vc = 0;
        if(g_model.swashCollectiveSource)
				{
					if ( ((txType==1) || (txType == 2) || (txType == 9)) )
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
					else
					{
         	  vc = anas[g_model.swashCollectiveSource-1];
					}
				}

        if(g_model.swashInvertELE) vp = -vp;
        if(g_model.swashInvertAIL) vr = -vr;
        if(g_model.swashInvertCOL) vc = -vc;

        switch (g_model.swashType)
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

//        calibratedStick[MIX_CYC1-1]=anas[MIX_CYC1-1];
//        calibratedStick[MIX_CYC2-1]=anas[MIX_CYC2-1];
//        calibratedStick[MIX_CYC3-1]=anas[MIX_CYC3-1];
    }
	}
    memset(chans,0,sizeof(chans));        // All outputs to 0

    uint8_t mixWarning = 0;
    //========== MIXER LOOP ===============

    // Set the trim pointers back to the master set
    trimptr[0] = &trim[0] ;
    trimptr[1] = &trim[1] ;
    trimptr[2] = &trim[2] ;
    trimptr[3] = &trim[3] ;
        
    if( (g_eeGeneral.throttleReversed) && (!g_model.thrTrim))
    {
        *trimptr[THR_STICK] *= -1;
    }
		{
			int8_t trims[4] ;
			int i ;
			int idx ;
	
      for ( i = 0 ;  i <= 3 ; i += 1 )
			{
				idx = i ;
//				if ( g_eeGeneral.crosstrim )
//				{
//					idx = 3 - idx ;			
//				}
        trims[i] = getTrimValue( CurrentPhase, idx ) ;
			}
		
  		if ( g_model.modelVersion >= 2 )
			{
				uint8_t stickIndex = g_eeGeneral.stickMode*4 ;
		
				uint8_t index ;
				index =g_eeGeneral.crosstrim ? 3 : 0 ;
				index =  stickScramble[stickIndex+index] ;
				ui->trimHLeft->setValue( trims[index]);  // mode=(0 || 1) -> rud trim else -> ail trim
				index =g_eeGeneral.crosstrim ? 2 : 1 ;
				index =  stickScramble[stickIndex+index] ;
    		ui->trimVLeft->setValue( trims[index]);  // mode=(0 || 2) -> thr trim else -> ele trim
				index =g_eeGeneral.crosstrim ? 1 : 2 ;
				index =  stickScramble[stickIndex+index] ;
    		ui->trimVRight->setValue(trims[index]);  // mode=(0 || 2) -> ele trim else -> thr trim
				index =g_eeGeneral.crosstrim ? 0 : 3 ;
				index =  stickScramble[stickIndex+index] ;
    		ui->trimHRight->setValue(trims[index]);  // mode=(0 || 1) -> ail trim else -> rud trim
			}
			else
			{
				ui->trimHLeft->setValue( trims[0]);  // mode=(0 || 1) -> rud trim else -> ail trim
    		ui->trimVLeft->setValue( trims[1]);  // mode=(0 || 2) -> thr trim else -> ele trim
    		ui->trimVRight->setValue(trims[2]);  // mode=(0 || 2) -> ele trim else -> thr trim
    		ui->trimHRight->setValue(trims[3]);  // mode=(0 || 1) -> ail trim else -> rud trim
      }
		
		}

		if( (g_eeGeneral.throttleReversed) && (!g_model.thrTrim))
    {
        *trimptr[THR_STICK] *= -1;
    }

    for(uint8_t i=0;i<MAX_SKYMIXERS;i++)
		{
        SKYMixData &md = g_model.mixData[i];
#if GVARS
				int16_t lweight = md.weight ;
				if ( (lweight <= -126) || (lweight >= 126) )
				{
					lweight = REG100_100( lweight ) ;
				}
				else
				{
					if ( md.extWeight == 1 )
					{
						lweight += 125 ; 
					}
					else if ( md.extWeight == 3 )
					{
						lweight -= 125 ; 
					}
					else if ( md.extWeight == 2 )
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
					if ( lweight > 350 )
					{
						lweight -= 360 ;
						if ( lweight < 0 )
						{
							lweight = -g_model.gvars[-lweight-1].gvar ;
						}
						else
						{
							lweight = g_model.gvars[lweight].gvar ;
						}
					}
				}
				int16_t mixweight = lweight ;
#endif
				int16_t loffset = md.sOffset ;
				if ( (loffset <= -126) || (loffset >= 126) )
				{
					loffset = REG100_100( loffset ) ;
				}
				else
				{
					if ( md.extOffset == 1 )
					{
						loffset += 125 ; 
					}
					else if ( md.extOffset == 3 )
					{
						loffset -= 125 ; 
					}
					else if ( md.extOffset == 2 )
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
					if ( loffset > 350 )
					{
						loffset -= 360 ;
						if ( loffset < 0 )
						{
							loffset = -g_model.gvars[-loffset-1].gvar ;
						}
						else
						{
							loffset = g_model.gvars[loffset].gvar ;
						}
					}
				}
				int16_t mixoffset = loffset ;
        
				if((md.destCh==0) || (md.destCh>NUM_SKYCHNOUT)) break;

        //Notice 0 = NC switch means not used -> always on line
        int16_t v  = 0;
        uint8_t swTog;
        uint8_t swon = swOn[i] ;

#define DEL_MULT 256


        bool t_switch = getSwitch(md.swtch,1) ;
        if ( t_switch )
				{
					if ( md.modeControl & ( 1 << CurrentPhase ) )
					{
						t_switch = 0 ;
					}
				}
        
				uint8_t k = md.srcRaw ;



        //swOn[i]=false;
        if(!t_switch)
				{ // switch on?  if no switch selected => on
            swTog = swon ;
            swon = false;
            if (k == Mix_3pos+MAX_GVARS+1) act[i] = chans[md.destCh-1] * DEL_MULT / 100 ;
            
//						if( k!=Mix_full && k!=Mix_max) continue;// if not MAX or FULL - next loop
//            if(md.mltpx==MLTPX_REP) continue; // if switch is off and REPLACE then off
            if ( ( k==Mix_full || k==Mix_max) || (md.mltpx!=MLTPX_REP) )
						{
            	v = md.srcRaw==Mix_full ? -RESX : 0; // switch is off => FULL=-RESX
						}
        }
        else
				{
            swTog = !swon ;
            swon = true;
            k -= 1 ;
						v = anas[k]; //Switch is on. MAX=FULL=512 or value.
								
//								if ( DebugOutputCount < 20 )
//								{
//									printf("\nk(-1) = %d", k ) ;
//									DebugOutputCount += 1 ;
//								}
						
						if ( k < 4 )
						{
							if ( md.disableExpoDr )
							{
     		      	v = rawSticks[k]; //Switch is on. MAX=FULL=512 or value.
							}
						}

						if( (k >= CHOUT_BASE) && (k<CHOUT_BASE+NUM_CHNOUT) )
						{
              if ( md.disableExpoDr )
							{
								v = chanOut[k-CHOUT_BASE] ;
							}
						}

						if ( txBitType & ( RADIO_BITTYPE_TARANIS | RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E | RADIO_BITTYPE_QX7 | RADIO_BITTYPE_XLITE | RADIO_BITTYPE_T12 | RADIO_BITTYPE_X9L | RADIO_BITTYPE_X12 | RADIO_BITTYPE_X10 | RADIO_BITTYPE_T16 | RADIO_BITTYPE_TX16S | RADIO_BITTYPE_X10E | RADIO_BITTYPE_TX18S )  )
						{
							if ( k == MIX_3POS-1 )
							{
                uint32_t sw = switchIndex[md.switchSource] ;

//								if ( DebugOutputCount < 20 )
//								{
//									printf("\nMIX_3POS-1, %d, %d", md.switchSource, sw ) ;
//									DebugOutputCount += 1 ;
//								}
								

//                EnumKeys sw = (EnumKeys)md.switchSource ;
								if ( md.switchSource > 7 )	// Logical switch
								{
									v = getSwitch( md.switchSource+2, 0, 0) ? 1024 : -1024 ;
								}
								else if ( ( md.switchSource == 5) || ( md.switchSource == 7) )
								{ // 2-POS switch
                  v = hwKeyState(sw) ? 1024 : -1024 ;
								}
                else if( md.switchSource == 32)
								{
									v = 0 ;
									if ( hwKeyState( HSW_Ele6pos1 ) )
									{
										v = 1 ;
									}
									else if ( hwKeyState( HSW_Ele6pos2 ) )
									{
										v = 2 ;
									}
									else if ( hwKeyState( HSW_Ele6pos3 ) )
									{
										v = 3 ;
									}
									else if ( hwKeyState( HSW_Ele6pos4 ) )
									{
										v = 4 ;
									}
									else if ( hwKeyState( HSW_Ele6pos5 ) )
									{
										v = 5 ;
									}
                  v = (v * 2048 - 5120)/5 ;
								}
								else
								{ // 3-POS switch
                  v = hwKeyState(sw) ? -1024 : (hwKeyState((sw+1)) ? 0 : 1024) ;
								}
							}
						}
						else
						{
							if ( k == MIX_3POS-1 )
							{
                uint32_t sw = getSw3PosList( md.switchSource) ;
                if ( getSw3PosCount(md.switchSource) == 2 )
								{
									if ( md.switchSource > 6 )	// Logical switch
									{
										v = getSwitch( md.switchSource+3, 0, 0) ? 1024 : -1024 ;
									}
									else
									{
        						v = hwKeyState(sw) ? 1024 : -1024 ;
									}
								}
								else if ( getSw3PosCount(md.switchSource) == 6 )
								{
									v = 0 ;
									if ( hwKeyState( HSW_Ele6pos1 ) )
									{
										v = 1 ;
									}
									else if ( hwKeyState( HSW_Ele6pos2 ) )
									{
										v = 2 ;
									}
									else if ( hwKeyState( HSW_Ele6pos3 ) )
									{
										v = 3 ;
									}
									else if ( hwKeyState( HSW_Ele6pos4 ) )
									{
										v = 4 ;
									}
									else if ( hwKeyState( HSW_Ele6pos5 ) )
									{
										v = 5 ;
									}
                  v = (v * 2048 - 5120)/5 ;
								}
								else
								{
        					v = hwKeyState(sw) ? -1024 : (hwKeyState(sw+1) ? 0 : 1024) ;
								}
							}
						}
            if(k>Chout_base && (k<i)) v = chans[k];
            if (k == Mix_3pos+MAX_GVARS) v = chans[md.destCh-1] / 100 ;
            if ( (k > MIX_3POS+MAX_GVARS) && ( k <= MIX_3POS+MAX_GVARS + NUM_SCALERS ) )
						{
							v = calc_scaler( k - (Mix_3pos+MAX_GVARS+1) ) ;
						}
            if (k > MIX_3POS+MAX_GVARS + NUM_SCALERS)
						{
							if ( k <= MIX_3POS+MAX_GVARS + NUM_SCALERS + NUM_EXTRA_PPM )
							{
								
							}
							else
							{
								v = calibratedStick[k-EXTRA_POTS_START+8] ;
							}
						}
//#define MIX_TRIMS_START 78
						if ( ( k >= MIX_TRIMS_START-1) && (k <= MIX_TRIMS_START-1 + 4 ) )
						{
              uint32_t t = k - MIX_TRIMS_START+1 ;
							v = trimA[t] ;
						}

						if ( k >= 224-1 )
						{
							v = getInputSourceValue( &g_model.inputs[k-224+1] ) ;
						}

            if(md.mixWarn) mixWarning |= 1<<(md.mixWarn-1); // Mix warning
//            if ( md.enableFmTrim )
//            {
//                if ( md.srcRaw <= 4 )
//                {
//                    trimptr[md.srcRaw-1] = &md.sOffset ;		// Use the value stored here for the trim
//                    if( (g_eeGeneral.throttleReversed) && (!g_model.thrTrim))
//                    {
//                      *trimptr[THR_STICK] *= -1;
//                    }
//										ui->trimHLeft->setValue( getTrimValue( CurrentPhase, 0 ));  // mode=(0 || 1) -> rud trim else -> ail trim
//    								ui->trimVLeft->setValue( getTrimValue( CurrentPhase, 1 ));  // mode=(0 || 2) -> thr trim else -> ele trim
//    								ui->trimVRight->setValue(getTrimValue( CurrentPhase, 2 ));  // mode=(0 || 2) -> ele trim else -> thr trim
//    								ui->trimHRight->setValue(getTrimValue( CurrentPhase, 3 ));  // mode=(0 || 1) -> ail trim else -> rud trim
//                    if( (g_eeGeneral.throttleReversed) && (!g_model.thrTrim))
//                    {
//                      *trimptr[THR_STICK] *= -1;
//                    }
//                }
//            }
        }
        swOn[i] = swon ;

        //========== INPUT OFFSET ===============
//        if ( ( md.enableFmTrim == 0 ) && ( md.lateOffset == 0 ) )
        if ( swon && md.lateOffset == 0 )
        {
#if GVARS
            if(mixoffset) v += calc100toRESX( mixoffset	) ;
//            if(md.sOffset) v += calc100toRESX( REG( md.sOffset, -125, 125 )	) ;
#else
            if(md.sOffset) v += calc100toRESX(md.sOffset);
#endif
        }

        //========== DELAY and PAUSE ===============
        if (md.speedUp || md.speedDown || md.delayUp || md.delayDown)  // there are delay values
        {
            if(init)
            {
                act[i]=(int32_t)v*DEL_MULT;
                swTog = false;
            }
						int16_t my_delay = sDelay[i] ;
            int32_t tact = act[i] ;
            int16_t diff = v-tact/DEL_MULT;

					if (swon )
					{
						if ( ( diff > 10 ) || ( diff < -10 ) )
						{
							if ( my_delay == 0 )
							{
								swTog = 1 ;
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
                if(md.mltpx==MLTPX_REP)
                {
                    tact = (int32_t)anas[md.destCh-1+Chout_base]*DEL_MULT * 100 ;
#if GVARS
                    if(mixweight) tact /= mixweight ;
#else
                    if(md.weight) tact /= md.weight;
#endif
                }
                diff = v-tact/DEL_MULT;
                if(diff) my_delay = (diff<0 ? md.delayUp :  md.delayDown) * 10;
            }

            if(my_delay > 0)
						{ // perform delay
              if (--my_delay != 0)
              { // At end of delay, use new V and diff
                v = tact/DEL_MULT;    // Stay in old position until delay over
                diff = 0;
              }
							else
							{
								my_delay = -1 ;
							}
            }
						sDelay[i] = my_delay ;
					}
            if(diff && (md.speedUp || md.speedDown))
						{
                //rate = steps/sec => 32*1024/100*md.speedUp/Down
                //act[i] += diff>0 ? (32768)/((int16_t)100*md.speedUp) : -(32768)/((int16_t)100*md.speedDown);
                //-100..100 => 32768 ->  100*83886/256 = 32768,   For MAX we divide by 2 since it's asymmetrical

                int32_t rate = (int32_t)DEL_MULT*2048*100;
#if GVARS
                if(mixweight) rate /= abs(mixweight);
#else
                if(md.weight) rate /= abs(md.weight);
#endif
                tact = (diff>0) ? ((md.speedUp>0)   ? tact+(rate)/((int16_t)10*md.speedUp)   :  (int32_t)v*DEL_MULT) :
                                    ((md.speedDown>0) ? tact-(rate)/((int16_t)10*md.speedDown) :  (int32_t)v*DEL_MULT) ;


                if(((diff>0) && (v<(tact/DEL_MULT))) || ((diff<0) && (v>(act[i]/DEL_MULT)))) tact=(int32_t)v*DEL_MULT; //deal with overflow
                v = tact/DEL_MULT;
            }
            else if (diff)
            {
              tact=(int32_t)v*DEL_MULT;
            }
						act[i] = tact ;
        }

			if (swon )
			{
        //========== CURVES ===============
				uint32_t diffValue ;
        diffValue = md.differential | (md.extDiff << 1 ) ;
				if ( diffValue )
				{
      		//========== DIFFERENTIAL or expo =========
         	int16_t curveParam = REG( md.curve, -100, 100 ) ;
					if ( diffValue == 3 )
					{
						// New Gvar
						if ( curveParam < 0 )
						{
							curveParam = -curveParam - 1 ;							
							curveParam = -g_model.gvars[curveParam].gvar ;
						}
						else
						{
							curveParam = g_model.gvars[curveParam].gvar ;
						}
					}
     			if (curveParam > 0 && v < 0)
     				v = ((int32_t)v * (100 - curveParam)) / 100;
     			else if (curveParam < 0 && v > 0)
     				v = ((int32_t)v * (100 + curveParam)) / 100;
				}
				else
				{
          if ( md.curve <= -28 )
					{
						// do expo using md->curve + 128
            v = expo( v, md.curve + 128 ) ;
					}
					else
					{
        		switch(md.curve)
						{
        		case 0:
        			  break;
        		case 1:
        			  if(md.srcRaw == MIX_FULL) //FUL
        			  {
        			      if( v<0 ) v=-RESX;   //x|x>0
        			      else      v=-RESX+2*v;
        			  }else{
        			      if( v<0 ) v=0;   //x|x>0
        			  }
        			  break;
        		case 2:
        			  if(md.srcRaw == MIX_FULL) //FUL
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
        			  int8_t idx = md.curve ;
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
        if((md.carryTrim==0) && (md.srcRaw>0) && (md.srcRaw<=4))
				{
					int32_t trim = trimA[md.srcRaw-1] ;
//					if ( ( md.srcRaw-1 != 2 ) || ( !g_model.thrTrim ) )
//					{
//          	if ( g_model.trimsScaled )
//						{
//							int32_t scale = 1024 ;
//              if ( ( trim > 0 ) && ( v > 0 ) )
//							{
//								scale -= trim ;
//							}
//              else if ( ( trim < 0 ) && ( v < 0 ) )
//							{
//								scale += trim ;
//							}
//							scale *= v ;
//							v = scale / 1024 ;
//						}
//					}
 					v += trim ;  //  0 = Trim ON  =  Default
				}

        //========== MULTIPLEX ===============
#if GVARS
        int32_t dv = (int32_t)v*mixweight ;
#else
        int32_t dv = (int32_t)v*md.weight;
#endif
        
				//========== lateOffset ===============
//        if ( ( md.enableFmTrim == 0 ) && ( md.lateOffset ) )
        if ( md.lateOffset )
        {
#if GVARS
            if(mixoffset) dv += calc100toRESX( mixoffset	) * 100 ;
//            if(md.sOffset) dv += calc100toRESX( REG( md.sOffset, -125, 125 )	) * 100  ;
#else
            if(md.sOffset) dv += calc100toRESX(md.sOffset) * 100 ;
#endif
        }
        switch(md.mltpx){
        case MLTPX_REP:
            chans[md.destCh-1] = dv;
            break;
        case MLTPX_MUL:
            chans[md.destCh-1] *= dv/100l;
            chans[md.destCh-1] /= RESXl;
            break;
        default:  // MLTPX_ADD
            chans[md.destCh-1] += dv; //Mixer output add up to the line (dv + (dv>0 ? 100/2 : -100/2))/(100);
            break;
        }
			}
			else
			{
        chans[md.destCh-1] = (int32_t)v*mixweight ;
			}
    }


    //========== MIXER WARNING ===============
    //1= 00,08
    //2= 24,32,40
    //3= 56,64,72,80
    if(mixWarning & 1) if(((g_tmr10ms&0xFF)==  0)) beepWarn1();
    if(mixWarning & 2) if(((g_tmr10ms&0xFF)== 64) || ((g_tmr10ms&0xFF)== 72)) beepWarn1();
    if(mixWarning & 4) if(((g_tmr10ms&0xFF)==128) || ((g_tmr10ms&0xFF)==136) || ((g_tmr10ms&0xFF)==144)) beepWarn1();


    //========== LIMITS ===============
    for(uint8_t i=0;i<NUM_SKYCHNOUT;i++)
		{
        // chans[i] holds data from mixer.   chans[i] = v*weight => 1024*100
        // later we multiply by the limit (up to 100) and then we need to normalize
        // at the end chans[i] = chans[i]/100 =>  -1024..1024
        // interpolate value with min/max so we get smooth motion from center to stop
        // this limits based on v original values and min=-1024, max=1024  RESX=1024

        int32_t q = chans[i] ;// + (int32_t)g_model.limitData[i].offset*100; // offset before limit

				if ( fadePhases )
				{
					
					int32_t l_fade = fade[i] ;
					if ( att & FADE_FIRST )
					{
						l_fade = 0 ;
					}
					l_fade += ( q / 100 ) * fadeScale[CurrentPhase] ;
					fade[i] = l_fade ;
			
					if ( ( att & FADE_LAST ) == 0 )
					{
						continue ;
					}
          if ( fadeWeight != 0)
          {
            l_fade /= fadeWeight ;
          }
					q = l_fade * 100 ;
				}
    	  chans[i] = q / 100 ; // chans back to -1024..1024

        ex_chans[i] = chans[i]; //for getswitch

        int16_t ofs = g_model.limitData[i].offset;
				int16_t xofs = ofs ;
				if ( xofs > g_model.sub_trim_limit )
				{
					xofs = g_model.sub_trim_limit ;
				}
				else if ( xofs < -g_model.sub_trim_limit )
				{
					xofs = -g_model.sub_trim_limit ;
				}
				int16_t lim_p = 10*(g_model.limitData[i].max+100) + xofs ;
        int16_t lim_n = 10*(g_model.limitData[i].min-100) + xofs ; //multiply by 10 to get same range as ofs (-1000..1000)
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

        if(q)
				{
					int16_t temp = (q<0) ? ((int16_t)ofs-lim_n) : ((int16_t)lim_p-ofs) ;
          q = ( q * temp ) / 100000 ; //div by 100000 -> output = -1024..1024
				}

				int16_t result ;
				result = calc1000toRESX(ofs);
  			result += q ; // we convert value to a 16bit value
        
				lim_p = calc1000toRESX(lim_p);
        lim_n = calc1000toRESX(lim_n);
        if(result>lim_p) result = lim_p;
        if(result<lim_n) result = lim_n;
        
				if(g_model.limitData[i].revert) result = -result ;// finally do the reverse.

				{
          uint8_t numSafety = 24 - g_model.numVoice ;
					if ( i < numSafety )
					{
        		if(g_model.safetySw[i].opt.ss.swtch)  //if safety sw available for channel check and replace val if needed
						{
							if ( ( g_model.safetySw[i].opt.ss.mode != 1 ) && ( g_model.safetySw[i].opt.ss.mode != 2 ) )	// And not used as an alarm
							{
								static uint8_t sticky = 0 ;
								uint8_t applySafety = 0 ;
								int8_t sSwitch = g_model.safetySw[i].opt.ss.swtch ;
								
								if(getSwitch( sSwitch,0))
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
						  			if ( g_model.modelVersion >= 2 )
										{
											uint32_t throttleOK = 0 ;
//											if ( g_model.throttleIdle )
//											{
//												if ( abs( calibratedStick[2] ) < 20 )
//												{
//													throttleOK = 1 ;
//												}
//											}
//											else
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
												sticky |= (1<<i) ;
											}
											if ( ( sticky & (1<<i) ) == 0 )
											{
												applySafety = 1 ;
											}
										}
										else
										{
											if ( calibratedStick[THR_STICK] < -1010 )
											{
												sticky = 1 ;
											}
											if ( sticky == 0 )
											{
												applySafety = 1 ;
											}
										}
									}
								}
								if ( applySafety ) result = calc100toRESX(g_model.safetySw[i].opt.ss.val) ;
							}
						}
					}
				}
        //cli();
        chanOut[i] = result ; //copy consistent word to int-level
        //sei();
    }
}


void simulatorDialog::on_GvarButton_clicked()
{
	gvar_or_scalers = !gvar_or_scalers ;
  ui->GvarButton->setText( gvar_or_scalers ? "Scalers" : "GVARs" ) ;
}

void simulatorDialog::on_holdLeftX_clicked(bool checked)
{
    nodeLeft->setCenteringX(!checked);
}

void simulatorDialog::on_holdLeftY_clicked(bool checked)
{
    nodeLeft->setCenteringY(!checked);
}

void simulatorDialog::on_holdRightX_clicked(bool checked)
{
    nodeRight->setCenteringX(!checked);
}

void simulatorDialog::on_holdRightY_clicked(bool checked)
{
    nodeRight->setCenteringY(!checked);
}


void simulatorDialog::on_FixLeftX_clicked(bool checked)
{
    nodeLeft->setFixedX(checked);
}

void simulatorDialog::on_FixLeftY_clicked(bool checked)
{

    nodeLeft->setFixedY(checked);
}

void simulatorDialog::on_FixRightX_clicked(bool checked)
{
    nodeRight->setFixedX(checked);
}

void simulatorDialog::on_FixRightY_clicked(bool checked)
{
    nodeRight->setFixedY(checked);
}

int16_t simulatorDialog::calc_scaler( uint8_t index )
{
	int32_t value ;
	int32_t exValue ;
	uint8_t lnest ;
	ScaleData *pscaler ;
	ExtScaleData *epscaler ;
	
	lnest = CalcScaleNest ;
	if ( lnest > 5 )
	{
		return 0 ;
	}
	CalcScaleNest = lnest + 1 ;
	// process
	pscaler = &g_model.Scalers[index] ;
	epscaler = &g_model.eScalers[index] ;
	if ( pscaler->source )
	{
		value = getValue( pscaler->source - 1 ) ;
	}
	else
	{
		value = 0 ;
	}
	CalcScaleNest = lnest ;
	if ( pscaler->offsetLast == 0 )
	{
		value += pscaler->offset ;
	}
	uint16_t t ;
	t = pscaler->mult + ( pscaler->multx << 8 ) ;
	value *= t+1 ;
	t = pscaler->div + ( pscaler->divx << 8 ) ;
	value /= t+1 ;
	if ( epscaler->mod )
	{
		value %= epscaler->mod+1 ;
	}
	if ( epscaler->exSource )
	{
		exValue = getValue( epscaler->exSource - 1 ) ;
		if ( pscaler->exFunction )
		{
			switch ( pscaler->exFunction )
			{
				case 1 :	// Add
					value += exValue ;
				break ;
				case 2 :	// Subtract
					value -= exValue ;
				break ;
				case 3 :	// Multiply
					value *= exValue ;
				break ;
				case 4 :	// Divide
					if ( exValue )
					{
						value /= exValue ;
					}
				break ;
				case 5 :	// Mod
					if ( exValue )
					{
						value %= exValue ;
					}
				break ;
			}
		}
	}
	if ( pscaler->offsetLast )
	{
		value += pscaler->offset ;
	}
	if ( pscaler->neg )
	{
		value = -value ;
	}

	return value ;
}
									 


void simulatorDialog::on_SendDataButton_clicked()
{
	QString portname ;
	
	if ( serialSending )
	{
		if ( port )
		{
			if (port->isOpen())
			{
		 	  port->close();
			}
		 	delete port ;
			port = NULL ;
		}
		serialSending = 0 ;
    ui->SendDataButton->setText("Send (SBUS)") ;
	}
	else
	{
	  portname = ui->serialPortCB->currentText() ;
    ui->SendDataButton->setText("Stop (SBUS)") ;
#ifdef Q_OS_UNIX
  	port = new QextSerialPort(portname, QextSerialPort::Polling) ;
#else
		port = new QextSerialPort(portname, QextSerialPort::Polling) ;
#endif /*Q_OS_UNIX*/
    port->setBaudRate(BAUD57600) ;
  	port->setFlowControl(FLOW_OFF) ;
		port->setParity(PAR_NONE) ;
  	port->setDataBits(DATA_8) ;
		port->setStopBits(STOP_1) ;
  	//set timeouts to 500 ms
  	port->setTimeout(-1) ;
  	if (!port->open(QIODevice::ReadWrite | QIODevice::Unbuffered) )
  	{
  		QMessageBox::critical(this, "eeSkyPe", tr("Com Port Unavailable"));
			if (port->isOpen())
			{
  		  port->close();
			}
  		delete port ;
			port = NULL ;
      ui->SendDataButton->setText("Send (SBUS)") ;
			return ;	// Failed
		}
		serialTimer = 0 ;
		serialSending = 1 ;

	}
}

void simulatorDialog::processVoiceAlarms()
{
//	uint32_t i ;
//	uint32_t curent_state ;
//	uint8_t flushSwitch ;
//	VoiceAlarmData *pvad = &g_model.vad[0] ;
//	i = 0 ;
//	if ( VoiceCheckFlag100mS & 4 )
//	{
//		i = NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS ;
//	}
//	flushSwitch = getSwitch00( g_model.voiceFlushSwitch ) ;
//	if ( ( VoiceCheckFlag100mS & 2 ) == 0 )
//	{
//		if ( flushSwitch && ( LastVoiceFlushSwitch == 0 ) )
//		{
//			flushVoiceQueue() ;			
//		}
//	}
//	LastVoiceFlushSwitch = flushSwitch ;
//  for ( ; i < NUM_SKY_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS + NUM_GLOBAL_VOICE_ALARMS ; i += 1 )
//	{
//		uint32_t play = 0 ;
//		uint32_t functionTrue = 0 ;
//		curent_state = 0 ;
//		int16_t ltimer = Nvs_timer[i] ;
//	 	if ( i == NUM_VOICE_ALARMS )
//		{
//			pvad = &g_model.vadx[0] ;
//		}
//	 	if ( i == NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
//		{
//			pvad = &g_eeGeneral.gvad[0] ;
//		}
//		if ( pvad->func )		// Configured
//		{
//  		int16_t x ;
//			int16_t y = pvad->offset ;
//			x = getValue( pvad->source - 1 ) ;
//  		switch (pvad->func)
//			{
//				case 1 :
//					x = x > y ;
//				break ;
//				case 2 :
//					x = x < y ;
//				break ;
//				case 3 :
//					x = abs(x) > y ;
//				break ;
//				case 4 :
//					x = abs(x) < y ;
//				break ;
//				case 5 :
//				{
//					if ( isAgvar( pvad->source ) )
//					{
//						x *= 10 ;
//						y *= 10 ;
//					}
//    			x = abs(x-y) < 32 ;
//				}
//				break ;
//				case 6 :
//					x = x == y ;
//				break ;
//				case 7 :
//					x = (x & y) != 0 ;
//				break ;
//				case 8 :
//				{	
//  				int16_t z ;
//					z = x - pc->nvs_last_value ;
//					z = abs(z) ;
//					if ( z > y )
//					{
//						pc->nvs_last_value = x ;
//						x = 1 ;
//					}
//					else
//					{
//						x = 0 ;
//					}
//				}
//				break ;
//			}
//			functionTrue = x ;
//// Start of invalid telemetry detection
////					if ( pvad->source > ( CHOUT_BASE - NUM_SKYCHNOUT ) )
////					{ // Telemetry item
////						if ( !telemItemValid( pvad->source - 1 - CHOUT_BASE - NUM_SKYCHNOUT ) )
////						{
////							x = 0 ;	// Treat as OFF
////						}
////					}
//// End of invalid telemetry detection
//			if ( pvad->swtch )
//			{
//				if ( pvad->swtch == MAX_SKYDRSWITCH + 1 )
//				{
//					if ( getFlightPhase() == 0 )
//					{
//						x = 0 ;
//					}
//				}
//				else if ( getSwitch( pvad->swtch,0,0 ) == 0 )
//				{
//					x = 0 ;
//				}
//			}
//			if ( x == 0 )
//			{
//				ltimer = 0 ;
//			}
//			else
//			{
//				play = 1 ;
//			}
//		}
//		else // No function
//		{
//			if ( pvad->swtch )
//			{
//				if ( pvad->swtch == MAX_SKYDRSWITCH + 1 )
//				{
//					curent_state = getFlightPhase() ? 1 : 0 ;
//				}
//				else
//				{
//					curent_state = getSwitch( pvad->swtch,0,0 ) ;
//				}	
//				if ( curent_state == 0 )
//				{
//					ltimer = -1 ;
//				}
//			}
//			else// No switch, no function
//			{ // Check for source with numeric rate
//				if ( pvad->rate >= 4 )	// A time
//				{
//					if ( pvad->vsource )
//					{
//						play = 1 ;
//					}
//				}
//			}
//		}
//		play |= curent_state ;

//		if ( ( VoiceCheckFlag & 2 ) == 0 )
//		{
//		 if ( pvad->rate == 3 )	// All
//		 {
//		 		uint32_t pos
//				pos = 1 ;
//				if ( pvad->func && ( functionTrue == 0 ) )
//				{
//					pos = 0 ;
//				}
//		 		if ( pos )
//				{
//					if ( pvad->swtch == MAX_SKYDRSWITCH + 1 )
//					{
//						pos = getFlightPhase() ;
//					}
//					else
//					{
//						pos = switchPosition( pvad->swtch ) ;
//					}
//					uint32_t state = Nvs_state[i] ;
//					play = 0 ;
//					if ( state != pos )
//					{
//						if ( state > 0x80 )
//						{
//							if ( --state == 0x80 )
//							{
//								state = pos ;
//								ltimer = 0 ;
//								play = pos + 1 ;
//							}
//						}
//						else
//						{
//							state = 0x83 ;
//						}
//						Nvs_state[i] = state ;
//					}
//			  }
//				else
//				{
//					pc->nvs_state = 0x40 ;
//				}
//		 }	
//		 else
//		 {
//			if ( play == 1 )
//			{
//				if ( Nvs_state[i] == 0 )
//				{ // just turned ON
//					if ( ( pvad->rate == 0 ) || ( pvad->rate == 2 ) )
//					{ // ON
//						if ( pvad->delay )
//						{
//							pc->nvs_delay = pvad->delay + 1 ;
//						}
//						ltimer = 0 ;
//					}
//				}
//				else
//				{ // just turned OFF
//					if ( ( pvad->rate == 1 ) )
//					{
//						if ( pvad->func == 8 )	// |d|>val
//						{
//							if ( pvad->delay )
//							{
//								pc->nvs_delay = pvad->delay + 1 ;
//								play = 0 ;
//							}
//						}
//					}
//				}
//				Nvs_state[i] = 1 ;
//				if ( ( pvad->rate == 1 ) )
//				{
//					play = 0 ;
//				}
//				if ( pc->nvs_delay )
//				{
//					if ( --pc->nvs_delay )
//					{
//						play = 0 ;
//					}
//				}
//			}
//			else
//			{
//				if ( ( pvad->func == 8 ) && ( pc->nvs_delay ) )	// |d|>val
//				{
//					play = 0 ;
//					if ( --pc->nvs_delay == 0 )
//					{
//						play = 1 ;
//					}
//				}
//				else
//				{
//					pc->nvs_delay = 0 ;
//					if ( Nvs_state[i] == 1 )
//					{
//						if ( ( pvad->rate == 1 ) || ( pvad->rate == 2 ) )
//						{
//							ltimer = 0 ;
//							play = 1 ;
//							if ( pvad->rate == 2 )
//							{
//								play = 2 ;
//							}
//						}
//					}
//				}
//				Nvs_state[i] = 0 ;
//			}
//			if ( pvad->rate == 33 )
//			{
//				play = 0 ;
//				ltimer = -1 ;
//			}
//		 }
//		}
//		else //( ( VoiceCheckFlag100mS & 2 ) != 0 )
//		{
//		 	uint32_t pos ;
//			if ( pvad->func == 8 )	// |d|>val
//			{
//				pc->nvs_last_value = getValue( pvad->source - 1 ) ;
//			}
//			if ( pvad->rate == 3 )
//			{
//				if ( pvad->swtch == MAX_SKYDRSWITCH + 1 )
//				{
//					pos = getFlightPhase() ;
//				}
//				else
//				{
//					pos = switchPosition( pvad->swtch ) ;
//				}
//			}
//			else
//			{
//				pos = play ;
//			}
//			Nvs_state[i] = pos ;
//			play = 0 ;
//			if ( pvad->rate == 33 )	// ONCE
//			{
//	 			if ( i >= NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
//				{	// Global alert
//					if ( VoiceCheckFlag100mS & 4 )
//					{
//						play = 1 ;
//					}
//				}
//				else
//				{
//					play = 1 ;
//				}
//			}
//			ltimer = -1 ;
//		}

//		if ( pvad->mute )
//		{
//			if ( pvad->source > ( CHOUT_BASE - NUM_SKYCHNOUT ) )
//			{ // Telemetry item
//				if ( !telemItemValid( pvad->source - 1 - CHOUT_BASE - NUM_SKYCHNOUT ) )
//				{
//					play = 0 ;	// Mute it
//				}
//			}
//		}

//		if ( play )
//		{
//			if ( ltimer < 0 )
//			{
//				if ( pvad->rate >= 4 )	// A time or ONCE
//				{
//					ltimer = 0 ;
//				}
//			}
//			if ( ltimer == 0 )
//			{
//				if ( pvad->vsource == 1 )
//				{
//					doVoiceAlarmSource( pvad ) ;
//				}
//				if ( pvad->fnameType == 0 )	// None
//				{
//					// Nothing!
//				}
//				else if ( pvad->fnameType == 1 )	// Name
//				{
//					char name[10] ;
//					char *p ;
//					p = (char *)ncpystr( (uint8_t *)name, pvad->file.name, 8 ) ;
//					if ( name[0] && ( name[0] != ' ' ) )
//					{
//						if ( play >= 2 )
//						{
//							while ( *(p-1) == ' ' )
//							{
//								p -= 1 ;
//							}
//							*(p-1) += ( play - 1 ) ;
//						}
//						putUserVoice( name, 0 ) ;
//					}
//				}
//				else if ( pvad->fnameType == 2 )	// Number
//				{
//					uint16_t value = pvad->file.vfile ;
//					if ( value > 507 )
//					{
//						value = calc_scaler( value-508, 0, 0 ) ;
//					}
//					else if ( value > 500 )
//					{
//						value = g_model.gvars[value-501].gvar ;
//					}
//					putVoiceQueue( ( value + ( play - 1 ) ) | VLOC_NUMUSER ) ;
//				}
//				else
//				{ // Audio
//					audio.event( pvad->file.vfile, 0, 1 ) ;
//				}
//				if ( pvad->vsource == 2 )
//				{
//					doVoiceAlarmSource( pvad ) ;
//				}
////        if ( pvad->haptic )
////				{
////					audioDefevent( (pvad->haptic > 1) ? ( ( pvad->haptic == 3 ) ? AU_HAPTIC3 : AU_HAPTIC2 ) : AU_HAPTIC1 ) ;
////				}
//				if ( ( pvad->rate < 4 ) || ( pvad->rate > 32 ) )	// Not a time
//				{
//					ltimer = -1 ;
//				}
//				else
//				{
//					ltimer = 1 ;
//				}
//			}
//			else if ( ltimer > 0 )
//			{
//				ltimer += 1 ;
//				if ( ltimer > ( (pvad->rate-2) * 10 ) )
//				{
//					ltimer = 0 ;
//				}
//			}
//		}
//		pvad += 1 ;
//		Nvs_timer[i] = ltimer ;
//	}
}

int16_t simulatorDialog::getInputSourceValue( struct te_InputsData *pinput )
{
	int16_t value ;
	uint32_t switchIdx ;
	if ( pinput )
	{
		if ( (pinput->srcRaw) && ( pinput->srcRaw <= 7 ) )
		{
			return rawSticks[pinput->srcRaw-1] ;
		}
		
		if (pinput->srcRaw >= 128)	// A Switch
		{
			switchIdx = pinput->srcRaw - 128 ;




			if ( switchIdx > 8 )
			{ // Logical switch
				value = getSwitch( CSW_INDEX+switchIdx-8, 0, 0 ) ;
				value = value ? 1024 : -1024 ;
				return value ;
			}
			else
			{
				if ( txBitType & ( RADIO_BITTYPE_SKY | RADIO_BITTYPE_TARANIS | RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E | RADIO_BITTYPE_QX7 | RADIO_BITTYPE_XLITE | RADIO_BITTYPE_T12 | RADIO_BITTYPE_X9L | RADIO_BITTYPE_X12 | RADIO_BITTYPE_X10 | RADIO_BITTYPE_T16 | RADIO_BITTYPE_TX16S | RADIO_BITTYPE_X10E | RADIO_BITTYPE_TX18S )  )
				{
					uint32_t sw = switchIndex[switchIdx] ;
					if ( ( switchIdx == 5) || ( switchIdx == 7) )
					{ // 2-POS switch
        		value = hwKeyState(sw) ? 1024 : -1024 ;
					}
					else if( switchIdx == 8)
					{
						value = 0 ;
						if ( hwKeyState( HSW_Ele6pos1 ) )
						{
							value = 1 ;
						}
						else if ( hwKeyState( HSW_Ele6pos2 ) )
						{
							value = 2 ;
						}
						else if ( hwKeyState( HSW_Ele6pos3 ) )
						{
							value = 3 ;
						}
						else if ( hwKeyState( HSW_Ele6pos4 ) )
						{
							value = 4 ;
						}
						else if ( hwKeyState( HSW_Ele6pos5 ) )
						{
							value = 5 ;
						}
            value = (value * 2048 - 5120)/5 ;
					}
					else
					{ // 3-POS switch
        		value = hwKeyState(sw) ? -1024 : (hwKeyState(sw+1) ? 0 : 1024) ;
					}
				}
				else
				{
					if ( switchIdx > 6 )
					{
						if ( switchIdx == 31 )
						{
							value = 0 ;
							if ( hwKeyState( HSW_Ele6pos1 ) )
							{
								value = 1 ;
							}
							else if ( hwKeyState( HSW_Ele6pos2 ) )
							{
								value = 2 ;
							}
							else if ( hwKeyState( HSW_Ele6pos3 ) )
							{
								value = 3 ;
							}
							else if ( hwKeyState( HSW_Ele6pos4 ) )
							{
								value = 4 ;
							}
							else if ( hwKeyState( HSW_Ele6pos5 ) )
							{
								value = 5 ;
							}
            	value = (value * 2048 - 5120)/5 ;
						}
						else
						{
							value = getSwitch( CSW_INDEX+switchIdx-6, 0, 0 ) ;
							value = value ? 1024 : -1024 ;
						}
					}
					else
					{
						uint32_t sw = getSw3PosList(switchIdx) ;
            if ( getSw3PosCount(switchIdx) == 2 )
						{
      		  	value = hwKeyState(sw) ? 1024 : -1024 ;
						}
            else if ( getSw3PosCount(switchIdx) == 6 )
						{
							value = 0 ;
							if ( hwKeyState( HSW_Ele6pos1 ) )
							{
								value = 1 ;
							}
							else if ( hwKeyState( HSW_Ele6pos2 ) )
							{
								value = 2 ;
							}
							else if ( hwKeyState( HSW_Ele6pos3 ) )
							{
								value = 3 ;
							}
							else if ( hwKeyState( HSW_Ele6pos4 ) )
							{
								value = 4 ;
							}
							else if ( hwKeyState( HSW_Ele6pos5 ) )
							{
								value = 5 ;
							}
            	value = (value * 2048 - 5120)/5 ;
						}
						else
						{
      		  	value = hwKeyState(sw) ? -1024 : (hwKeyState(sw+1) ? 0 : 1024) ;
						}
					}
				}
				return value ;
			}
		}



//		if (pinput->srcRaw >= EXTRA_POTS_START )
//		{
//#ifdef PCBX7
//			return calibratedStick[pinput->srcRaw-EXTRA_POTS_START+6] ;
//#else
// #ifdef PCBX9LITE
//			return calibratedStick[pinput->srcRaw-EXTRA_POTS_START+5] ;
// #else
//			return calibratedStick[pinput->srcRaw-EXTRA_POTS_START+7] ;
// #endif
//#endif
//		}

		if ( (pinput->srcRaw >= 21 ) && (pinput->srcRaw < 45 ) )
		{
			return ex_chans[pinput->srcRaw - 21] ;
		}
		if ( (pinput->srcRaw >= 70 ) && (pinput->srcRaw < 78 ) )
		{
			return ex_chans[pinput->srcRaw - 70] ;
		}
		
		if ( (pinput->srcRaw >= 13 ) && (pinput->srcRaw < PPM_BASE + 21 ) )
		{
			return 0 ;
//			if ( ppmInValid )
//			{			
//				return g_ppmIns[pinput->srcRaw - PPM_BASE-1] * 2 ;
//			}
		}
		if ( (pinput->srcRaw >= 62 ) && (pinput->srcRaw < 70 ) )
		{
			return 0 ;
//			if ( ppmInValid )
//			{			
//				return g_ppmIns[pinput->srcRaw - (EXTRA_PPM_BASE-NUM_EXTRA_PPM)-1] * 2 ;
//			}
		}
		if ( (pinput->srcRaw >= 78 ) && (pinput->srcRaw < 82 ) )
		{
 			return getTrimValue( CurrentPhase, pinput->srcRaw - 78 ) * 8 ;
			
		}




	}
  return 0 ;
}







