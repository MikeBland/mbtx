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

#ifndef mixer_h
#define ersky9x_h

extern int16_t anas [] ;

extern void perOutPhase( int16_t *chanOut, uint8_t att ) ;
extern void perOut( int16_t *chanOut, uint8_t att ) ;

#ifdef PCBX9D
void checkMixerNeeded( void ) ;
#endif

#endif
