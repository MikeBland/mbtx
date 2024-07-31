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

#include "pers.h"
#include "vars.h"
#include "myeeprom.h"

int16_t VarValues[NUM_VARS] ;

uint16_t VarOffset[NUM_VARS] ;	// Offset into data for each Var

uint16_t TotalStorageUsed ;

uint32_t countVarOptions( struct t_varPack *pvar, SKYModelData *g_model )
{
	uint32_t nVals = 0 ;
	uint32_t nActs = 0 ;
	struct t_valVarPack *pVal = (struct t_valVarPack *)( pvar+1) ;
	while ( pVal->tag == 1 )
	{
		pVal += 1 ;
		nVals += 1 ;
		if ( (uint8_t *)pVal > (uint8_t *) &g_model->varStore[VAR_STORAGE_UINTS-1] )
		{
			break ;
		}		 
	}
	struct t_actVarPack *pAct = (struct t_actVarPack *)pVal ;
	if ( (uint8_t *)pAct <= (uint8_t *) &g_model->varStore[VAR_STORAGE_UINTS-1] )
	{
		while ( pAct->tag == 2 )
		{
			pAct += 1 ;
			nActs += 1 ;		
			if ( (uint8_t *)pAct > (uint8_t *) &g_model->varStore[VAR_STORAGE_UINTS-1] )
			{
				break ;
			}		 
		}
	}
	return ( nActs << 16 ) + nVals ;
}

// Set offsets and count options and actions, fill in counts
void buildVarOffsetTable( SKYModelData *g_model )
{
	uint32_t i ;
	struct t_varPack *pvar ;
	uint32_t options ;
	uint32_t totalOptions = 0 ;
	uint32_t totalActions = 0 ;

	pvar = (struct t_varPack *) g_model->varStore ;
	for ( i = 0 ; i < NUM_VARS ;  i += 1 )
	{
		VarOffset[i] = (uint8_t *) pvar - (uint8_t *) g_model->varStore ;
    options = countVarOptions( pvar, g_model ) ;
		pvar->numOpt = options & 0xFFFF ;
		totalOptions += pvar->numOpt ;
		pvar->numAct = options >> 16 ;
		totalActions += pvar->numAct ;

		uint8_t *t = (uint8_t *) (pvar + 1) ;
		t += pvar->numOpt * sizeof(struct t_valVarPack) + pvar->numAct * sizeof(struct t_actVarPack) ;
		pvar = (t_varPack *) t ;
	}
	TotalStorageUsed = NUM_VARS*sizeof(struct t_varPack) + totalOptions*sizeof(struct t_valVarPack) + totalActions*sizeof(struct t_actVarPack) ;
}

struct t_varPack *getVarAddress( uint32_t index, SKYModelData *g_model )
{
	struct t_varPack *pvar ;
	uint8_t *p = (uint8_t *) g_model->varStore ;
	p += VarOffset[index] ;	// Offset into data for each Var
	pvar = (struct t_varPack *) p ;
	return pvar ;
}


struct t_valVarPack *getValueAddress( struct t_varPack *pvar, uint32_t position )
{
	struct t_valVarPack *pvalue ;
	pvar += 1 ;	// skip var, point to values
	pvalue = (struct t_valVarPack *) pvar ;	// First value
	pvalue += position ;	// Place to insert
	return pvalue ;
}

uint32_t insertValue( struct t_varPack *pvar, uint32_t position, SKYModelData *g_model )
{
	struct t_valVarPack *pvalue ;
	if ( ( sizeof(g_model->varStore) - TotalStorageUsed ) >= sizeof(struct t_valVarPack) )
	{
//		LockVars = 1 ;
		pvalue = getValueAddress( pvar, position ) ;
		memmove( pvalue+1, pvalue, (uint8_t *) &g_model->varStore[VAR_STORAGE_UINTS-1] - (uint8_t *) pvalue ) ;
		pvalue->tag = 1 ;
		pvalue->item = 0 ;
		pvalue->value = 0 ;
		pvalue->category = 0 ;
		pvalue->valIsSource = 0 ;
    buildVarOffsetTable( g_model ) ;
//		LockVars = 0 ;
		return 1 ;
	}
	else
	{
		// Report out of memory
//		pushMenu(menuVarStorageLimit) ;
	}
	return 0 ;
}

void deleteValue( struct t_varPack *pvar, uint32_t position, SKYModelData *g_model )
{
	struct t_valVarPack *pvalue ;
//	LockVars = 1 ;
	pvalue = getValueAddress( pvar, position ) ;
	memmove( pvalue, pvalue+1, (uint8_t *) &g_model->varStore[VAR_STORAGE_UINTS-1] - (uint8_t *) (pvalue+1) ) ;
  buildVarOffsetTable( g_model ) ;
//	LockVars = 0 ;
}

uint32_t insertAction( struct t_varPack *pvar, uint32_t position, uint32_t numValues, SKYModelData *g_model )
{
	struct t_valVarPack *pvalue ;
	struct t_actVarPack *pact ;
	if ( ( sizeof(g_model->varStore) - TotalStorageUsed ) >= sizeof(struct t_actVarPack) )
	{
//		LockVars = 1 ;
		pvar += 1 ;	// skip var, point to values
		pvalue = (struct t_valVarPack *) pvar ;	// First value
		pvalue += numValues ;	// First Action place
		pact = (struct t_actVarPack *) pvalue ;
		pact += position ;
		memmove( pact+1, pact, (uint8_t *) &g_model->varStore[VAR_STORAGE_UINTS-1] - (uint8_t *) (pact) ) ;
		pact->tag = 2 ;
		pact->category = 0 ;
		pact->item = 0 ;
		pact->function = 0 ;
		pact->value = 10 ;
    buildVarOffsetTable( g_model ) ;
//		LockVars = 0 ;
		return 1 ;
	}
	else
	{
		// Report out of memory
//		pushMenu(menuVarStorageLimit) ;
	}
	return 0 ;
}

void deleteAction( struct t_varPack *pvar, uint32_t position, uint32_t numValues, SKYModelData *g_model )
{
	struct t_valVarPack *pvalue ;
	struct t_actVarPack *pact ;
//	LockVars = 1 ;
	pvar += 1 ;	// skip var, point to values
	pvalue = (struct t_valVarPack *) pvar ;	// First value
	pvalue += numValues ;	// First Action place
	pact = (struct t_actVarPack *) pvalue ;
	pact += position ;
	memmove( pact, pact+1, (uint8_t *) &g_model->varStore[VAR_STORAGE_UINTS-1] - (uint8_t *) (pact+1) ) ;
  buildVarOffsetTable( g_model ) ;
//	LockVars = 0 ;
}



