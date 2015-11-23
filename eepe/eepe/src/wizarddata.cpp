/*
 * Author - Kjell Kernen
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
#include <string.h>
//#include "eeprominterface.h"
#include "pers.h"
#include "myeeprom.h"
#include "wizarddata.h"

const uint8_t stickScramble[] =
{
    0, 1, 2, 3,
    0, 2, 1, 3,
    3, 1, 2, 0,
    3, 2, 1, 0 } ;


Channel::Channel()
{
  clear();
}

void Channel::clear()
{
  page = Page_None;
  prebooked = false;
  input1 = NO_INPUT;
  input2 = NO_INPUT;
  weight1 = 0;  
  weight2 = 0;
}

WizMix::WizMix(const EEGeneral & settings, const unsigned int modelId):
  complete(false),
  modelId(modelId),
  settings(settings),
  vehicle(NOVEHICLE)
{
  strcpy(name, "            ");
	stickMode = settings.stickMode ;

}


//#define CHECK_IN_ARRAY(T, index) ((unsigned int)index < (unsigned int)(sizeof(T)/sizeof(T[0])) ? T[(unsigned int)index] : "???")

//QString AnalogString(int index)
//{
//  static const QString sticks[]  = { QObject::tr("Rud"), QObject::tr("Ele"), QObject::tr("Thr"), QObject::tr("Ail") };
//  static const QString pots9X[]  = { QObject::tr("P1"), QObject::tr("P2"), QObject::tr("P3") };
////  static const QString potsTaranis[] = { QObject::tr("S1"), QObject::tr("S2"), QObject::tr("S3"), QObject::tr("LS"), QObject::tr("RS") };

//  if (index < 4)
//    return CHECK_IN_ARRAY(sticks, index);
//  else
////    return (IS_TARANIS(GetEepromInterface()->getBoard()) ? CHECK_IN_ARRAY(potsTaranis, index-4) : CHECK_IN_ARRAY(pots9X, index-4));
//    return CHECK_IN_ARRAY(pots9X, index-4) ) ;
//}


void WizMix::maxMixSwitch(char *name, MixData &mix, int channel, int sw, int weight)
{
//  strncpy(mix.name, name, MIXDATA_NAME_LEN);
//  mix.name[MIXDATA_NAME_LEN] = '\0';
  mix.destCh = channel;
//  mix.srcRaw = RawSource(SOURCE_TYPE_MAX);
//  mix.swtch  = RawSwitch(SWITCH_TYPE_SWITCH, sw);
  mix.weight = weight;
}

void WizMix::addMix(ModelData &model, Input input, int weight, int channel, int & mixIndex)
{
  if (input != NO_INPUT)  {
//    bool isTaranis = IS_TARANIS(GetEepromInterface()->getBoard());
    bool isTaranis = false ;

    if (input >= RUDDER_INPUT && input <= AILERONS_INPUT) {
      MixData & mix = model.mixData[mixIndex++];
      mix.destCh = channel+1;
      if (isTaranis){
//        int channel = settings.getDefaultChannel(input-1);
        int channel = 1 ;
//        mix.srcRaw = RawSource(SOURCE_TYPE_VIRTUAL_INPUT, channel);
      }
      else
			{
				int x = stickMode ;
				int stickIndex = stickMode*4 ;
        x = stickScramble[stickIndex+input-1] ;

        mix.srcRaw = x+1 ; // RawSource(SOURCE_TYPE_STICK, input-1);
			}
      mix.weight = weight;
    }
    else if (input==FLAPS_INPUT)
		{
      // There ought to be some kind of constants for switches somewhere...
      MixData & mix = model.mixData[mixIndex++];
      mix.destCh = channel+1;
		  mix.srcRaw = MIX_3POS ;
			mix.sw23pos = 0 ;	// Idx
		  mix.weight = weight;
			
//      maxMixSwitch((char *)"Flaps Up",   model.mixData[mixIndex++], channel+1, isTaranis ? SWITCH_SA0 :-SWITCH_ELE ,  weight); //Taranis SA-UP, 9X ELE-UP
//      maxMixSwitch((char *)"Flaps Down", model.mixData[mixIndex++], channel+1, isTaranis ? SWITCH_SA2 : SWITCH_ELE , -weight); //Taranis SA-DOWN, 9X ELE-DOWN

    }
    else if (input==AIRBRAKES_INPUT)
		{
      MixData & mix = model.mixData[mixIndex++];
      mix.destCh = channel+1;
		  mix.srcRaw = MIX_3POS ;
			mix.sw23pos = 3 ;	// Ele
		  mix.weight = weight;
//      maxMixSwitch((char *)"Airbrk Off", model.mixData[mixIndex++], channel+1, isTaranis ? SWITCH_SE0 :-SWITCH_RUD , -weight); //Taranis SE-UP, 9X RUD-UP
//      maxMixSwitch((char *)"Airbrk On",  model.mixData[mixIndex++], channel+1, isTaranis ? SWITCH_SE2 : SWITCH_RUD , weight); //Tatanis SE-DOWN, 9X RUD-DOWN
    }
  }
}

WizMix::operator ModelData()
{
  int throttleChannel = -1;
  //    bool isTaranis = IS_TARANIS(GetEepromInterface()->getBoard());
      bool isTaranis = false ;

  ModelData model;
  memset(&model, 0, sizeof(model));

//  model.used = true;
//  model.modelId = modelId;
//  model.setDefaultInputs(settings);

  int mixIndex = 0;
  int switchIndex = 0;
  int timerIndex = 0;

  // Safe copy model name
//  strncpy(model.name, name, WIZ_MODEL_NAME_LENGTH);
//  model.name[WIZ_MODEL_NAME_LENGTH] = 0;
  strncpy(model.name, name, 10);
//  model.name[WIZ_MODEL_NAME_LENGTH] = 0;

  // Add the channel mixes
  for (int i=0; i<WIZ_MAX_CHANNELS; i++ ) 
  {
    Channel ch = channel[i];
    if (ch.input1 == THROTTLE_INPUT || ch.input2 == THROTTLE_INPUT)
      throttleChannel = i;

    addMix(model, ch.input1, ch.weight1, i, mixIndex);
    addMix(model, ch.input2, ch.weight2, i, mixIndex);
  }

  // Add the Throttle Cut option
  if( options[THROTTLE_CUT_OPTION] && throttleChannel >=0 )
	{
#ifndef V2
    SafetySwData *sd = &model.safetySw[throttleChannel] ;
#else
    V2SafetySwData *sd = &model.safetySw[throttleChannel] ;
#endif

#ifndef V2
    sd->opt.ss.mode = 3 ;
		sd->opt.ss.swtch = DSW_THR ;
    sd->opt.ss.val = model.throttleIdle ? 0 : -100 ;
#endif
    //    model.funcSw[switchIndex].swtch.type = SWITCH_TYPE_SWITCH;
//    model.funcSw[switchIndex].swtch.index = isTaranis ? SWITCH_SF0 : SWITCH_THR;
//    model.funcSw[switchIndex].enabled = 1;
//    model.funcSw[switchIndex].func = (AssignFunc)throttleChannel;
//    model.funcSw[switchIndex].param = -100;
  }

  // Add the Flight Timer option
  if (options[FLIGHT_TIMER_OPTION] ){
//    model.timers[timerIndex].mode.type = SWITCH_TYPE_TIMER_MODE;
//    model.timers[timerIndex].mode.index = TMRMODE_THR_TRG;
    timerIndex++;
  }

  // Add the Throttle Timer option
  if (options[THROTTLE_TIMER_OPTION] && throttleChannel >=0){
//    model.timers[timerIndex].mode.type = SWITCH_TYPE_TIMER_MODE;
//    model.timers[timerIndex].mode.index = TMRMODE_THR;
    timerIndex++;
  }

  return model;
}


