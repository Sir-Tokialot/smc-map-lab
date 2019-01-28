#include "cbase.h"
#include "lab_gamerules.h"
#include "ammodef.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

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

CLabGameRules::CLabGameRules()
{
}
	
CLabGameRules::~CLabGameRules()
{
}

#ifdef CLIENT_DLL

#else
void CLabGameRules::Think()
{
	BaseClass::Think();
}
#endif

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