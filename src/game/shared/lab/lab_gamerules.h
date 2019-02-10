#ifndef LAB_GAMERULES_H
#define LAB_GAMERULES_H
#ifdef _WIN32
#pragma once
#endif

#include "teamplay_gamerules.h"
#ifdef CLIENT_DLL

#else
	#include "lab_player.h"
#endif

#ifdef CLIENT_DLL
	#define CLabGameRules C_LabGameRules
	#define CLabGameRulesProxy C_LabGameRulesProxy
#endif

class CLabGameRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CLabGameRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();
#ifndef CLIENT_DLL
	virtual void Spawn();
#endif
};

class LabViewVectors : public CViewVectors
{
public:
	LabViewVectors(
		Vector vView,
		Vector vHullMin,
		Vector vHullMax,
		Vector vDuckHullMin,
		Vector vDuckHullMax,
		Vector vDuckView,
		Vector vObsHullMin,
		Vector vObsHullMax,
		Vector vDeadViewHeight,
		Vector vCrouchTraceMin,
		Vector vCrouchTraceMax ) :
			CViewVectors( 
				vView,
				vHullMin,
				vHullMax,
				vDuckHullMin,
				vDuckHullMax,
				vDuckView,
				vObsHullMin,
				vObsHullMax,
				vDeadViewHeight )
	{
		m_vCrouchTraceMin = vCrouchTraceMin;
		m_vCrouchTraceMax = vCrouchTraceMax;
	}

	Vector m_vCrouchTraceMin;
	Vector m_vCrouchTraceMax;	
};

#define VEC_CROUCH_TRACE_MIN	LabGameRules()->GetLabViewVectors()->m_vCrouchTraceMin
#define VEC_CROUCH_TRACE_MAX	LabGameRules()->GetLabViewVectors()->m_vCrouchTraceMax

enum
{
	TEAM_COMBINE = 2,
	TEAM_REBELS,
};

class CLabGameRules : public CTeamplayRules
{
public:
	DECLARE_CLASS( CLabGameRules, CTeamplayRules );
	DECLARE_NETWORKCLASS_NOBASE();

	CLabGameRules();
	virtual ~CLabGameRules();
	
	virtual bool ShouldCollide( int collisionGroup0, int collisionGroup1 );

	// derive this function if you mod uses encrypted weapon info files
	virtual const unsigned char *GetEncryptionKey( void ) { return (unsigned char *)"x9Ke0BY7"; }
	virtual const CViewVectors* GetViewVectors() const;
	const LabViewVectors* GetLabViewVectors() const;

	bool	     IsTeamplay(void) { return m_bTeamPlayEnabled; }

	virtual bool IsConnectedUserInfoChangeAllowed(CBasePlayer *pPlayer);

#ifndef CLIENT_DLL

	virtual bool IsDeathmatch(void);
	virtual bool IsCoOp(void);
	virtual bool IsSingleplayer(void);
	virtual bool IsMultiplayer(void);

	virtual void Precache(void);
	virtual bool ClientCommand(CBaseEntity *pEdict, const CCommand &args);

	virtual void			InitDefaultAIRelationships( void );
	virtual const char*		AIClassText(int classType);
	virtual float			GetAmmoDamage( CBaseEntity *pAttacker, CBaseEntity *pVictim, int nAmmoType );
	bool AllowDamage( CBaseEntity *pVictim, const CTakeDamageInfo &info );

	virtual float FlWeaponRespawnTime( CBaseCombatWeapon *pWeapon );
	virtual float FlWeaponTryRespawn( CBaseCombatWeapon *pWeapon );
	virtual Vector VecWeaponRespawnSpot( CBaseCombatWeapon *pWeapon );
	virtual int WeaponShouldRespawn( CBaseCombatWeapon *pWeapon );
	virtual void Think( void );
	virtual void CreateStandardEntities( void );
	virtual void ClientSettingsChanged( CBasePlayer *pPlayer );
	virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
	virtual void GoToIntermission( void );
	virtual void DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	virtual const char *GetGameDescription( void );

	float GetMapRemainingTime();
	void CleanUpMap();
	void CheckRestartGame();
	void RestartGame();
	
	virtual Vector VecItemRespawnSpot( CItem *pItem );
	virtual QAngle VecItemRespawnAngles( CItem *pItem );
	virtual float	FlItemRespawnTime( CItem *pItem );
	virtual bool	CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pItem );
	virtual bool FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon );

	void	AddLevelDesignerPlacedObject( CBaseEntity *pEntity );
	void	RemoveLevelDesignerPlacedObject( CBaseEntity *pEntity );
	void	ManageObjectRelocation( void );
	void    CheckChatForReadySignal( CLabPlayer *pPlayer, const char *chatmsg );
	const char *GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer );

	
	bool	NPC_ShouldDropGrenade( CBasePlayer *pRecipient );
	bool	NPC_ShouldDropHealth( CBasePlayer *pRecipient );
	void	NPC_DroppedHealth( void );
	void	NPC_DroppedGrenade( void );
	bool	MegaPhyscannonActive( void ) { return m_bMegaPhysgun; } //might want to move this around since if one player has the megaphyscannon all players have it
	
	virtual bool IsAlyxInDarknessMode();

	virtual void ClientDisconnected( edict_t *pClient );

	bool CheckGameOver( void );
	bool IsIntermission( void );

	void PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	void	CheckAllPlayersReady(void);

#endif
	
	

private:

	CNetworkVar( bool, m_bTeamPlayEnabled );
	CNetworkVar( float, m_flGameStartTime );
#ifndef CLIENT_DLL
	CUtlVector<EHANDLE> m_hRespawnableItemsAndWeapons;
	float m_tmNextPeriodicThink;
	float m_flRestartGameTime;
	bool m_bCompleteReset;
	bool m_bAwaitingReadyRestart;
	bool m_bHeardAllPlayersReady;
	bool m_bChangelevelDone;
	float	m_flLastHealthDropTime;
	float	m_flLastGrenadeDropTime;
#endif

private:
	CNetworkVar(bool, m_bMegaPhysgun);
	
};

inline CLabGameRules* LabGameRules()
{
	return static_cast<CLabGameRules*>(g_pGameRules);
}

#endif // LAB_GAMERULES_H
