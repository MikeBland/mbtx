/*
 * Author - Mike Blandford
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#ifndef vars_h
#define vars_h

#define MAX_VALS		6
#define MAX_ACTS		4

#ifdef USE_VARS

#ifndef PACK
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#if NUM_VARS > 25
#define NUM_VAR25		25
#else
#define NUM_VAR25		NUM_VARS
#endif

PACK(struct t_varPack // 12 bytes
{
	int16_t tag:2 ;
	int16_t min:14 ;
	int16_t max:13 ;
	uint16_t numOpt:3 ;	
  int16_t value:13 ;
	uint16_t numAct:3 ;	
  uint8_t name[6] ;
}) ;

PACK(struct t_valVarPack // 4 bytes
{
	uint32_t tag:2 ;
	int32_t value:13 ;
	uint32_t valIsSource:1 ;
	int32_t item:8 ;
	uint32_t spare2:5 ;
	uint32_t category:3 ;
}) ;

PACK(struct t_actVarPack // 4 bytes
{
	uint32_t tag:2 ;
	int32_t value:11 ;
	uint32_t timer:3 ;
	int32_t item:8 ;
	uint32_t function:3 ;
	uint32_t swPosition:1 ;
	uint32_t tFlag:1 ;		// Not yet used
	uint32_t category:3 ;
}) ;

void displayVarName( uint16_t x, uint16_t y, int32_t index, uint16_t attr ) ;
int16_t getVarValue( int32_t index ) ;
int32_t getVarValWholeL100( int32_t index ) ;
void initVars() ;
void processVars( uint8_t event ) ;
// Value in is 1000 + var (positive or negative if var)
int16_t editVarCapableValue( uint16_t x, uint16_t y, int16_t value, int16_t min, int16_t max, int16_t varLimit, uint32_t attr, uint8_t event ) ;
int16_t editVarCapable100Value( uint16_t x, uint16_t y, int16_t value, uint32_t attr, uint8_t event ) ;
void menuOneAction( uint8_t event ) ;
void menuOneValue( uint8_t event ) ;
void menuOneVar(uint8_t event) ;
void menuVars(uint8_t event) ;

extern uint8_t VarUsesTrim ;	// 4 bits

#endif // USE_VARS

#endif // vars_h

