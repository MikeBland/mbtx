#include <QtGui>
#include <QMessageBox>

#include "pers.h"

#include "helpers.h"
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>

uint8_t ProtocolOptionsX9de[2][5] = { {2,0,1}, {4,0,1,2,3} };
uint8_t ProtocolOptionsSKY[2][5] = { {3,0,2,3}, {4,0,1,2,3} };
uint8_t ProtocolOptions9XT[2][5] = { {4,0,1,2,3}, {4,0,1,2,3} };

QString MultiProtocols[] = {
"Flysky",
"Hubsan",
"FrskyD",
"Hisky",
"V2x2",
"DSM",
"Devo",
"YD717",
"KN",
"SymaX",
"SLT",
"CX10",
"CG023",
"Bayang",
"FrskyX",
"ESky",
"MT99xx",
"MJXq",
"Shenqi",
"FY326",
"SFHSS",
"J6PRO",
"FQ777",
"ASSAN",
"FrskyV",
"HONTAI",
"OpnLrs",
"AFHD2SA",
"Q2X2"
} ;

uint32_t MultiProtocolCount = sizeof( MultiProtocols ) ;

QString AudioAlarms[] = {
  "Warn1",
	"Warn2",
	"Cheap",
	"Ring",
	"SciFi",
	"Robot",
	"Chirp",
	"Tada",
	"Crickt",
	"Siren",
	"AlmClk",
	"Ratata",
	"Tick",
	"Haptc1",
	"Haptc2",
	"Haptc3"
} ;

QString TelemItems[] = {
	"----",
	"A1= ",
	"A2= ",
	"RSSI",
	"TSSI/SWR",
	"Tim1",	// 4
	"Tim2",
	"Alt ",
	"Galt",
	"Gspd", // 8
	"T1= ",
	"T2= ",
	"RPM ",
	"FUEL", // 12
	"Mah1",
	"Mah2",
	"Cvlt",
	"Batt", // 16
	"Amps",
	"Mah ",
	"Ctot",
	"FasV",	// 20
	"AccX",
	"AccY",
	"AccZ",
	"Vspd", // 24
	"Gvr1",
	"Gvr2",
	"Gvr3",
	"Gvr4", // 28
	"Gvr5",
	"Gvr6",
	"Gvr7",
	"Fwat", // 32
	"RxV ",
	"Hdg ",
	"A3= ",
	"A4= ",
	"SC1 ", // 37
	"SC2 ",
	"SC3 ",
	"SC4 ",
#ifdef SKY
	"SC5 ", // 41
	"SC6 ",
	"SC7 ",
	"SC8 ",
	"RTC ",
#endif
	"TmOK"  // 41 or 46(SKY)
#ifdef SKY
	,"Aspd"
	,"Cel1"
	,"Cel2"
	,"Cel3"
	,"Cel4"
	,"Cel5"
	,"Cel6"
	,"RBv1"
	,"RBa1"
	,"RBv2"
	,"RBa2"
	,"RBm1"
	,"RBm2"
	,"RBSV"
	,"RBST"
	,"Cel7"
	,"Cel8"
	,"Cel9"
	,"Cl10"
	,"Cl11"
	,"Cl12"
	,"Cus1"
	,"Cus2"
	,"Cus3"
	,"Cus4"
	,"Cus5"
	,"Cus6"
	,"Fmd "
	,"RunT"	// 75
	,"ModT"	// 76
#endif
} ;

#ifdef SKY
#define NUM_TELEM_ITEMS	77
#else
#define NUM_TELEM_ITEMS	42
#endif

QString GvarItems[] = {
	"---", // 0
	"Rtm",
	"Etm",
	"Ttm",
	"Atm",
	"REN",	// 5
	"Rud",
	"Ele",
	"Thr",
	"Ail", // 9
	"P1 ",
	"P2 ",
	"P3 ", //12
	"C1 ",
	"C2 ",
	"C3 ",
	"C4 ",
	"C5 ",
	"C6 ",
	"C7 ",
	"C8 ",
	"C9 ",
	"C10",
	"C11",
	"C12",
	"C13",
	"C14",
	"C15",
	"C16", //28
	"C17",
	"C18",
	"C19",
	"C20",
	"C21",
	"C22",
	"C23",
	"C24",
	"SC1",
	"SC2",
	"SC3",
	"SC4",
	"SC5",
	"SC6",
	"SC7",
	"SC8",
	"O1 ",
	"O2 ",
	"O3 ",
	"O4 ",
	"O5 ",
	"O6 ",
	"O7 ",
	"O8 ",
	"O9 ",
	"O10",
	"O11",
	"O12",
	"O13",
	"O14",
	"O15",
	"O16",
	"O17",
	"O18",
	"O19",
	"O20",
	"O21",
	"O22",
	"O23",
	"O24",
	"Rts",
	"Ets",
	"Tts",
	"Ats"
} ;

QString ExtraGvarItems[] = {
	"L1L2",
	"L3L4",
	"L5L6",
	"L7L8",
	"L9LA",
	"LBLC",
	"LDLE",
	"LFLG",
	"LHLI",
	"LJLK",
	"LLLM",
	"LNLO"
} ;

#ifdef V2
#define NUM_HW_ITEMS	14
QString HardwareItems[] = {
  "NONE",
	"PB7 ",
	"PC0 ",
	"PC4 ",
  "PC6 ",
  "PC7 ",
	"PG2 ",
	"PG5 ",
	"EXT1",
	"XPD2",
	"XPD3",
	"XPD4",
	"XPB1",
	"XPC0",
	"EXT2",
} ;
#else
#define NUM_HW_ITEMS	7
QString HardwareItems[] = {
	"NONE",
	"EXT1",
	"EXT2",
	"PC0 ",
	"PG2 ",
	"PB7 ",
	"PG5 ",
	"L-WR"
} ;
#endif

QString AnaVolumeItems[] = {
	"---",
  "P1 ",
	"P2 ",
	"P3 ",
	"GV4",
	"GV5",
	"GV6",
	"GV7"
} ;

const uint8_t csTypeTable[] =
#ifdef SKY
{ CS_VOFS, CS_VOFS, CS_VOFS, CS_VOFS, CS_VBOOL, CS_VBOOL, CS_VBOOL,
 CS_VCOMP, CS_VCOMP, CS_VCOMP, CS_VCOMP, CS_VCOMP, CS_VCOMP, CS_TIMER, CS_TIMER, CS_TMONO, CS_TMONO, CS_VOFS, CS_U16, CS_VCOMP, CS_VOFS
} ;
#else
{ CS_VOFS, CS_VOFS, CS_VOFS, CS_VOFS, CS_VBOOL, CS_VBOOL, CS_VBOOL,
 CS_VCOMP, CS_VCOMP, CS_VCOMP, CS_VCOMP, CS_VCOMP, CS_VCOMP, CS_TIMER
} ;
#endif

QString subSubProtocolText( int type, int index, QComboBox *b )
{
	QString names[8] ;
	names[0] = "0" ;
	names[1] = "1" ;
	names[2] = "2" ;
	names[3] = "3" ;
	names[4] = "4" ;
	names[5] = "5" ;
	names[6] = "6" ;
	names[7] = "7" ;

	switch ( type )
	{
		case M_Flysky :
			names[0] = "Flysky" ;
			names[1] = "V9x9" ;
			names[2] = "V6x6" ;
			names[3] = "V912" ;
			names[4] = "CX20" ;
		break ;
		case M_Hisky :
			names[0] = "Hisky" ;
			names[1] = "HK310" ;
		break ;
		case M_V2x2 :
			names[0] = "V2x2" ;
			names[1] = "JXD506" ;
		break ;
		case M_DSM2 :
			names[0] = "DSM2-22" ;
			names[1] = "DSM2-11" ;
			names[2] = "DSMX-22" ;
			names[3] = "DSMX-11" ;
			names[4] = "AUTO" ;
		break ;
		case M_YD717 :
			names[0] = "YD717" ;
			names[1] = "SKYWLKR" ;
			names[2] = "SYMAX4" ;
      names[3] = "XINXUN" ;
      names[4] = "NIHUI" ;
		break ;
		case M_KN :
			names[0] = "WLTOYS" ;
			names[1] = "FEILUN" ;
		break ;
		case M_SymaX :
			names[0] = "SYMAX" ;
			names[1] = "SYMAX5C" ;
		break ;
		case M_SLT :
			names[0] = "SLT" ;
			names[1] = "VISTA" ;
		break ;
		case M_CX10 :
			names[0] = "GREEN" ;
			names[1] = "BLUE" ;
			names[2] = "DM007" ;
			names[3] = "---" ;
			names[4] = "J3015_1" ;
			names[5] = "J3015_2" ;
			names[6] = "MK33041" ;
		break ;
		case M_FRSKYX :
			names[0] = "CH-16" ;
			names[1] = "CH-8" ;
			names[2] = "EU-16" ;
			names[3] = "EU-8" ;
		break ;
		case M_CG023 :
			names[0] = "CG023" ;
			names[1] = "YD829" ;
			names[2] = "H8_3D" ;
		break ;
		case M_MT99XX :
			names[0] = "MT" ;
			names[1] = "H7" ;
			names[2] = "YZ" ;
			names[3] = "LS" ;
		break ;
		case M_MJXQ :
			names[0] = "WLH08" ;
			names[1] = "X600" ;
			names[2] = "X800" ;
			names[3] = "H26D" ;
		break ;
		case M_HONTAI :
			names[0] = "HONTAI" ;
			names[1] = "JJRCX1" ;
			names[2] = "X5C1" ;
		break ;
		case M_AFHDS2A :
			names[0] = "PWM_IBUS" ;
			names[1] = "PPM_IBUS" ;
			names[2] = "PWM_SBUS" ;
			names[3] = "PPM_SBUS" ;
		break ;
		case M_Q2X2 :
			names[0] = "Q222" ;
			names[1] = "Q242" ;
			names[2] = "Q282" ;
		break ;

		default :
			names[0] = "NONE" ;
		break ;
	}
	if ( b )
	{
		b->clear() ;
		b->addItem(names[0]);
		b->addItem(names[1]);
		b->addItem(names[2]);
		b->addItem(names[3]);
		b->addItem(names[4]);
		b->addItem(names[5]);
		b->addItem(names[6]);
		b->addItem(names[7]);
	}

	return names[index] ;
}

//void setSubSubProtocol( QComboBox *b, int type )
//{
//	int x = 8 ;
//	b->clear() ;
//  if ( type > 29 )
//	{
//		b->addItem("0");
//		b->addItem("1");
//		b->addItem("2");
//		b->addItem("3");
//		b->addItem("4");
//		b->addItem("5");
//		b->addItem("6");
//		b->addItem("7");
//		return ;
//	}
//	switch ( type )
//	{
//		case M_Flysky :
//			b->addItem("Flysky");
//			b->addItem("V9x9");
//			b->addItem("V6x6");
//			b->addItem("V912");
//			b->addItem("CX20");
//			x = 5 ;
//		break ;
//		case M_Hisky :
//			b->addItem("Hisky");
//			b->addItem("HK310");
//			x = 2 ;
//		break ;
//		case M_V2x2 :
//			b->addItem("V2x2");
//			b->addItem("JXD506");
//			x = 2 ;
//		break ;
//		case M_DSM2 :
//			b->addItem("DSM2-22");
//			b->addItem("DSM2-11");
//			b->addItem("DSMX-22");
//			b->addItem("DSMX-11");
//			b->addItem("AUTO");
//			x = 5 ;
//		break ;
//		case M_YD717 :
//			b->addItem("YD717");
//			b->addItem("SKYWLKR");
//			b->addItem("SYMAX4");
//      b->addItem("XINXUN");
//      b->addItem("NIHUI");
//			x = 5 ;
//		break ;
//		case M_KN :
//			b->addItem("WLTOYS");
//			b->addItem("FEILUN");
//			x = 2 ;
//		break ;
//		case M_SymaX :
//			b->addItem("SYMAX");
//			b->addItem("SYMAX5C");
//			x = 2 ;
//		break ;
//		case M_SLT :
//			b->addItem("SLT");
//			b->addItem("VISTA");
//			x = 2 ;
//		break ;
//		case M_CX10 :
//			b->addItem("GREEN");
//			b->addItem("BLUE");
//			b->addItem("DM007");
//			b->addItem("---");
//			b->addItem("J3015_1");
//			b->addItem("J3015_2");
//			b->addItem("MK33041");
//			x = 7 ;
//		break ;
//		case M_FRSKYX :
//			b->addItem("CH-16");
//			b->addItem("CH-8");
//			x = 2 ;
//		break ;
//		case M_CG023 :
//			b->addItem("CG023");
//			b->addItem("YD829");
//			b->addItem("H8_3D");
//			x = 3 ;
//		break ;
//		case M_MT99XX :
//			b->addItem("MT");
//			b->addItem("H7");
//			b->addItem("YZ");
//			b->addItem("LS");
//			x = 4 ;
//		break ;
//		case M_MJXQ :
//			b->addItem("WLH08");
//			b->addItem("X600");
//			b->addItem("X800");
//			b->addItem("H26D");
//			x = 4 ;
//		break ;
//		case M_HONTAI :
//			b->addItem("HONTAI");
//			b->addItem("JJRCX1");
//			b->addItem("X5C1");
//			x = 3 ;
//		break ;
//		case M_AFHDS2A :
//			b->addItem("PWM_IBUS");
//			b->addItem("PPM_IBUS");
//			b->addItem("PWM_SBUS");
//			b->addItem("PPM_SBUS");
//			x = 4 ;
//		break ;
//		case M_Q2X2 :
//			b->addItem("Q222");
//			b->addItem("Q242");
//			b->addItem("Q282");
//			x = 3 ;
//		break ;

//		default :
//			b->addItem("NONE");
//			x = 1 ;
//		break ;
//	}
//	switch ( x )
//	{
//		case 0 :
//			b->addItem("0");
//		case 1 :
//			b->addItem("1");
//		case 2 :
//			b->addItem("2");
//		case 3 :
//			b->addItem("3");
//		case 4 :
//			b->addItem("4");
//		case 5 :
//			b->addItem("5");
//		case 6 :
//			b->addItem("6");
//		case 7 :
//			b->addItem("7");
//		break ;
//	}
//}


uint8_t CS_STATE( uint8_t x, uint8_t modelVersion )
{
	if ( modelVersion >= 3 )
	{
		if ( (x==CS_LATCH) || (x==CS_FLIP) )
		{
			return CS_VBOOL ;
		}
	}
	return csTypeTable[x-1] ;
}

//uint8_t CS_STATE( uint8_t x, uint8_t modelVersion )
//{
//	if ( modelVersion >= 3 )
//	{
//    return ((x)<CS_AND ? CS_VOFS : ((((x)<CS_EQUAL) || ((x)==CS_LATCH)|| ((x)==CS_FLIP)) ? CS_VBOOL : ((x)<CS_TIME ? CS_VCOMP : ((x)==CS_MONO) ? CS_TMONO : CS_TIMER))) ;
//	}
//	else
//	{
//		return ((x)<CS_AND ? CS_VOFS : ((x)<CS_EQUAL ? CS_VBOOL : ((x)<CS_TIME ? CS_VCOMP : CS_TIMER))) ;
		
//	}
//}

#ifdef SKY

uint8_t switchMapTable[2][80] ;
uint8_t switchUnMapTable[2][80] ;
uint8_t MaxSwitchIndex[2] ;		// For ON and OFF
uint8_t Sw3PosList[8] ;
uint8_t Sw3PosCount[8] ;

void createSwitchMapping( EEGeneral *pgeneral, uint8_t max_switch, int type )
{
  uint16_t map = pgeneral->switchMapping ;
	int x = ( (type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_QX7 ) ) ? 1 : 0 ;
	uint8_t *p = switchMapTable[x] ;
	*p++ = 0 ;
	if ( ( (type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_QX7 ) ) )
	{
		*p++ = HSW_SA0 ;
		*p++ = HSW_SA1 ;
		*p++ = HSW_SA2 ;
	
		*p++ = HSW_SB0 ;
		*p++ = HSW_SB1 ;
		*p++ = HSW_SB2 ;

		*p++ = HSW_SC0 ;
		*p++ = HSW_SC1 ;
		*p++ = HSW_SC2 ;
	
		*p++ = HSW_SD0 ;
		*p++ = HSW_SD1 ;
		*p++ = HSW_SD2 ;

	  if ( type != RADIO_TYPE_QX7 )
		{
			*p++ = HSW_SE0 ;
			*p++ = HSW_SE1 ;
			*p++ = HSW_SE2 ;
		}

	//	*p++ = HSW_SF0 ;
		*p++ = HSW_SF2 ;

	  if ( type != RADIO_TYPE_QX7 )
		{
			*p++ = HSW_SG0 ;
			*p++ = HSW_SG1 ;
			*p++ = HSW_SG2 ;
	  }

	//	*p++ = HSW_SH0 ;
		*p++ = HSW_SH2 ;
	
    if ( pgeneral->analogMapping & 0x0C /*MASK_6POS*/ )
		{
			*p++ = HSW_Ele6pos0 ;
			*p++ = HSW_Ele6pos1 ;
			*p++ = HSW_Ele6pos2 ;
			*p++ = HSW_Ele6pos3 ;
			*p++ = HSW_Ele6pos4 ;
			*p++ = HSW_Ele6pos5 ;
		}
		if ( map & USE_PB1 )
		{
			*p++ = HSW_Pb1 ;
		}
		if ( map & USE_PB2 )
		{
			*p++ = HSW_Pb2 ;
		}
	}
	else
	{
		if ( map & USE_THR_3POS )
		{
			*p++ = HSW_Thr3pos0 ;
			*p++ = HSW_Thr3pos1 ;
			*p++ = HSW_Thr3pos2 ;
		}
		else
		{
			*p++ = HSW_ThrCt ;
		}
	
		if ( map & USE_RUD_3POS )
		{
			*p++ = HSW_Rud3pos0 ;
			*p++ = HSW_Rud3pos1 ;
			*p++ = HSW_Rud3pos2 ;
		}
		else
		{
			*p++ = HSW_RuddDR ;
		}

		if ( map & USE_ELE_3POS )
		{
			*p++ = HSW_Ele3pos0 ;
			*p++ = HSW_Ele3pos1 ;
			*p++ = HSW_Ele3pos2 ;
		}
		else if ( map & USE_ELE_6POS )
		{
			*p++ = HSW_Ele6pos0 ;
			*p++ = HSW_Ele6pos1 ;
			*p++ = HSW_Ele6pos2 ;
			*p++ = HSW_Ele6pos3 ;
			*p++ = HSW_Ele6pos4 ;
			*p++ = HSW_Ele6pos5 ;
		}
		else
		{
			*p++ = HSW_ElevDR ;
		}
	
    if ( pgeneral->analogMapping & MASK_6POS )
		{
			*p++ = HSW_Ele6pos0 ;
			*p++ = HSW_Ele6pos1 ;
			*p++ = HSW_Ele6pos2 ;
			*p++ = HSW_Ele6pos3 ;
			*p++ = HSW_Ele6pos4 ;
			*p++ = HSW_Ele6pos5 ;
		}

		*p++ = HSW_ID0 ;
		*p++ = HSW_ID1 ;
		*p++ = HSW_ID2 ;
	
		if ( map & USE_AIL_3POS )
		{
			*p++ = HSW_Ail3pos0 ;
			*p++ = HSW_Ail3pos1 ;
			*p++ = HSW_Ail3pos2 ;
		}
		else
		{
			*p++ = HSW_AileDR ;
		}

		if ( map & USE_GEA_3POS )
		{
			*p++ = HSW_Gear3pos0 ;
			*p++ = HSW_Gear3pos1 ;
			*p++ = HSW_Gear3pos2 ;
		}
		else
		{
			*p++ = HSW_Gear ;
		}
		*p++ = HSW_Trainer ;
		if ( map & USE_PB1 )
		{
			*p++ = HSW_Pb1 ;
		}
		if ( map & USE_PB2 )
		{
			*p++ = HSW_Pb2 ;
		}
		if ( map & USE_PB3 )
		{
			*p++ = HSW_Pb3 ;
		}
		if ( map & USE_PB4 )
		{
			*p++ = HSW_Pb4 ;
		}
	}
	*p++ = HSW_Ttrmup ;
	*p++ = HSW_Ttrmdn ;
  *p++ = HSW_Rtrmup ;
  *p++ = HSW_Rtrmdn ;
  *p++ = HSW_Atrmup ;
  *p++ = HSW_Atrmdn ;
	*p++ = HSW_Etrmup ;
	*p++ = HSW_Etrmdn ;
	for ( uint32_t i = 10 ; i <=33 ; i += 1  )
	{
		*p++ = i ;	// Custom switches
	}
	*p = max_switch ;
	MaxSwitchIndex[x] = p - switchMapTable[x] ;
  *++p = max_switch+1 ;
  *++p = max_switch+2 ;
//  *++p = max_switch+3 ;
//  *++p = max_switch+4 ;
//  *++p = max_switch+5 ;

  for ( uint32_t i = 0 ; i <= (uint32_t)MaxSwitchIndex[x]+2 ; i += 1  )
	{
		switchUnMapTable[x][switchMapTable[x][i]] = i ;
	}

	uint32_t index = 1 ;
	Sw3PosList[0] = HSW_ID0 ;
	Sw3PosCount[0] = 3 ;
	Sw3PosCount[index] = 2 ;
	Sw3PosList[index] = HSW_ThrCt ;
	if (map & USE_THR_3POS )
	{
		Sw3PosCount[index] = 3 ;
		Sw3PosList[index] = HSW_Thr3pos0 ;
	}
	Sw3PosCount[++index] = 2 ;
	Sw3PosList[index] = HSW_RuddDR ;
	if (map & USE_RUD_3POS )
	{
		Sw3PosCount[index] = 3 ;
		Sw3PosList[index] = HSW_Rud3pos0 ;
	}
	Sw3PosCount[++index] = 2 ;
	Sw3PosList[index] = HSW_ElevDR ;
	if ( map & USE_ELE_3POS )
	{
		Sw3PosCount[index] = 3 ;
		Sw3PosList[index] = HSW_Ele3pos0 ;
	}
	if ( map & USE_ELE_6POS )
	{
		Sw3PosCount[index] = 6 ;
		Sw3PosList[index] = HSW_Ele6pos0 ;
	}
	Sw3PosCount[++index] = 2 ;
	Sw3PosList[index] = HSW_AileDR ;
	if (map & USE_AIL_3POS )
	{
		Sw3PosCount[index] = 3 ;
		Sw3PosList[index] = HSW_Ail3pos0 ;
	}
	Sw3PosCount[++index] = 2 ;
	Sw3PosList[index] = HSW_Gear ;
	if (map & USE_GEA_3POS )
	{
		Sw3PosCount[index] = 3 ;
		Sw3PosList[index] = HSW_Gear3pos0 ;
	}
	Sw3PosCount[++index] = 2 ;
	Sw3PosList[index] = HSW_Trainer ;
}

int8_t switchUnMap( int8_t x, int type )
{
	int y = ( (type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_QX7 ) ) ? 1 : 0 ;
	uint8_t sign = 0 ;
	if ( x < 0 )
	{
		sign = 1 ;
		x = -x ;
	}
	x = switchUnMapTable[y][x] ;
	if ( sign )
	{
		x = -x ;
	}
	return x ;
}

int8_t switchMap( int8_t x, int type )
{
  int y = ( (type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_QX7 ) ) ? 1 : 0 ;
	uint8_t sign = 0 ;
	if ( x < 0 )
	{
		sign = 1 ;
		x = -x ;
	}
	x = switchMapTable[y][x] ;
	if ( sign )
	{
		x = -x ;
	}
	return x ;
}

uint8_t getSw3PosList( int index )
{
	return Sw3PosList[index] ;
}

uint8_t getSw3PosCount( int index )
{
	if ( index == 31 )
	{
		return 6 ;
	}
	if ( index > 6 )
	{
		return 2 ;
	}
	return Sw3PosCount[index] ;
}

#else // SKY

#ifndef V2
uint8_t switchMapTable[80] ;
uint8_t switchUnMapTable[80] ;
uint8_t MaxSwitchIndex ;		// For ON and OFF

void createSwitchMapping( EEGeneral *pgeneral, int type )
{
  uint16_t map = pgeneral->switchMapping ;
	uint8_t *p = switchMapTable ;
	*p++ = 0 ;
	*p++ = HSW_ThrCt ;
	
	if ( map & USE_RUD_3POS )
	{
		*p++ = HSW_Rud3pos0 ;
		*p++ = HSW_Rud3pos1 ;
		*p++ = HSW_Rud3pos2 ;
	}
	else
	{
		*p++ = HSW_RuddDR ;
	}

	if ( map & USE_ELE_3POS )
	{
		*p++ = HSW_Ele3pos0 ;
		*p++ = HSW_Ele3pos1 ;
		*p++ = HSW_Ele3pos2 ;
	}
	else
	{
		*p++ = HSW_ElevDR ;
	}
	*p++ = HSW_ID0 ;
	*p++ = HSW_ID1 ;
	*p++ = HSW_ID2 ;
	
	if ( map & USE_AIL_3POS )
	{
		*p++ = HSW_Ail3pos0 ;
		*p++ = HSW_Ail3pos1 ;
		*p++ = HSW_Ail3pos2 ;
	}
	else
	{
		*p++ = HSW_AileDR ;
	}

	if ( map & USE_GEA_3POS )
	{
		*p++ = HSW_Gear3pos0 ;
		*p++ = HSW_Gear3pos1 ;
		*p++ = HSW_Gear3pos2 ;
	}
	else
	{
		*p++ = HSW_Gear ;
	}
	*p++ = HSW_Trainer ;
	if ( map & USE_PB1 )
	{
		*p++ = HSW_Pb1 ;
	}
	if ( map & USE_PB2 )
	{
		*p++ = HSW_Pb2 ;
	}

#ifdef V2
	uint8_t limit = 27 ;
#else
	uint8_t limit = ( (type == 1 ) || ( type == 2 ) ) ? 27 : 21 ;
#endif
	 
	for ( uint32_t i = 10 ; i <=limit ; i += 1  )
	{
		*p++ = i ;	// Custom switches
	}
  *p = limit+1 ;
  MaxSwitchIndex = p - switchMapTable ;
  *++p = limit+2 ;

  for ( uint8_t i = 0 ; i <= (uint8_t)MaxSwitchIndex+1 ; i += 1  )
	{
		switchUnMapTable[switchMapTable[i]] = i ;
	}
}

int8_t switchUnMap( int8_t x )
{
	uint8_t sign = 0 ;
	if ( x < 0 )
	{
		sign = 1 ;
		x = -x ;
	}
	x = switchUnMapTable[x] ;
	if ( sign )
	{
		x = -x ;
	}
	return x ;
}

int8_t switchMap( int8_t x )
{
	uint8_t sign = 0 ;
	if ( x < 0 )
	{
		sign = 1 ;
		x = -x ;
	}
	x = switchMapTable[x] ;
	if ( sign )
	{
		x = -x ;
	}
	return x ;
}
#endif

#endif // SKY


#ifdef SKY
void populateAnaVolumeCB( QComboBox *b, int value, int type )
#else
void populateAnaVolumeCB( QComboBox *b, int value )
#endif
{
  b->clear();
  for(int i=0; i<8; i++)
	{
#ifdef SKY
    if ( ( (type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_QX7 ) ) && (i == 3) )
		{
    	b->addItem( "SL " );
		}
		else if ( ( (type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_QX7 ) ) && (i == 4) )
		{
    	b->addItem( "SR " );
		}
		else
#endif
    {
    	b->addItem(AnaVolumeItems[i]);
		}
	}
  b->setCurrentIndex(value);
  b->setMaxVisibleItems(8);
}

void populateHardwareSwitch(QComboBox *b, int value )
{
  for(int i=0; i<=NUM_HW_ITEMS; i++)
	{
		b->addItem(HardwareItems[i]) ;
	}
   b->setCurrentIndex(value) ;
}


#ifdef SKY
QString gvarSourceString( int index, int type, uint32_t extraPots)
{
	int limit = 36+8+24+4 ;
	if ( ( (type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_QX7 ) ) )
	{
		limit = 37+8+24+4 ;
	}
	if ( index <= limit )
	{
		if ( ( (type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_QX7 ) ) )	// Taranis
		{
			if ( index == 12 )
			{
    		return "S1  " ;
			}
			if ( index == 13 )
			{
    		return "S2  " ;
			}
			if ( index > 13 )
			{
				index -= 1 ;
			}
		}
    return GvarItems[index] ;
	}
	if ( extraPots )
	{
		index -= limit ;
		switch ( index )
		{
			case 0 :
				return "P4" ;
			break ;
      case 1 :
				return "P5" ;
			break ;
      case 2 :
				return "P6" ;
			break ;
      case 3 :
				return "P7" ;
			break ;
		}
	}
	return "----" ;
}
#endif

#ifdef SKY
void populateGvarCB(QComboBox *b, int value, int type, uint32_t extraPots)
#else
void populateGvarCB(QComboBox *b, int value, int type)
#endif
{
    b->clear();
#ifdef SKY
		int limit = 36+8+24+4 ;
		if ( ( (type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_QX7 ) ) )
		{
			limit = 37+8+24+4 ;
		}
    for(int i=0; i<=limit; i++)
		{
			int idx = i ;
				if ( ( (type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_QX7 ) ) )	// Taranis
				{
					if ( idx == 12 )
					{
		        b->addItem("S1  ");
						continue ;
					}
					if ( idx == 13 )
					{
		        b->addItem("S2  ");
						continue ;
					}
					if ( idx > 13 )
					{
						idx -= 1 ;
					}
				}
        b->addItem(GvarItems[idx]);
		}
		if ( extraPots )
		{
      b->addItem("P4") ;
			if ( extraPots > 1 )
			{
      	b->addItem("P5") ;
			}
			if ( extraPots > 2 )
			{
      	b->addItem("P6") ;
			}
			if ( extraPots > 3 )
			{
      	b->addItem("P7") ;
			}
		}
//    for(int i=0; i<=11; i++)
//        b->addItem(ExtraGvarItems[i]);
#else
    for(int i=0; i<=28; i++)
        b->addItem(GvarItems[i]);
    int limit = ( (type == 1 ) || ( type == 2 ) ) ? 9 : 6 ;

    for(int i=0; i< limit; i++)
		{
      b->addItem(ExtraGvarItems[i]);
		}
#endif
    b->setCurrentIndex(value);
    b->setMaxVisibleItems(13);
}

int numericSpinGvarValue( QSpinBox *sb, QComboBox *cb, QCheckBox *ck, int value, int defvar, int extended )
{
	if ( ( value < -125 ) || ( value > 125) )
	{
		// Was a GVAR
		if ( ck->checkState() )
		{ // stil is
			value = cb->currentIndex() ;
			if ( extended )
			{
				value += 501 ;
			}
			else
			{
				value += 126 ;
				if ( value > 127 )
				{
      	  value -= 256 ;
				}
      }
		}
		else
		{
			// Now isn't
			value = defvar ;		// Default value
			sb->setValue( value ) ;
			sb->setVisible( true ) ;
			cb->setVisible( false ) ;
		}
	}
	else
	{ // Not a GVAR
		if ( ck->checkState() )
		{ // Now is a GVAR
      value = extended ? 501 : 126 ;
			cb->setCurrentIndex( 0 ) ;
			cb->setVisible( true ) ;
			sb->setVisible( false ) ;
		}
		else
		{ // Still isn't a GVAR
			value = sb->value() ;
		}
	}
	return value ;		 
}

void populateSpinGVarCB( QSpinBox *sb, QComboBox *cb, QCheckBox *ck, int value, int min, int max, int xvalue )
{
  cb->clear() ;
  for (int i=1; i<=5; i++)
	{
    cb->addItem(QObject::tr("GV%1").arg(i));
  }
	sb->setMinimum( min ) ;
	sb->setMaximum( max ) ;

	if ( ( value < -125 ) || ( value > 125) )
	{
		// A GVAR
		if ( value < 0 )
		{
			value += 128+2 ;			
		}
		else
		{
			value -= 126 ;
		}
		// value is now 0-4 for GVAR 1-5
		ck->setChecked( true ) ;
		cb->setCurrentIndex(value) ;
		cb->setVisible( true ) ;
		sb->setVisible( false ) ;
	}
	else
	{
		ck->setChecked( false ) ;
		if ( xvalue == 1 )
		{
			value += 125 ; 
		}
		else if ( xvalue == 3 )
		{
			value -= 125 ; 
		}
		sb->setValue( value ) ;
		sb->setVisible( true ) ;
		cb->setVisible( false ) ;
	}
}



void populateNumericGVarCB( QComboBox *b, int value, int min, int max)
{
  b->clear();
  
	if ( ( value < -125 ) || ( value > 125) )
	{
		// A GVAR
		if ( value < 0 )
		{
			value += 128+2 ;			
		}
		else
		{
			value -= 126 ;
		}
		// value is now 0-4 for GVAR 1-5
		value += max+1 ;		
	}
	value -= min ;

	for (int i=min; i<=max; i++)
	{
    b->addItem(QString::number(i, 10), i);
//    if (value == i)
//      b->setCurrentIndex(b->count()-1);
  }
  for (int i=1; i<=5; i++)
	{
//    int16_t gval = (int16_t)(10000+i);
    b->addItem(QObject::tr("GV%1").arg(i));
//    if (value == gval)
//      b->setCurrentIndex(b->count()-1);
  }
   b->setCurrentIndex(value) ;
}

int numericGvarValue( QComboBox *b, int min, int max )
{
	int value ;

	value = b->currentIndex() ;
	value += min ;
	if ( value > max )
	{
		// A GVAR
		value -= (max+1) ;	// value is now 0-4 for GVAR 1-5 (126,127,-128,-127,-126
    if ( value <= 2 )
		{
			value += 126 ;
		}
		else
		{
			value -= 128+2 ;
		}
	}
	return value ;	
}

int16_t m_to_ft( int16_t metres )
{
	int16_t result ;

  // m to ft *105/32
	result = metres * 3 ;
	metres >>= 2 ;
	result += metres ;
	metres >>= 2 ;
  return result + (metres >> 1 );
}


//	"A1= ",
//	"A2= ",
//	"RSSI",
//	"TSSI",
#define TIMER1		4
#define TIMER2		5
#define FR_ALT_BARO 6
#define FR_GPS_ALT 7
//	"Galt",
//	"Gspd", // 8
//	"T1= ",
//	"T2= ",
//	"RPM ",
//	"FUEL", // 12
//	"Mah1",
//	"Mah2",
//	"Cvlt",
//	"Batt", // 16
//	"Amps",
//	"Mah ",
//	"Ctot",
//	"FasV",	// 20
//	"AccX",
//	"AccY",
//	"AccZ",
#define FR_VSPD	24
#define V_GVAR1	25	//	"Gvr1",
//	"Gvr2",
//	"Gvr3",
//	"Gvr4", // 28
//	"Gvr5",
//	"Gvr6",
#define V_GVAR7	31	 //	"Gvr7",
#define FR_WATT	32
#define FR_RXV			33
#define FR_COURSE   34
#define FR_A3       35
#define FR_A4       36
#define V_SC1       37
#define V_SC2       38
#define V_SC3       39
#define V_SC4       40
#define V_SC5       41
#define V_SC6       42
#define V_SC7       43
#define V_SC8       44
#define V_RTC       45
#define TMOK        46
#define FR_AIRSPEED 47
#define CELL_1      48
#define CELL_2      49
#define CELL_3      50
#define CELL_4      51
#define CELL_5      52
#define CELL_6      53
#define FMODE	      74
#define RUNTIME     (75+8)
#define MODELTIME   (76+8)



// This routine converts an 8 bit value for custom switch use
#ifdef SKY
int16_t convertTelemConstant( int8_t index, int8_t value, SKYModelData *model )
#else
int16_t convertTelemConstant( int8_t index, int8_t value, ModelData *model )
#endif
{
  int16_t result;

	if ( ( index <= V_GVAR7 ) && ( index >= V_GVAR1 ) )
	{
		return value ;
	}
	result = value + 125 ;
  switch (index)
	{
#ifdef SKY
    case MODELTIME :
      result *= 20 ;
    break ;
    case RUNTIME :
      result *= 3 ;
    break;
    case V_RTC :
      result *= 12 ;
    break;
#endif
    case TIMER1 :	// Timer1
    case TIMER2 :	// Timer2
      result *= 10 ;
    break;
		case FR_ALT_BARO:
    case FR_GPS_ALT:
			if ( result > 63 )
			{
      	result *= 2 ;
      	result -= 64 ;
			}
			if ( result > 192 )
			{
      	result *= 2 ;
      	result -= 192 ;
			}
			if ( result > 448 )
			{
      	result *= 2 ;
      	result -= 488 ;
			}
			result *= 10 ;		// Allow for decimal place
      if ( model->FrSkyImperial )
      {
        // m to ft *105/32
        result = m_to_ft( result ) ;
      }
    break;
    case 11:	// RPM
      result *= 100;
    break;
    case 9:   // Temp1
    case 10:	// temp2
      result -= 30;
    break;
    case 13:	// Mah1
    case 14:	// Mah2
    case 18:	// Mah
      result *= 50;
    break;

#ifdef SKY
		case CELL_1:
		case CELL_2:
		case CELL_3:
		case CELL_4:
		case CELL_5:
		case CELL_6:
#endif
		case 15:	// Cell volts
      result *= 2;
		break ;
		case 19 :	// Cells total
		case 20 :	// FAS100 volts
      result *= 2;
		break ;
    case FR_WATT:
      result *= 8 ;
    break;
		case FR_VSPD :
			result = value * 10 ;
		break ;
		case FMODE :
			result = value ;
		break ;
  }
  return result;
}

#define PREC1		1
#define PREC2		2

#ifdef SKY
void stringTelemetryChannel( char *string, int8_t index, int16_t val, SKYModelData *model )
#else
void stringTelemetryChannel( char *string, int8_t index, int16_t val, ModelData *model )
#endif
{
	uint8_t unit = ' ' ;
	uint8_t displayed = 0 ;
	uint8_t att = 0 ;

  switch (index)
	{
#ifdef SKY
    case MODELTIME :
    case V_RTC :
    case RUNTIME :
#endif
    case TIMER1 :
    case TIMER2 :
			{	
				int16_t rem ;

        rem = val % 60 ;
				val /= 60 ;
				sprintf( string, "%d:%02d", val, rem ) ;
//      putsTime(x-FW, y, val, att, att) ;
        displayed = 1 ;
//    	unit = channel + 2 + '1';
			}
		break ;
    
		case 0:
    case 1:
    	{
    	  uint32_t value = val ;
        uint8_t times2 = 0 ;
#ifdef SKY
        SKYFrSkyChannelData *fd ;
#else
#ifndef V2
        FrSkyChannelData *fd ;
#else
        V2FrSkyChannelData *fd ;
#endif
#endif

  			fd = &model->frsky.channels[index] ;
    	  value = val ;
#ifndef V2
#ifdef SKY
        if (fd->units == 2/*V*/)
#else
        if (fd->type == 2/*V*/)
#endif
    		{
    		    times2 = 1 ;
    		}
    		else
    		{
    		    times2 = 0 ;
    		}
#endif
        uint16_t ratio ;
	
#ifdef SKY
        ratio = fd->lratio ;
#else
        ratio = fd->ratio ;
#endif
  			if ( times2 )
  			{
  			    ratio <<= 1 ;
  			}
  			value *= ratio ;
#ifndef V2
#ifdef SKY
        if ( fd->units == 3/*A*/)
#else
        if ( fd->type == 3/*A*/)
#endif
  			{
  			    value /= 100 ;
  			    att = PREC1 ;
  			}
  			else if ( ratio < 100 )
  			{
  			    value *= 2 ;
  			    value /= 51 ;  // Same as *10 /255 but without overflow
  			    att = PREC2 ;
  			}
  			else
  			{
  			    value /= 255 ;
  			}
#endif

#ifndef V2
#ifdef SKY
        if ( (fd->units == 0/*v*/) || (fd->units == 2/*v*/) )
#else
        if ( (fd->type == 0/*v*/) || (fd->type == 2/*v*/) )
#endif
    	  {
 			    att = PREC1 ;
					unit = 'v' ;
    	  }
    	  else
    	  {
#ifdef SKY
          if (fd->units == 3/*A*/)
#else
          if (fd->type == 3/*A*/)
#endif
					{
						unit = 'A' ;
					}
    	  }
#endif
        val = value ;
    	}
    break ;

    case 9:
    case 10:
			unit = 'C' ;
  		if ( model->FrSkyImperial )
  		{
  		  val += 18 ;
  		  val *= 115 ;
  		  val >>= 6 ;
  		  unit = 'F' ;
  		}
    break;
    
		case FR_ALT_BARO:
      unit = 'm' ;
			if (model->FrSkyUsrProto == 1)  // WS How High
			{
      	if ( model->FrSkyImperial )
        	unit = 'f' ;
				break ;
			}
    case FR_GPS_ALT:
      unit = 'm' ;
      if ( model->FrSkyImperial )
      {
        // m to ft *105/32
        val *= 105 ;
				val /= 32 ;
        unit = 'f' ;
      }
    break;
		
		case 17 :
			att |= PREC1 ;
      unit = 'A' ;
		break ;

#ifdef SKY
		case CELL_1:
		case CELL_2:
		case CELL_3:
		case CELL_4:
		case CELL_5:
		case CELL_6:
#endif
		case 15:
			att |= PREC2 ;
      unit = 'v' ;
		break ;
		case 19 :
		case 20 :
			att |= PREC1 ;
      unit = 'v' ;
		break ;
		case 16:
			att |= PREC1 ;
      unit = 'v' ;
		break ;
		case FR_WATT :
      unit = 'w' ;
		break ;
		case FR_VSPD :
			att |= PREC1 ;
			val /= 10 ;
		break ;
		case 37 :	// SC1-4
		case 38 :
		case 39 :
		case 40 :
#ifdef SKY
		case 41 : // SC5-8
		case 42 :
		case 43 :
		case 44 :
#endif
		{
			uint32_t x = index - 37 ;
			ScaleData *pscaler ;
			pscaler = &model->Scalers[x] ;
      unit = "FVCFmAmW"[pscaler->unit] ;
			if ( pscaler->precision )
			{
				att |= ( pscaler->precision == 1 ) ? PREC1 : PREC2 ;
			}
		}			 
    break;
    default:
    break;
  }
	if ( !displayed )
	{
		int16_t rem ;

		switch( att )
		{
			case PREC1 :
				rem = val % 10 ;
				val /= 10 ;
				sprintf( string, "%d.%01d %c", val, rem, unit ) ;
			break ;
			case PREC2 :
				rem = val % 100 ;
				val /= 100 ;
				sprintf( string, "%d.%02d %c", val, rem, unit ) ;
			break ;
			default :
				sprintf( string, "%d %c", val, unit ) ;
			break ;
		}
	}
}

QString getTelemString( int index )
{
	if ( index >= 75 )
	{
		index -= 8 ;
	}
  return TelemItems[index] ;
}

void populateTelItemsCB(QComboBox *b, int start, int value)
{
    b->clear();
		if ( start )
		{
			start = 1 ;			
		}
    for(int i=start; i<=NUM_TELEM_ITEMS; i++)
        b->addItem(TelemItems[i]);
    b->setCurrentIndex(value);
    b->setMaxVisibleItems(30);
}

QString getAudioAlarmName(int val)
{
  return AudioAlarms[val] ;
}

void populateAlarmCB(QComboBox *b, int value=0)
{
    b->clear();
    for(int i=0; i<=15; i++)
        b->addItem(AudioAlarms[i]);
    b->setCurrentIndex(value);
    b->setMaxVisibleItems(16);
}

#ifndef V2
#ifdef SKY
QString getMappedSWName(int val, int eepromType)
{
  int x = ( (eepromType == RADIO_TYPE_TARANIS ) || ( eepromType == RADIO_TYPE_TPLUS ) || ( eepromType == RADIO_TYPE_QX7 ) ) ? 1 : 0 ;
  int limit = MaxSwitchIndex[x] ;

  if(!val) return "---";
  if(val == limit) return "ON";
	if(val == -limit) return "OFF";

  val = switchMap( val, eepromType) ;
  return getSWName( val, -1-eepromType ) ;
}
#else
QString getMappedSWName(int val, int eepromType )
{
  int limit = MaxSwitchIndex ;

  if(!val) return "---";
  if(val == limit) return "ON";
	if(val == -limit) return "OFF";

  val = switchMap( val) ;
  return getSWName( val, eepromType ) ;
}
#endif

#ifdef SKY
QString getSWName(int val, int eepromType)
#else
QString getSWName(int val, int extra )
#endif
{
#ifdef SKY
  int x ;
	if ( eepromType >= 0 )
	{
  	x = ( (eepromType == RADIO_TYPE_TARANIS ) || ( eepromType == RADIO_TYPE_TPLUS ) || ( eepromType == RADIO_TYPE_QX7 ) ) ? 1 : 0 ;
	}
	else
	{
		x = ( (eepromType == -1 ) || ( eepromType == -4 ) ) ? 0 : 1 ;
	}
  int limit = MaxSwitchIndex[x] ;
//	if ( eepromType )
//	{
//		limit = MAX_XDRSWITCH ;
//	}
#endif
#ifndef SKY
//  int x = extra ? 1 : 0 ;
  int limit = MaxSwitchIndex ; //MAX_DRSWITCH ;
	if ( extra )
	{
		limit += EXTRA_CSW ;
	}
#endif
  if(!val) return "---";
#ifdef SKY
//  if ( eepromType )
//	{
//  	if(val== limit) return "ON";
//  	if(val== -limit) return "OFF";
//	}
//	else
//	{
//	if ( (eepromType == 1 ) || ( eepromType == 2 ) )
	{
		int mval = switchUnMap( val, eepromType ) ;
#else
  int mval = switchUnMap( val ) ;
#endif
    if(mval == limit) return "ON";
  	if(mval == -limit) return "OFF";
#ifdef SKY
    if(mval == limit+1) return "Fmd";
  }
//  }
#endif

	QString switches ;
	switches = SWITCHES_STR ;
#ifdef SKY
	if ( x > 0 )
	{
		switches = XSWITCHES_STR ;
	}
	int sw = val ;
	if ( sw < 0 )
	{
		sw = -val ;
	}
//  if ( sw >= limit )
//	{
//		sw -= 2 ;
//	}
#else
	int sw = val ;
	if ( sw < 0 )
	{
		sw = -val ;
	}
  if ( sw >= limit )
	{
		sw -= 3 ;
	}
#endif

  QString temp ;
  temp = switches.mid((sw-1)*3,3) ;
  return QString(val<0 ? "!" : "") + temp ;
}
#endif

#ifdef SKY
int getSwitchCbValue( QComboBox *b, int eepromType )
{
	int value ;
  int x = ( (eepromType == RADIO_TYPE_TARANIS ) || ( eepromType == RADIO_TYPE_TPLUS ) || ( eepromType == RADIO_TYPE_QX7 ) ) ? 1 : 0 ;
  int limit = MaxSwitchIndex[x] ;
//	if ( eepromType )
//	{
//		limit = MAX_XDRSWITCH ;
//	}
	value = b->currentIndex()-limit ;
	return switchMap( value, eepromType ) ;
}
#else
#ifndef V2
int getSwitchCbValue( QComboBox *b, int eepromType )
{
	int value ;
  int limit = MaxSwitchIndex ;
	value = b->currentIndex()-limit ;
  return switchMap( value ) ;
}
#endif
#endif

#ifdef SKY

int getAndSwitchCbValue( QComboBox *b )
{
  int limit = MaxSwitchIndex[0] - 1 ;
	int value = b->currentIndex() - limit ;
	value = switchMap( value, 0 ) ;
	if ( value == 9 )
	{
		value = 9+NUM_SKYCSW+1 ;
	}
	if ( value == -9 )
	{
		value = -(9+NUM_SKYCSW+1) ;
	}
	if ( ( value > 9 ) && ( value <= 9+NUM_SKYCSW+1 ) )
	{
		value -= 1 ;
	}
	if ( ( value < -9 ) && ( value >= -(9+NUM_SKYCSW+1) ) )
	{
		value += 1 ;
	}

	return value ;
}

int getSwitchCbValueShort( QComboBox *b, int eepromType )
{
	int value ;
  int x = ( (eepromType == 1 ) || ( eepromType == 2 ) || ( eepromType == RADIO_TYPE_QX7 ) ) ? 1 : 0 ;
  int limit = MaxSwitchIndex[x]-1 ;
//	if ( eepromType )
//	{
//		limit = MAX_XDRSWITCH ;
//	}
	value = b->currentIndex()-limit ;
	return switchMap( value, eepromType ) ;
}
#endif

#ifdef SKY
int getTimerSwitchCbValue( QComboBox *b, int eepromType )
{
	int value ;
  int x = ( (eepromType == RADIO_TYPE_TARANIS ) || ( eepromType == RADIO_TYPE_TPLUS ) || ( eepromType == RADIO_TYPE_QX7 ) ) ? 1 : 0 ;
  int limit = MaxSwitchIndex[x] ;
  int hsw_max = ( (eepromType == RADIO_TYPE_TARANIS ) || ( eepromType == RADIO_TYPE_TPLUS ) || ( eepromType == RADIO_TYPE_QX7 ) ) ? HSW_MAX_X9D : HSW_MAX ;

//	if ( eepromType )
//	{
//		limit = MAX_XDRSWITCH ;
//	}
	value = b->currentIndex()-(limit-1) ;
//	if ( eepromType == 0 )
//  {
		if ( value > limit-1 )
		{
			value = switchMap( value - limit + 1, eepromType ) + hsw_max ;
		}
		else
		{
			value = switchMap( value, eepromType ) ;
		}
//	}
	return value ;
}

#else
#ifndef V2
int getTimerSwitchCbValue( QComboBox *b, int eepromType )
{
	int value ;
//  int x = ( (eepromType == 1 ) || ( eepromType == 2 ) ) ? 1 : 0 ;
  int limit = MaxSwitchIndex ;
	value = b->currentIndex()-(limit-1) ;
	if ( value > limit-1 )
	{
    value = switchMap( value - limit + 1 ) + HSW_MAX ;
	}
	else
	{
    value = switchMap( value ) ;
	}
	return value ;
}

#endif
#endif


#ifndef V2
#ifdef SKY
void populateSwitchCB(QComboBox *b, int value, int eepromType)
#else
void populateSwitchCB(QComboBox *b, int value, int eepromType)
#endif
{
  b->clear();
#ifdef SKY
  int x = ( (eepromType == RADIO_TYPE_TARANIS ) || ( eepromType == RADIO_TYPE_TPLUS ) || ( eepromType == RADIO_TYPE_QX7 ) ) ? 1 : 0 ;
  int limit = MaxSwitchIndex[x] ;
//	if ( eepromType )
//	{
//		limit = MAX_XDRSWITCH ;
//	}
#endif
#ifndef SKY
  int limit = MaxSwitchIndex ;
#endif
    for(int i=-limit; i<=limit; i++)
		{
      b->addItem(getMappedSWName(i,eepromType));
		}
		int j ;
#ifdef SKY
    j = switchUnMap(value, eepromType) ;
#else
    j = switchUnMap(value) ;
#endif
    b->setCurrentIndex(j+limit);
    b->setMaxVisibleItems(10);
}
#endif


#ifndef V2
void populateTrainerSwitchCB(QComboBox *b, int value=0)
{
    b->clear();
    for(int i=-15; i<=15; i++)
#ifdef SKY
        b->addItem(getSWName(i,0));
#else
        b->addItem(getSWName(i,0));
#endif
    b->setCurrentIndex(value+15);
    b->setMaxVisibleItems(10);
}
#endif


#ifndef V2
#ifdef SKY
void populateSwitchShortCB(QComboBox *b, int value, int eepromType)
#else
void populateSwitchShortCB(QComboBox *b, int value, int eepromType)
#endif
{
    b->clear();
#ifndef SKY
	int limit = MAX_DRSWITCH-1 ;
  if ( (eepromType == 1 ) || ( eepromType == 2 ) )
	{
    limit += EXTRA_CSW ;
	}
#endif
#ifdef SKY
  int x = ( (eepromType == RADIO_TYPE_TARANIS ) || ( eepromType == RADIO_TYPE_TPLUS ) || ( eepromType == RADIO_TYPE_QX7 ) ) ? 1 : 0 ;
  int limit = MaxSwitchIndex[x]-1 ;
//	if ( eepromType )
//	{
//		limit = MAX_XDRSWITCH-1 ;
//	}
#endif
    for(int i = -limit; i<=limit; i++)
#ifdef SKY
		{
//			int j ;
//			j = eepromType ? i : switchMap(i, eepromType) ;
      b->addItem(getMappedSWName(i,eepromType));
		}
#else
        b->addItem(getSWName(i,eepromType));
#endif
#ifdef SKY
		int j ;
		j = switchUnMap(value, eepromType) ;
    b->setCurrentIndex(j+limit);
#else
    b->setCurrentIndex(value+limit);
#endif
    b->setMaxVisibleItems(10);
}
#endif

int populatePhasetrim(QComboBox *b, int which, int value)
{	// which i s0 for FP1, 1 for FP2 etc.
	char name[4] ;
	name[0] = 'F' ;
	name[1] = 'M' ;
	name[3] = 0 ;
	
	b->clear();
#ifdef SKY
#else
		value += TRIM_EXTENDED_MAX+1 ;
#endif
	if ( value > TRIM_EXTENDED_MAX )
	{
		value -= TRIM_EXTENDED_MAX ;
	}
	else
	{
		value = 0 ;
	}
 	b->addItem( "Own trim" ) ;
#ifdef SKY
 	for( int i= 0 ; i < 7 ; i += 1 )
#else
 	for( int i= 0 ; i < 5 ; i += 1 )
#endif
	{
		if ( i == which )
		{
			continue ;			
		}
		name[2] = i + '0' ;
   	b->addItem( name ) ;
	}
	b->setCurrentIndex(value);
	b->setMaxVisibleItems( 7 ) ;
	return value ;
}

int decodePhaseTrim( int16_t *existing, int index )
{
	if ( index == 0 )
	{
#ifdef SKY
		if ( *existing > TRIM_EXTENDED_MAX )
		{
			*existing = 0 ;			
		}
#else
		if ( *existing >= 0 )
		{
			*existing = -(TRIM_EXTENDED_MAX+1) ;
		}
#endif
		return -1 ;
	}
#ifdef SKY
	*existing = TRIM_EXTENDED_MAX + index ;
#else
	*existing = TRIM_EXTENDED_MAX + index - (TRIM_EXTENDED_MAX+1) ;
#endif
	return index ;
}

#ifndef V2
#ifndef SKY
void populateSwitchxAndCB(QComboBox *b, int value, int eepromType)
{
	b->clear();
  int limit = MaxSwitchIndex-1 ;
//  b->addItem("!TRN");
  for(int j=-limit; j<=limit; j++)
	{
		int i ;
  	i = switchMap(j) ;

//		if ( ( i > 8 ) && ( i <= 9+NUM_CSW+EXTRA_CSW ) )
//		{
//			i = i + 1 ;
//		}
//		else if ( ( i < -8 ) && ( i >= -(9+NUM_CSW+EXTRA_CSW) ) )
//		{
//			i= i - 1 ;
//		}
		b->addItem(getSWName(i,eepromType));
	}
//  b->addItem("TRN");
	int x = value ;
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
		x = 9 ;
	}
	if ( x == -(9+NUM_CSW+EXTRA_CSW+1) )
	{
		x = -9 ;
	}
  x = switchUnMap(x) ;

  b->setCurrentIndex(x+limit);
	b->setMaxVisibleItems(10);
}
#endif

#ifndef SKY
int getxAndSwitchCbValue( QComboBox *b, int eepromType )
{
  int limit = MaxSwitchIndex - 1 ;
	int value = b->currentIndex() - limit ;
//	if ( abs(value) == limit )
//	{
//		return (value > 0) ? 9 : -9 ;	// "TRN"
//	}
	value = switchMap( value ) ;
	if ( value == 9 )
	{
		value = 9+NUM_CSW+EXTRA_CSW+1 ;
	}
	if ( value == -9 )
	{
		value = -(9+NUM_CSW+EXTRA_CSW+1) ;
	}
	if ( ( value > 9 ) && ( value <= 9+NUM_CSW+EXTRA_CSW+1 ) )
	{
		value -= 1 ;
	}
	if ( ( value < -9 ) && ( value >= -(9+NUM_CSW+EXTRA_CSW+1) ) )
	{
		value += 1 ;
	}

	return value ;
}
#endif
#endif

void x9dPopulateSwitchAndCB(QComboBox *b, int value=0)
{
#ifdef SKY
	char name[6] ;
	name[0] = '!' ;
	name[1] = 'L' ;
//	name[2] = 'W' ;
	name[3] = 0 ;
	int maxsw ;

	maxsw = '9' ;

  b->clear();
//  b->addItem("!SH");
	for(int i='O' ; i>= 'A' ; i -= 1 )
	{
		name[2] = i ;
     b->addItem(name);
	}
  for(int i= maxsw ; i>='1'; i -= 1 )
	{
		name[2] = i ;
    b->addItem(name);
	}
 	
  for(int i=-(MaxSwitchIndex[1]-1-NUM_SKYCSW) ; i<=(MaxSwitchIndex[1]-1-NUM_SKYCSW); i += 1)
  {
		int j ;
    j = switchMap(i, 1) ;
    b->addItem(getSWName(j,-2));
  }

	name[0] = 'L' ;
//	name[1] = 'W' ;
	name[2] = 0 ;
  for(int i='1'; i<= maxsw; i++)
	{
		name[1] = i ;
    b->addItem(name);
	}
  for(int i='A' ; i<= 'O' ; i++)
	{
		name[1] = i ;
    b->addItem(name);
	}
//  b->addItem("SH");

	int j ;
  j = switchUnMap(value, 1) ;
  b->setCurrentIndex(j+MaxSwitchIndex[1]-1) ;
  b->setMaxVisibleItems(10);
#endif
	 
}


#ifdef SKY

int32_t andSwitchMap( int32_t x )
{
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
		x = 9 ;
	}
	if ( x == -(9+NUM_SKYCSW+1) )
	{
		x = -9 ;
	}
  x = switchUnMap(x, 0 ) ;
	return x ;	
}
#endif

#ifndef V2
void populateSwitchAndCB(QComboBox *b, int value=0)
{
	char name[6] ;
	name[0] = '!' ;
	name[1] = 'L' ;
//	name[2] = 'W' ;
	name[3] = 0 ;
	int maxsw ;

#ifdef SKY
	maxsw = '9' ;
#else
	maxsw = '7' ;
#endif
	
	b->clear();
  
#ifdef SKY
	for(int i='O' ; i>= 'A' ; i -= 1 )
	{
		name[2] = i ;
     b->addItem(name);
	}
  for(int i= maxsw ; i>='1'; i -= 1 )
	{
		name[2] = i ;
    b->addItem(name);
	}
#endif
#ifdef SKY
//  b->addItem("!TRN");
  for(int i=-(MaxSwitchIndex[0]-2-NUM_SKYCSW+1) ; i<=(MaxSwitchIndex[0]-2-NUM_SKYCSW+1); i += 1)
#else 	
	for(int i=0 ; i<=8; i += 1)
#endif
#ifdef SKY
  {
		int j ;
    j = switchMap(i, 0) ;
    b->addItem(getSWName(j,0));
  }
#else
    b->addItem(getSWName(i,0));
#endif
  
//#ifdef SKY
//	b->addItem("TRN");
//#endif

	name[0] = 'L' ;
//	name[1] = 'W' ;
	name[2] = 0 ;
  for(int i='1'; i<= maxsw; i++)
	{
		name[1] = i ;
    b->addItem(name);
	}
#ifdef SKY
  for(int i='A' ; i<= 'O' ; i++)
	{
		name[1] = i ;
    b->addItem(name);
	}
#endif
#ifdef SKY
    b->setCurrentIndex(andSwitchMap( value )+MaxSwitchIndex[0]-1) ;
#else
    b->setCurrentIndex(value);//+MAX_DRSWITCH-1);
#endif
  b->setMaxVisibleItems(10);
}
#endif

//void populateSwitchAndCB(QComboBox *b, int value=0)
//{
//	char name[4] ;
//	name[0] = 'C' ;
//	name[1] = 'S' ;
//	name[3] = 0 ;
//    b->clear();
//    for(int i=0 ; i<=8; i++)
//        b->addItem(getSWName(i));

//    for(int i=1; i<=7; i++)
//		{
//			name[2] = i + '0' ;
//      b->addItem(name);
//		}
//    b->setCurrentIndex(value);
//    b->setMaxVisibleItems(10);
//}

#ifndef V2
#ifdef SKY
void populateSafetySwitchCB(QComboBox *b, int type, int value, int eepromType )
#else
void populateSafetySwitchCB(QComboBox *b, int type, int value, int extra )
#endif
{
	int offset = MAX_DRSWITCH ;
	int start = -MAX_DRSWITCH ;
	int last = MAX_DRSWITCH ;
#ifdef SKY
  int x = ( (eepromType == RADIO_TYPE_TARANIS ) || ( eepromType == RADIO_TYPE_TPLUS ) || ( eepromType == RADIO_TYPE_QX7 ) ) ? 1 : 0 ;
//	if ( eepromType )
//	{
//		offset = MAX_XDRSWITCH ;
//		start = -MAX_XDRSWITCH ;
//		last = MAX_XDRSWITCH ;
//	}
//	else
//	{
  offset = MaxSwitchIndex[x] ;
  start = -MaxSwitchIndex[x] ;
  last = MaxSwitchIndex[x] ;
//	}
#endif
	if ( type == VOICE_SWITCH )
	{
#ifndef SKY
		offset = 0 ;
		start = 0 ;		
		last = MAX_DRSWITCH - 1 ;
#endif
#ifdef SKY
		offset -= 1 ;
    start += 1 ;
		last -=  1 ;
#endif
#ifndef SKY
		if ( extra )
		{
			last += EXTRA_CSW ;
		}
#endif
	}
    b->clear();
    for(int i= start ; i<=last; i++)
#ifdef SKY
		{
      b->addItem(getMappedSWName(i,eepromType));
		}
#else
        b->addItem(getSWName(i,extra));
#endif
		if (type == 2 )
		{
      	b->addItem(" 8 seconds");
      	b->addItem("12 seconds");
      	b->addItem("16 seconds");
		}
#ifdef SKY
		int j ;
		j = switchUnMap(value, eepromType) ;
    b->setCurrentIndex(j+offset);
#else
    b->setCurrentIndex(value+ offset ) ;
#endif
    b->setMaxVisibleItems(10);
}
#endif


QString SafetyType[] = {"S","A","V", "X"};
#ifndef V2
QString VoiceType[] = {"ON", "OFF", "BOTH", "15Secs", "30Secs", "60Secs", "Varibl"} ;
#else
QString VoiceType[] = {"ON", "OFF", "BOTH", "ALL", "ONCE" } ;
#endif

void populateSafetyVoiceTypeCB(QComboBox *b, int type, int value=0)
{
  b->clear();
#ifndef V2
		if ( type == 0 )
		{
    	for(int i= 0 ; i<=3; i++)
        b->addItem(SafetyType[i]);
    	b->setCurrentIndex(value);
    	b->setMaxVisibleItems(4);
		}
		else
		{
    	for(int i= 0 ; i<=6; i++)
        b->addItem(VoiceType[i]);
    	b->setCurrentIndex(value);
    	b->setMaxVisibleItems(7);
		}
#else
	if ( type == 0 )
	{
	  b->addItem("Safety") ;
  	b->addItem("Sticky") ;
   	b->setCurrentIndex(value) ;
	  b->setMaxVisibleItems(2) ;
	}
	else
	{
   	for(int i= 0 ; i<=4; i++)
		{
       b->addItem(VoiceType[i]);
		}
   	b->setCurrentIndex(value);
   	b->setMaxVisibleItems(5);
	}
#endif
}

#ifndef V2
#ifdef SKY
void populateTmrBSwitchCB(QComboBox *b, int value, int eepromType)
#else
void populateTmrBSwitchCB(QComboBox *b, int value, int eepromType )
#endif
{
	int i ;
    b->clear();
#ifdef SKY
  int x = ( (eepromType == RADIO_TYPE_TARANIS ) || ( eepromType == RADIO_TYPE_TPLUS ) || ( eepromType == RADIO_TYPE_QX7 ) ) ? 1 : 0 ;
  int limit = MaxSwitchIndex[x]-1 ;
  int hsw_max = ( (eepromType == RADIO_TYPE_TARANIS ) || ( eepromType == RADIO_TYPE_TPLUS ) || ( eepromType == RADIO_TYPE_QX7 ) ) ? HSW_MAX_X9D : HSW_MAX ;
//	if ( eepromType )
//	{
//		limit = MAX_XDRSWITCH - 1 ;
//	}
#endif
#ifndef SKY
//  int limit = MaxSwitchIndex - 1 ;
	int limit = MaxSwitchIndex-1 ;
//	if ( extra )
//	{
//		limit += EXTRA_CSW ;
//	}
#endif
    for( i= -limit; i<= limit; i++)
#ifdef SKY
		{
      b->addItem(getMappedSWName(i,eepromType));
		}
#else
      b->addItem(getMappedSWName(i,eepromType));
//        b->addItem(getSWName(i,extra));
#endif
#ifdef SKY    
		for( i=1 ; i<=limit; i++)
		{
      b->addItem('m'+getMappedSWName(i,eepromType));
		}
#endif    
#ifndef SKY
		for( i=1 ; i<=limit; i++)
		{
      b->addItem('m'+getMappedSWName(i,eepromType));
//      b->addItem('m'+getSWName(i,extra));
		}
#endif    
#ifdef SKY
//		if ( eepromType )
//		{
//			b->setCurrentIndex(value+limit);
//		}
//		else
//		{
			int j ;
			if ( value > hsw_max )
			{
        j = switchUnMap( value - hsw_max, eepromType ) + MaxSwitchIndex[x] - 1 ;
			}
			else
			{
				j = switchUnMap( value, eepromType ) ;
			}
	    b->setCurrentIndex(j+limit) ;
//		}
#else
			int j ;
			if ( value > HSW_MAX )
			{
        j = switchUnMap( value - HSW_MAX ) + MaxSwitchIndex - 1 ;
			}
			else
			{
				j = switchUnMap( value ) ;
			}
	    b->setCurrentIndex(j+limit) ;
//		b->setCurrentIndex(value+limit);
#endif
    b->setMaxVisibleItems(10);
}
#endif

void populateCurvesCB(QComboBox *b, int value)
{
	int j ;
  QString str = CURV_STR;
  j = (str.length()/3)-1 ;
#ifdef SKY
	j = 6 + 19 ;
#endif

    b->clear();
    for(int i=j; i >= 7 ; i -= 1)
		{
			b->addItem(str.mid(i*3,3).replace("c","!Curve "));
		}
    for(int i=0; i<=j; i++)  b->addItem(str.mid(i*3,3).replace("c","Curve "));
#ifdef SKY    
		b->setCurrentIndex(value+19);
#else
    b->setCurrentIndex(value+16);
#endif
    b->setMaxVisibleItems(10);
}


#ifdef SKY
void populateTimerSwitchCB(QComboBox *b, int value )
#else
void populateTimerSwitchCB(QComboBox *b, int value=0, int eepromType=0)
#endif
{
    b->clear();
#ifdef SKY
    for(int i=0 ; i<TMR_NUM_OPTION; i++)
        b->addItem(getTimerMode(i));
    b->setCurrentIndex(value);
#else
    for(int i=0 ; i<TMR_NUM_OPTION; i++)
        b->addItem(getTimerMode(i,eepromType));
    b->setCurrentIndex(value);
//		int num_options = TMR_NUM_OPTION ;
//    if ( eepromType )
//		{
//			num_options += EXTRA_CSW * 2 ;
//		}

//    for(int i=-num_options; i<=num_options+16; i++)
//        b->addItem(getTimerMode(i,eepromType));
//    b->setCurrentIndex(value+num_options);
#endif
    b->setMaxVisibleItems(10);
}

#ifdef SKY
QString getTimerMode(int tm)
#else
QString getTimerMode(int tm, int eepromType )
#endif
{

//#ifdef SKY    
    QString str = CURV_STR;
    QString stt = "OFFON THsTH%";
//#else
//    QString str = SWITCHES_STR;
//    QString stt = "OFFABSRUsRU%ELsEL%THsTH%ALsAL%P1 P1%P2 P2%P3 P3%";
//#endif

    QString s;
//#ifdef SKY    
    if(tm<TMR_VAROFS)
    {
        s = stt.mid(abs(tm)*3,3);
//        if(tm<-1) s.prepend("!");
        return s;
    }
		tm -= TMR_VAROFS ;

		if ( tm < 9 )
		{
       s = str.mid(tm*3+21,2) ;
		}
		else
		{
       s = str.mid(tm*3+21,3) ;
		}
     return s +'%' ;


//    s = "m" + str.mid((abs(tm)-(TMR_VAROFS+MAX_DRSWITCH-1))*3,3);
//    if(tm<0) s.prepend("!");
//    return s;
//#else
//    if(abs(tm)<TMR_VAROFS)
//    {
//        s = stt.mid(abs(tm)*3,3);
//        if(tm<-1) s.prepend("!");
//        return s;
//    }

//		int max_drswitch = MAX_DRSWITCH ;
//		if ( eepromType )
//		{
//			max_drswitch += EXTRA_CSW ;			
//		}

//    if(abs(tm)<(TMR_VAROFS+max_drswitch-1))
//    {
//        s = str.mid((abs(tm)-TMR_VAROFS)*3,3);
//        if(tm<0) s.prepend("!");
//        return s;
//    }

//    if(abs(tm)<(TMR_VAROFS+max_drswitch-1+max_drswitch-1))
//		{
//	    s = "m" + str.mid((abs(tm)-(TMR_VAROFS+max_drswitch-1))*3,3);
//  	  if(tm<0) s.prepend("!");
//    	return s;
//		}

//    str = CURV_STR ;
//		tm -= TMR_VAROFS+max_drswitch-1+max_drswitch-1 ;
//		if ( tm < 9 )
//		{
//       s = str.mid(tm*3+21,2) ;
//		}
//		else
//		{
//       s = str.mid(tm*3+21,3) ;
//		}
//    return s +'%' ;

//#endif

}

#define MODI_STR  "Rud Ele Thr Ail Rud Thr Ele Ail Ail Ele Thr Rud Ail Thr Ele Rud "
#ifdef SKY    
#define SRCP_STR  "P1  P2  P3  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGV1 GV2 GV3 GV4 GV5 GV6 GV7 THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 PPM9PP10PP11PP12PP13PP14PP15PP16CH25CH26CH27CH28CH29CH30CH31CH32"
#else
#define SRCP_STR  "P1  P2  P3  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH163POSGV1 GV2 GV3 GV4 GV5 GV6 GV7 THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#endif

#ifdef SKY    
QString getSourceStr(int stickMode, int idx, int modelVersion, int type, uint32_t extraPots )
#else
QString getSourceStr(int stickMode, int idx, int modelVersion )
#endif
{
    if(!idx)
        return "----";
    else if(idx>=1 && idx<=4)
    {
        QString modi = MODI_STR;
				if ( modelVersion >= 2 )
				{
        	return modi.mid( (idx-1) * 4, 4 ) ;
				}
        return modi.mid((idx-1)*4+stickMode*16,4);
    }
    else
    {
        QString str = SRCP_STR;
#ifdef SKY
        if ( ( type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_X9E ) || ( type == RADIO_TYPE_QX7 ) )	// Taranis
				{
					if ( idx == 7 )
					{
						return "SL  " ;
					}
					if ( idx == 8 )
					{
						return "SR  " ;
					}
//					int offset = 1 ;
					if ( type == RADIO_TYPE_X9E )
					{
						if ( idx == 9 )
						{
							return "S1  " ;
						}
						if ( idx == 10 )
						{
							return "S2  " ;
						}
						if ( idx > 10 )
						{
          	  idx -= 3 ;
						}
					}
					else if ( type == RADIO_TYPE_TPLUS )	// Plus
					{
						if ( idx == 9 )
						{
							return "P3  " ;
						}
						if ( idx > 9 )
						{
							idx -= 2 ;
						}
					}
					else
					{
						if ( idx > 8 )
						{
							idx -= 1 ;
						}
					}
				}
				else if ( ( type == RADIO_TYPE_SKY ) || ( type == RADIO_TYPE_9XTREME ) )
				{
					if ( extraPots )
					{
						switch ( idx )
						{
							case 8 :
							return "P4  " ;
							case 9 :
								if ( extraPots > 1 )
								{
									return "P5  " ;
								}
							break ;
							case 10 :
								if ( extraPots > 2 )
								{
									return "P6  " ;
								}
							break ;
							case 11 :
								if ( extraPots > 3 )
								{
									return "P7  " ;
								}
							break ;
						}
						if ( idx > 7 )
						{
							idx -= extraPots ;
						}
					}
				}
#endif
				idx -= 5 ;
#ifdef SKY
				if ( idx != 40 )
#else
				if ( idx != 32 )
#endif
				{
        	return str.mid((idx)*4,4);
				}
       	return "s" ;
    }

    return "";
}

#ifdef SKY    
void populateSourceCB(QComboBox *b, int stickMode, int telem, int value, int modelVersion, int type, uint32_t extraPots )
#else
void populateSourceCB(QComboBox *b, int stickMode, int telem, int value, int modelVersion)
#endif
{
    b->clear();

		if ( modelVersion >= 2 )
		{
			stickMode = 0 ;
		}

#ifdef SKY    
		int limit = 45 ;
    if ( ( type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_X9E ) )	// Taranis
		{
			limit = 46 ;
			if ( type == RADIO_TYPE_TPLUS )
			{
				limit = 47 ;
			}
			if ( type == RADIO_TYPE_X9E )
			{
				limit = 48 ;
			}
		}
		if ( type == RADIO_TYPE_QX7 )
		{
			limit = 44 ;
		}
		if ( ( type == RADIO_TYPE_SKY ) || ( type == RADIO_TYPE_9XTREME ) )
		{
			limit += extraPots ;
		}
    for(int i=0; i<limit; i++)
		{
			int j = i ;
			if ( type == RADIO_TYPE_QX7 )
			{
				if ( j > 6 )
				{
					j += 2 ;	// Skip 3rd pot
				}
			}
			b->addItem(getSourceStr(stickMode,j,modelVersion, type, extraPots));
		}
#else
    for(int i=0; i<37; i++) b->addItem(getSourceStr(stickMode,i));
#endif
		if ( telem )
		{
    	for(int i=1; i<=NUM_TELEM_ITEMS; i++)
    	    b->addItem(TelemItems[i]);
		}
#ifdef SKY    
    if ( ( type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_X9E ) )	// Taranis
		{
			if ( value >= EXTRA_POTS_POSITION )
			{
				if ( value >= EXTRA_POTS_START )
				{
					if ( value < EXTRA_POTS_START + 8 )
					{
						value -= ( EXTRA_POTS_START - EXTRA_POTS_POSITION ) ;
					}
					else
					{
						value -= 8 ;
						value += (type == RADIO_TYPE_TPLUS) ? 2 : (type == RADIO_TYPE_X9E) ? 3 : NUM_EXTRA_POTS ;
					}
				}
				else
				{
					value += (type == RADIO_TYPE_TPLUS) ? 2 : type == RADIO_TYPE_X9E ? 3 : NUM_EXTRA_POTS ;
				}
			}
		}
		if ( ( type == RADIO_TYPE_SKY ) || ( type == RADIO_TYPE_9XTREME ) )
		{
			if ( value >= EXTRA_POTS_POSITION )
			{
				if ( value >= EXTRA_POTS_START )
				{
					if ( value < EXTRA_POTS_START + 8 )
					{
						value -= ( EXTRA_POTS_START - EXTRA_POTS_POSITION ) ;
					}
					else
					{
						value -= 8 ;
						value += extraPots ;
					}
				}
				else
				{
					value += extraPots ;
				}
			}
		}
		if ( type == RADIO_TYPE_QX7 )
		{
			if ( value > 6 )
			{
			 value -= 1 ;	
			}
		}
#endif
    b->setCurrentIndex(value);
    b->setMaxVisibleItems(10);
}

#ifdef SKY    
uint32_t decodePots( uint32_t value, int type, uint32_t extraPots )
{
	if ( type == RADIO_TYPE_QX7 )
	{
		if ( value > 6 )
		{
		 value += 1 ;	
		}
	}
  if ( ( type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_X9E ) )	// Taranis
	{
		if ( value >= EXTRA_POTS_POSITION )
		{
      if ( value < EXTRA_POTS_POSITION + ( type == RADIO_TYPE_TPLUS ? 2 : type == RADIO_TYPE_X9E ? 3 : NUM_EXTRA_POTS) )
			{
				value += EXTRA_POTS_START - EXTRA_POTS_POSITION ;
			}
			else
			{
        value -= type == RADIO_TYPE_TPLUS ? 2 : type == RADIO_TYPE_X9E ? 3 : NUM_EXTRA_POTS ;
				if ( value >= EXTRA_POTS_START )
				{
					value += 8 ;
				}
			}
		}
	}
	if ( ( type == RADIO_TYPE_SKY ) || ( type == RADIO_TYPE_9XTREME ) )
	{
		if ( value >= EXTRA_POTS_POSITION )
		{
      if ( value < EXTRA_POTS_POSITION + extraPots )
			{
				value += EXTRA_POTS_START - EXTRA_POTS_POSITION ;
			}
			else
			{
        value -= extraPots ;
				if ( value >= EXTRA_POTS_START )
				{
					value += 8 ;
				}
			}
		}
	}
  return value ;
}
#endif


QString getCSWFunc(int val, uint8_t modelVersion )
{
	if ( modelVersion >= 3 )
	{
		if ( val == CS_LATCH )
		{
			return "Latch" ;
		}
		if ( val == CS_FLIP )
		{
			return "F-Flop" ;
		}
	}
  return QString(CSWITCH_STR).mid(val*CSW_LEN_FUNC,CSW_LEN_FUNC);
}

#ifdef SKY    
const uint8_t SwitchFunctionMap[] = { 0,1,2,3,4,18,21,19,5,6,7,8,9,10,11,20,12,13,14,15,16,17} ;

uint8_t locateSwFunc( int value )
{
	uint32_t c ;
  uint8_t *options = ( uint8_t *)SwitchFunctionMap ;
	for ( c = 0 ; c < sizeof(SwitchFunctionMap) ; c += 1 )
	{
		if ( options[c] == value )
		{
			value = c ;
			break ;
		}
	}
  if ( c >= sizeof(SwitchFunctionMap ) )
	{
		return 0 ;
	}
	return value ;
}

uint8_t unmapSwFunc( int value )
{
	return SwitchFunctionMap[value] ;
}


#endif

void populateCSWCB(QComboBox *b, int value, uint8_t modelVersion)
{
	int last = CSW_NUM_FUNC ;
	if ( modelVersion & 0x80 )
	{
		last += 1 ;
		modelVersion &= 0x7F ;
	}
    b->clear();
#ifdef SKY    
    for(int i=0; i<last; i++) b->addItem(getCSWFunc( SwitchFunctionMap[i], modelVersion));
    b->setCurrentIndex( locateSwFunc( value ) ) ;
#else
    for(int i=0; i<last; i++) b->addItem(getCSWFunc( i, modelVersion));
    b->setCurrentIndex(value);
#endif
    b->setMaxVisibleItems(10);
}

int getValueFromLine(const QString &line, int pos, int len=2)
{
    bool ok;
    int hex = line.mid(pos,len).toInt(&ok, 16);
    return ok ? hex : -1;
}

QString iHEXLine(quint8 * data, quint16 addr, quint8 len)
{
    QString str = QString(":%1%2000").arg(len,2,16,QChar('0')).arg(addr,4,16,QChar('0')); //write start, bytecount (32), address and record type
    quint8 chkSum = 0;
    chkSum = -len; //-bytecount; recordtype is zero
    chkSum -= addr & 0xFF;
    chkSum -= addr >> 8;
    for(int j=0; j<len; j++)
    {
        str += QString("%1").arg(data[addr+j],2,16,QChar('0'));
        chkSum -= data[addr+j];
    }

    str += QString("%1").arg(chkSum,2,16,QChar('0'));
    return str.toUpper(); // output to file and lf;
}

uint32_t BlockIndex ;
uint32_t BlockSizes[10] ;
uint32_t BlockAddresses[10] ;

int loadiHEX(QWidget *parent, QString fileName, quint8 * data, int datalen, QString header)
{
  int offset = 0 ;
    //load from intel hex type file
    QFile file(fileName);
    int finalSize = 0;
		uint32_t nextAddress = 0 ;
    BlockAddresses[0] = 0 ;
		BlockIndex = 0 ;

    if(!file.exists())
    {
        QMessageBox::critical(parent, QObject::tr("Error"),QObject::tr("Unable to find file %1!").arg(fileName));
        return 0;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {  //reading HEX TEXT file
        QMessageBox::critical(parent, QObject::tr("Error"),
                              QObject::tr("Error opening file %1:\n%2.")
                              .arg(fileName)
                              .arg(file.errorString()));
        return 0;
    }

    memset(data,0,datalen);
		BlockIndex = 0 ;

    QTextStream in(&file);

    if(!header.isEmpty())
    {
        QString hline = in.readLine();

        if(hline!=header)
        {
            QMessageBox::critical(parent, QObject::tr("Error"),
                                  QObject::tr("Invalid EEPE File Format %1")
                                  .arg(fileName));
            file.close();
            return 0;
        }
    }

    while (!in.atEnd())
    {
        QString line = in.readLine();

        if(line.left(1)!=":") continue;

        int byteCount = getValueFromLine(line,1);
        int address = getValueFromLine(line,3,4);
        int recType = getValueFromLine(line,7);
				if (recType==0x02)
				{
        	offset+=0x010000;
				}

        if(byteCount<0 || address<0 || recType<0)
        {
            QMessageBox::critical(parent, QObject::tr("Error"),QObject::tr("Error reading file %1!").arg(fileName));
            file.close();
            return 0;
        }

        QByteArray ba;
        ba.clear();

        quint8 chkSum = 0;
        chkSum -= byteCount;
        chkSum -= recType;
        chkSum -= address & 0xFF;
        chkSum -= address >> 8;
        for(int i=0; i<byteCount; i++)
        {
            quint8 v = getValueFromLine(line,(i*2)+9) & 0xFF;
            chkSum -= v;
            ba.append(v);
        }


        quint8 retV = getValueFromLine(line,(byteCount*2)+9) & 0xFF;
        if(chkSum!=retV) {
            QMessageBox::critical(parent, QObject::tr("Error"),QObject::tr("Checksum Error reading file %1!").arg(fileName));
            file.close();
            return 0;
        }

        if((recType == 0x00) && ((address+offset+byteCount)<=datalen)) //data record - ba holds record
        {
          if ( nextAddress != (uint32_t)(address+offset) )
					{
						BlockSizes[BlockIndex++] = finalSize ;
						BlockAddresses[BlockIndex] = address+offset ;
						finalSize = 0 ;
					}
          memcpy(&data[address+offset],ba.data(),byteCount);
					nextAddress = address + offset + byteCount ;
          finalSize += byteCount;
        }

    }
		BlockSizes[BlockIndex] = finalSize ;
    file.close();

		finalSize = 0 ;
    for ( offset = 0 ; (uint32_t)offset <= BlockIndex ; offset += 1 )
		{
			finalSize += BlockSizes[offset] ;
		}
    return finalSize ;
}

bool saveiHEX(QWidget *parent, QString fileName, quint8 * data, int datalen, QString header, int notesIndex, int useBlocks )
{
  int nextbank = 1;
    QFile file(fileName);


    //open file
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(parent, QObject::tr("Error"),
                             QObject::tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);

    //write file header in Intel HEX format
    if(!header.isEmpty())
    {
        out << header << "\n";
    }

    uint32_t block ;
    int addr = 0;
    if(notesIndex>0)
        addr =0;

		if ( !useBlocks )
		{
			BlockIndex = 0 ;
			BlockAddresses[0] = 0 ;
			BlockSizes[0] = datalen ;
		}

		for ( block = 0 ; block <= BlockIndex ; block += 1 )
		{
			addr = BlockAddresses[block] ;
      datalen = BlockSizes[block] + addr ;

    	while (addr<datalen)
    	{
	  	  if (addr>(nextbank*0x010000)-1)
				{
  				QString str = QString(":02000002"); //write record type 2 record
  				quint8 chkSum = 0;
  				chkSum = -2; //-bytecount; recordtype is zero
  				chkSum -= 2; // type 2 record type
  				str += QString("%1000").arg((nextbank&0x0f)<<4,1,16,QChar('0'));
  				chkSum -= ((nextbank&0x0f)<<4); // type 2 record type
  				str += QString("%1").arg(chkSum, 2, 16, QChar('0'));
    	    out << str.toUpper() << "\n";
    		  nextbank++;
					data += 0x010000 ;
    		}
    	    int llen = 32;
    	    if((datalen-addr)<llen)
    	        llen = datalen-addr;

    	    out << iHEXLine(data, addr, llen) << "\n";

    	    addr += llen;
    	}
		}

    out << ":00000001FF";  // write EOF

    file.close();

    return true;
}

bool getSplashBIN(QString fileName, uchar * b, QWidget *parent)
{
    quint8 temp[BIN_FILE_SIZE] = {0};

    QFile file(fileName);

    if(!file.exists())
    {
        QMessageBox::critical(parent, QObject::tr("Error"),QObject::tr("Unable to find file %1!").arg(fileName));
        return 0;
    }

    if (!file.open(QIODevice::ReadOnly )) {  //reading file
        QMessageBox::critical(parent, QObject::tr("Error"),
                              QObject::tr("Error opening file %1:\n%2.")
                              .arg(fileName)
                              .arg(file.errorString()));
        return 0;
    }

    memset(temp,0,BIN_FILE_SIZE) ;

    long result = file.read((char*)&temp,file.size()) ;
    file.close();

;
    file.close();

    if (result!=file.size())
    {
      QMessageBox::critical(parent, QObject::tr("Error"),
                             QObject::tr("Error reading file %1:%2. %3 %4")
                             .arg(fileName)
                             .arg(file.errorString())
                             .arg(result)
                             .arg(file.size())										 
													 );

      return false;
    }

    QByteArray rawData = QByteArray::fromRawData((const char *)&temp, BIN_FILE_SIZE);
    QString mark;
    mark.clear();
    mark.append("SPS");
    mark.append('\0');
    int pos = rawData.indexOf(mark);

    if(pos<0)
        return false;

    memcpy(b, (const uchar *)&temp[pos + 7], SPLASH_SIZE);
    return true;
}



bool getSplashHEX(QString fileName, uchar * b, QWidget *parent)
{
    quint8 temp[HEX_FILE_SIZE] = {0};

    if(!loadiHEX(parent, fileName, (quint8*)&temp, HEX_FILE_SIZE, ""))
        return false;

    QByteArray rawData = QByteArray::fromRawData((const char *)&temp, 10000);
    QString mark;
    mark.clear();
    mark.append(SPLASH_MARKER);
    mark.append('\0');
    int pos = rawData.indexOf(mark);

    if(pos<0)
        return false;

    memcpy(b, (const uchar *)&temp[pos + SPLASH_OFFSET], SPLASH_SIZE);
    return true;
}

bool putSplashHEX(QString fileName, uchar * b, QWidget *parent)
{
    quint8 temp[HEX_FILE_SIZE] = {0};
    int fileSize = loadiHEX(parent, fileName, (quint8*)&temp, HEX_FILE_SIZE, "");

    if(!fileSize)
        return false;

    QByteArray rawData = QByteArray::fromRawData((const char *)&temp, HEX_FILE_SIZE);
    QString mark;
    mark.clear();
    mark.append(SPLASH_MARKER);
    mark.append('\0');
    int pos = rawData.indexOf(QString(mark));

    if(pos<0)
        return false;

    memcpy((uchar *)&temp[pos + SPLASH_OFFSET], b, SPLASH_SIZE);

    if(!saveiHEX(parent, fileName, (quint8*)&temp, fileSize, "", 0, 1 ) )
        return false;

    return true;
}


void appendTextElement(QDomDocument * qdoc, QDomElement * pe, QString name, QString value)
{
    QDomElement e = qdoc->createElement(name);
    QDomText t = qdoc->createTextNode(name);
    t.setNodeValue(value);
    e.appendChild(t);
    pe->appendChild(e);
}

void appendNumberElement(QDomDocument * qdoc, QDomElement * pe,QString name, int value, bool forceZeroWrite)
{
    if(value || forceZeroWrite)
    {
        QDomElement e = qdoc->createElement(name);
        QDomText t = qdoc->createTextNode(name);
        t.setNodeValue(QString("%1").arg(value));
        e.appendChild(t);
        pe->appendChild(e);
    }
}

void appendCDATAElement(QDomDocument * qdoc, QDomElement * pe,QString name, const char * data, int size)
{
        QDomElement e = qdoc->createElement(name);
        QDomCDATASection t = qdoc->createCDATASection(name);
        t.setData(QByteArray(data, size).toBase64());
        e.appendChild(t);
        pe->appendChild(e);
}

QDomElement getGeneralDataXML(QDomDocument * qdoc, EEGeneral * tgen)
{
    QDomElement gd = qdoc->createElement("GENERAL_DATA");

    appendNumberElement(qdoc, &gd, "Version", tgen->myVers, true); // have to write value here
    appendTextElement(qdoc, &gd, "Owner", QString::fromLatin1(tgen->ownerName,sizeof(tgen->ownerName)).trimmed());
    appendCDATAElement(qdoc, &gd, "Data", (const char *)tgen,sizeof(EEGeneral));
    return gd;

}


#ifdef SKY
QDomElement getModelDataXML(QDomDocument * qdoc, SKYModelData * tmod, int modelNum, int mdver)
#else
QDomElement getModelDataXML(QDomDocument * qdoc, ModelData * tmod, int modelNum, int mdver)
#endif
{
    QDomElement md = qdoc->createElement("MODEL_DATA");
    md.setAttribute("number", modelNum);

    appendNumberElement(qdoc, &md, "Version", mdver, true); // have to write value here
    appendTextElement(qdoc, &md, "Name", QString::fromLatin1(tmod->name,sizeof(tmod->name)).trimmed());
#ifdef SKY
    appendCDATAElement(qdoc, &md, "Data", (const char *)tmod,sizeof(SKYModelData));
#else
    appendCDATAElement(qdoc, &md, "Data", (const char *)tmod,sizeof(ModelData));
#endif
    return md;
}


bool loadGeneralDataXML(QDomDocument * qdoc, EEGeneral * tgen)
{
  memset( tgen, 0, sizeof( *tgen) ) ;
    //look for "GENERAL_DATA" tag
    QDomElement gde = qdoc->elementsByTagName("GENERAL_DATA").at(0).toElement();

    if(gde.isNull()) // couldn't find
        return false;

    //load cdata into tgen
    QDomNode n = gde.elementsByTagName("Data").at(0).firstChild();// get all children in Data
    while (!n.isNull())
    {
        if (n.isCDATASection())
        {
            QString ds = n.toCDATASection().data();
            QByteArray ba = QByteArray::fromBase64(ds.toLatin1());
            int size = ba.length() ;
            const char * data = ba.data();
            memcpy(tgen, data, size );
            break;
        }
        n = n.nextSibling();
    }

    //check version?

    return true;
}

#ifdef SKY
bool loadModelDataXML(QDomDocument * qdoc, SKYModelData * tmod, int modelNum)
#else
bool loadModelDataXML(QDomDocument * qdoc, ModelData * tmod, int modelNum)
#endif
{
  memset( tmod, 0, sizeof( *tmod) ) ;
	
    //look for MODEL_DATA with modelNum attribute.
    //if modelNum = -1 then just pick the first one
    QDomNodeList ndl = qdoc->elementsByTagName("MODEL_DATA");

    //cycle through nodes to find correct model number
    QDomNode k = ndl.at(0);
    if(modelNum>=0) //if we should look for SPECIFIC model cycle through models
    {
        while(!k.isNull())
        {
            int a = k.toElement().attribute("number").toInt();
            if(a==modelNum)
                break;
            k = k.nextSibling();
        }
    }

    if(k.isNull()) // couldn't find
        return false;


    //load cdata into tgen
    QDomNode n = k.toElement().elementsByTagName("Data").at(0).firstChild();// get all children in Data
    while (!n.isNull())
    {
        if (n.isCDATASection())
        {
            QString ds = n.toCDATASection().data();
            QByteArray ba = QByteArray::fromBase64(ds.toLatin1());
            int size = ba.length() ;
            const char * data = ba.data();
#ifdef SKY
            memcpy(tmod, data, size) ;
#else
            memcpy(tmod, data, size) ;
#endif
            break;
        }
        n = n.nextSibling();
    }

    //check version?

    return true;
}

#ifdef SKY
void populateCustomAlarmCB( QComboBox *b, int type )
{
  b->clear() ;
  b->addItem("---") ;
  b->addItem("Rud") ;
  b->addItem("Ele") ;
  b->addItem("Thr") ;
  b->addItem("Ail") ;
  b->addItem("P1") ;
  b->addItem("P2") ;
	if ( ( (type == RADIO_TYPE_TARANIS ) || ( type == 2 ) ) )	// Taranis
	{
	  b->addItem("SL") ;
  	b->addItem("SR") ;
	}
	else
	{
  	b->addItem("P3") ;
	}
}
#endif


//#ifdef SKY
//uint8_t CONVERT_MODE( uint8_t x )
//#else
uint8_t CONVERT_MODE( uint8_t x, int modelVersion, int stickMode )
//#endif
{
	if ( modelVersion >= 2 )
	{
		return x ;
	}
  return (((x)<=4) ? modn12x3[stickMode][((x)-1)] : (x)) ;
}

#ifdef SKY
uint8_t throttleReversed( EEGeneral *g_eeGeneral, SKYModelData *g_model )
#else
uint8_t throttleReversed( EEGeneral *g_eeGeneral, ModelData *g_model )
#endif
{
	return g_model->throttleReversed ^ g_eeGeneral->throttleReversed ;
}

#ifdef SKY

#if defined WIN32 || !defined __GNUC__
#include <windows.h>
#else
#include <unistd.h>
#include "mountlist.h"
#endif

int Found9Xtreme ;

//QString VolNames[8] ;

QString FindErskyPath( int type )
{
    int pathcount=0;
    QString path;
    QStringList drives;
    QString eepromfile;
    QString fsname;
//#if defined WIN32 || !defined __GNUC__
//    int x = 0 ;
//#endif

//		VolNames[0] = "" ;
//		VolNames[1] = "" ;
//		VolNames[2] = "" ;
//		VolNames[3] = "" ;
//		VolNames[4] = "" ;
//		VolNames[5] = "" ;
//		VolNames[6] = "" ;
//		VolNames[7] = "" ;

#if defined WIN32 || !defined __GNUC__
    foreach( QFileInfo drive, QDir::drives() )
		{
      WCHAR szVolumeName[256] ;
      WCHAR szFileSystemName[256];
      DWORD dwSerialNumber = 0;
      DWORD dwMaxFileNameLength=256;
      DWORD dwFileSystemFlags=0;

      bool ret = GetVolumeInformationW( (WCHAR *) drive.absolutePath().utf16(),szVolumeName,256,&dwSerialNumber,&dwMaxFileNameLength,&dwFileSystemFlags,szFileSystemName,256);
      if(ret)
			{
				Found9Xtreme = 0 ;
        QString vName=QString::fromUtf16 ( (const ushort *) szVolumeName) ;
//				VolNames[x++] = vName ;
//				if ( x > 7 )
//				{
//					x = 7 ;
//				}
				if ( (vName.contains("ERSKY_9X")) || (vName.contains("9XTREME")) )
				{
					if ( vName.contains("9XTREME") )
					{
						Found9Xtreme = 1 ;
					}
          eepromfile=drive.absolutePath();
					if ( eepromfile.right(1) == "/" )
					{
						eepromfile = eepromfile.left( eepromfile.size() - 1 ) ;
					}
          eepromfile.append( ( type == RADIO_TYPE_TARANIS ) ? "/FIRMWARE.BIN" : "/ERSKY9X.BIN");
          if (QFile::exists(eepromfile))
					{
            pathcount++;
            path=eepromfile;
          }
        }
				else if ( (vName.contains("TARANIS")) || (vName.contains("Taranis")) )
				{
          eepromfile=drive.absolutePath();
					if ( eepromfile.right(1) == "/" )
					{
						eepromfile = eepromfile.left( eepromfile.size() - 1 ) ;
					}
          eepromfile.append( ( type == RADIO_TYPE_TARANIS ) || ( type == 2 ) ? "/FIRMWARE.BIN" : "/EEPROM.BIN");
          if (QFile::exists(eepromfile))
					{
            pathcount++;
            path=eepromfile;
          }
        }
      }
    }
#else
    struct mount_entry *entry;
    entry = read_file_system_list(true);
    while (entry != NULL)
		{
      if (!drives.contains(entry->me_devname))
			{
    		QString saveeepromfile ;
        drives.append(entry->me_devname);
        eepromfile=entry->me_mountdir;
				saveeepromfile = eepromfile ;
        eepromfile.append( ( type == RADIO_TYPE_TARANIS ) || ( type == 2 ) ? "/FIRMWARE.BIN" : "/ERSKY9X.BIN");
//  #if !defined __APPLE__ && !defined WIN32
//        QString fstype=entry->me_type;
//        qDebug() << fstype;
//        if (QFile::exists(eepromfile) && fstype.contains("fat") ) {
//  #else
        if (QFile::exists(eepromfile))
				{
//  #endif
          pathcount++;
          path=eepromfile;
        }
				else
				{
					eepromfile = saveeepromfile ;
        	eepromfile.append( ( type == RADIO_TYPE_TARANIS ) || ( type == 2 ) ? "/FIRMWARE.BIN" : "/EEPROM.BIN");
	        if (QFile::exists(eepromfile))
					{
	          pathcount++;
  	        path=eepromfile;
    	    }
				}
      }
      entry = entry->me_next; ;
    }
#endif
    if (pathcount==1) {
      return path;
    } else {
      return "";
    }
}

#include "myeeprom.h"

extern uint8_t stickScramble[] ;

void modelConvert1to2( EEGeneral *g_eeGeneral, SKYModelData *g_model )
{
	if ( g_model->modelVersion < 2 )
	{
    for(uint8_t i=0;i<MAX_MIXERS;i++)
		{
      SKYMixData *md = &g_model->mixData[i] ;
      if (md->srcRaw)
			{
        if (md->srcRaw <= 4)		// Stick
				{
					md->srcRaw = stickScramble[g_eeGeneral->stickMode*4+md->srcRaw-1] + 1 ;
				}
			}
		}
		for (uint8_t i = 0 ; i < NUM_SKYCSW ; i += 1 )
		{
      SKYCSwData *cs = &g_model->customSw[i];
      uint8_t cstate = CS_STATE(cs->func, g_model->modelVersion);
    	if(cstate == CS_VOFS)
			{
      	if (cs->v1)
				{
    		  if (cs->v1 <= 4)		// Stick
					{
    	    	cs->v1 = stickScramble[g_eeGeneral->stickMode*4+cs->v1-1] + 1 ;
					}
				}
			}
			else if(cstate == CS_VCOMP)
			{
      	if (cs->v1)
				{
    		  if (cs->v1 <= 4)		// Stick
					{
		    	  cs->v1 = stickScramble[g_eeGeneral->stickMode*4+cs->v1-1] + 1 ;
    	    }
				}
      	if (cs->v2)
				{
    		  if (cs->v2 <= 4)		// Stick
					{
						cs->v2 = stickScramble[g_eeGeneral->stickMode*4+cs->v2-1] + 1 ;
				  }
				}
			}
		}

		// EXPO/DR corrections
		
		int i, j, k ;
		int dest, src ;
	  ExpoData  lexpoData[4];
		
		for ( i = 0 ; i < 3 ; i += 1 )
		{ // 0=High, 1=Mid, 2=Low
			for ( j = 0 ; j < 2 ; j += 1 )
			{ // 0=Weight, 1=Expo - WRONG - 0=expo, 1=weight
				for ( k = 0 ; k < 2 ; k += 1 )
				{ // 0=Right, 1=Left
    			dest = CONVERT_MODE(1, 2, g_eeGeneral->stickMode)-1 ;

    			src = CONVERT_MODE(1, 1, g_eeGeneral->stickMode)-1 ;
					lexpoData[dest].expo[i][j][k] = g_model->expoData[src].expo[i][j][k] ;
    			dest = CONVERT_MODE(2, 2, g_eeGeneral->stickMode)-1 ;
    			src = CONVERT_MODE(2, 1, g_eeGeneral->stickMode)-1 ;
					lexpoData[dest].expo[i][j][k] = g_model->expoData[src].expo[i][j][k] ;
    			dest = CONVERT_MODE(3, 2, g_eeGeneral->stickMode)-1 ;
    			src = CONVERT_MODE(3, 1, g_eeGeneral->stickMode)-1 ;

					lexpoData[dest].expo[i][j][k] = g_model->expoData[src].expo[i][j][k] ;
    			dest = CONVERT_MODE(4, 2, g_eeGeneral->stickMode)-1 ;
    			src = CONVERT_MODE(4, 1, g_eeGeneral->stickMode)-1 ;
					lexpoData[dest].expo[i][j][k] = g_model->expoData[src].expo[i][j][k] ;
		    }
			}
		}
    for ( i = 1 ; i <= 4 ; i += 1 )
		{
 			dest = CONVERT_MODE(i, 2, g_eeGeneral->stickMode)-1 ;
 			src = CONVERT_MODE(i, 1, g_eeGeneral->stickMode)-1 ;
			lexpoData[dest].drSw1 = g_model->expoData[src].drSw1 ;
			lexpoData[dest].drSw2 = g_model->expoData[src].drSw2 ;
		}
    memmove( &g_model->expoData, &lexpoData, sizeof(lexpoData));
		 
		g_model->modelVersion = 2 ;
	}
}


#endif




