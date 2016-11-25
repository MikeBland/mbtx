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

#define GBALL_SIZE  20
#define GVARS	1

#define RESX    1024
#define RESXu   1024u
#define RESXul  1024ul
#define RESXl   1024l
#define RESKul  100ul
#define RESX_PLUS_TRIM (RESX+128)

//#define IS_THROTTLE(x)  (((2-(g_eeGeneral.stickMode&1)) == x) && (x<4))

uint8_t simulatorDialog::IS_THROTTLE( uint8_t x)
{
	if ( g_model.modelVersion >= 2 )
	{
		return ((x) == 2) ;
	}
	return (((2-(g_eeGeneral.stickMode&1)) == x) && (x<4)) ;
}

#define GET_DR_STATE(x) (!getSwitch(g_model.expoData[x].drSw1,0) ?   \
    DR_HIGH :                                  \
    !getSwitch(g_model.expoData[x].drSw2,0)?   \
    DR_MID : DR_LOW);

extern int GlobalModified ;
extern EEGeneral Sim_g ;
extern int GeneralDataValid ;
extern ModelData Sim_m ;
extern int ModelDataValid ;

uint8_t Last_switch[NUM_CSW+EXTRA_CSW] ;

simulatorDialog::simulatorDialog( QWidget *parent) :
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
		port = NULL ;

    memset(&chanOut,0,sizeof(chanOut));
    memset(&calibratedStick,0,sizeof(calibratedStick));
    memset(&g_ppmIns,0,sizeof(g_ppmIns));
    memset(&ex_chans,0,sizeof(ex_chans));
    memset(&fade,0,sizeof(ex_chans));
    memset(&trim,0,sizeof(trim));
//    memset(&internalChans,0,sizeof(internalChans));

    memset(&sDelay,0,sizeof(sDelay));
    memset(&act,0,sizeof(act));

    memset(&anas,0,sizeof(anas));
//    memset(&chans,0,sizeof(chans));

    memset(&swOn,0,sizeof(swOn));

    memset(&CsTimer,0,sizeof(CsTimer));

    trimptr[0] = &trim[0] ;
    trimptr[1] = &trim[1] ;
    trimptr[2] = &trim[2] ;
    trimptr[3] = &trim[3] ;

		fadeRate = 0 ;
		fadePhases = 0 ;

		for ( int i = 0 ; i < MAX_PHASES+1 ; i += 1 )
		{
			fadeScale[i] = 0 ;
		}
    fadeScale[0] = 25600 ;

    setupSticks();
		timer = 0 ;
    CalcScaleNest = 0 ;

		ui->SEleSlider->setValue(0) ;

		QList<QextPortInfo> ports = QextSerialEnumerator::getPorts() ;
    ui->serialPortCB->clear() ;
	  foreach (QextPortInfo info, ports)
		{
			if ( info.portName.length() )
			{
	  		ui->serialPortCB->addItem(info.portName) ;
			}
		}
//    setupTimer();
   	if(g_eeGeneral.stickMode & 1)
   	{
//   		  nodeLeft->setCenteringY(false);   //mode 1,3 -> THR on left
//   		  ui->holdLeftY->setChecked(true);
   	}
   	else
   	{
//   		  nodeRight->setCenteringY(false);   //mode 1,3 -> THR on right
//   		  ui->holdRightY->setChecked(true);
   	}
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
	if ( timer )
	{
    timer->stop() ;
    delete timer ;
	}
	timer = 0 ;
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
  if (timer == 0)
  {
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timerEvent()));
  }
  getValues();
	CurrentPhase = getFlightPhase() ;
  perOut(true,0);
  timer->start(10);
}

void simulatorDialog::setCsVisibles()
{
  ui->labelCSW_13->setVisible( ee_type ) ;
  ui->labelCSW_14->setVisible( ee_type ) ;
  ui->labelCSW_15->setVisible( ee_type ) ;
  ui->labelCSW_16->setVisible( ee_type ) ;
  ui->labelCSW_17->setVisible( ee_type ) ;
  ui->labelCSW_18->setVisible( ee_type ) ;

  ui->cswitch_13->setVisible( ee_type ) ;
  ui->cswitch_14->setVisible( ee_type ) ;
  ui->cswitch_15->setVisible( ee_type ) ;
  ui->cswitch_16->setVisible( ee_type ) ;
  ui->cswitch_17->setVisible( ee_type ) ;
  ui->cswitch_18->setVisible( ee_type ) ;

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
  	  	memcpy(&g_model,&Sim_m,sizeof(ModelData));
				ModelDataValid = 0 ;
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

    	ui->trimHLeft->setValue( g_model.trim[(g_eeGeneral.stickMode>2)   ? 3 : 0]);  // mode=(0 || 1) -> rud trim else -> ail trim
    	ui->trimVLeft->setValue( g_model.trim[(g_eeGeneral.stickMode & 1) ? 2 : 1]);  // mode=(0 || 2) -> thr trim else -> ele trim
    	ui->trimVRight->setValue(g_model.trim[(g_eeGeneral.stickMode & 1) ? 1 : 2]);  // mode=(0 || 2) -> ele trim else -> thr trim
    	ui->trimHRight->setValue(g_model.trim[(g_eeGeneral.stickMode>2)   ? 0 : 3]);  // mode=(0 || 1) -> ail trim else -> rud trim
			GlobalModified = 0 ;

			setCsVisibles() ;

		}

    getValues();

    perOutPhase(false,0);

    setValues();
    centerSticks();

    timerTick();
    //    if(s_timerState != TMR_OFF)
#ifndef V2
    setWindowTitle(modelName + QString(" - Timer: (%3, %4) %1:%2")
                   .arg(abs(-s_timerVal[0])/60, 2, 10, QChar('0'))
                   .arg(abs(-s_timerVal[0])%60, 2, 10, QChar('0'))
                   .arg(getTimerMode(g_model.tmrMode, g_model.modelVersion))
                   .arg(g_model.tmrDir ? "Count Up" : "Count Down"));
#endif

		ui->Timer1->setText(QString("%1:%2").arg(abs(-s_timerVal[0])/60, 2, 10, QChar('0'))
                   .arg(abs(-s_timerVal[0])%60, 2, 10, QChar('0'))) ;
		ui->Timer2->setText(QString("%1:%2").arg(abs(-s_timerVal[1])/60, 2, 10, QChar('0'))
                   .arg(abs(-s_timerVal[1])%60, 2, 10, QChar('0'))) ;
    if(beepVal)
    {
        beepVal = 0;
        QApplication::beep();
    }


#define CBEEP_ON  "QLabel { background-color: #FF364E }"
#define CBEEP_OFF "QLabel { }"

    ui->label_beep->setStyleSheet(beepShow ? CBEEP_ON : CBEEP_OFF);
    if(beepShow) beepShow--;

		if ( ++one_sec_precount >= 10 )
		{
			one_sec_precount -= 10 ;
			// 0.1 second has elapsed			
			for ( i = 0 ; i < NUM_CSW ; i += 1 )
			{
#ifndef V2
        CSwData *cs = &g_model.customSw[i];
#else
        CxSwData *cs = &g_model.customSw[i];
#endif
        uint8_t cstate = CS_STATE(cs->func, g_model.modelVersion);

    		if(cstate == CS_TIMER)
				{
					int16_t y ;
					y = CsTimer[i] ;
					if ( y == 0 )
					{
						int8_t z ;
						z = cs->v1 ;
						if ( z >= 0 )
						{
							z = -z-1 ;
							y = z * 10 ;					
						}
						else
						{
							y = z ;
						}
					}
					else if ( y < 0 )
					{
						if ( ++y == 0 )
						{
							int8_t z ;
							z = cs->v2 ;
							if ( z >= 0 )
							{
								z += 1 ;
								y = z * 10 ;
							}
							else
							{
								y = -z-1 ;
							}
						}
					}
					else  // if ( CsTimer[i] > 0 )
					{
						y -= 1 ;
					}
					if ( cs->andsw )
					{
						int8_t x ;
						x = cs->andsw ;
						if ( x > 8 )
						{
							x += 1 ;
						}
	      	  if (getSwitch( x, 0, 0) == 0 )
					  {
							y = -1 ;
						}	
					}
					CsTimer[i] = y ;
				}
			
  			if ( g_model.modelVersion >= 3 )
				{
			
					if ( cs->func == CS_LATCH )
					{
		  		  if (getSwitch( cs->v1, 0, 0) )
						{
							Last_switch[i] = 1 ;
						}
						else
						{
					    if (getSwitch( cs->v2, 0, 0) )
							{
								Last_switch[i] = 0 ;
							}
						}
					}
					if ( cs->func == CS_FLIP )
					{
		  		  if (getSwitch( cs->v1, 0, 0) )
						{
							if ( ( Last_switch[i] & 2 ) == 0 )
							{
								// Clock it!
					      if (getSwitch( cs->v2, 0, 0) )
								{
									Last_switch[i] = 3 ;
								}
								else
								{
									Last_switch[i] = 2 ;
								}
							}
						}
						else
						{
							Last_switch[i] &= ~2 ;
						}
					}
			  }
			}

			if ( ee_type )
			{
			
#ifndef V2
        for ( i = NUM_CSW ; i < NUM_CSW+EXTRA_CSW ; i += 1 )
				{
    			CxSwData *cs = &g_model.xcustomSw[i-NUM_CSW];
    	
          uint8_t cstate = CS_STATE(cs->func, g_model.modelVersion);

    			if(cstate == CS_TIMER)
					{
						int16_t y ;
						y = CsTimer[i] ;
						if ( y == 0 )
						{
							int8_t z ;
							z = cs->v1 ;
							if ( z >= 0 )
							{
								z = -z-1 ;
								y = z * 10 ;					
							}
							else
							{
								y = z ;
							}
						}
						else if ( y < 0 )
						{
							if ( ++y == 0 )
							{
								int8_t z ;
								z = cs->v2 ;
								if ( z >= 0 )
								{
									z += 1 ;
									y = z * 10 - 1  ;
								}
								else
								{
									y = -z-1 ;
								}
							}
						}
						else  // if ( CsTimer[i] > 0 )
						{
							y -= 1 ;
						}
						if ( cs->andsw )
						{
							int8_t x ;
							x = cs->andsw ;
							if ( x > 8 )
							{
								x += 1 ;
							}
							if ( x < -8 )
							{
								x -= 1 ;
							}
							if ( x > 9+NUM_CSW+EXTRA_CSW )
							{
								x = 9 ;			// Tag TRN on the end, keep EEPROM values
							}
							if ( x < -(9+NUM_CSW+EXTRA_CSW) )
							{
								x = -9 ;			// Tag TRN on the end, keep EEPROM values
							}
	  		      if (getSwitch( x, 0, 0) == 0 )
						  {
								y = -1 ;
							}	
						}
						CsTimer[i] = y ;
					}
  				if ( g_model.modelVersion >= 3 )
					{
			
						if ( cs->func == CS_LATCH )
						{
		  			  if (getSwitch( cs->v1, 0, 0) )
							{
								Last_switch[i] = 1 ;
							}
							else
							{
						    if (getSwitch( cs->v2, 0, 0) )
								{
									Last_switch[i] = 0 ;
								}
							}
						}
						if ( cs->func == CS_FLIP )
						{
		  			  if (getSwitch( cs->v1, 0, 0) )
							{
								if ( ( Last_switch[i] & 2 ) == 0 )
								{
									// Clock it!
						      if (getSwitch( cs->v2, 0, 0) )
									{
										Last_switch[i] = 3 ;
									}
									else
									{
										Last_switch[i] = 2 ;
									}
								}
							}
							else
							{
								Last_switch[i] &= ~2 ;
							}
						}
			  	}
				}
#endif
			} 
		}
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

//				serialTimer = 0 ;
//  			uint8_t serialCmd[24] = {0,0,0};
//  			uint8_t *p = serialCmd ;

//  			  // Send current values to serial
//  			if ( port )
//  			{
//  			  if (port->isOpen())
//  			  {
//				    for (int i=0; i<=7; i++)
//						{
//							int16_t x = chanOut[i] / 2 ;
//  				  	uint chval = x + 1500 ;
//  			  		*p++ = i; // Channel
//				    	*p++ = (chval >> 8) & 0xFF; // 2nd byte of value
//  				  	*p++ = chval & 0xFF; // 1st byte of value
//  			  	}
// 			  		port->write( QByteArray::fromRawData ( ( char *)serialCmd, 24 ), 24 );
//  			  }
//  			}
			}
		} 

}

void simulatorDialog::centerSticks()
{
    if(ui->leftStick->scene()) nodeLeft->stepToCenter();
    if(ui->rightStick->scene()) nodeRight->stepToCenter();
}

void simulatorDialog::setType( uint8_t type )
{
	ee_type = type ;
	setCsVisibles() ;
}

void simulatorDialog::configSwitches()
{
	if ( g_eeGeneral.switchMapping & USE_ELE_3POS )
	{
		ui->SAwidget->show() ;
		ui->switchELE->hide() ;
	}
	else
	{
//		ui->SEleSlider->setMaximum( 2 ) ;
//		ui->labelSA->setText("ELE") ;
		ui->switchELE->show() ;
		ui->SAwidget->hide() ;
	}
	if ( g_eeGeneral.switchMapping & USE_AIL_3POS )
	{
		ui->SBwidget->show() ;
		ui->switchAIL->hide() ;
	}
	else
	{
//		ui->SAilSlider->setMaximum( 2 ) ;
//		ui->labelSB->setText("AIL") ;
		ui->switchAIL->show() ;
		ui->SBwidget->hide() ;
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

}



void simulatorDialog::loadParams(const EEGeneral gg, const ModelData gm)
{
    memcpy(&g_eeGeneral,&gg,sizeof(EEGeneral));
    memcpy(&g_model,&gm,sizeof(ModelData));

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

		CurrentPhase = getFlightPhase() ;

		ui->trimHLeft->setValue( getTrimValue( CurrentPhase, 0 ));  // mode=(0 || 1) -> rud trim else -> ail trim
    ui->trimVLeft->setValue( getTrimValue( CurrentPhase, 1 ));  // mode=(0 || 2) -> thr trim else -> ele trim
    ui->trimVRight->setValue(getTrimValue( CurrentPhase, 2 ));  // mode=(0 || 2) -> ele trim else -> thr trim
    ui->trimHRight->setValue(getTrimValue( CurrentPhase, 3 ));  // mode=(0 || 1) -> ail trim else -> rud trim

    beepVal = 0;
    beepShow = 0;
    bpanaCenter = 0;
    g_tmr10ms = 0;

    s_timeCumTot = 0;
    s_timeCumAbs = 0;
    s_timeCumSw[0] = 0;
    s_timeCumSw[1] = 0;
    s_timeCumThr[0] = 0;
    s_timeCumThr[1] = 0;
    s_timeCum16ThrP[0] = 0;
    s_timeCum16ThrP[1] = 0;
    s_timerState[0] = 0;
    s_timerState[1] = 0;
    beepAgain = 0;
    g_LightOffCounter = 0;
    s_timerVal[0] = 0;
    s_timerVal[1] = 0;
    s_time[0] = 0;
    s_time[1] = 0;
    s_cnt = 0;
    s_sum = 0;
    sw_toggled[0] = 0;
    sw_toggled[1] = 0;

		configSwitches() ;
    setupTimer();
		GlobalModified = 0 ;
}


uint32_t simulatorDialog::getFlightPhase()
{
	uint32_t i ;
  for ( i = 0 ; i < MAX_PHASES ; i += 1 )
	{
#ifndef V2
    PhaseData *phase = &g_model.phaseData[i];
#else
    V2PhaseData *phase = &g_model.phaseData[i];
#endif
    if ( phase->swtch && getSwitch( phase->swtch, 0 ) )
		{
      return i + 1 ;
    }
  }
  return 0 ;
}

int16_t simulatorDialog::getRawTrimValue( uint8_t phase, uint8_t idx )
{
	if ( phase )
	{
		return g_model.phaseData[phase-1].trim[idx] + TRIM_EXTENDED_MAX + 1 ;
	}	
	else
	{
		return *trimptr[idx] ;
	}
}

uint32_t simulatorDialog::getTrimFlightPhase( uint8_t phase, uint8_t idx )
{
  for ( uint32_t i=0 ; i<MAX_PHASES ; i += 1 )
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
  	g_model.phaseData[phase-1].trim[idx] = trim - ( TRIM_EXTENDED_MAX + 1 ) ;
	}
	else
	{
    if(trim < -125 || trim > 125)
		{
			trim = ( trim > 0 ) ? 125 : -125 ;
		}	
   	*trimptr[idx] = trim ;
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
    calibratedStick[6] = ui->dialP_3->value();

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
		// ToDo, test for trim inputs here
		if ( g_model.gvars[i].gvsource )
		{
			int value ;
      value = 0 ;
			uint8_t src = g_model.gvars[i].gvsource ;
			if ( src <= 4 )
			{
				uint32_t y ;
				y = g_model.gvars[i].gvsource - 1 ;

//				uint32_t phaseNo = getTrimFlightPhase( CurrentPhase, y ) ;
				value = getTrimValue( CurrentPhase, y ) ;
				 
			}
			else if ( src == 5 )	// REN
			{
				//g_model.gvars[i].gvar = RotaryControl ;
			}
			else if ( src <= 9 )	// Stick
			{
        value = calibratedStick[CONVERT_MODE(src-5,g_model.modelVersion,g_eeGeneral.stickMode)-1] / 8 ;
			}
			else if ( src <= 12 )	// Pot
			{
				uint32_t y ;
				y = src - 6 ;

//				y = adjustMode( y ) ;
				
				value = calibratedStick[ y ] / 8 ;
			}
			else if ( src <= 28 )	// Chans
			{
				value = ex_chans[g_model.gvars[i].gvsource-13] / 10 ;
			}
			else
			{
static uint8_t GvLastSw[6] ;
				uint8_t j = ( src - 29 ) * 2 ;
				uint8_t sw0 = getSwitch(DSW_SW1+j,0) ;
				value = g_model.gvars[i].gvar ;
				if ( sw0 & !GvLastSw[j] )
				{
        	value += 1 ;
				}
				GvLastSw[j] = sw0 ;
				sw0 = getSwitch(DSW_SW2+j,0) ;
				if ( sw0 )
				{
        	value = 0 ;
				}
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

    ui->labelCSW_1->setStyleSheet(getSwitch(DSW_SW1,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_2->setStyleSheet(getSwitch(DSW_SW2,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_3->setStyleSheet(getSwitch(DSW_SW3,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_4->setStyleSheet(getSwitch(DSW_SW4,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_5->setStyleSheet(getSwitch(DSW_SW5,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_6->setStyleSheet(getSwitch(DSW_SW6,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_7->setStyleSheet(getSwitch(DSW_SW7,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_8->setStyleSheet(getSwitch(DSW_SW8,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_9->setStyleSheet(getSwitch(DSW_SW9,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_10->setStyleSheet(getSwitch(DSW_SWA,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_11->setStyleSheet(getSwitch(DSW_SWB,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_12->setStyleSheet(getSwitch(DSW_SWC,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_13->setStyleSheet(getSwitch(DSW_SWD,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_14->setStyleSheet(getSwitch(DSW_SWE,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_15->setStyleSheet(getSwitch(DSW_SWF,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_16->setStyleSheet(getSwitch(DSW_SWG,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_17->setStyleSheet(getSwitch(DSW_SWH,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_18->setStyleSheet(getSwitch(DSW_SWI,0)  ? CSWITCH_ON : CSWITCH_OFF);
		
#define CRED  "QSlider::handle:horizontal:disabled { background: #CC0000;border: 1px solid #aaa;border-radius: 4px; }"
#define CBLUE "QSlider::handle:horizontal:disabled { background: #0000CC;border: 1px solid #aaa;border-radius: 4px; }"
	int onoff[16] ;
	int i ;
	for ( i = 0 ; i < 16 ; i += 1 )
	{
		onoff[i] = 0 ;
    if ( ( g_model.protocol == PROTO_PPM ) || ( g_model.protocol == PROTO_PPM16 ) )
		{
			if ( i >= g_model.ppmStart )
			{
				if ( i < g_model.ppmStart + g_model.ppmNCH*2+8 )
				{
					onoff[i] = 1 ;				
				}
			}
		}
		else if ( g_model.protocol == PROTO_PXX )
		{
			if ( i >= g_model.ppmStart )
			{
				if ( i < g_model.ppmStart + 8 )
				{
					onoff[i] = 1 ;				
				}
			}
		}
	}
  ui->chnout_1->setStyleSheet( onoff[0] ? CRED : CBLUE ) ;
  ui->chnout_2->setStyleSheet( onoff[1] ? CRED : CBLUE ) ;
  ui->chnout_3->setStyleSheet( onoff[2] ? CRED : CBLUE ) ;
  ui->chnout_4->setStyleSheet( onoff[3] ? CRED : CBLUE ) ;
  ui->chnout_5->setStyleSheet( onoff[4] ? CRED : CBLUE ) ;
  ui->chnout_6->setStyleSheet( onoff[5] ? CRED : CBLUE ) ;
  ui->chnout_7->setStyleSheet( onoff[6] ? CRED : CBLUE ) ;
  ui->chnout_8->setStyleSheet( onoff[7] ? CRED : CBLUE ) ;
  ui->chnout_9->setStyleSheet( onoff[8] ? CRED : CBLUE ) ;
  ui->chnout_10->setStyleSheet( onoff[9] ? CRED : CBLUE ) ;
  ui->chnout_11->setStyleSheet( onoff[10] ? CRED : CBLUE ) ;
  ui->chnout_12->setStyleSheet( onoff[11] ? CRED : CBLUE ) ;
  ui->chnout_13->setStyleSheet( onoff[12] ? CRED : CBLUE ) ;
  ui->chnout_14->setStyleSheet( onoff[13] ? CRED : CBLUE ) ;
  ui->chnout_15->setStyleSheet( onoff[14] ? CRED : CBLUE ) ;
  ui->chnout_16->setStyleSheet( onoff[14] ? CRED : CBLUE ) ;
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


inline qint16 calc100toRESX(qint8 x)
{
    return (qint16)x*10 + x/4 - x/64;
}

inline qint16 calc1000toRESX(qint16 x)
{
    return x + x/32 - x/128 + x/512;
}

bool simulatorDialog::hwKeyState(int key)
{
  switch (key)
  {
    case (HSW_ThrCt):   return ui->switchTHR->isChecked(); break;
    case (HSW_RuddDR):  return ui->switchRUD->isChecked(); break;
    case (HSW_ElevDR):  return ui->switchELE->isChecked(); break;
    case (HSW_ID0):     return ui->switchID0->isChecked(); break;
    case (HSW_ID1):     return ui->switchID1->isChecked(); break;
    case (HSW_ID2):     return ui->switchID2->isChecked(); break;
    case (HSW_AileDR):  return ui->switchAIL->isChecked(); break;
    case (HSW_Gear):    return ui->switchGEA->isChecked(); break;
    case (HSW_Trainer): return ui->switchTRN->isDown(); break;
			
//		case HSW_Thr3pos0	:	return ui->SEslider->value() == 0 ; break ;
//		case HSW_Thr3pos1	:	return ui->SEslider->value() == 1 ; break ;
//		case HSW_Thr3pos2	:	return ui->SEslider->value() == 2 ; break ;
//		case HSW_Rud3pos0	:	return ui->SEslider->value() == 0 ; break ;
//		case HSW_Rud3pos1	:	return ui->SEslider->value() == 1 ; break ;
//		case HSW_Rud3pos2	:	return ui->SEslider->value() == 2 ; break ;
    case HSW_Ele3pos0	:	return ui->SEleSlider->value() == 0 ; break ;
    case HSW_Ele3pos1	:	return ui->SEleSlider->value() == 1 ; break ;
    case HSW_Ele3pos2	:	return ui->SEleSlider->value() == 2 ; break ;
    case HSW_Ail3pos0	:	return ui->SAilSlider->value() == 0 ; break ;
    case HSW_Ail3pos1	:	return ui->SAilSlider->value() == 1 ; break ;
    case HSW_Ail3pos2	:	return ui->SAilSlider->value() == 2 ; break ;
//		case HSW_Gear3pos0 :	return ui->SCslider->value() == 0 ; break ;
//		case HSW_Gear3pos1 :	return ui->SCslider->value() == 1 ; break ;
//		case HSW_Gear3pos2 :	return ui->SCslider->value() == 2 ; break ;
//		case HSW_Ele6pos0 :	return ui->SAslider->value() == 0 ; break ;
//		case HSW_Ele6pos1 :	return ui->SAslider->value() == 1 ; break ;
//		case HSW_Ele6pos2 :	return ui->SAslider->value() == 2 ; break ;
//		case HSW_Ele6pos3 :	return ui->SAslider->value() == 3 ; break ;
//		case HSW_Ele6pos4 :	return ui->SAslider->value() == 4 ; break ;
//		case HSW_Ele6pos5 :	return ui->SAslider->value() == 5 ; break ;
    case HSW_Pb1	:	return ui->switchPB1->isDown() ; break ;
    case HSW_Pb2	:	return ui->switchPB2->isDown() ; break ;
//    case HSW_Pb3	:	return ui->switchPB3->isDown() ; break ;
//    case HSW_Pb4	:	return ui->switchPB4->isDown() ; break ;
    default:
      return keyState( (EnumKeys) key ) ;
    break;
	}
}

bool simulatorDialog::keyState(EnumKeys key)
{
    switch (key)
    {
    case (SW_ThrCt):   return ui->switchTHR->isChecked(); break;
    case (SW_RuddDR):  return ui->switchRUD->isChecked(); break;
    case (SW_ElevDR):  return ui->switchELE->isChecked(); break;
    case (SW_ID0):     return ui->switchID0->isChecked(); break;
    case (SW_ID1):     return ui->switchID1->isChecked(); break;
    case (SW_ID2):     return ui->switchID2->isChecked(); break;
    case (SW_AileDR):  return ui->switchAIL->isChecked(); break;
    case (SW_Gear):    return ui->switchGEA->isChecked(); break;
    case (SW_Trainer): return ui->switchTRN->isDown(); break;
    default:
        return false;
        break;
    }
}

qint16 simulatorDialog::getValue(qint8 i)
{
	qint8 j ;
    if(i<7) return calibratedStick[i];//-512..512
    if(i<PPM_BASE) return 0 ;
    else if(i<CHOUT_BASE) return g_ppmIns[i-PPM_BASE];// - g_eeGeneral.ppmInCalib[i-PPM_BASE];
    else if(i<CHOUT_BASE+NUM_CHNOUT) return ex_chans[i-CHOUT_BASE];
		else
		{
			j = i-CHOUT_BASE-NUM_CHNOUT - 25 ;
			if ( ( j >= 0 ) && ( j < 7 ) )
			{
        return g_model.gvars[j].gvar ;
			}
			if ( ( j >= 12 ) && ( j <= 15 ) )
			{
        return calc_scaler( j - 12 ) ;
			}
		}
		return 0;
}


bool simulatorDialog::getSwitch(int swtch, bool nc, qint8 level)
{
    bool ret_value ;
    uint8_t cs_index ;
		int limit = ee_type ? MAX_DRSWITCH+EXTRA_CSW : MAX_DRSWITCH ;
    
		if(level>5) return false; //prevent recursive loop going too deep

		if ( swtch == 0 )
		{
			return  nc ;
		}

		if ( ee_type )
		{
    	switch(swtch)
			{
    	case  MAX_DRSWITCH+EXTRA_CSW: return  true;
    	case -MAX_DRSWITCH-EXTRA_CSW: return  false;
    	}
		}
		else
		{
    	switch(swtch)
			{
    	case  MAX_DRSWITCH: return  true;
    	case -MAX_DRSWITCH: return  false;
    	}
		}

		if ( abs(swtch) > limit )
		{
			uint8_t value = hwKeyState( abs(swtch) ) ;
			if ( swtch > 0 )
			{
				return value ;
			}
			else
			{
				return !value ;
			}
		}

    uint8_t dir = swtch>0;
		
		if(abs(swtch)<(PHY_SWITCH))
		{
        if(!dir) return ! keyState((EnumKeys)(SW_BASE-swtch-1));
        return            keyState((EnumKeys)(SW_BASE+swtch-1));
    }
		
    //custom switch, Issue 78
    //use putsChnRaw
    //input -> 1..4 -> sticks,  5..8 pots
    //MAX,FULL - disregard
    //ppm
    cs_index = abs(swtch)-(PHY_SWITCH);

#ifndef V2
    if ( ee_type )
		{
			if ( cs_index >= NUM_CSW )
			{
				CxSwData &cs = g_model.xcustomSw[cs_index-NUM_CSW];

    		if(!cs.func) return false;
		
    		if ( level>4 )
    		{
    		  ret_value = Last_switch[cs_index] & 1 ;
    		  return swtch>0 ? ret_value : !ret_value ;
    		}

    		int8_t a = cs.v1;
    		int8_t b = cs.v2;
    		int16_t x = 0;
    		int16_t y = 0;

    		// init values only if needed
        uint8_t s = CS_STATE(cs.func, g_model.modelVersion);
    		if(s == CS_VOFS)
    		{
    		    x = getValue(cs.v1-1);
          if (cs.v1 > CHOUT_BASE+NUM_CHNOUT)
					{
            uint8_t idx = cs.v1-CHOUT_BASE-NUM_CHNOUT-1 ;
            y = convertTelemConstant( idx, cs.v2, &g_model ) ;
//						valid = telemItemValid( idx ) ;
					}
    	    else
    		    y = calc100toRESX(cs.v2);
    		}
    		else if(s == CS_VCOMP)
    		{
    		    x = getValue(cs.v1-1);
    		    y = getValue(cs.v2-1);
    		}

    		switch (cs.func) {
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

    		case (CS_AND):
					x = getSwitch(a,0,level+1) ;
					y = getSwitch(b,0,level+1) ;
    		    ret_value = x && y ;
    		    break;
    		case (CS_OR):
    		    ret_value = (getSwitch(a,0,level+1) || getSwitch(b,0,level+1));
    		    break;
    		case (CS_XOR):
    		    ret_value = (getSwitch(a,0,level+1) ^ getSwitch(b,0,level+1));
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
    		case (CS_EGREATER):	// CS_LATCH
    		    if ( g_model.modelVersion < 3 )
						{
							ret_value = (x>=y);
						}
						else
						{
							ret_value = Last_switch[cs_index] & 1 ;
						}
    		    break;
    		case (CS_ELESS):		// CS_FLIP
    		    if ( g_model.modelVersion < 3 )
						{
    		    	ret_value = (x<=y);
						}
						else
						{
							ret_value = Last_switch[cs_index] & 1 ;
						}
    		    break;
    		case (CS_TIME):
    		    ret_value = CsTimer[cs_index] >= 0 ;
    		    break;
    		default:
    		    return false;
    		    break;
    		}
				if ( ret_value )
				{
					if ( cs.andsw )
					{
						int8_t x ;
						x = cs.andsw ;
						if ( ( x > 8 ) && ( x <= 9+NUM_CSW+EXTRA_CSW ) )
						{
							x += 1 ;
						}
						if ( ( x < -8 ) && ( x >= -(9+NUM_CSW+EXTRA_CSW) ) )
						{
							x -= 1 ;
						}
						if ( x == 9+NUM_CSW+EXTRA_CSW+1 )
						{
							x = 9 ;			// Tag TRN on the end, keep EEPROM values
						}
						if ( x == -(9+NUM_CSW+EXTRA_CSW+1) )
						{
							x = -9 ;			// Tag TRN on the end, keep EEPROM values
						}
    		    ret_value = getSwitch( x, 0, level+1) ;
					}
				}
    		if ( g_model.modelVersion >= 3 )
				{
          if ( cs.func < CS_LATCH )
					{
						Last_switch[cs_index] = ret_value ;
					}
				}
				else
				{
					Last_switch[cs_index] = ret_value ;
				}
				return swtch>0 ? ret_value : !ret_value ;
			}
		}
#endif

#ifndef V2
    CSwData &cs = g_model.customSw[cs_index];
#else
    CxSwData &cs = g_model.customSw[cs_index];
#endif
    if(!cs.func) return false;
		
    if ( level>4 )
    {
      ret_value = Last_switch[cs_index] & 1 ;
      return swtch>0 ? ret_value : !ret_value ;
    }

    int8_t a = cs.v1;
    int8_t b = cs.v2;
    int16_t x = 0;
    int16_t y = 0;

    // init values only if needed
    uint8_t s = CS_STATE(cs.func, g_model.modelVersion);
    if(s == CS_VOFS)
    {
        x = getValue(cs.v1-1);
        if (cs.v1 > CHOUT_BASE+NUM_CHNOUT)
				{
          uint8_t idx = cs.v1-CHOUT_BASE-NUM_CHNOUT-1 ;
          y = convertTelemConstant( idx, cs.v2, &g_model ) ;
//						valid = telemItemValid( idx ) ;
				}
   	    else
	        y = calc100toRESX(cs.v2);
    }
    else if(s == CS_VCOMP)
    {
        x = getValue(cs.v1-1);
        y = getValue(cs.v2-1);
    }

    switch (cs.func) {
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

    case (CS_AND):
        ret_value = (getSwitch(a,0,level+1) && getSwitch(b,0,level+1));
        break;
    case (CS_OR):
        ret_value = (getSwitch(a,0,level+1) || getSwitch(b,0,level+1));
        break;
    case (CS_XOR):
        ret_value = (getSwitch(a,0,level+1) ^ getSwitch(b,0,level+1));
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
    		case (CS_EGREATER):	// CS_LATCH
    		    if ( g_model.modelVersion < 3 )
						{
							ret_value = (x>=y);
						}
						else	// CS_LATCH
						{
							ret_value = Last_switch[cs_index] & 1 ;
						}
    		    break;
    		case (CS_ELESS):		// CS_FLIP
    		    if ( g_model.modelVersion < 3 )
						{
    		    	ret_value = (x<=y);
						}
						else		// CS_FLIP
						{
							ret_value = Last_switch[cs_index] & 1 ;
						}
    		    break;
    case (CS_TIME):
        ret_value = CsTimer[cs_index] >= 0 ;
        break;
    default:
        return false;
        break;
    }
		if ( ret_value )
		{
			if ( cs.andsw )
			{
				int8_t x ;
				x = cs.andsw ;
				if ( x > 8 )
				{
					x += 1 ;
				}
        ret_value = getSwitch( x, 0, level+1) ;
			}
		}
    if ( g_model.modelVersion >= 3 )
		{
      if ( cs.func < CS_LATCH )
			{
				Last_switch[cs_index] = ret_value ;
			}
		}
		else
		{
			Last_switch[cs_index] = ret_value ;
		}
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
    bool    cv9 = idx >= MAX_CURVE5;
    int8_t *crv = cv9 ? g_model.curves9[idx-MAX_CURVE5] : g_model.curves5[idx];
    int16_t erg;

    x+=RESXu;
    if(x < 0) {
        erg = (int16_t)crv[0] * (RESX/4);
    } else if(x >= (RESX*2)) {
        erg = (int16_t)crv[(cv9 ? 8 : 4)] * (RESX/4);
    } else {
        int16_t a,dx;
        if(cv9){
            a   = (uint16_t)x / D9;
            dx  =((uint16_t)x % D9) * 2;
        } else {
            a   = (uint16_t)x / D5;
            dx  = (uint16_t)x % D5;
        }
        erg  = (int16_t)crv[a]*((D5-dx)/2) + (int16_t)crv[a+1]*(dx/2);
    }
    return erg / 25; // 100*D5/RESX;
}

static uint8_t lastSwPos[2] ;
void simulatorDialog::timerTick()
{
  int16_t val = 0;
  int16_t v ;
	uint8_t tma ;
//    if((abs(g_model.tmrMode)>1) && (abs(g_model.tmrMode)<TMR_VAROFS)) {
//        val = calibratedStick[CONVERT_MODE(abs(g_model.tmrMode)/2,g_model.modelVersion,g_eeGeneral.stickMode)-1];
//        val = (g_model.tmrMode<0 ? RESX-val : val+RESX ) / (RESX/16);  // only used for %
//    }

#ifndef V2
    int8_t tm = g_model.tmrMode;
#endif
    //    val = calibratedStick[3-1];
//    if(tm>=TMR_VAROFS) // Cxx%
//    {
//      val = chanOut[tm-TMR_VAROFS] ;
//    }
  	int8_t tmb ;
//		uint8_t switch_b ;
		uint8_t max_drswitch ;
		max_drswitch = ( ee_type ) ? MAX_DRSWITCH+EXTRA_CSW : MAX_DRSWITCH ;

  int itimer ;
  for( itimer = 0 ; itimer < 2 ; itimer += 1 )
	{
		uint8_t resetting = 0 ;
#ifndef V2
    if ( itimer == 0 )
		{
			tmb = g_model.timer1RstSw ;
		}
		else
		{
			tmb = g_model.timer2RstSw ;
		}
		if ( tmb )
		{
    	if(tmb>=(HSW_MAX))	 // toggeled switch
			{
        uint8_t swPos = getSwitch( tmb-(HSW_MAX), 0 ) ;
        if ( swPos != lastResetSwPos[itimer] )
				{
          lastResetSwPos[itimer] = swPos ;
					if ( swPos )	// Now on
					{
						resetting = 1 ;
					}
				}
			}
			else
			{
        if ( getSwitch( tmb, 0) )
				{
					resetting = 1 ;
				}
			}
		}
		if ( resetting )
		{
			if ( itimer == 0 )
			{
//				resetTimer1() 
				s_timeCumAbs=0 ;
        s_timerVal[0] = ( g_model.tmrDir ) ? 0 : g_model.tmrVal ;
  			s_timeCumThr[0]=0;
  			s_timeCumSw[0]=0;
  			s_timeCum16ThrP[0]=0;
				s_sum = 0 ;
				
			}
			else
			{
        s_timerVal[1] = ( g_model.tmr2Dir ) ? 0 : g_model.tmr2Val ;
  			s_timeCumThr[1]=0;
  			s_timeCumSw[1]=0;
  			s_timeCum16ThrP[1]=0;
//				resetTimer2() ;
			}
		}
#endif
  }
		
#ifndef V2
    tma = g_model.tmrMode ;
		tmb = g_model.tmrModeB ;


    v = 0 ;
    if(( tma > 1 ) && ( tma < TMR_VAROFS ) )
		{
 			v = calibratedStick[3-1] ;
    }
//		if ( ThrottleStickyOn )
//		{
//			v = -RESX ;
//		}
   	if(tma>=TMR_VAROFS) // Cxx%
		{
			v = chanOut[tma-TMR_VAROFS] ;
		}		
		val = ( v + RESX ) / (RESX/16) ;

    if(tmb > HSW_MAX)
		{ //toggeled switch//abs(g_model.tmrMode)<(10+MAX_DRSWITCH-1)
      if(!(sw_toggled[0] | s_sum | s_cnt | s_time[0] | lastSwPos[itimer])) lastSwPos[itimer] = 0 ;  // if initializing then init the lastSwPos
      uint8_t swPos = getSwitch( tmb - HSW_MAX, 0 ) ;
      if(swPos && !lastSwPos[itimer])  sw_toggled[0] = !sw_toggled[0];  //if switcdh is flipped first time -> change counter state
      lastSwPos[itimer] = swPos;
    }
		else
		{
		 	if ( tmb )
			{
        sw_toggled[0] = getSwitch( tmb ,0) ; //normal switch
			}
			else
			{
				sw_toggled[0] = 1 ;
			}
		}
		
		if ( sw_toggled[0] == 0 )
		{
			val = 0 ;		// Stop TH%
		}
#endif

    if((uint16_t)( g_tmr10ms-s_time[0])>99)
		{
      s_time[0] += 100 ;
//    	if(abs(tm)<TMR_VAROFS) sw_toggled[0] = false; // not switch - sw timer off
//    	else if(abs(tm)<(TMR_VAROFS+max_drswitch-1)) sw_toggled[0] = getSwitch((tm>0 ? tm-(TMR_VAROFS-1) : tm+(TMR_VAROFS-1)) ,0); //normal switch

    	s_timeCumTot               += 1;
    	s_timeCumAbs               += 1;
    	if(val) s_timeCumThr[0]    += 1;
//    	if(abs(g_model.tmrMode)==TMRMODE_ABS) sw_toggled[0] = true ;
    
			if(sw_toggled[0] ) s_timeCumSw[0] += 1;
    	s_timeCum16ThrP[0]            += val/2;

#ifndef V2
      s_timerVal[0] = g_model.tmrVal;
//    	uint8_t tmrM = abs(g_model.tmrMode);
      uint16_t subtrahend = 0 ;
      if(tma==TMRMODE_NONE)// s_timerState[0] = TMR_OFF;
			{
			}	
    	else if(tma==TMRMODE_ABS)
			{
				if ( tmb == 0 ) subtrahend = s_timeCumAbs ;
    		else subtrahend = s_timeCumSw[0] ; //switch
    	}
			else if(tma<TMR_VAROFS-1) subtrahend = s_timeCumThr[0];// stick% : stick
    	else subtrahend = s_timeCum16ThrP[0]/16 ; //switch
			s_timerVal[0] -= subtrahend ;

    	switch(s_timerState[0])
    	{
    	case TMR_OFF:
    	    if(g_model.tmrMode != TMRMODE_NONE) s_timerState[0]=TMR_RUNNING;
    	    break;
    	case TMR_RUNNING:
    	    if(s_timerVal[0]<=0 && g_model.tmrVal) s_timerState[0]=TMR_BEEPING;
    	    break;
    	case TMR_BEEPING:
    	    if(s_timerVal[0] <= -MAX_ALERT_TIME)   s_timerState[0]=TMR_STOPPED;
    	    if(g_model.tmrVal == 0)             s_timerState[0]=TMR_RUNNING;
    	    break;
    	case TMR_STOPPED:
    	    break;
    	}
#endif

    	static int16_t last_tmr;

    	if(last_tmr != s_timerVal[0])  //beep only if seconds advance
    	{
    	    if(s_timerState[0]==TMR_RUNNING)
    	    {
#ifndef V2
              if(g_eeGeneral.preBeep && g_model.tmrVal) // beep when 30, 15, 10, 5,4,3,2,1 seconds remaining
    	        {
    	            if(s_timerVal[0]==30) {beepAgain=2; beepWarn2();} //beep three times
    	            if(s_timerVal[0]==20) {beepAgain=1; beepWarn2();} //beep two times
    	            if(s_timerVal[0]==10)  beepWarn2();
    	            if(s_timerVal[0]<= 3)  beepWarn2();

    	            if(g_eeGeneral.flashBeep && (s_timerVal[0]==30 || s_timerVal[0]==20 || s_timerVal[0]==10 || s_timerVal[0]<=3))
    	                g_LightOffCounter = FLASH_DURATION;
    	        }

    	        if(g_eeGeneral.minuteBeep && (((g_model.tmrDir ? g_model.tmrVal-s_timerVal[0] : s_timerVal[0])%60)==0)) //short beep every minute
    	        {
    	            beepWarn2();
    	            if(g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
    	        }
#endif
          }
    	    else if(s_timerState[0]==TMR_BEEPING)
    	    {
    	        beepWarn();
    	        if(g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
    	    }
    	}
    	last_tmr = s_timerVal[0];
#ifndef V2
      if(g_model.tmrDir) s_timerVal[0] = g_model.tmrVal-s_timerVal[0]; //if counting backwards - display backwards
#endif
    }

#ifndef V2
  tm = g_model.tmr2Mode;
	tmb = g_model.tmr2ModeB ;

  if(tmb > HSW_MAX)
	{ //toggeled switch//abs(g_model.tmrMode)<(10+MAX_DRSWITCH-1)
      static uint8_t lastSwPos2;
      if(!(sw_toggled[1] | s_sum | s_cnt | s_time[1] | lastSwPos2)) lastSwPos2 = 0 ;  // if initializing then init the lastSwPos
      uint8_t swPos = getSwitch( tmb-(HSW_MAX), 0 ) ;
      if(swPos && !lastSwPos2)  sw_toggled[1] = !sw_toggled[1];  //if switcdh is flipped first time -> change counter state
      lastSwPos2 = swPos;
  }
	else
	{
		if ( tmb )
		{
      sw_toggled[1] = getSwitch( tmb ,0) ; //normal switch
		}
		else
		{
			sw_toggled[1] = 1 ;
		}
	}
		
	if ( sw_toggled[1] == 0 )
	{
		val = 0 ;		// Stop TH%
	}

  if((uint16_t)( g_tmr10ms-s_time[1])<100)
	{
		return  ;
	}
  s_time[1] += 100 ;
  
//	if(abs(tm)<TMR_VAROFS) sw_toggled[1] = false; // not switch - sw timer off
//  else if(abs(tm)<(TMR_VAROFS+max_drswitch-1)) sw_toggled[1] = getSwitch((tm>0 ? tm-(TMR_VAROFS-1) : tm+(TMR_VAROFS-1)) ,0); //normal switch

  if(val) s_timeCumThr[1]    += 1;
//  if(abs(g_model.tmr2Mode)==TMRMODE_ABS) sw_toggled[1] = true ;
    
	if(sw_toggled[1]) s_timeCumSw[1] += 1;
  s_timeCum16ThrP[1]            += val/2;
#endif

#ifndef V2
  s_timerVal[1] = g_model.tmr2Val;
  uint8_t tmrM = abs(g_model.tmr2Mode);
  if(tmrM==TMRMODE_NONE) s_timerState[1] = TMR_OFF;
  else if(tmrM==TMRMODE_ABS)
	{
		if ( tmb == 0 ) s_timerVal[1] -= s_timeCumAbs ;
    else s_timerVal[1] -= s_timeCumSw[1] ; //switch
  }
	else if(tmrM<TMR_VAROFS) s_timerVal[1] -= (tmrM&1) ? s_timeCum16ThrP[1]/16 : s_timeCumThr[1];// stick% : stick
  else s_timerVal[1] -= s_timeCumSw[1]; //switch
  if(g_model.tmr2Dir) s_timerVal[1] = g_model.tmr2Val-s_timerVal[1]; //if counting backwards - display backwards
#endif

}

// GVARS helpers

int8_t simulatorDialog::REG100_100(int8_t x)
{
	return REG( x, -100, 100 ) ;
}


int16_t simulatorDialog::calcExpo( uint8_t channel, int16_t value )
{
  uint8_t expoDrOn = GET_DR_STATE(channel);
  uint8_t stkDir = value > 0 ? DR_RIGHT : DR_LEFT;

  if(IS_THROTTLE(channel) && g_model.thrExpo){
#if GVARS
      value  = 2*expo((value+RESX)/2,REG100_100(g_model.expoData[channel].expo[expoDrOn][DR_EXPO][DR_RIGHT]));
#else
      value  = 2*expo((value+RESX)/2,g_model.expoData[channel].expo[expoDrOn][DR_EXPO][DR_RIGHT]);
#endif                    
			stkDir = DR_RIGHT;
  }
  else
#if GVARS
      value  = expo(value,REG100_100(g_model.expoData[channel].expo[expoDrOn][DR_EXPO][stkDir]));
#else
      value  = expo(value,g_model.expoData[channel].expo[expoDrOn][DR_EXPO][stkDir]);
#endif                    

#if GVARS
  int32_t x = (int32_t)value * (REG(g_model.expoData[channel].expo[expoDrOn][DR_WEIGHT][stkDir]+100, 0, 100))/100;
#else
  int32_t x = (int32_t)value * (g_model.expoData[channel].expo[expoDrOn][DR_WEIGHT][stkDir]+100)/100;
#endif                    
  value = (int16_t)x;
  if (IS_THROTTLE(channel) && g_model.thrExpo) value -= RESX;
	return value ;
}


int8_t simulatorDialog::REG(int8_t x, int8_t min, int8_t max)
{
  int8_t result = x;
  if (x >= 126 || x <= -126)
	{
		uint8_t y ;
		y = x ;
		if ( x < 0 )
		{
      y = x + 256 ;
		}
		y -= 126 ;
    x = y ;
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
			fadeRate = (25600 / 50) / time1 ;
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
			uint16_t l_fadeScale = fadeScale[p] ;
			
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
//        uint32_t v = (calibratedStick[ele_stick]*calibratedStick[ele_stick] +
//                      calibratedStick[ail_stick]*calibratedStick[ail_stick]);
//        uint32_t q = RESX*g_model.swashRingValue/100;
//        q *= q;
//        if(v>q)
//            d = isqrt32(v);
//    }
    //===========Swash Ring================


    for(uint8_t i=0;i<7;i++)
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

        if(!(v/16)) anaCenter |= 1<<index ;//(CONVERT_MODE((i+1),g_model.modelVersion,g_eeGeneral.stickMode)-1);

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
					v = calcExpo( index, v ) ;

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
							if ( throttleReversed( &g_eeGeneral, &g_model ) )
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
				if ( throttleReversed( &g_eeGeneral, &g_model ) )
				{
					ttrim = -ttrim ;
				}
       	trimA[2] = ((int32_t)ttrim+125)*(RESX-anas[2])/(RESX) ;
			}
		}
	if ( att & FADE_FIRST )
	{
    //===========BEEP CENTER================
    anaCenter &= g_model.beepANACenter;
    if(((bpanaCenter ^ anaCenter) & anaCenter)) beepWarn1();
    bpanaCenter = anaCenter;


    calibratedStick[MIX_MAX-1]=calibratedStick[MIX_FULL-1]=1024;
    anas[MIX_MAX-1]  = RESX;     // MAX
    anas[MIX_FULL-1] = RESX;     // FULL
//		anas[MIX_3POS-1] = keyState(SW_ID0) ? -1024 : (keyState(SW_ID1) ? 0 : 1024) ;


    for(uint8_t i=0;i<NUM_PPM;i++)    anas[i+PPM_BASE]   = g_ppmIns[i];// - g_eeGeneral.ppmInCalib[i]; //add ppm channels
//    for(uint8_t i=0;i<NUM_CHNOUT;i++) anas[i+CHOUT_BASE] = chans[i]; //other mixes previous outputs
//#if GVARS
//        for(uint8_t i=0;i<MAX_GVARS;i++) anas[i+MIX_3POS] = g_model.gvars[i].gvar * 8 ;
//#endif

    //===========Swash Ring================
    if(g_model.swashRingValue)
    {
//			int8_t x ;
//			uint16_t y ;

//			x = anas[ele_stick] / 10 ;
//			y = x * x ;
//			x = anas[ail_stick] / 10 ;
//			y += x * x ;

//			uint8_t t1 = calc100toRESX(g_model.swashRingValue) / 10 ;
//			if ( y > (t1 * t1) )
//			{
//        uint16_t d = isqrt32((uint32_t) y);
//        anas[ele_stick] = (int32_t)anas[ele_stick]*t1/((int32_t)d) ;
//        anas[ail_stick] = (int32_t)anas[ail_stick]*t1/((int32_t)d) ;
				
//			}

      uint32_t v = ((int32_t)anas[ele_stick]*anas[ele_stick] + (int32_t)anas[ail_stick]*anas[ail_stick]);
		  int16_t tmp = calc100toRESX(g_model.swashRingValue) ;
      uint32_t q ;
      q =(int32_t)tmp * tmp ;
      if(v>q)
      {
        uint16_t d = isqrt32(v);
        anas[ele_stick] = (int32_t)anas[ele_stick]*tmp/((int32_t)d) ;
        anas[ail_stick] = (int32_t)anas[ail_stick]*tmp/((int32_t)d) ;
      }
    }

    //===========Swash Mix================
#define REZ_SWASH_X(x)  ((x) - (x)/8 - (x)/128 - (x)/512)   //  1024*sin(60) ~= 886
#define REZ_SWASH_Y(x)  ((x))   //  1024 => 1024

    if(g_model.swashType)
    {
        int16_t vp = anas[ele_stick]+trimA[ele_stick];
        int16_t vr = anas[ail_stick]+trimA[ail_stick];
        int16_t vc = 0;
        if(g_model.swashCollectiveSource)
            vc = anas[g_model.swashCollectiveSource-1];

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
//    memset(chans,0,sizeof(chans));        // All outputs to 0

    uint8_t mixWarning = 0;
    //========== MIXER LOOP ===============

    // Set the trim pointers back to the master set
    trimptr[0] = &trim[0] ;
    trimptr[1] = &trim[1] ;
    trimptr[2] = &trim[2] ;
    trimptr[3] = &trim[3] ;
        
    if( (throttleReversed( &g_eeGeneral, &g_model )) && (!g_model.thrTrim))
    {
			uint8_t stick = THR_STICK ;
//			if ( g_model.modelVersion >= 2 )
//			{
//				stick = 2 ;
//			}
      *trimptr[stick] *= -1;
    }
		{
			int8_t trims[4] ;
			int i ;
			int idx ;
	
      for ( i = 0 ;  i <= 3 ; i += 1 )
			{
				idx = i ;
				if ( g_eeGeneral.crosstrim )
				{
					idx = 3 - idx ;			
				}
        trims[i] = getTrimValue( CurrentPhase, idx ) ;
			}
		
      if ( g_model.modelVersion >= 2 )
			{
				uint8_t stickIndex = g_eeGeneral.stickMode*4 ;
		
				uint8_t index ;
				index = 0 ;//g_eeGeneral.crosstrim ? 3 : 0 ;
				index =  stickScramble[stickIndex+index] ;
				ui->trimHLeft->setValue( trims[index]);  // mode=(0 || 1) -> rud trim else -> ail trim
				index = 1 ;//g_eeGeneral.crosstrim ? 2 : 1 ;
				index =  stickScramble[stickIndex+index] ;
    		ui->trimVLeft->setValue( trims[index]);  // mode=(0 || 2) -> thr trim else -> ele trim
				index = 2 ;//g_eeGeneral.crosstrim ? 1 : 2 ;
				index =  stickScramble[stickIndex+index] ;
    		ui->trimVRight->setValue(trims[index]);  // mode=(0 || 2) -> ele trim else -> thr trim
				index = 3 ;//g_eeGeneral.crosstrim ? 0 : 3 ;
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

		if( (throttleReversed( &g_eeGeneral, &g_model )) && (!g_model.thrTrim))
    {
			uint8_t stick = THR_STICK ;
//			if ( g_model.modelVersion >= 2 )
//			{
//				stick = 2 ;
//			}
        *trimptr[stick] *= -1;
    }

  uint8_t mixIndex = 0 ;
  MixData *md = &g_model.mixData[0];
	for(uint8_t chanIndex=0;chanIndex<NUM_CHNOUT;chanIndex++)
	{
    qint32 thisChan = 0 ;

//    for(uint8_t mixIndex=0;mixIndex<MAX_MIXERS;mixIndex++)
		for( ; md->destCh-1 == chanIndex ; md += 1,	mixIndex += 1 ) // So continue increments
		{
//        MixData &md = g_model.mixData[mixIndex];
#if GVARS
        int8_t mixweight = REG100_100( md->weight) ;
#endif

//        if((md->destCh==0) || (md->destCh>NUM_CHNOUT)) break;

        //Notice 0 = NC switch means not used -> always on line
        int16_t v  = 0;
        uint8_t swTog;

#define DEL_MULT 256

        bool t_switch = getSwitch(md->swtch,1) ;
        if ( t_switch )
				{
					if ( md->modeControl & ( 1 << CurrentPhase ) )
					{
						t_switch = 0 ;
					}
				}
        
				uint8_t k = md->srcRaw ;
				
				//swOn[mixIndex]=false;
        if(!t_switch)
				{ // switch on?  if no switch selected => on
            swTog = swOn[mixIndex];
            swOn[mixIndex] = false;
            if (k == MIX_3POS+MAX_GVARS+1) act[mixIndex] = thisChan * DEL_MULT / 100 ;
            if( k!=MIX_FULL && k!=MIX_MAX) continue;// if not MAX or FULL - next loop
            if(md->mltpx==MLTPX_REP) continue; // if switch is off and REPLACE then off
            v = md->srcRaw==MIX_FULL ? -RESX : 0; // switch is off => FULL=-RESX
        }
        else {
            swTog = !swOn[mixIndex];
            swOn[mixIndex] = true;
            k -= 1 ;

//            v = anas[k]; //Switch is on. MAX=FULL=512 or value.
						if ( k < CHOUT_BASE )
						{
              v = anas[k]; //Switch is on. MAX=FULL=512 or value.
							if ( k < 4 )
							{
								if ( md->disableExpoDr )
								{
      		      	v = rawSticks[k]; //Switch is on. MAX=FULL=512 or value.
								}
							}
						}
						else if(k<CHOUT_BASE+NUM_CHNOUT)
						{
              if ( md->disableExpoDr )
							{
								v = chanOut[k-CHOUT_BASE] ;
							}
							else
							{
//              	if(k<CHOUT_BASE+chanIndex)
//								{
//									v = chans[k-CHOUT_BASE] / 100 ; // if we've already calculated the value - take it instead // anas[i+CHOUT_BASE] = chans[i]
//								}
//								else
								{
									v = ex_chans[k-CHOUT_BASE] ;
								}
							}
						}
						else if( k == MIX_3POS-1 )
						{
              uint8_t sw = md->sw23pos ;
							if ( sw )
							{
								sw += SW_ThrCt - KEY_MENU - 1 ;
								if ( sw >= SW_ID0 )
								{
									sw += SW_AileDR - SW_ID0 ;
								}
        				v = keyState((EnumKeys)sw) ? 1024 : -1024 ;
							}
							else
							{
        				v = keyState(SW_ID0) ? -1024 : (keyState(SW_ID1) ? 0 : 1024) ;
							}
						}
						else if ( k < MIX_3POS+MAX_GVARS )	// GVAR
						{
			        v = g_model.gvars[k-MIX_3POS].gvar * 8 ;
						}
            else if (k == MIX_3POS+MAX_GVARS)
						{
							v = thisChan / 100 ;
						}
						else
						{
							v = calc_scaler( k - (MIX_3POS+MAX_GVARS+1) ) ;
						}
            if(md->mixWarn) mixWarning |= 1<<(md->mixWarn-1); // Mix warning
//            if ( md->enableFmTrim )
//            {
//                if ( md->srcRaw <= 4 )
//                {
//                    trimptr[md->srcRaw-1] = &md->sOffset ;		// Use the value stored here for the trim
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
        //========== INPUT OFFSET ===============
//        if ( ( md->enableFmTrim == 0 ) && ( md->lateOffset == 0 ) )
#ifndef V2
				if ( md->lateOffset == 0 )
        {
#if GVARS
            if(md->sOffset) v += calc100toRESX( REG( md->sOffset, -125, 125 )	) ;
#else
            if(md->sOffset) v += calc100toRESX(md->sOffset);
#endif
        }
#endif

        //========== DELAY and PAUSE ===============
        if (md->speedUp || md->speedDown || md->delayUp || md->delayDown)  // there are delay values
        {
#ifndef V2
          uint8_t timing = g_model.mixTime ? 20 : 100 ;
#else
          uint8_t timing = 20 ;
#endif
          if(init)
            {
                act[mixIndex]=(int32_t)v*DEL_MULT;
                swTog = false;
            }
						int16_t my_delay = sDelay[mixIndex] ;
            int32_t tact = act[mixIndex] ;
            int16_t diff = v-tact/DEL_MULT;

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
//                    tact = (int32_t)anas[chanIndex+CHOUT_BASE]*DEL_MULT * 100 ;
                    tact = (int32_t)ex_chans[chanIndex]*DEL_MULT * 100 ;
#if GVARS
                    if(mixweight) tact /= mixweight ;
#else
//                    if(md->weight) tact /= md->weight;
#endif
                }
                diff = v-tact/DEL_MULT;
                if(diff) my_delay = (diff<0 ? md->delayUp :  md->delayDown) * timing ;
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
						sDelay[mixIndex] = my_delay ;

            if(diff && (md->speedUp || md->speedDown)){
                //rate = steps/sec => 32*1024/100*md->speedUp/Down
                //act[mixIndex] += diff>0 ? (32768)/((int16_t)100*md->speedUp) : -(32768)/((int16_t)100*md->speedDown);
                //-100..100 => 32768 ->  100*83886/256 = 32768,   For MAX we divide by 2 since it's asymmetrical

                int32_t rate = (int32_t)DEL_MULT*2048*100;
#if GVARS
                if(mixweight) rate /= abs(mixweight);
#else
                if(md->weight) rate /= abs(md->weight);
#endif
                tact = (diff>0) ? ((md->speedUp>0)   ? tact+(rate)/((int16_t)timing*md->speedUp)   :  (int32_t)v*DEL_MULT) :
                                    ((md->speedDown>0) ? tact-(rate)/((int16_t)timing*md->speedDown) :  (int32_t)v*DEL_MULT) ;


                if(((diff>0) && (v<(tact/DEL_MULT))) || ((diff<0) && (v>(act[mixIndex]/DEL_MULT)))) tact=(int32_t)v*DEL_MULT; //deal with overflow
                v = tact/DEL_MULT;
            }
            else if (diff)
            {
              tact=(int32_t)v*DEL_MULT;
            }
					act[mixIndex] = tact ;
        }

        //========== CURVES ===============
        if ( md->differential )
				{
      		//========== DIFFERENTIAL =========
          int16_t curveParam = REG( md->curve, -100, 100 ) ;
      		if (curveParam > 0 && v < 0)
      		  v = ((int32_t)v * (100 - curveParam)) / 100;
      		else if (curveParam < 0 && v > 0)
      		  v = ((int32_t)v * (100 + curveParam)) / 100;
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
//        		    if(md->srcRaw == MIX_FULL) //FUL
//        		    {
//        		        if( v<0 ) v=-RESX;   //x|x>0
//        		        else      v=-RESX+2*v;
//        		    }else{
        		        if( v<0 ) v=0;   //x|x>0
//        		    }
        		    break;
        		case 2:
//        		    if(md->srcRaw == MIX_FULL) //FUL
//        		    {
//        		        if( v>0 ) v=RESX;   //x|x<0
//        		        else      v=RESX+2*v;
//        		    }else{
        		        if( v>0 ) v=0;   //x|x<0
//        		    }
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
        if((md->carryTrim==0) && (md->srcRaw>0) && (md->srcRaw<=4)) v += trimA[md->srcRaw-1];  //  0 = Trim ON  =  Default

        //========== MULTIPLEX ===============
#if GVARS
        int32_t dv = (int32_t)v*mixweight ;
#else
        int32_t dv = (int32_t)v*md->weight;
#endif
        
        //========== lateOffset ===============
//        if ( ( md->enableFmTrim == 0 ) && ( md->lateOffset ) )
#ifndef V2
        if ( md->lateOffset )
#endif
        {
#if GVARS
            if(md->sOffset) dv += calc100toRESX( REG( md->sOffset, -125, 125 )	) * 100  ;
#else
            if(md->sOffset) dv += calc100toRESX(md->sOffset) * 100 ;
#endif
        }
				switch(md->mltpx){
        case MLTPX_REP:
            thisChan = dv;
            break;
        case MLTPX_MUL:
            thisChan *= dv/100l;
            thisChan /= RESXl;
            break;
        default:  // MLTPX_ADD
            thisChan += dv; //Mixer output add up to the line (dv + (dv>0 ? 100/2 : -100/2))/(100);
            break;
        }
    }

        // chans[i] holds data from mixer.   chans[i] = v*weight => 1024*100
        // later we multiply by the limit (up to 100) and then we need to normalize
        // at the end chans[i] = chans[i]/100 =>  -1024..1024
        // interpolate value with min/max so we get smooth motion from center to stop
        // this limits based on v original values and min=-1024, max=1024  RESX=1024

        int32_t q = thisChan;// + (int32_t)g_model.limitData[i].offset*100; // offset before limit
			
				if ( fadePhases )
				{

					int32_t l_fade = fade[chanIndex] ;
					if ( att & FADE_FIRST )
					{
						l_fade = 0 ;
					}
					l_fade += ( q / 100 ) * fadeScale[CurrentPhase] ;
					fade[chanIndex] = l_fade ;
			
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
        thisChan = q / 100; // chans back to -1024..1024
        
				ex_chans[chanIndex] = thisChan; //for getswitch

        int16_t ofs = g_model.limitData[chanIndex].offset;
				int16_t xofs = ofs ;
				if ( xofs > g_model.sub_trim_limit )
				{
					xofs = g_model.sub_trim_limit ;
				}
				else if ( xofs < -g_model.sub_trim_limit )
				{
					xofs = -g_model.sub_trim_limit ;
				}
        int16_t lim_p = 10*(g_model.limitData[chanIndex].max+100) + xofs ;
        int16_t lim_n = 10*(g_model.limitData[chanIndex].min-100) + xofs ; //multiply by 10 to get same range as ofs (-1000..1000)
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

        if(g_model.limitData[chanIndex].revert) result = -result ;// finally do the reverse.

				{
#ifndef V2
          uint8_t numSafety = 16 - g_model.numVoice ;
          if ( chanIndex < numSafety )
					{
        		if(g_model.safetySw[chanIndex].opt.ss.swtch)  //if safety sw available for channel check and replace val if needed
						{
							if ( ( g_model.safetySw[chanIndex].opt.ss.mode != 1 ) && ( g_model.safetySw[chanIndex].opt.ss.mode != 2 ) )	// And not used as an alarm
							{
								static uint8_t sticky = 0 ;
								uint8_t applySafety = 0 ;
                int8_t sSwitch = g_model.safetySw[chanIndex].opt.ss.swtch ;
								
								if(getSwitch( sSwitch,0))
								{
									applySafety = 1 ;
								}

								if ( g_model.safetySw[chanIndex].opt.ss.mode == 3 )
								{
									// Special case, sticky throttle
									if( applySafety )
									{
										sticky = 0 ;
									}
									else
									{
										uint8_t stick = THR_STICK ;
										if ( g_model.modelVersion >= 2 )
										{
											stick = 2 ;
										}
										
										if ( calibratedStick[stick] < -1010 )
										{
											sticky = 1 ;
										}
									}
									if ( sticky == 0 )
									{
										applySafety = 1 ;
									}
								}
                if ( applySafety ) result = calc100toRESX(g_model.safetySw[chanIndex].opt.ss.val) ;
							}
						}
					}
#endif
        }
        //cli();
        chanOut[chanIndex] = result ; //copy consistent word to int-level
        //sei();




	}

    //========== MIXER WARNING ===============
    //1= 00,08
    //2= 24,32,40
    //3= 56,64,72,80
    if(mixWarning & 1) if(((g_tmr10ms&0xFF)==  0)) beepWarn1();
    if(mixWarning & 2) if(((g_tmr10ms&0xFF)== 64) || ((g_tmr10ms&0xFF)== 72)) beepWarn1();
    if(mixWarning & 4) if(((g_tmr10ms&0xFF)==128) || ((g_tmr10ms&0xFF)==136) || ((g_tmr10ms&0xFF)==144)) beepWarn1();


//    //========== LIMITS ===============
//    for(uint8_t chanIndex=0;chanIndex<NUM_CHNOUT;chanIndex++)
//		{
//        // chans[i] holds data from mixer.   chans[i] = v*weight => 1024*100
//        // later we multiply by the limit (up to 100) and then we need to normalize
//        // at the end chans[i] = chans[i]/100 =>  -1024..1024
//        // interpolate value with min/max so we get smooth motion from center to stop
//        // this limits based on v original values and min=-1024, max=1024  RESX=1024

//        int32_t q = chans[chanIndex];// + (int32_t)g_model.limitData[i].offset*100; // offset before limit
			
//				if ( fadePhases )
//				{

//					int32_t l_fade = fade[chanIndex] ;
//					if ( att & FADE_FIRST )
//					{
//						l_fade = 0 ;
//					}
//					l_fade += ( q / 100 ) * fadeScale[CurrentPhase] ;
//					fade[chanIndex] = l_fade ;
			
//					if ( ( att & FADE_LAST ) == 0 )
//					{
//						continue ;
//					}
//          if ( fadeWeight != 0)
//          {
//            l_fade /= fadeWeight ;
//          }
//					q = l_fade * 100 ;
//				}
//        chans[chanIndex] = q / 100; // chans back to -1024..1024
        
//				ex_chans[chanIndex] = chans[chanIndex]; //for getswitch

//        int16_t ofs = g_model.limitData[chanIndex].offset;
//				int16_t xofs = ofs ;
//				if ( xofs > g_model.sub_trim_limit )
//				{
//					xofs = g_model.sub_trim_limit ;
//				}
//				else if ( xofs < -g_model.sub_trim_limit )
//				{
//					xofs = -g_model.sub_trim_limit ;
//				}
//        int16_t lim_p = 10*(g_model.limitData[chanIndex].max+100) + xofs ;
//        int16_t lim_n = 10*(g_model.limitData[chanIndex].min-100) + xofs ; //multiply by 10 to get same range as ofs (-1000..1000)
//				if ( lim_p > 1250 )
//				{
//					lim_p = 1250 ;
//				}
//				if ( lim_n < -1250 )
//				{
//					lim_n = -1250 ;
//				}
//        if(ofs>lim_p) ofs = lim_p;
//        if(ofs<lim_n) ofs = lim_n;

//        if(q)
//				{
//					int16_t temp = (q<0) ? ((int16_t)ofs-lim_n) : ((int16_t)lim_p-ofs) ;
//          q = ( q * temp ) / 100000 ; //div by 100000 -> output = -1024..1024
//				}
				
//				int16_t result ;
//				result = calc1000toRESX(ofs);
//  			result += q ; // we convert value to a 16bit value
        
//				lim_p = calc1000toRESX(lim_p);
//        lim_n = calc1000toRESX(lim_n);
//        if(result>lim_p) result = lim_p;

//        if(result<lim_n) result = lim_n;

//        if(g_model.limitData[chanIndex].revert) result = -result ;// finally do the reverse.

//				{
//					uint8_t numSafety = 16 - g_model.numVoice ;
//          if ( chanIndex < numSafety )
//					{
//        		if(g_model.safetySw[chanIndex].opt.ss.swtch)  //if safety sw available for channel check and replace val if needed
//						{
//							if ( ( g_model.safetySw[chanIndex].opt.ss.mode != 1 ) && ( g_model.safetySw[chanIndex].opt.ss.mode != 2 ) )	// And not used as an alarm
//							{
//								static uint8_t sticky = 0 ;
//								uint8_t applySafety = 0 ;
//                int8_t sSwitch = g_model.safetySw[chanIndex].opt.ss.swtch ;
								
//								if(getSwitch( sSwitch,0))
//								{
//									applySafety = 1 ;
//								}

//								if ( g_model.safetySw[chanIndex].opt.ss.mode == 3 )
//								{
//									// Special case, sticky throttle
//									if( applySafety )
//									{
//										sticky = 0 ;
//									}
//									else
//									{
//										uint8_t stick = THR_STICK ;
//										if ( g_model.modelVersion >= 2 )
//										{
//											stick = 2 ;
//										}
										
//										if ( calibratedStick[stick] < -1010 )
//										{
//											sticky = 1 ;
//										}
//									}
//									if ( sticky == 0 )
//									{
//										applySafety = 1 ;
//									}
//								}
//                if ( applySafety ) result = calc100toRESX(g_model.safetySw[chanIndex].opt.ss.val) ;
//							}
//						}
//					}
//				}
//        //cli();
//        chanOut[chanIndex] = result ; //copy consistent word to int-level
//        //sei();
//    }
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
	ScaleData *pscaler ;
	
	if ( CalcScaleNest > 5 )
	{
		return 0 ;
	}
	CalcScaleNest += 1 ;
	// process
	pscaler = &g_model.Scalers[index] ;
	if ( pscaler->source )
	{
		value = getValue( pscaler->source - 1 ) ;
	}
	else
	{
		value = 0 ;
	}
	if ( pscaler->offsetLast == 0 )
	{
		value += pscaler->offset ;
	}
	value *= pscaler->mult+1 ;
	value /= pscaler->div+1 ;
	if ( pscaler->offsetLast )
	{
		value += pscaler->offset ;
	}
	if ( pscaler->neg )
	{
		value = -value ;
	}

	CalcScaleNest -= 1 ;
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
  		QMessageBox::critical(this, "eePe", tr("Com Port Unavailable"));
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

