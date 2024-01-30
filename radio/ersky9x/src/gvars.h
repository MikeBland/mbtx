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


#ifndef gvars_h
#define gvars_h


#if MULTI_GVARS

#define GVAR_MAX				1024

#endif // MULTI_GVARS


int16_t getGvar( int32_t gv ) ;
void setGVar(uint32_t gv, int16_t value ) ;
void initFmGvars() ;
uint32_t getGVarFlightMode(uint32_t fm, uint32_t gv) ;

#endif // gvars_h
