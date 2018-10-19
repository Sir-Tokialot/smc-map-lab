#ifndef C_LAB_PLAYER_H
#define C_LAB_PLAYER_H
#ifdef _WIN32
#pragma once
#endif

#include "c_basehlplayer.h"

class C_LabPlayer : public C_BaseHLPlayer
{
public:
	DECLARE_CLASS( C_LabPlayer , C_BaseHLPlayer );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_LabPlayer();
	~C_LabPlayer();

};


inline C_LabPlayer* ToLabPlayer( CBaseEntity *pPlayer )
{
	Assert( dynamic_cast<C_LabPlayer*>( pPlayer ) != NULL );
	return static_cast<C_LabPlayer*>( pPlayer );
}


#endif // C_SDK_PLAYER_H
