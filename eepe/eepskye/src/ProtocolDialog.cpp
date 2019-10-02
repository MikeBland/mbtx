#include "ProtocolDialog.h"
#include "ui_ProtocolDialog.h"
#include "pers.h"
#include "file.h"
#include "helpers.h"

extern uint8_t ProtocolOptionsX9de[][7] ;
extern uint8_t ProtocolOptionsSKY[][7] ;
extern uint8_t ProtocolOptions9XT[][7] ;
extern uint8_t ProtocolOptionsT12[][7] ;
extern uint8_t ProtocolOptionsX9L[][7] ;
extern QString ProtocolNames[] ;
extern QString Polarity[] ;
extern QString PxxTypes[] ;
extern QString PxxCountry[] ;
extern QString DsmTypes[] ;


//**CRC**
//uint16_t CRC_Short[]={
//	0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
//	0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7 };
//static uint16_t CRCTable(uint8_t val)
//{
//	uint16_t word ;
//	word = CRC_Short[val&0x0F] ;
//	val /= 16 ;
//	return word ^ (0x1081 * val) ;
//}
//static uint16_t __attribute__((unused)) crc_x(uint8_t *data, uint8_t len)
//{
//	uint16_t crc = 0;
//	for(uint8_t i=0; i < len; i++)
//		crc = (crc<<8) ^ CRCTable((uint8_t)(crc>>8) ^ *data++);
//	return crc;
//}

//uint8_t xdata[] = {
//	0x0e, 0x1c, 0x02, 0x26, 0x04, 0,0,0,0,0x0C,0xC0,0, 0x0C,0xC0,0, 0x0C,0xC0,0, 0x0C,0xC0,0x08,0,0,0,0,0,0,0,0,0
//} ;

ProtocolDialog::ProtocolDialog(QWidget *parent, uint32_t module, struct t_radioData *radioData, struct t_moduleData *pmodule, uint32_t modelVersion ) :
    QDialog(parent),
    ui(new Ui::ProtocolDialog)
{
  ui->setupUi(this) ;

	lVersion = modelVersion ;
	lModule = module ? 1 : 0 ;
  ppd = pmodule ;
	rData = radioData ;

//	uint16_t crc ;
//	crc = crc_x( &xdata[2], 28 ) ;
//	ui->tSB->setValue( crc ) ;

	// This will only be used if model version is >= 4

	setBoxes() ;

//    md = mixdata;
//		leeType = rData->type ;
//		lextraPots = rData->extraPots ;

//    this->setWindowTitle(tr("DEST -> CH%1%2").arg(md->destCh/10).arg(md->destCh%10));
//		int type = leeType ;
//		if ( type == RADIO_TYPE_TPLUS )
//		{
//			if ( rData->sub_type == 1 )
//			{
//				type = RADIO_TYPE_X9E ;
//			}
//		}

//    populateSourceCB(ui->sourceCB, g_eeGeneral->stickMode, 0, md->srcRaw, modelVersion, type, lextraPots ) ;
    
//    connect(ui->Fm6CB,SIGNAL(stateChanged(int)),this,SLOT(valuesChanged()));
}

ProtocolDialog::~ProtocolDialog()
{
    delete ui;
}

void ProtocolDialog::setBoxes()
{
	protocolEditLock = true ;
	ui->ProtocolCB->clear() ;
	if ( lModule == 0 )
	{
    if ( rData->bitType & ( RADIO_BITTYPE_X9L ) )
		{
      ui->ProtocolCB->addItem("ACCESS");
		}
    if ( rData->bitType & (RADIO_BITTYPE_TARANIS | RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E | RADIO_BITTYPE_QX7 | RADIO_BITTYPE_X9L ) )
		{
      ui->ProtocolCB->addItem("XJT");
		}	
    if ( rData->bitType & (RADIO_BITTYPE_SKY | RADIO_BITTYPE_9XRPRO | RADIO_BITTYPE_AR9X | RADIO_BITTYPE_9XTREME ) )
		{
		  ui->ProtocolCB->addItem("PPM");
	    if ( rData->bitType & ( RADIO_BITTYPE_9XTREME ) )
			{
      	ui->ProtocolCB->addItem("XJT");
			}
      ui->ProtocolCB->addItem("DSM");
      ui->ProtocolCB->addItem("Multi");
		}	
	}
	else
	{
	  ui->ProtocolCB->addItem("PPM");
    if ( (rData->bitType & (RADIO_BITTYPE_T12 ) ) == 0 )
		{
			ui->ProtocolCB->addItem("XJT");
		}
  	ui->ProtocolCB->addItem("DSM");
  	ui->ProtocolCB->addItem("MULTI");
	  ui->ProtocolCB->addItem("CRSF");
    if ( rData->bitType & ( RADIO_BITTYPE_X9L ) )
		{
      ui->ProtocolCB->addItem("ACCESS");
		}
	}
  ui->ProtocolCB->addItem("OFF");
  uint32_t i = ppd->module.protocol ;
	uint32_t save = i ;

	if ( i == PROTO_OFF )
	{
    i = ui->ProtocolCB->count() - 1 ;
	}
	else
	{
    uint8_t *options ;
		options = &ProtocolOptionsSKY[lModule][0] ;
		if ( rData->bitType & (RADIO_BITTYPE_TARANIS | RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E | RADIO_BITTYPE_QX7 ) )
		{
			options = &ProtocolOptionsX9de[lModule][0] ;
		}
		if ( rData->bitType & ( RADIO_BITTYPE_X9L ) )
		{
			options = &ProtocolOptionsX9L[lModule][0] ;
		}
		else if ( rData->bitType & RADIO_BITTYPE_9XTREME )
		{
			options = &ProtocolOptions9XT[lModule][0] ;
		}
		else if ( rData->bitType &  RADIO_BITTYPE_T12 )
		{
			options = &ProtocolOptionsT12[lModule][0] ;
		}
		uint32_t count = *options++ ;
		i = PROTO_OFF ;
		for ( uint32_t c = 0 ; c < count ; c += 1 )
		{
			if ( options[c] == save )
			{
				i = c ;
				break ;
			}
		}
	}
  ui->ProtocolCB->setCurrentIndex(i);

	ui->xjtChannelsLabel->hide() ;
	ui->xjtChannelsCB->hide() ;
	ui->rxNumberLabel->hide() ;
	ui->rxNumberSB->hide() ;
	ui->xjtTypeLabel->hide() ;
	ui->xjtTypeCB->hide() ;
	ui->xjtCountryLabel->hide() ;
	ui->xjtCountryCB->hide() ;
	ui->ppmFrameLengthCB->hide() ;
	ui->ppmFrameLabel->hide() ;
	ui->ppmDelayCB->hide() ;
  ui->ppmDelayLabel->hide() ;
	ui->ppmPolarityCB->hide() ;
  ui->ppmPolarityLabel->hide() ;
	ui->channelsLabel->hide() ;
  ui->channelsSB->hide() ;
	ui->dsmTypeLabel->hide() ;
	ui->dsmTypeCB->hide() ;
	ui->powerLabel->hide() ;
  ui->powerCB->hide() ;
  ui->autobindLabel->hide() ;
  ui->autobindCB->hide() ;
	ui->optionLabel->hide() ;
  ui->optionSB->hide() ;
	ui->multiTypeLabel->hide() ;
	ui->multiTypeCB->hide() ;
  ui->multiSubProtocolCB->hide() ;
  ui->multiSubProtocolLabel->hide() ;
	ui->startChannelLabel->hide() ;
  ui->startChannelSB->hide() ;
	ui->rateLabel->hide() ;
  ui->rateCB->hide() ;
	ui->followLabel->hide() ;
	ui->R9MpowerLabel->hide() ;
  ui->R9MpowerCB->hide() ;
	ui->R9MflexLabel->hide() ;
	ui->R9MflexWarnLabel->hide() ;
  ui->FlexCB->hide() ;
  ui->labelRx1->hide() ;
  ui->labelRx2->hide() ;
  ui->labelRx3->hide() ;

  if ( ( lModule == 0 ) && ( lVersion < 4 ) && ( rData->bitType & (RADIO_BITTYPE_SKY | RADIO_BITTYPE_9XRPRO | RADIO_BITTYPE_AR9X ) ) )
	{
		ui->startChannelSB->setMinimum( 0 ) ;
		ui->startChannelSB->setValue(ppd->module.startChannel) ;
		if (ppd->module.startChannel == 0 )
		{
			ui->followLabel->show() ;
		}
	}
	else
	{
		ui->startChannelSB->setMinimum( 1 ) ;
		ui->startChannelSB->setValue(ppd->module.startChannel + 1 ) ;
	}
	if ( save != PROTO_OFF )
	{
		ui->startChannelLabel->show() ;
  	ui->startChannelSB->show() ;
	}

	if ( save == PROTO_PPM )
	{
		ui->ppmFrameLengthCB->setCurrentIndex(ppd->module.ppmFrameLength + 20 ) ;
		ui->ppmDelayCB->setCurrentIndex(ppd->module.ppmDelay + 4 ) ;
		ui->ppmPolarityCB->setCurrentIndex(ppd->module.pulsePol) ;
		ui->channelsSB->setMinimum( 4 ) ;
		ui->channelsSB->setMaximum( 16 ) ;
		ui->channelsSB->setValue(ppd->module.channels + 8 ) ;
		ui->ppmFrameLengthCB->show() ;
    ui->ppmFrameLabel->show() ;
		ui->ppmDelayCB->show() ;
    ui->ppmDelayLabel->show() ;
		ui->ppmPolarityCB->show() ;
    ui->ppmPolarityLabel->show() ;
		ui->channelsLabel->show() ;
    ui->channelsSB->show() ;
	}
	else if ( save == PROTO_ACCESS )
	{
		ui->xjtChannelsCB->clear() ;
		ui->xjtChannelsCB->addItem("8") ;
		ui->xjtChannelsCB->addItem("16") ;
		ui->xjtChannelsCB->addItem("24") ;
		ui->xjtChannelsCB->setCurrentIndex(ppd->access.numChannels) ;
		ui->xjtChannelsLabel->show() ;
		ui->xjtChannelsCB->show() ;
		ui->rxNumberSB->setMaximum(63) ;
		ui->rxNumberSB->setValue(ppd->module.pxxRxNum) ;
		ui->rxNumberLabel->show() ;
		ui->rxNumberSB->show() ;
	}
	else if ( save == PROTO_PXX )
	{
		ui->channelsLabel->hide() ;
    ui->channelsSB->hide() ;
		ui->xjtChannelsCB->clear() ;
		ui->xjtChannelsCB->addItem("8") ;
		ui->xjtChannelsCB->addItem("16") ;
		ui->xjtChannelsCB->setCurrentIndex(ppd->module.channels) ;
		ui->rxNumberSB->setMaximum(63) ;
		ui->rxNumberSB->setValue(ppd->module.pxxRxNum) ;
		ui->xjtTypeCB->setCurrentIndex(ppd->module.sub_protocol) ;
		ui->xjtCountryCB->setCurrentIndex(ppd->module.country) ;
		ui->rxNumberLabel->show() ;
		ui->rxNumberSB->show() ;
		ui->xjtChannelsLabel->show() ;
		ui->xjtChannelsCB->show() ;
		ui->xjtTypeLabel->show() ;
		ui->xjtTypeCB->show() ;
		ui->xjtCountryLabel->show() ;
		ui->xjtCountryCB->show() ;
		if ( ppd->module.sub_protocol == 3 )	// R9M
		{
	    
      ui->FlexCB->setCurrentIndex(ppd->module.r9MflexMode) ;
			ui->R9MflexLabel->show() ;
			ui->FlexCB->show() ;
			if ( ppd->module.r9MflexMode )
			{
				ui->R9MflexWarnLabel->show() ;
			}
			ui->R9MpowerCB->clear();
			if ( ( ppd->module.country == 2 ) && ( ppd->module.r9MflexMode == 0 ) )
			{
				ui->R9MpowerCB->addItem("25 mW(8ch)") ;
				ui->R9MpowerCB->addItem("25 mW(16ch)") ;
				ui->R9MpowerCB->addItem("200 mW") ;
				ui->R9MpowerCB->addItem("500 mW") ;
			}
			else
			{
				ui->R9MpowerCB->addItem("10 mW") ;
				ui->R9MpowerCB->addItem("100 mW") ;
				ui->R9MpowerCB->addItem("500 mW") ;
				ui->R9MpowerCB->addItem("1000 mW") ;
			}
			ui->R9MpowerCB->setCurrentIndex(ppd->module.r9mPower) ;
			ui->R9MpowerLabel->show() ;
		  ui->R9MpowerCB->show() ;
		}



//		ui->startChannelLabel->hide() ;
//    ui->startChannelSB->hide() ;
	}
  else if ( save == PROTO_DSM2 )
	{
		ui->dsmTypeCB->setCurrentIndex(ppd->module.sub_protocol) ;
		ui->dsmTypeLabel->show() ;
		ui->dsmTypeCB->show() ;
		if ( ppd->module.sub_protocol == 3 )
		{
			ui->channelsSB->setMinimum( 6 ) ;
			ui->channelsSB->setMaximum( 14 ) ;
      if (ppd->module.channels < 6)
      {
        ppd->module.channels = 6 ;
      }
			ui->channelsSB->setValue(ppd->module.channels) ;
			ui->channelsLabel->show() ;
  	  ui->channelsSB->show() ;
		}
		else
		{
			ui->rxNumberSB->setMaximum(15) ;
			ui->rxNumberSB->setValue(ppd->module.pxxRxNum) ;
			ui->rxNumberLabel->show() ;
			ui->rxNumberSB->show() ;
		}
	}
  else if ( save == PROTO_MULTI )
	{
		uint32_t i ;
		ui->rxNumberSB->setMaximum(15) ;
		ui->rxNumberSB->setValue(ppd->module.pxxRxNum) ;
    ui->autobindCB->setCurrentIndex((ppd->module.sub_protocol>>6)&0x01) ;
    ui->powerCB->setCurrentIndex((ppd->module.channels>>7)&0x01) ;
		ui->optionSB->setValue(ppd->module.option_protocol) ;
		i = ppd->module.sub_protocol & 0x3F ;
    ui->multiTypeCB->setCurrentIndex( i ) ;
    subSubProtocolText( i, 0, ui->multiSubProtocolCB ) ;
		ui->multiSubProtocolCB->setCurrentIndex( (ppd->module.channels >> 4) & 0x07 ) ;
    ui->rateCB->setCurrentIndex(ppd->module.ppmFrameLength) ;
		ui->autobindLabel->show() ;
		ui->autobindCB->show() ;
		ui->powerLabel->show() ;
		ui->powerCB->show() ;
		ui->optionLabel->show() ;
		ui->optionSB->show() ;
		ui->rxNumberLabel->show() ;
		ui->rxNumberSB->show() ;
		ui->multiTypeLabel->show() ;
		ui->multiTypeCB->show() ;
	  ui->multiSubProtocolCB->show() ;
	  ui->multiSubProtocolLabel->show() ;
		ui->rateLabel->show() ;
  	ui->rateCB->show() ;
	}
	if ( hasFailsafe() )
	{
		ui->FailsafeCB->setCurrentIndex( ppd->module.failsafeMode ) ;
//		ui->RepeatSendCB->setChecked( !ppd->failsafeRepeat ) ;
		ui->failsafeLabel->show() ;
		ui->FailsafeCB->show() ;
//    ui->RepeatSendCB->show() ;
//		ui->RepeatLabel->show() ;
	}
	else
	{
		ui->failsafeLabel->hide() ;
		ui->FailsafeCB->hide() ;
//    ui->RepeatSendCB->hide() ;
//		ui->RepeatLabel->hide() ;
	}

	protocolEditLock = false ;
}

uint32_t ProtocolDialog::hasFailsafe(void)
{
	if ( ppd->module.protocol == PROTO_MULTI )
	{
	  if ( ( ppd->module.sub_protocol & 0x3F ) == 6 ) return 1 ;	// Devo
		if ( ( ppd->module.sub_protocol & 0x3F ) == 20 ) return 1 ;
		if ( ( ppd->module.sub_protocol & 0x3F ) == 27 ) return 1 ;
		if ( ( ppd->module.sub_protocol & 0x3F ) == 29 ) return 1 ;
		if ( ( ppd->module.sub_protocol & 0x3F ) == 14 )	// FrskyX
		{
      if ( ( ( (ppd->module.channels >> 4) & 0x07 ) & 1 ) == 0 )
			{
				return 1 ;
			}
		}
	}
	if ( ppd->module.protocol == PROTO_PXX )
	{
	  if ( ppd->module.sub_protocol == 0 ) return 1 ;		
	  if ( ppd->module.sub_protocol == 3 ) return 1 ;		
	}
	if ( ppd->module.protocol == PROTO_ACCESS )
	{
		return 1 ;
	}

	return 0 ;
}


void ProtocolDialog::on_ProtocolCB_currentIndexChanged(int index)
{
  if(protocolEditLock) return;
  const uint8_t *p ;
	p = &ProtocolOptionsSKY[lModule][1] ;

  if ( index >= ui->ProtocolCB->count() - 1 )
	{
		index = PROTO_OFF ;
	}
	else
	{
		if ( rData->bitType & (RADIO_BITTYPE_TARANIS | RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E | RADIO_BITTYPE_QX7 ) )
		{
      p = &ProtocolOptionsX9de[lModule][1] ;
		}
		if ( rData->bitType & ( RADIO_BITTYPE_X9L ) )
		{
      p = &ProtocolOptionsX9L[lModule][1] ;
		}
		else if ( rData->bitType & RADIO_BITTYPE_9XTREME )
		{
			p = &ProtocolOptions9XT[lModule][1] ;
		}
		else if ( rData->bitType &  RADIO_BITTYPE_T12 )
		{
      p = &ProtocolOptionsT12[lModule][1] ;
		}
		index = p[index] ;
	}
  ppd->module.protocol = index ;
  ppd->module.channels = 0 ;
	
  setBoxes();

//  updateSettings();
}

void ProtocolDialog::on_xjtCountryCB_currentIndexChanged(int index)
{
	if ( protocolEditLock ) return ;
	ppd->module.country = index ;
	if ( ppd->module.sub_protocol == 3 )	// R9M
	{
		if ( index > 1 )
		{
 			ppd->module.r9mPower = 0 ;
		}
	}
  setBoxes();
}

void ProtocolDialog::on_xjtChannelsCB_currentIndexChanged(int index)
{
	if ( protocolEditLock ) return ;

	if ( ppd->module.protocol == PROTO_ACCESS )
	{
		ppd->access.numChannels = index ;
	}
	else
	{
		ppd->module.channels = index ;
	}
}

void ProtocolDialog::on_ppmFrameLengthCB_currentIndexChanged(int index)
{
	if ( protocolEditLock ) return ;
	ppd->module.ppmFrameLength = index - 20 ;
}

void ProtocolDialog::on_ppmDelayCB_currentIndexChanged(int index)
{
	if ( protocolEditLock ) return ;
	ppd->module.ppmDelay = index - 4 ;
}

void ProtocolDialog::on_startChannelSB_valueChanged(int value)
{
	if ( protocolEditLock ) return ;

  if ( ( lModule == 0 ) && ( lVersion < 4 ) && ( rData->bitType & (RADIO_BITTYPE_SKY | RADIO_BITTYPE_9XRPRO | RADIO_BITTYPE_AR9X ) ) )
	{
		if ( value == 0 )
		{
			ui->followLabel->show() ;
		}
		else
		{
			ui->followLabel->hide() ;
		}
		ppd->module.startChannel = value ;
	}
	else
	{
		ppd->module.startChannel = value - 1 ;
	}
}

void ProtocolDialog::on_rxNumberSB_valueChanged(int value)
{
	if ( protocolEditLock ) return ;
	ppd->module.pxxRxNum = value ;
}

void ProtocolDialog::on_xjtTypeCB_currentIndexChanged(int index)
{
	if ( protocolEditLock ) return ;
	ppd->module.sub_protocol = index ;
  setBoxes();
}

void ProtocolDialog::on_ppmPolarityCB_currentIndexChanged(int index)
{
	if ( protocolEditLock ) return ;
	ppd->module.pulsePol = index ;
}

void ProtocolDialog::on_channelsSB_valueChanged(int value)
{
	if ( protocolEditLock ) return ;
	int chans = value - 8 ;
  if ( ppd->module.protocol == PROTO_DSM2 )
	{
    if ( ppd->module.sub_protocol == 3 )
		{
			chans = value ;
		}	
  }
  ppd->module.channels = chans ;
}

void ProtocolDialog::on_dsmTypeCB_currentIndexChanged(int index)
{
	if ( protocolEditLock ) return ;
  ppd->module.sub_protocol = index ;
  setBoxes();
}

void ProtocolDialog::on_autobindCB_currentIndexChanged(int index)
{
	if ( protocolEditLock ) return ;
	ppd->module.sub_protocol = (index<<6) + (ppd->module.sub_protocol&0xBF);
}

void ProtocolDialog::on_powerCB_currentIndexChanged(int index)
{
	if ( protocolEditLock ) return ;
	ppd->module.channels = (index<<7) + (ppd->module.channels & 0x7F) ;
}

void ProtocolDialog::on_optionSB_valueChanged(int value)
{
	if ( protocolEditLock ) return ;
	ppd->module.option_protocol = value ;
}

void ProtocolDialog::on_multiTypeCB_currentIndexChanged(int index)
{
	if ( protocolEditLock ) return ;
	ppd->module.sub_protocol = index + (ppd->module.sub_protocol & 0xC0) ;
  subSubProtocolText( ppd->module.sub_protocol & 0x3F, 0, ui->multiSubProtocolCB ) ;
	ui->multiSubProtocolCB->setCurrentIndex( (ppd->module.channels >> 4) & 0x07 ) ;
  setBoxes();
}

void ProtocolDialog::on_multiSubProtocolCB_currentIndexChanged(int value)
{
	if ( protocolEditLock ) return ;
  if ( value < 0 ) return ;
	ppd->module.channels = ( value << 4) + (ppd->module.channels & 0x8F);
  setBoxes();
}

void ProtocolDialog::on_rateCB_currentIndexChanged(int index)
{
	if ( protocolEditLock ) return ;
	ppd->module.ppmFrameLength = index ;
}

void ProtocolDialog::on_FailsafeCB_currentIndexChanged(int index)
{
	if ( protocolEditLock ) return ;
	ppd->module.failsafeMode = index ;
}

//void ProtocolDialog::on_RepeatSendCB_toggled(bool checked)
//{
//	if ( protocolEditLock ) return ;
//	ppd->failsafeRepeat = !checked  ;
	
//}

void ProtocolDialog::on_R9MpowerCB_currentIndexChanged(int index)
{
	if ( protocolEditLock ) return ;
	  ppd->module.r9mPower = index ;
}

void ProtocolDialog::on_FlexCB_currentIndexChanged(int index)
{
	if ( protocolEditLock ) return ;
	  ppd->module.r9MflexMode = index ;
  setBoxes() ;
}

