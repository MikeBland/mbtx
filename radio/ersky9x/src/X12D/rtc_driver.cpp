/*
 * Authors (alphabetical order)
 * - Andre Bernet <bernet.andre@gmail.com>
 * - Andreas Weitl
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Gabriel Birkus
 * - Jean-Pierre Parisy
 * - Karl Szmutny
 * - Michael Blandford
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
 *
 * opentx is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
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

#include "../ersky9x.h"
#include "X12D/stm32f4xx_rtc.h"
#include "X12D/stm32f4xx_rcc.h"
#include "X12D/stm32f4xx_pwr.h"

void rtcSetTime( t_time *t )
{
  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;

  RTC_TimeStructInit(&RTC_TimeStruct);
  RTC_DateStructInit(&RTC_DateStruct);

  RTC_TimeStruct.RTC_Hours = t->hour ;
  RTC_TimeStruct.RTC_Minutes = t->minute ;
  RTC_TimeStruct.RTC_Seconds = t->second ;
  RTC_DateStruct.RTC_Year = t->year - 2000 ;
  RTC_DateStruct.RTC_Month = t->month ;
  RTC_DateStruct.RTC_Date = t->date;
  
  RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);
  RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
}

void rtc_gettime( t_time *t )
{
  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;

  RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
  RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
  
  t->hour = RTC_TimeStruct.RTC_Hours ;
  t->minute  = RTC_TimeStruct.RTC_Minutes ;
  t->second  = RTC_TimeStruct.RTC_Seconds ;
  t->year = RTC_DateStruct.RTC_Year + 2000 ;
  t->month  = RTC_DateStruct.RTC_Month ;
  t->date = RTC_DateStruct.RTC_Date ;
}

void rtcInit()
{
  RTC_InitTypeDef RTC_InitStruct;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  PWR_BackupAccessCmd(ENABLE);
  RCC_LSEConfig(RCC_LSE_ON);
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{
		wdt_reset() ;
	}
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
  RCC_RTCCLKCmd(ENABLE);
  RTC_WaitForSynchro();

  // RTC time base = LSE / ((AsynchPrediv+1) * (SynchPrediv+1)) = 1 Hz*/
  RTC_InitStruct.RTC_HourFormat = RTC_HourFormat_24;
  RTC_InitStruct.RTC_AsynchPrediv = 127;
  RTC_InitStruct.RTC_SynchPrediv = 255;
  RTC_Init(&RTC_InitStruct);
  
//  struct gtm utm;
//  rtc_gettime(&utm);
//  g_rtcTime = gmktime(&utm);
}

void rtcSetCal( uint32_t RTC_CalibSign, uint32_t Value )
{
	RTC_CoarseCalibConfig( RTC_CalibSign, Value) ;
}

