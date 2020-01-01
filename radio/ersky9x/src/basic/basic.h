/*
 * Author - Mike Blandford
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

// type of basic load
#define BASIC_LOAD_NONE			0
#define BASIC_LOAD_ALONE		1
#define BASIC_LOAD_TEL0			2
#define BASIC_LOAD_TEL1			3
#define BASIC_LOAD_BG				4

// States
#define BASIC_IDLE				0
#define BASIC_LOAD_START	1
#define BASIC_LOADING			2
#define BASIC_RUNNING			3

struct t_loadedScripts
{
//	struct t_basicRunTime *runtime ;
	uint32_t offsetOfStart ;
	uint16_t size ;
	uint8_t loaded ;
	uint8_t type ;
} ;

uint32_t basicExecute( uint32_t begin, uint16_t event, uint32_t index ) ;
int32_t expression( void ) ;
uint32_t loadBasic( char *fileName, uint32_t type ) ;
uint32_t basicTask( uint8_t event, uint8_t flags ) ;
void basicLoadModelScripts( void ) ;

extern uint8_t BasicErrorText[] ;
extern uint8_t BasicLoadedType ;
#ifndef QT	
extern FIL MultiBasicFile ;
#endif


