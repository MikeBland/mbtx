/*
 * Author - Mike Blandford
 *
 * Based on er9x by Erez Raviv <erezraviv@gmail.com>
 *
 * Based on th9x -> http://code.google.com/p/th9x/
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

#ifndef maintain_h
#define maintain_h

struct fileControl
{
	uint32_t nameCount ;
	uint32_t vpos ;
	uint32_t hpos ;
	uint16_t index ;
	uint8_t ext[4] ;
} ;

extern uint8_t MaintenanceRunning ;
extern TCHAR Filenames[8][50] ;

extern FRESULT readBinDir( DIR *dj, FILINFO *fno, struct fileControl *fc ) ;
extern uint32_t fileList(uint8_t event, struct fileControl *fc ) ;
extern uint32_t fillNames( uint32_t index, struct fileControl *fc ) ;
extern void maintenance_receive_packet( uint8_t *packet, uint32_t check ) ;

#endif

