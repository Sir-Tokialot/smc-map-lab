#include "cbase.h"
#include "lab_player.h"

LINK_ENTITY_TO_CLASS(player, CLabPlayer);
PRECACHE_REGISTER(player);

BEGIN_DATADESC( CLabPlayer )
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CLabPlayer, DT_LabPlayer )
	
END_SEND_TABLE()

CLabPlayer::CLabPlayer()
{
}


CLabPlayer::~CLabPlayer()
{
}


CLabPlayer *CLabPlayer::CreatePlayer( const char *className, edict_t *ed )
{
	CLabPlayer::s_PlayerEdict = ed;
	return (CLabPlayer*)CreateEntityByName( className );
}

void CLabPlayer::PreThink(void)
{
	BaseClass::PreThink();
}


void CLabPlayer::PostThink()
{
	BaseClass::PostThink();
}


void CLabPlayer::Precache()
{
	BaseClass::Precache();
}

void CLabPlayer::Spawn()
{
	BaseClass::Spawn();
}
