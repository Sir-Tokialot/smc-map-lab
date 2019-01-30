#include "cbase.h"
#include "lab_gamerules.h"
#include "ammodef.h"
#include "viewport_panel_names.h"
#include "hl2_shareddefs.h"
#ifdef CLIENT_DLL

#else
	#include "ai_basenpc.h"
	#include "hl2mp_bot_temp.h"
	#include "hl2mp_cvars.h"
	#include "hl2mp_gameinterface.h"
	#include "weapon_hl2mpbasehlmpcombatweapon.h"
	#include "team.h"
	#include "game.h"
	#include "items.h"
	#include "eventqueue.h"
	#include "globalstate.h"
	#include "mapentities.h"
	#include "voice_gamemgr.h"
#endif
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern void respawn(CBaseEntity *pEdict, bool fCopyCorpse);

extern bool FindInList( const char **pStrings, const char *pToFind );

ConVar  physcannon_mega_enabled( "physcannon_mega_enabled", "0", FCVAR_CHEAT | FCVAR_REPLICATED );

ConVar sv_hl2mp_weapon_respawn_time( "sv_hl2mp_weapon_respawn_time", "20", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar sv_hl2mp_item_respawn_time  ( "sv_hl2mp_item_respawn_time", "30", FCVAR_GAMEDLL | FCVAR_NOTIFY );

ConVar sv_report_client_settings   ( "sv_report_client_settings", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY );

ConVar	sk_plr_health_drop_time    ( "sk_plr_health_drop_time", "30", FCVAR_REPLICATED );
ConVar	sk_plr_grenade_drop_time   ( "sk_plr_grenade_drop_time", "30", FCVAR_REPLICATED );

ConVar	sk_plr_dmg_grenade		   ( "sk_plr_dmg_grenade","0", FCVAR_REPLICATED );

ConVar	sk_plr_dmg_buckshot	       ( "sk_plr_dmg_buckshot","0", FCVAR_REPLICATED);	
ConVar	sk_npc_dmg_buckshot        ( "sk_npc_dmg_buckshot","0", FCVAR_REPLICATED);
ConVar	sk_max_buckshot            ( "sk_max_buckshot","0", FCVAR_REPLICATED);
ConVar	sk_plr_num_shotgun_pellets ( "sk_plr_num_shotgun_pellets","7", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_smg1_grenade	   ( "sk_plr_dmg_smg1_grenade","0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_smg1_grenade    ( "sk_npc_dmg_smg1_grenade","0", FCVAR_REPLICATED);
ConVar	sk_max_smg1_grenade	       ( "sk_max_smg1_grenade","0", FCVAR_REPLICATED );

ConVar  alyx_darkness_force        ( "alyx_darkness_force", "0", FCVAR_CHEAT | FCVAR_REPLICATED );



#ifdef CLIENT_DLL

ConVar cl_autowepswitch(
	"cl_autowepswitch",
	"1",
	FCVAR_ARCHIVE | FCVAR_USERINFO,
	"Automatically switch to picked up weapons (if more powerful)");

#endif

extern ConVar mp_chattime;

extern CBaseEntity	 *g_pLastCombineSpawn;
extern CBaseEntity	 *g_pLastRebelSpawn;

#define WEAPON_MAX_DISTANCE_FROM_SPAWN 64

REGISTER_GAMERULES_CLASS( CLabGameRules );

BEGIN_NETWORK_TABLE_NOBASE( CLabGameRules, DT_LabGameRules )
#ifdef CLIENT_DLL
	RecvPropBool(RECVINFO(m_bMegaPhysgun)),
#else
	SendPropBool(SENDINFO(m_bMegaPhysgun)),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( lab_gamerules, CLabGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( LabGameRulesProxy, DT_LabGameRulesProxy )


#ifdef CLIENT_DLL
	void RecvProxy_LabGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CLabGameRules *pRules = LabGameRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CLabGameRulesProxy, DT_LabGameRulesProxy )
		RecvPropDataTable( "lab_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_LabGameRules ), RecvProxy_LabGameRules )
	END_RECV_TABLE()
#else
	void *SendProxy_LabGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CLabGameRules *pRules = LabGameRules();
		Assert( pRules );
		pRecipients->SetAllRecipients();
		return pRules;
	}

	BEGIN_SEND_TABLE( CLabGameRulesProxy, DT_LabGameRulesProxy )
		SendPropDataTable( "lab_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_LabGameRules ), SendProxy_LabGameRules )
	END_SEND_TABLE()
#endif


#ifndef CLIENT_DLL

	class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
	{
	public:
		virtual bool CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker, bool &bProximity )
		{
			return ( pListener->GetTeamNumber() == pTalker->GetTeamNumber() );
		}
	};
	CVoiceGameMgrHelper g_VoiceGameMgrHelper;
	IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;

#endif

	
static LabViewVectors g_LabViewVectors(
	Vector( 0, 0, 64 ),       //VEC_VIEW (m_vView) 
							  
	Vector(-16, -16, 0 ),	  //VEC_HULL_MIN (m_vHullMin)
	Vector( 16,  16,  72 ),	  //VEC_HULL_MAX (m_vHullMax)
							  					
	Vector(-16, -16, 0 ),	  //VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector( 16,  16,  36 ),	  //VEC_DUCK_HULL_MAX	(m_vDuckHullMax)
	Vector( 0, 0, 28 ),		  //VEC_DUCK_VIEW		(m_vDuckView)
							  					
	Vector(-10, -10, -10 ),	  //VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector( 10,  10,  10 ),	  //VEC_OBS_HULL_MAX	(m_vObsHullMax)
							  					
	Vector( 0, 0, 14 ),		  //VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight)

	Vector(-16, -16, 0 ),	  //VEC_CROUCH_TRACE_MIN (m_vCrouchTraceMin)
	Vector( 16,  16,  60 )	  //VEC_CROUCH_TRACE_MAX (m_vCrouchTraceMax)
);

static const char *s_PreserveEnts[] =
{
	"ai_network",
	"ai_hint",
	"hl2mp_gamerules",
	"team_manager",
	"player_manager",
	"env_soundscape",
	"env_soundscape_proxy",
	"env_soundscape_triggerable",
	"env_sun",
	"env_wind",
	"env_fog_controller",
	"func_brush",
	"func_wall",
	"func_buyzone",
	"func_illusionary",
	"infodecal",
	"info_projecteddecal",
	"info_node",
	"info_target",
	"info_node_hint",
	"info_player_deathmatch",
	"info_player_combine",
	"info_player_rebel",
	"info_map_parameters",
	"keyframe_rope",
	"move_rope",
	"info_ladder",
	"player",
	"point_viewcontrol",
	"scene_manager",
	"shadow_control",
	"sky_camera",
	"soundent",
	"trigger_soundscape",
	"viewmodel",
	"predicted_viewmodel",
	"worldspawn",
	"point_devshot_camera",
	"", // END Marker
};

// NOTE: the indices here must match TEAM_TERRORIST, TEAM_CT, TEAM_SPECTATOR, etc.
char *sTeamNames[] =
{
	"Unassigned",
	"Spectator",
	"Combine",
	"Rebels",
};


// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION			3.5
// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)


CAmmoDef *GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;

	if (!bInitted)
	{
		bInitted = true;

		def.AddAmmoType("AR2", DMG_BULLET, TRACER_LINE_AND_WHIZ, 0, 0, 60, BULLET_IMPULSE(200, 1225), 0);
		def.AddAmmoType("AR2AltFire", DMG_DISSOLVE, TRACER_NONE, 0, 0, 3, 0, 0);
		def.AddAmmoType("Pistol", DMG_BULLET, TRACER_LINE_AND_WHIZ, 0, 0, 150, BULLET_IMPULSE(200, 1225), 0);
		def.AddAmmoType("SMG1", DMG_BULLET, TRACER_LINE_AND_WHIZ, 0, 0, 225, BULLET_IMPULSE(200, 1225), 0);
		def.AddAmmoType("357", DMG_BULLET, TRACER_LINE_AND_WHIZ, 0, 0, 12, BULLET_IMPULSE(800, 5000), 0);
		def.AddAmmoType("XBowBolt", DMG_BULLET, TRACER_LINE, 0, 0, 10, BULLET_IMPULSE(800, 8000), 0);
		def.AddAmmoType("Buckshot", DMG_BULLET | DMG_BUCKSHOT, TRACER_LINE, 0, 0, 30, BULLET_IMPULSE(400, 1200), 0);
		def.AddAmmoType("RPG_Round", DMG_BURN, TRACER_NONE, 0, 0, 3, 0, 0);
		def.AddAmmoType("SMG1_Grenade", DMG_BURN, TRACER_NONE, 0, 0, 3, 0, 0);
		def.AddAmmoType("Grenade", DMG_BURN, TRACER_NONE, 0, 0, 5, 0, 0);
		def.AddAmmoType("slam", DMG_BURN, TRACER_NONE, 0, 0, 5, 0, 0);
	}

	return &def;
}


CLabGameRules::CLabGameRules()
{
#ifndef CLIENT_DLL
	// Create the team managers
	for ( int i = 0; i < ARRAYSIZE( sTeamNames ); i++ )
	{
		CTeam *pTeam = static_cast<CTeam*>(CreateEntityByName( "team_manager" ));
		pTeam->Init( sTeamNames[i], i );

		g_Teams.AddToTail( pTeam );
	}

	m_bTeamPlayEnabled = teamplay.GetBool();
	m_flIntermissionEndTime = 0.0f;
	m_flGameStartTime = 0;

	m_hRespawnableItemsAndWeapons.RemoveAll();
	m_tmNextPeriodicThink = 0;
	m_flRestartGameTime = 0;
	m_bCompleteReset = false;
	m_bHeardAllPlayersReady = false;
	m_bAwaitingReadyRestart = false;
	m_bChangelevelDone = false;

#endif
}

const CViewVectors* CLabGameRules::GetViewVectors()const
{
	return &g_LabViewVectors;
}

const LabViewVectors* CLabGameRules::GetLabViewVectors()const
{
	return &g_LabViewVectors;
}
	
CLabGameRules::~CLabGameRules( void )
{
#ifndef CLIENT_DLL
	// Note, don't delete each team since they are in the gEntList and will 
	// automatically be deleted from there, instead.
	g_Teams.Purge();
#endif
}

bool CLabGameRules::ShouldCollide(int collisionGroup0, int collisionGroup1)
{
	if (collisionGroup0 > collisionGroup1)
	{
		// swap so that lowest is always first
		V_swap(collisionGroup0, collisionGroup1);
	}

	if ((collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT) &&
		collisionGroup1 == COLLISION_GROUP_WEAPON)
	{
		return false;
	}

	return BaseClass::ShouldCollide(collisionGroup0, collisionGroup1);

}

bool CLabGameRules::IsConnectedUserInfoChangeAllowed( CBasePlayer *pPlayer )
{
	return true;
}

#ifndef CLIENT_DLL

//------------------------------------------------------------------------------
// Purpose : Initialize all default class relationships
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CLabGameRules::InitDefaultAIRelationships( void )
{
	int i, j;

	//  Allocate memory for default relationships
	CBaseCombatCharacter::AllocateDefaultRelationships();

	// --------------------------------------------------------------
	// First initialize table so we can report missing relationships
	// --------------------------------------------------------------
	for (i=0;i<NUM_AI_CLASSES;i++)
	{
		for (j=0;j<NUM_AI_CLASSES;j++)
		{
			// By default all relationships are neutral of priority zero
			CBaseCombatCharacter::SetDefaultRelationship( (Class_T)i, (Class_T)j, D_NU, 0 );
		}
	}

	// ------------------------------------------------------------
	//	> CLASS_ANTLION
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_SCANNER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PROTOSNIPER,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_ANTLION,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

	// ------------------------------------------------------------
	//	> CLASS_BARNACLE
	//
	//  In this case, the relationship D_HT indicates which characters
	//  the barnacle will try to eat.
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_BARNACLE,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_MANHACK,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_EARTH_FAUNA,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

	// ------------------------------------------------------------
	//	> CLASS_BULLSEYE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PLAYER,			D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_ANTLION,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_VORTIGAUNT,		D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PLAYER_ALLY,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_HACKED_ROLLERMINE,D_NU, 0);

	// ------------------------------------------------------------
	//	> CLASS_BULLSQUID
	// ------------------------------------------------------------
	/*
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BARNACLE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BULLSEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HEADCRAB,			D_HT, 1);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HOUNDEYE,			D_HT, 1);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_MANHACK,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
	*/
	// ------------------------------------------------------------
	//	> CLASS_CITIZEN_PASSIVE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER,			D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_BARNACLE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_BULLSQUID,		D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_COMBINE_HUNTER,	D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_HEADCRAB,			D_FR, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_HOUNDEYE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_MANHACK,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_MISSILE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_VORTIGAUNT,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_ZOMBIE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER_ALLY,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_HACKED_ROLLERMINE,D_NU, 0);

	// ------------------------------------------------------------
	//	> CLASS_CITIZEN_REBEL
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PLAYER,			D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_BARNACLE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_BULLSQUID,		D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_MISSILE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_SCANNER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_VORTIGAUNT,		D_LI, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PLAYER_ALLY,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_HACKED_ROLLERMINE,D_NU, 0);

	// ------------------------------------------------------------
	//	> CLASS_COMBINE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_BARNACLE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_COMBINE,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_COMBINE_GUNSHIP,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_COMBINE_HUNTER,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

	// ------------------------------------------------------------
	//	> CLASS_COMBINE_GUNSHIP
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_COMBINE,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_COMBINE_GUNSHIP,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_COMBINE_HUNTER,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_MISSILE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_HACKED_ROLLERMINE,D_HT, 0);

	// ------------------------------------------------------------
	//	> CLASS_COMBINE_HUNTER
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,	CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_COMBINE,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_COMBINE_GUNSHIP,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_COMBINE_HUNTER,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,	CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_HACKED_ROLLERMINE,D_HT, 0);

	// ------------------------------------------------------------
	//	> CLASS_CONSCRIPT
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PLAYER,			D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_BARNACLE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_SCANNER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_VORTIGAUNT,		D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PLAYER_ALLY,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_HACKED_ROLLERMINE,D_NU, 0);
		
	// ------------------------------------------------------------
	//	> CLASS_FLARE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PLAYER,			D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_ANTLION,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_VORTIGAUNT,		D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PLAYER_ALLY,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_HACKED_ROLLERMINE,D_NU, 0);

	// ------------------------------------------------------------
	//	> CLASS_HEADCRAB
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_BULLSQUID,		D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_HACKED_ROLLERMINE,D_FR, 0);

	// ------------------------------------------------------------
	//	> CLASS_HOUNDEYE
	// ------------------------------------------------------------
	/*
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_BULLSQUID,		D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_HEADCRAB,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
	*/

	// ------------------------------------------------------------
	//	> CLASS_MANHACK
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_HEADCRAB,			D_HT,-1);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_HOUNDEYE,			D_HT,-1);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

	// ------------------------------------------------------------
	//	> CLASS_METROPOLICE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_HACKED_ROLLERMINE,D_HT, 0);

	// ------------------------------------------------------------
	//	> CLASS_MILITARY
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

	// ------------------------------------------------------------
	//	> CLASS_MISSILE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

	// ------------------------------------------------------------
	//	> CLASS_NONE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PLAYER,			D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_ANTLION,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_BULLSEYE,			D_NU, 0);	
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_VORTIGAUNT,		D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PLAYER_ALLY,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_HACKED_ROLLERMINE,D_NU, 0);

	// ------------------------------------------------------------
	//	> CLASS_PLAYER
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PLAYER,			D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_BARNACLE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_BULLSEYE,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_CITIZEN_PASSIVE,	D_LI, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_CITIZEN_REBEL,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_COMBINE_GUNSHIP,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_SCANNER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_VORTIGAUNT,		D_LI, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PROTOSNIPER,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PLAYER_ALLY,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_HACKED_ROLLERMINE,D_LI, 0);

	// ------------------------------------------------------------
	//	> CLASS_PLAYER_ALLY
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PLAYER,			D_LI, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_BARNACLE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_HEADCRAB,			D_FR, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_SCANNER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_VORTIGAUNT,		D_LI, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_ZOMBIE,			D_FR, 1);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PROTOSNIPER,		D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PLAYER_ALLY,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_HACKED_ROLLERMINE,D_LI, 0);

	// ------------------------------------------------------------
	//	> CLASS_PLAYER_ALLY_VITAL
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER,			D_LI, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_BARNACLE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_COMBINE_HUNTER,	D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_SCANNER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_VORTIGAUNT,		D_LI, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PROTOSNIPER,		D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER_ALLY,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_HACKED_ROLLERMINE,D_LI, 0);

	// ------------------------------------------------------------
	//	> CLASS_SCANNER
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_COMBINE,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_COMBINE_GUNSHIP,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_COMBINE_HUNTER,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_MANHACK,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_METROPOLICE,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_MILITARY,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_SCANNER,			D_LI, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_STALKER,			D_LI, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PROTOSNIPER,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

	// ------------------------------------------------------------
	//	> CLASS_STALKER
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

	// ------------------------------------------------------------
	//	> CLASS_VORTIGAUNT
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PLAYER,			D_LI, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_BARNACLE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_CITIZEN_PASSIVE,	D_LI, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_CITIZEN_REBEL,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_SCANNER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_VORTIGAUNT,		D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PLAYER_ALLY,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_HACKED_ROLLERMINE,D_LI, 0);

	// ------------------------------------------------------------
	//	> CLASS_ZOMBIE
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_MANHACK,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_MILITARY,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

	// ------------------------------------------------------------
	//	> CLASS_PROTOSNIPER
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_MISSILE,			D_NU, 5);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

	// ------------------------------------------------------------
	//	> CLASS_EARTH_FAUNA
	//
	// Hates pretty much everything equally except other earth fauna.
	// This will make the critter choose the nearest thing as its enemy.
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_NONE,				D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PLAYER,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_COMBINE_GUNSHIP,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_FLARE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_MISSILE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_SCANNER,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_STALKER,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_VORTIGAUNT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PROTOSNIPER,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_HACKED_ROLLERMINE,D_NU, 0);

	// ------------------------------------------------------------
	//	> CLASS_HACKED_ROLLERMINE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER,			D_LI, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_VORTIGAUNT,		D_LI, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_ZOMBIE,			D_HT, 1);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_EARTH_FAUNA,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER_ALLY,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_HACKED_ROLLERMINE,D_LI, 0);
}


//------------------------------------------------------------------------------
// Purpose : Return classify text for classify type
// Input   :
// Output  :
//------------------------------------------------------------------------------
const char* CLabGameRules::AIClassText(int classType)
{
	switch (classType)
	{
		case CLASS_NONE:			return "CLASS_NONE";
		case CLASS_PLAYER:			return "CLASS_PLAYER";
		case CLASS_ANTLION:			return "CLASS_ANTLION";
		case CLASS_BARNACLE:		return "CLASS_BARNACLE";
		case CLASS_BULLSEYE:		return "CLASS_BULLSEYE";
		//case CLASS_BULLSQUID:		return "CLASS_BULLSQUID";	
		case CLASS_CITIZEN_PASSIVE: return "CLASS_CITIZEN_PASSIVE";		
		case CLASS_CITIZEN_REBEL:	return "CLASS_CITIZEN_REBEL";
		case CLASS_COMBINE:			return "CLASS_COMBINE";
		case CLASS_COMBINE_GUNSHIP:	return "CLASS_COMBINE_GUNSHIP";
		case CLASS_COMBINE_HUNTER:	return "CLASS_COMBINE_HUNTER";
		case CLASS_CONSCRIPT:		return "CLASS_CONSCRIPT";
		case CLASS_HEADCRAB:		return "CLASS_HEADCRAB";
		//case CLASS_HOUNDEYE:		return "CLASS_HOUNDEYE";
		case CLASS_MANHACK:			return "CLASS_MANHACK";
		case CLASS_METROPOLICE:		return "CLASS_METROPOLICE";
		case CLASS_MILITARY:		return "CLASS_MILITARY";	
		case CLASS_SCANNER:			return "CLASS_SCANNER";		
		case CLASS_STALKER:			return "CLASS_STALKER";		
		case CLASS_VORTIGAUNT:		return "CLASS_VORTIGAUNT";
		case CLASS_ZOMBIE:			return "CLASS_ZOMBIE";
		case CLASS_PROTOSNIPER:		return "CLASS_PROTOSNIPER";
		case CLASS_MISSILE:			return "CLASS_MISSILE";
		case CLASS_FLARE:			return "CLASS_FLARE";
		case CLASS_EARTH_FAUNA:		return "CLASS_EARTH_FAUNA";

		default:					return "MISSING CLASS in ClassifyText()";
	}
}



void CLabGameRules::Think( void )
{
	BaseClass::Think();

	if( physcannon_mega_enabled.GetBool() == true )
	{
		m_bMegaPhysgun = true;
	}
	else
	{
		// FIXME: Is there a better place for this?
		m_bMegaPhysgun = ( GlobalEntity_GetState("super_phys_gun") == GLOBAL_ON );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns how much damage the given ammo type should do to the victim
//			when fired by the attacker.
// Input  : pAttacker - Dude what shot the gun.
//			pVictim - Dude what done got shot.
//			nAmmoType - What been shot out.
// Output : How much hurt to put on dude what done got shot (pVictim).
//-----------------------------------------------------------------------------
float CLabGameRules::GetAmmoDamage( CBaseEntity *pAttacker, CBaseEntity *pVictim, int nAmmoType )
{
	float flDamage = 0.0f;
	CAmmoDef *pAmmoDef = GetAmmoDef();

	if ( pAmmoDef->DamageType( nAmmoType ) & DMG_SNIPER )
	{
		// If this damage is from a SNIPER, we do damage based on what the bullet
		// HITS, not who fired it. All other bullets have their damage values
		// arranged according to the owner of the bullet, not the recipient.
		if ( pVictim->IsPlayer() )
		{
			// Player
			flDamage = pAmmoDef->PlrDamage( nAmmoType );
		}
		else
		{
			// NPC or breakable
			flDamage = pAmmoDef->NPCDamage( nAmmoType );
		}
	}
	else
	{
		flDamage = BaseClass::GetAmmoDamage( pAttacker, pVictim, nAmmoType );
	}

	if( pAttacker->IsPlayer() && pVictim->IsNPC() )
	{
		if( pVictim->MyCombatCharacterPointer() )
		{
			// Player is shooting an NPC. Adjust the damage! This protects breakables
			// and other 'non-living' entities from being easier/harder to break
			// in different skill levels.
			flDamage = pAmmoDef->PlrDamage( nAmmoType );
			flDamage = AdjustPlayerDamageInflicted( flDamage );
		}
	}

	return flDamage;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CLabGameRules::AllowDamage( CBaseEntity *pVictim, const CTakeDamageInfo &info )
{
	/*if( (info.GetDamageType() & DMG_CRUSH) && info.GetInflictor() && pVictim->MyNPCPointer() )
	{
		if( pVictim->MyNPCPointer()->IsPlayerAlly() )
		{
			// A physics object has struck a player ally. Don't allow damage if it
			// came from the player's physcannon. 
			CBasePlayer *pPlayer = UTIL_PlayerByIndex(1);

			if( pPlayer )
			{
				CBaseEntity *pWeapon = pPlayer->HasNamedPlayerItem("weapon_physcannon");

				if( pWeapon )
				{
					CBaseCombatWeapon *pCannon = assert_cast <CBaseCombatWeapon*>(pWeapon);

					if( pCannon )
					{
						if( PhysCannonAccountableForObject(pCannon, info.GetInflictor() ) )
						{
							// Antlions can always be squashed!
							if ( pVictim->Classify() == CLASS_ANTLION )
								return true;

  							return false;
						}
					}
				}
			}
		}
	}*/
  	return true;
}

void CLabGameRules::CreateStandardEntities( void )
{

	// Create the entity that will send our data to the client.

	BaseClass::CreateStandardEntities();

	g_pLastCombineSpawn = NULL;
	g_pLastRebelSpawn = NULL;

#ifdef DBGFLAG_ASSERT
	CBaseEntity *pEnt = 
#endif
	CBaseEntity::Create( "hl2mp_gamerules", vec3_origin, vec3_angle );
	Assert( pEnt );
}

//=========================================================
// FlWeaponRespawnTime - what is the time in the future
// at which this weapon may spawn?
//=========================================================
float CLabGameRules::FlWeaponRespawnTime( CBaseCombatWeapon *pWeapon )
{
	if ( weaponstay.GetInt() > 0 )
	{
		// make sure it's only certain weapons
		if ( !(pWeapon->GetWeaponFlags() & ITEM_FLAG_LIMITINWORLD) )
		{
			return 0;		// weapon respawns almost instantly
		}
	}

	return sv_hl2mp_weapon_respawn_time.GetFloat();
}


bool CLabGameRules::IsIntermission( void )
{
	return m_flIntermissionEndTime > gpGlobals->curtime;
}

void CLabGameRules::PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
	if ( IsIntermission() )
		return;
	BaseClass::PlayerKilled( pVictim, info );
}


void CLabGameRules::Think( void )
{

	
	CGameRules::Think();

	if ( g_fGameOver )   // someone else quit the game already
	{
		// check to see if we should change levels now
		if ( m_flIntermissionEndTime < gpGlobals->curtime )
		{
			if ( !m_bChangelevelDone )
			{
				ChangeLevel(); // intermission is over
				m_bChangelevelDone = true;
			}
		}

		return;
	}

//	float flTimeLimit = mp_timelimit.GetFloat() * 60;
	float flFragLimit = fraglimit.GetFloat();
	
	if ( GetMapRemainingTime() < 0 )
	{
		GoToIntermission();
		return;
	}

	if ( flFragLimit )
	{
		if( IsTeamplay() == true )
		{
			CTeam *pCombine = g_Teams[TEAM_COMBINE];
			CTeam *pRebels = g_Teams[TEAM_REBELS];

			if ( pCombine->GetScore() >= flFragLimit || pRebels->GetScore() >= flFragLimit )
			{
				GoToIntermission();
				return;
			}
		}
		else
		{
			// check if any player is over the frag limit
			for ( int i = 1; i <= gpGlobals->maxClients; i++ )
			{
				CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

				if ( pPlayer && pPlayer->FragCount() >= flFragLimit )
				{
					GoToIntermission();
					return;
				}
			}
		}
	}

	if ( gpGlobals->curtime > m_tmNextPeriodicThink )
	{		
		CheckAllPlayersReady();
		CheckRestartGame();
		m_tmNextPeriodicThink = gpGlobals->curtime + 1.0;
	}

	if ( m_flRestartGameTime > 0.0f && m_flRestartGameTime <= gpGlobals->curtime )
	{
		RestartGame();
	}

	if( m_bAwaitingReadyRestart && m_bHeardAllPlayersReady )
	{
		UTIL_ClientPrintAll( HUD_PRINTCENTER, "All players ready. Game will restart in 5 seconds" );
		UTIL_ClientPrintAll( HUD_PRINTCONSOLE, "All players ready. Game will restart in 5 seconds" );

		m_flRestartGameTime = gpGlobals->curtime + 5;
		m_bAwaitingReadyRestart = false;
	}

	ManageObjectRelocation();

}

void CLabGameRules::GoToIntermission( void )
{
	if ( g_fGameOver )
		return;

	g_fGameOver = true;

	m_flIntermissionEndTime = gpGlobals->curtime + mp_chattime.GetInt();

	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;

		pPlayer->ShowViewPortPanel( PANEL_SCOREBOARD );
		pPlayer->AddFlag( FL_FROZEN );
	}
	
}

bool CLabGameRules::CheckGameOver()
{
	if ( g_fGameOver )   // someone else quit the game already
	{
		// check to see if we should change levels now
		if ( m_flIntermissionEndTime < gpGlobals->curtime )
		{
			ChangeLevel(); // intermission is over			
		}

		return true;
	}

	return false;
}


// when we are within this close to running out of entities,  items 
// marked with the ITEM_FLAG_LIMITINWORLD will delay their respawn
#define ENTITY_INTOLERANCE	100

//=========================================================
// FlWeaponRespawnTime - Returns 0 if the weapon can respawn 
// now,  otherwise it returns the time at which it can try
// to spawn again.
//=========================================================
float CLabGameRules::FlWeaponTryRespawn( CBaseCombatWeapon *pWeapon )
{
	if ( pWeapon && (pWeapon->GetWeaponFlags() & ITEM_FLAG_LIMITINWORLD) )
	{
		if ( gEntList.NumberOfEntities() < (gpGlobals->maxEntities - ENTITY_INTOLERANCE) )
			return 0;

		// we're past the entity tolerance level,  so delay the respawn
		return FlWeaponRespawnTime( pWeapon );
	}
	return 0;
}

//=========================================================
// VecWeaponRespawnSpot - where should this weapon spawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CLabGameRules::VecWeaponRespawnSpot( CBaseCombatWeapon *pWeapon )
{
	CWeaponHL2MPBase *pHL2Weapon = dynamic_cast< CWeaponHL2MPBase*>( pWeapon );

	if ( pHL2Weapon )
	{
		return pHL2Weapon->GetOriginalSpawnOrigin();
	}
	
	return pWeapon->GetAbsOrigin();
}


const char *CLabGameRules::GetGameDescription( void )
{ 
	if ( IsTeamplay() )
		return "Team Deathmatch"; 

	return "Deathmatch"; 
} 

float CLabGameRules::GetMapRemainingTime()
{
	// if timelimit is disabled, return 0
	if ( mp_timelimit.GetInt() <= 0 )
		return 0;

	// timelimit is in minutes

	float timeleft = (m_flGameStartTime + mp_timelimit.GetInt() * 60.0f ) - gpGlobals->curtime;

	return timeleft;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLabGameRules::Precache( void )
{
	CBaseEntity::PrecacheScriptSound( "AlyxEmp.Charge" );
}

CItem* IsManagedObjectAnItem( CBaseEntity *pObject )
{
	return dynamic_cast< CItem*>( pObject );
}

CWeaponHL2MPBase* IsManagedObjectAWeapon( CBaseEntity *pObject )
{
	return dynamic_cast< CWeaponHL2MPBase*>( pObject );
}

bool GetObjectsOriginalParameters( CBaseEntity *pObject, Vector &vOriginalOrigin, QAngle &vOriginalAngles )
{
	if ( CItem *pItem = IsManagedObjectAnItem( pObject ) )
	{
		if ( pItem->m_flNextResetCheckTime > gpGlobals->curtime )
			 return false;
		
		vOriginalOrigin = pItem->GetOriginalSpawnOrigin();
		vOriginalAngles = pItem->GetOriginalSpawnAngles();

		pItem->m_flNextResetCheckTime = gpGlobals->curtime + sv_hl2mp_item_respawn_time.GetFloat();
		return true;
	}
	else if ( CWeaponHL2MPBase *pWeapon = IsManagedObjectAWeapon( pObject )) 
	{
		if ( pWeapon->m_flNextResetCheckTime > gpGlobals->curtime )
			 return false;

		vOriginalOrigin = pWeapon->GetOriginalSpawnOrigin();
		vOriginalAngles = pWeapon->GetOriginalSpawnAngles();

		pWeapon->m_flNextResetCheckTime = gpGlobals->curtime + sv_hl2mp_weapon_respawn_time.GetFloat();
		return true;
	}

	return false;
}

void CLabGameRules::ManageObjectRelocation( void )
{
	int iTotal = m_hRespawnableItemsAndWeapons.Count();

	if ( iTotal > 0 )
	{
		for ( int i = 0; i < iTotal; i++ )
		{
			CBaseEntity *pObject = m_hRespawnableItemsAndWeapons[i].Get();
			
			if ( pObject )
			{
				Vector vSpawOrigin;
				QAngle vSpawnAngles;

				if ( GetObjectsOriginalParameters( pObject, vSpawOrigin, vSpawnAngles ) == true )
				{
					float flDistanceFromSpawn = (pObject->GetAbsOrigin() - vSpawOrigin ).Length();

					if ( flDistanceFromSpawn > WEAPON_MAX_DISTANCE_FROM_SPAWN )
					{
						bool shouldReset = false;
						IPhysicsObject *pPhysics = pObject->VPhysicsGetObject();

						if ( pPhysics )
						{
							shouldReset = pPhysics->IsAsleep();
						}
						else
						{
							shouldReset = (pObject->GetFlags() & FL_ONGROUND) ? true : false;
						}

						if ( shouldReset )
						{
							pObject->Teleport( &vSpawOrigin, &vSpawnAngles, NULL );
							pObject->EmitSound( "AlyxEmp.Charge" );

							IPhysicsObject *pPhys = pObject->VPhysicsGetObject();

							if ( pPhys )
							{
								pPhys->Wake();
							}
						}
					}
				}
			}
		}
	}
}

//=========================================================
//AddLevelDesignerPlacedWeapon
//=========================================================
void CLabGameRules::AddLevelDesignerPlacedObject( CBaseEntity *pEntity )
{
	if ( m_hRespawnableItemsAndWeapons.Find( pEntity ) == -1 )
	{
		m_hRespawnableItemsAndWeapons.AddToTail( pEntity );
	}
}

//=========================================================
//RemoveLevelDesignerPlacedWeapon
//=========================================================
void CLabGameRules::RemoveLevelDesignerPlacedObject( CBaseEntity *pEntity )
{
	if ( m_hRespawnableItemsAndWeapons.Find( pEntity ) != -1 )
	{
		m_hRespawnableItemsAndWeapons.FindAndRemove( pEntity );
	}
}

//=========================================================
// Where should this item respawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CLabGameRules::VecItemRespawnSpot( CItem *pItem )
{
	return pItem->GetOriginalSpawnOrigin();
}

//=========================================================
// What angles should this item use to respawn?
//=========================================================
QAngle CLabGameRules::VecItemRespawnAngles( CItem *pItem )
{
	return pItem->GetOriginalSpawnAngles();
}

//=========================================================
// At what time in the future may this Item respawn?
//=========================================================
float CLabGameRules::FlItemRespawnTime( CItem *pItem )
{
	return sv_hl2mp_item_respawn_time.GetFloat();
}


//=========================================================
// CanHaveWeapon - returns false if the player is not allowed
// to pick up this weapon
//=========================================================
bool CLabGameRules::CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pItem )
{
	if ( weaponstay.GetInt() > 0 )
	{
		if ( pPlayer->Weapon_OwnsThisType( pItem->GetClassname(), pItem->GetSubType() ) )
			 return false;
	}

	return BaseClass::CanHavePlayerItem( pPlayer, pItem );
}


//=========================================================
// WeaponShouldRespawn - any conditions inhibiting the
// respawning of this weapon?
//=========================================================
int CLabGameRules::WeaponShouldRespawn( CBaseCombatWeapon *pWeapon )
{
	if ( pWeapon->HasSpawnFlags( SF_NORESPAWN ) )
	{
		return GR_WEAPON_RESPAWN_NO;
	}

	return GR_WEAPON_RESPAWN_YES;
}

//-----------------------------------------------------------------------------
// Purpose: Player has just left the game
//-----------------------------------------------------------------------------
void CLabGameRules::ClientDisconnected(edict_t *pClient)
{
	// Msg( "CLIENT DISCONNECTED, REMOVING FROM TEAM.\n" );

	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pClient);
	if (pPlayer)
	{
		// Remove the player from his team
		if (pPlayer->GetTeam())
		{
			pPlayer->GetTeam()->RemovePlayer(pPlayer);
		}
	}

	BaseClass::ClientDisconnected(pClient);

}


//=========================================================
// Deathnotice. 
//=========================================================
void CLabGameRules::DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
	// Work out what killed the player, and send a message to all clients about it
	const char *killer_weapon_name = "world";		// by default, the player is killed by the world
	int killer_ID = 0;

	// Find the killer & the scorer
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pKiller = info.GetAttacker();
	CBasePlayer *pScorer = GetDeathScorer( pKiller, pInflictor );

	// Custom kill type?
	if ( info.GetDamageCustom() )
	{
		killer_weapon_name = GetDamageCustomString( info );
		if ( pScorer )
		{
			killer_ID = pScorer->GetUserID();
		}
	}
	else
	{
		// Is the killer a client?
		if ( pScorer )
		{
			killer_ID = pScorer->GetUserID();
			
			if ( pInflictor )
			{
				if ( pInflictor == pScorer )
				{
					// If the inflictor is the killer,  then it must be their current weapon doing the damage
					if ( pScorer->GetActiveWeapon() )
					{
						killer_weapon_name = pScorer->GetActiveWeapon()->GetClassname();
					}
				}
				else
				{
					killer_weapon_name = pInflictor->GetClassname();  // it's just that easy
				}
			}
		}
		else
		{
			killer_weapon_name = pInflictor->GetClassname();
		}

		// strip the NPC_* or weapon_* from the inflictor's classname
		if ( strncmp( killer_weapon_name, "weapon_", 7 ) == 0 )
		{
			killer_weapon_name += 7;
		}
		else if ( strncmp( killer_weapon_name, "npc_", 4 ) == 0 )
		{
			killer_weapon_name += 4;
		}
		else if ( strncmp( killer_weapon_name, "func_", 5 ) == 0 )
		{
			killer_weapon_name += 5;
		}
		else if ( strstr( killer_weapon_name, "physics" ) )
		{
			killer_weapon_name = "physics";
		}

		if ( strcmp( killer_weapon_name, "prop_combine_ball" ) == 0 )
		{
			killer_weapon_name = "combine_ball";
		}
		else if ( strcmp( killer_weapon_name, "grenade_ar2" ) == 0 )
		{
			killer_weapon_name = "smg1_grenade";
		}
		else if ( strcmp( killer_weapon_name, "satchel" ) == 0 || strcmp( killer_weapon_name, "tripmine" ) == 0)
		{
			killer_weapon_name = "slam";
		}


	}

	IGameEvent *event = gameeventmanager->CreateEvent( "player_death" );
	if( event )
	{
		event->SetInt("userid", pVictim->GetUserID() );
		event->SetInt("attacker", killer_ID );
		event->SetString("weapon", killer_weapon_name );
		event->SetInt( "priority", 7 );
		gameeventmanager->FireEvent( event );
	}

}

void CLabGameRules::ClientSettingsChanged( CBasePlayer *pPlayer )
{
	
	CLabPlayer *pHL2Player = ToLabPlayer( pPlayer );

	if ( pHL2Player == NULL )
		return;

	const char *pCurrentModel = modelinfo->GetModelName( pPlayer->GetModel() );
	const char *szModelName = engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), "cl_playermodel" );

	//If we're different.
	if ( stricmp( szModelName, pCurrentModel ) )
	{
		//Too soon, set the cvar back to what it was.
		//Note: this will make this function be called again
		//but since our models will match it'll just skip this whole dealio.
		if ( pHL2Player->GetNextModelChangeTime() >= gpGlobals->curtime )
		{
			char szReturnString[512];

			Q_snprintf( szReturnString, sizeof (szReturnString ), "cl_playermodel %s\n", pCurrentModel );
			engine->ClientCommand ( pHL2Player->edict(), szReturnString );

			Q_snprintf( szReturnString, sizeof( szReturnString ), "Please wait %d more seconds before trying to switch.\n", (int)(pHL2Player->GetNextModelChangeTime() - gpGlobals->curtime) );
			ClientPrint( pHL2Player, HUD_PRINTTALK, szReturnString );
			return;
		}

		if ( IsTeamplay() == false )
		{
			pHL2Player->SetPlayerModel();

			const char *pszCurrentModelName = modelinfo->GetModelName( pHL2Player->GetModel() );

			char szReturnString[128];
			Q_snprintf( szReturnString, sizeof( szReturnString ), "Your player model is: %s\n", pszCurrentModelName );

			ClientPrint( pHL2Player, HUD_PRINTTALK, szReturnString );
		}
		else
		{
			if ( Q_stristr( szModelName, "models/human") )
			{
				pHL2Player->ChangeTeam( TEAM_REBELS );
			}
			else
			{
				pHL2Player->ChangeTeam( TEAM_COMBINE );
			}
		}
	}
	if ( sv_report_client_settings.GetInt() == 1 )
	{
		UTIL_LogPrintf( "\"%s\" cl_cmdrate = \"%s\"\n", pHL2Player->GetPlayerName(), engine->GetClientConVarValue( pHL2Player->entindex(), "cl_cmdrate" ));
	}

	BaseClass::ClientSettingsChanged( pPlayer );
	
}

int CLabGameRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
	// half life multiplay has a simple concept of Player Relationships.
	// you are either on another player's team, or you are not.
	if ( !pPlayer || !pTarget || !pTarget->IsPlayer() || IsTeamplay() == false )
		return GR_NOTTEAMMATE;

	if ( (*GetTeamID(pPlayer) != '\0') && (*GetTeamID(pTarget) != '\0') && !stricmp( GetTeamID(pPlayer), GetTeamID(pTarget) ) )
	{
		return GR_TEAMMATE;
	}

	return GR_NOTTEAMMATE;
}


bool CLabGameRules::ClientCommand( CBaseEntity *pEdict, const CCommand &args )
{
	if( BaseClass::ClientCommand( pEdict, args ) )
		return true;


	CLabPlayer *pPlayer = (CLabPlayer *) pEdict;

	if ( pPlayer->ClientCommand( args ) )
		return true;

	return false;
}


#ifdef DEBUG

	// Handler for the "bot" command.
	void Bot_f()
	{		
		// Look at -count.
		int count = 1;
		count = clamp( count, 1, 16 );

		int iTeam = TEAM_COMBINE;
				
		// Look at -frozen.
		bool bFrozen = false;
			
		// Ok, spawn all the bots.
		while ( --count >= 0 )
		{
			BotPutInServer( bFrozen, iTeam );
		}
	}


	ConCommand cc_Bot( "bot", Bot_f, "Add a bot.", FCVAR_CHEAT );

#endif

	bool CLabGameRules::FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon )
	{		
		if ( pPlayer->GetActiveWeapon() && pPlayer->IsNetClient() )
		{
			// Player has an active item, so let's check cl_autowepswitch.
			const char *cl_autowepswitch = engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), "cl_autowepswitch" );
			if ( cl_autowepswitch && atoi( cl_autowepswitch ) <= 0 )
			{
				return false;
			}
		}

		return BaseClass::FShouldSwitchWeapon( pPlayer, pWeapon );
	}

void CLabGameRules::RestartGame()
{
	// bounds check
	if ( mp_timelimit.GetInt() < 0 )
	{
		mp_timelimit.SetValue( 0 );
	}
	m_flGameStartTime = gpGlobals->curtime;
	if ( !IsFinite( m_flGameStartTime.Get() ) )
	{
		Warning( "Trying to set a NaN game start time\n" );
		m_flGameStartTime.GetForModify() = 0.0f;
	}

	CleanUpMap();
	
	// now respawn all players
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CLabPlayer *pPlayer = (CLabPlayer*) UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;

		if ( pPlayer->GetActiveWeapon() )
		{
			pPlayer->GetActiveWeapon()->Holster();
		}
		pPlayer->RemoveAllItems( true );
		respawn( pPlayer, false );
		pPlayer->Reset();
	}

	// Respawn entities (glass, doors, etc..)

	CTeam *pRebels = GetGlobalTeam( TEAM_REBELS );
	CTeam *pCombine = GetGlobalTeam( TEAM_COMBINE );

	if ( pRebels )
	{
		pRebels->SetScore( 0 );
	}

	if ( pCombine )
	{
		pCombine->SetScore( 0 );
	}

	m_flIntermissionEndTime = 0;
	m_flRestartGameTime = 0.0;		
	m_bCompleteReset = false;

	IGameEvent * event = gameeventmanager->CreateEvent( "round_start" );
	if ( event )
	{
		event->SetInt("fraglimit", 0 );
		event->SetInt( "priority", 6 ); // HLTV event priority, not transmitted

		event->SetString("objective","DEATHMATCH");

		gameeventmanager->FireEvent( event );
	}
}

void CLabGameRules::CleanUpMap()
{
	// Recreate all the map entities from the map data (preserving their indices),
	// then remove everything else except the players.

	// Get rid of all entities except players.
	CBaseEntity *pCur = gEntList.FirstEnt();
	while ( pCur )
	{
		CBaseHL2MPCombatWeapon *pWeapon = dynamic_cast< CBaseHL2MPCombatWeapon* >( pCur );
		// Weapons with owners don't want to be removed..
		if ( pWeapon )
		{
			if ( !pWeapon->GetPlayerOwner() )
			{
				UTIL_Remove( pCur );
			}
		}
		// remove entities that has to be restored on roundrestart (breakables etc)
		else if ( !FindInList( s_PreserveEnts, pCur->GetClassname() ) )
		{
			UTIL_Remove( pCur );
		}

		pCur = gEntList.NextEnt( pCur );
	}

	// Really remove the entities so we can have access to their slots below.
	gEntList.CleanupDeleteList();

	// Cancel all queued events, in case a func_bomb_target fired some delayed outputs that
	// could kill respawning CTs
	g_EventQueue.Clear();

	// Now reload the map entities.
	class CHL2MPMapEntityFilter : public IMapEntityFilter
	{
	public:
		virtual bool ShouldCreateEntity( const char *pClassname )
		{
			// Don't recreate the preserved entities.
			if ( !FindInList( s_PreserveEnts, pClassname ) )
			{
				return true;
			}
			else
			{
				// Increment our iterator since it's not going to call CreateNextEntity for this ent.
				if ( m_iIterator != g_MapEntityRefs.InvalidIndex() )
					m_iIterator = g_MapEntityRefs.Next( m_iIterator );

				return false;
			}
		}


		virtual CBaseEntity* CreateNextEntity( const char *pClassname )
		{
			if ( m_iIterator == g_MapEntityRefs.InvalidIndex() )
			{
				// This shouldn't be possible. When we loaded the map, it should have used 
				// CCSMapLoadEntityFilter, which should have built the g_MapEntityRefs list
				// with the same list of entities we're referring to here.
				Assert( false );
				return NULL;
			}
			else
			{
				CMapEntityRef &ref = g_MapEntityRefs[m_iIterator];
				m_iIterator = g_MapEntityRefs.Next( m_iIterator );	// Seek to the next entity.

				if ( ref.m_iEdict == -1 || engine->PEntityOfEntIndex( ref.m_iEdict ) )
				{
					// Doh! The entity was delete and its slot was reused.
					// Just use any old edict slot. This case sucks because we lose the baseline.
					return CreateEntityByName( pClassname );
				}
				else
				{
					// Cool, the slot where this entity was is free again (most likely, the entity was 
					// freed above). Now create an entity with this specific index.
					return CreateEntityByName( pClassname, ref.m_iEdict );
				}
			}
		}

	public:
		int m_iIterator; // Iterator into g_MapEntityRefs.
	};
	CHL2MPMapEntityFilter filter;
	filter.m_iIterator = g_MapEntityRefs.Head();

	// DO NOT CALL SPAWN ON info_node ENTITIES!

	MapEntity_ParseAllEntities( engine->GetMapEntitiesString(), &filter, true );
}

void CLabGameRules::CheckChatForReadySignal( CLabPlayer *pPlayer, const char *chatmsg )
{
	if( m_bAwaitingReadyRestart && FStrEq( chatmsg, mp_ready_signal.GetString() ) )
	{
		if( !pPlayer->IsReady() )
		{
			pPlayer->SetReady( true );
		}		
	}
}

void CLabGameRules::CheckRestartGame( void )
{
	// Restart the game if specified by the server
	int iRestartDelay = mp_restartgame.GetInt();

	if ( iRestartDelay > 0 )
	{
		if ( iRestartDelay > 60 )
			iRestartDelay = 60;


		// let the players know
		char strRestartDelay[64];
		Q_snprintf( strRestartDelay, sizeof( strRestartDelay ), "%d", iRestartDelay );
		UTIL_ClientPrintAll( HUD_PRINTCENTER, "Game will restart in %s1 %s2", strRestartDelay, iRestartDelay == 1 ? "SECOND" : "SECONDS" );
		UTIL_ClientPrintAll( HUD_PRINTCONSOLE, "Game will restart in %s1 %s2", strRestartDelay, iRestartDelay == 1 ? "SECOND" : "SECONDS" );

		m_flRestartGameTime = gpGlobals->curtime + iRestartDelay;
		m_bCompleteReset = true;
		mp_restartgame.SetValue( 0 );
	}

	if( mp_readyrestart.GetBool() )
	{
		m_bAwaitingReadyRestart = true;
		m_bHeardAllPlayersReady = false;
		

		const char *pszReadyString = mp_ready_signal.GetString();


		// Don't let them put anything malicious in there
		if( pszReadyString == NULL || Q_strlen(pszReadyString) > 16 )
		{
			pszReadyString = "ready";
		}

		IGameEvent *event = gameeventmanager->CreateEvent( "hl2mp_ready_restart" );
		if ( event )
			gameeventmanager->FireEvent( event );

		mp_readyrestart.SetValue( 0 );

		// cancel any restart round in progress
		m_flRestartGameTime = -1;
	}
}

void CLabGameRules::CheckAllPlayersReady( void )
{
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CLabPlayer *pPlayer = (CLabPlayer*) UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;
		if ( !pPlayer->IsReady() )
			return;
	}
	m_bHeardAllPlayersReady = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CLabGameRules::GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer )
{
	if ( !pPlayer )  // dedicated server output
	{
		return NULL;
	}

	const char *pszFormat = NULL;

	// team only
	if ( bTeamOnly == TRUE )
	{
		if ( pPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		{
			pszFormat = "HL2MP_Chat_Spec";
		}
		else
		{
			const char *chatLocation = GetChatLocation( bTeamOnly, pPlayer );
			if ( chatLocation && *chatLocation )
			{
				pszFormat = "HL2MP_Chat_Team_Loc";
			}
			else
			{
				pszFormat = "HL2MP_Chat_Team";
			}
		}
	}
	// everyone
	else
	{
		if ( pPlayer->GetTeamNumber() != TEAM_SPECTATOR )
		{
			pszFormat = "HL2MP_Chat_All";	
		}
		else
		{
			pszFormat = "HL2MP_Chat_AllSpec";
		}
	}

	return pszFormat;
}

//-----------------------------------------------------------------------------
// Purpose: Whether or not the NPC should drop a health vial
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CLabGameRules::NPC_ShouldDropHealth( CBasePlayer *pRecipient )
{
	// Can only do this every so often
	if ( m_flLastHealthDropTime > gpGlobals->curtime )
		return false;

	//Try to throw dynamic health
	float healthPerc = ( (float) pRecipient->m_iHealth / (float) pRecipient->m_iMaxHealth );

	if ( random->RandomFloat( 0.0f, 1.0f ) > healthPerc*1.5f )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Whether or not the NPC should drop a health vial
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CLabGameRules::NPC_ShouldDropGrenade( CBasePlayer *pRecipient )
{
	// Can only do this every so often
	if ( m_flLastGrenadeDropTime > gpGlobals->curtime )
		return false;
		
	int grenadeIndex = GetAmmoDef()->Index( "grenade" );
	int numGrenades = pRecipient->GetAmmoCount( grenadeIndex );

	// If we're not maxed out on grenades and we've randomly okay'd it
	if ( ( numGrenades < GetAmmoDef()->MaxCarry( grenadeIndex ) ) && ( random->RandomInt( 0, 2 ) == 0 ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Update the drop counter for health
//-----------------------------------------------------------------------------
void CLabGameRules::NPC_DroppedHealth( void )
{
	m_flLastHealthDropTime = gpGlobals->curtime + sk_plr_health_drop_time.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: Update the drop counter for grenades
//-----------------------------------------------------------------------------
void CLabGameRules::NPC_DroppedGrenade( void )
{
	m_flLastGrenadeDropTime = gpGlobals->curtime + sk_plr_grenade_drop_time.GetFloat();
}

//-----------------------------------------------------------------------------
// Returns whether or not Alyx cares about light levels in order to see.
//-----------------------------------------------------------------------------
bool CLabGameRules::IsAlyxInDarknessMode()
{
	if ( alyx_darkness_force.GetBool() )
		return true;

	return ( GlobalEntity_GetState( "ep_alyx_darknessmode" ) == GLOBAL_ON );
}



//-----------------------------------------------------------------------------
// Purpose: MULTIPLAYER BODY QUE HANDLING
//-----------------------------------------------------------------------------
class CCorpse : public CBaseAnimating
{
public:
	DECLARE_CLASS( CCorpse, CBaseAnimating );
	DECLARE_SERVERCLASS();

	virtual int ObjectCaps( void ) { return FCAP_DONT_SAVE; }	

public:
	CNetworkVar( int, m_nReferencePlayer );
};

IMPLEMENT_SERVERCLASS_ST(CCorpse, DT_Corpse)
	SendPropInt( SENDINFO(m_nReferencePlayer), 10, SPROP_UNSIGNED )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( bodyque, CCorpse );


CCorpse		*g_pBodyQueueHead;

void InitBodyQue(void)
{
	CCorpse *pEntity = ( CCorpse * )CreateEntityByName( "bodyque" );
	pEntity->AddEFlags( EFL_KEEP_ON_RECREATE_ENTITIES );
	g_pBodyQueueHead = pEntity;
	CCorpse *p = g_pBodyQueueHead;
		
	// Reserve 3 more slots for dead bodies
	for ( int i = 0; i < 3; i++ )
	{
		CCorpse *next = ( CCorpse * )CreateEntityByName( "bodyque" );
		next->AddEFlags( EFL_KEEP_ON_RECREATE_ENTITIES );
		p->SetOwnerEntity( next );
		p = next;
	}
		
	p->SetOwnerEntity( g_pBodyQueueHead );
}

//-----------------------------------------------------------------------------
// Purpose: make a body que entry for the given ent so the ent can be respawned elsewhere
// GLOBALS ASSUMED SET:  g_eoBodyQueueHead
//-----------------------------------------------------------------------------
void CopyToBodyQue( CBaseAnimating *pCorpse ) 
{
	if ( pCorpse->IsEffectActive( EF_NODRAW ) )
		return;

	CCorpse *pHead	= g_pBodyQueueHead;

	pHead->CopyAnimationDataFrom( pCorpse );

	pHead->SetMoveType( MOVETYPE_FLYGRAVITY );
	pHead->SetAbsVelocity( pCorpse->GetAbsVelocity() );
	pHead->ClearFlags();
	pHead->m_nReferencePlayer	= ENTINDEX( pCorpse );

	pHead->SetLocalAngles( pCorpse->GetAbsAngles() );
	UTIL_SetOrigin(pHead, pCorpse->GetAbsOrigin());

	UTIL_SetSize(pHead, pCorpse->WorldAlignMins(), pCorpse->WorldAlignMaxs());
	g_pBodyQueueHead = (CCorpse *)pHead->GetOwnerEntity();
}

#endif
