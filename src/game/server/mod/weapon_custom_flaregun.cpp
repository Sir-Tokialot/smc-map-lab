//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Flare gun (fffsssssssssss!!)
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "basehlcombatweapon.h"
#include "decals.h"
#include "soundenvelope.h"
#include "IEffects.h"
#include "engine/IEngineSound.h"
#include "weapon_flaregun.h"
#include "props.h"
#include "ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Custom derived class for flare gun projectiles
class CFlareGunProjectile : public CFlare
{
public:
	DECLARE_CLASS(CFlareGunProjectile, CFlare);
	static CFlareGunProjectile *Create(Vector vecOrigin, QAngle vecAngles, CBaseEntity *pOwner, float lifetime);
	void	IgniteOtherIfAllowed(CBaseEntity *pOther);
	void	FlareGunProjectileTouch(CBaseEntity *pOther);
	void	FlareGunProjectileBurnTouch(CBaseEntity *pOther);

};

class CFlaregunCustom : public CFlaregun
{
public:
	DECLARE_CLASS(CFlaregunCustom, CFlaregun);
	virtual bool			Reload(void);

};

/********************************************************************
 NOTE: if you are looking at this file becase you would like flares 
 to be considered as fires (and thereby trigger gas traps), be aware 
 that the env_flare class is actually found in weapon_flaregun.cpp 
 and is really a repurposed piece of ammunition. (env_flare isn't the 
 rod-like safety flare prop, but rather the bit of flame on the end.)

 You will have some difficulty making it work here, because CFlare 
 does not inherit from CFire and will thus not be enumerated by 
 CFireSphere::EnumElement(). In order to have flares be detected and 
 used by this system, you will need to promote certain member functions 
 of CFire into an interface class from which both CFire and CFlare 
 inherit. You will also need to modify CFireSphere::EnumElement so that
 it properly disambiguates between fires and flares.

 For some partial work towards this end, see changelist 192474.

 ********************************************************************/


#define	FLARE_LAUNCH_SPEED	1500

IMPLEMENT_SERVERCLASS_ST(CFlaregun, DT_Flaregun)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_flaregun, CFlaregunCustom);
PRECACHE_WEAPON_REGISTER( weapon_flaregun );

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CFlaregun::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( "Flare.Touch" );

	PrecacheScriptSound( "Weapon_FlareGun.Burn" );

	UTIL_PrecacheOther( "env_flare" );
}

//-----------------------------------------------------------------------------
// Purpose: Main attack
//-----------------------------------------------------------------------------
void CFlaregun::PrimaryAttack( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	if ( m_iClip1 <= 0 )
	{
		SendWeaponAnim( ACT_VM_DRYFIRE );
		pOwner->m_flNextAttack = gpGlobals->curtime + SequenceDuration();
		return;
	}

	m_iClip1 = m_iClip1 - 1;

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pOwner->m_flNextAttack = gpGlobals->curtime + 1;

	CFlare *pFlare = CFlareGunProjectile::Create( pOwner->Weapon_ShootPosition(), pOwner->EyeAngles(), pOwner, FLARE_DURATION );
	

	if ( pFlare == NULL )
		return;

	Vector forward;
	pOwner->EyeVectors( &forward );

	pFlare->SetAbsVelocity( forward * 1500 );
	pFlare->SetGravity(1.0f);
	pFlare->SetFriction(0.85f);
	pFlare->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);

	WeaponSound( SINGLE );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFlaregun::SecondaryAttack( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	if ( m_iClip1 <= 0 )
	{
		SendWeaponAnim( ACT_VM_DRYFIRE );
		pOwner->m_flNextAttack = gpGlobals->curtime + SequenceDuration();
		return;
	}

	m_iClip1 = m_iClip1 - 1;

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pOwner->m_flNextAttack = gpGlobals->curtime + 1;

	CFlare *pFlare = CFlareGunProjectile::Create( pOwner->Weapon_ShootPosition(), pOwner->EyeAngles(), pOwner, FLARE_DURATION );

	if ( pFlare == NULL )
		return;

	Vector forward;
	pOwner->EyeVectors( &forward );

	pFlare->SetAbsVelocity( forward * 500 );
	pFlare->SetGravity(1.0f);
	pFlare->SetFriction( 0.85f );
	pFlare->SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );

	WeaponSound( SINGLE );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CFlaregunCustom::Reload(void)
{
	bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		WeaponSound(RELOAD);
	}
	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: Create function for Flare Gun projectile
// Input  : vecOrigin - 
//			vecAngles - 
//			*pOwner - 
// Output : CFlare
//-----------------------------------------------------------------------------
CFlareGunProjectile *CFlareGunProjectile::Create(Vector vecOrigin, QAngle vecAngles, CBaseEntity *pOwner, float lifetime)
{
	CFlareGunProjectile *pFlare = (CFlareGunProjectile *)CreateEntityByName("env_flare");

	if (pFlare == NULL)
		return NULL;

	UTIL_SetOrigin(pFlare, vecOrigin);

	pFlare->SetLocalAngles(vecAngles);
	pFlare->Spawn();
	pFlare->SetTouch(&CFlareGunProjectile::FlareGunProjectileTouch);
	pFlare->SetThink(&CFlare::FlareThink);

	//Start up the flare
	pFlare->Start(lifetime);

	//Don't start sparking immediately
	pFlare->SetNextThink(gpGlobals->curtime + 0.5f);

	//Burn out time
	pFlare->m_flTimeBurnOut = gpGlobals->curtime + lifetime;

	// Time to next burn damage
	pFlare->m_flNextDamage = gpGlobals->curtime;


	pFlare->RemoveSolidFlags(FSOLID_NOT_SOLID);
	pFlare->AddSolidFlags(FSOLID_NOT_STANDABLE);

	pFlare->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);

	pFlare->SetOwnerEntity(pOwner);
	pFlare->m_pOwner = pOwner;

	return pFlare;
}

//-----------------------------------------------------------------------------
// Purpose: Touch function for flaregun projectiles
// Input  : *pOther - The entity that the flare has collided with
//-----------------------------------------------------------------------------
void CFlareGunProjectile::FlareGunProjectileTouch(CBaseEntity *pOther)
{
	Assert(pOther);
	if (!pOther->IsSolid())
		return;

	if ((m_nBounces < 10) && (GetWaterLevel() < 1))
	{
		// Throw some real chunks here
		g_pEffects->Sparks(GetAbsOrigin());
	}

	//If the flare hit a person or NPC, do damage here.
	if (pOther && pOther->m_takedamage)
	{
			Vector vecNewVelocity = GetAbsVelocity();
			vecNewVelocity *= 0.1f;
			SetAbsVelocity(vecNewVelocity);
			SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
			SetGravity(1.0f);
			Die(0.5);
			IgniteOtherIfAllowed(pOther);
			m_nBounces++;
			return;
	}
	else
	{
		// hit the world, check the material type here, see if the flare should stick.
		trace_t tr;
		tr = CBaseEntity::GetTouchTrace();

		//Only do this on the first bounce
		if (m_nBounces == 0)
		{
			const surfacedata_t *pdata = physprops->GetSurfaceData(tr.surface.surfaceProps);

			if (pdata != NULL)
			{
				//Only embed into concrete and wood (jdw: too obscure for players?)
				//if ( ( pdata->gameMaterial == 'C' ) || ( pdata->gameMaterial == 'W' ) )
				{
					Vector	impactDir = (tr.endpos - tr.startpos);
					VectorNormalize(impactDir);

					float	surfDot = tr.plane.normal.Dot(impactDir);

					//Do not stick to ceilings or on shallow impacts
					if ((tr.plane.normal.z > -0.5f) && (surfDot < -0.9f))
					{
						RemoveSolidFlags(FSOLID_NOT_SOLID);
						AddSolidFlags(FSOLID_TRIGGER);
						UTIL_SetOrigin(this, tr.endpos + (tr.plane.normal * 2.0f));
						SetAbsVelocity(vec3_origin);
						SetMoveType(MOVETYPE_NONE);

						SetTouch(&CFlareGunProjectile::FlareGunProjectileBurnTouch);

						int index = decalsystem->GetDecalIndexForName("SmallScorch");
						if (index >= 0)
						{
							CBroadcastRecipientFilter filter;
							te->Decal(filter, 0.0, &tr.endpos, &tr.startpos, ENTINDEX(tr.m_pEnt), tr.hitbox, index);
						}

						CPASAttenuationFilter filter2(this, "Flare.Touch");
						EmitSound(filter2, entindex(), "Flare.Touch");

						return;
					}
				}
			}
		}

		//Scorch decal
		if (GetAbsVelocity().LengthSqr() > (250 * 250))
		{
			int index = decalsystem->GetDecalIndexForName("FadingScorch");
			if (index >= 0)
			{
				CBroadcastRecipientFilter filter;
				te->Decal(filter, 0.0, &tr.endpos, &tr.startpos, ENTINDEX(tr.m_pEnt), tr.hitbox, index);
			}
		}

		// Change our flight characteristics
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
		SetGravity(UTIL_ScaleForGravity(640));

		m_nBounces++;

		//After the first bounce, smacking into whoever fired the flare is fair game
		SetOwnerEntity(this);

		// Slow down
		Vector vecNewVelocity = GetAbsVelocity();
		vecNewVelocity.x *= 0.8f;
		vecNewVelocity.y *= 0.8f;
		SetAbsVelocity(vecNewVelocity);

		//Stopped?
		if (GetAbsVelocity().Length() < 64.0f)
		{
			SetAbsVelocity(vec3_origin);
			SetMoveType(MOVETYPE_NONE);
			RemoveSolidFlags(FSOLID_NOT_SOLID);
			AddSolidFlags(FSOLID_TRIGGER);
			SetTouch(&CFlareGunProjectile::FlareGunProjectileBurnTouch);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CFlareGunProjectile::FlareGunProjectileBurnTouch(CBaseEntity *pOther)
{
	if (pOther && pOther->m_takedamage && (m_flNextDamage < gpGlobals->curtime))
	{
		IgniteOtherIfAllowed(pOther);
		pOther->TakeDamage(CTakeDamageInfo(this, m_pOwner, 1, (DMG_BULLET | DMG_BURN)));
		m_flNextDamage = gpGlobals->curtime + 1.0f;
	}
}

void CFlareGunProjectile::IgniteOtherIfAllowed(CBaseEntity * pOther)
{
	// Don't burn the player
	if (pOther->IsPlayer())
		return;

	// Don't burn friendly NPCs
	CAI_BaseNPC *pNPC;
	pNPC = dynamic_cast<CAI_BaseNPC*>(pOther);
	if (pNPC && pNPC->IsPlayerAlly())
		return;

	// If this is an ignitable entity that isn't the player or an ally, ignite it!
	CBaseAnimating *pAnim;
	pAnim = dynamic_cast<CBaseAnimating*>(pOther);
	if (pAnim)
		pAnim->IgniteLifetime(30.0f);
}