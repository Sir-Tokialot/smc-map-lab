//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: modifiedconvarts_t from CommentarySystem.cpp moved to a header file so Mapbase can use it.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"


// Convar restoration save/restore
#define MAX_MODIFIED_CONVAR_STRING		128
struct modifiedconvars_t 
{
	DECLARE_SIMPLE_DATADESC();

	char pszConvar[MAX_MODIFIED_CONVAR_STRING];
	char pszCurrentValue[MAX_MODIFIED_CONVAR_STRING];
	char pszOrgValue[MAX_MODIFIED_CONVAR_STRING];
};
