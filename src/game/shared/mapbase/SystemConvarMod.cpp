//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Mostly just Mapbase's convar mod code.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "saverestore_utlvector.h"
#include "SystemConvarMod.h"


BEGIN_SIMPLE_DATADESC( modifiedconvars_t )
	DEFINE_ARRAY( pszConvar, FIELD_CHARACTER, MAX_MODIFIED_CONVAR_STRING ),
	DEFINE_ARRAY( pszCurrentValue, FIELD_CHARACTER, MAX_MODIFIED_CONVAR_STRING ),
	DEFINE_ARRAY( pszOrgValue, FIELD_CHARACTER, MAX_MODIFIED_CONVAR_STRING ),
END_DATADESC()
