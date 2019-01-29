//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose:		Flare gun (fffsssssssssss!!)
//
//				This is a custom extension of Valve's CFlaregun class.
//				Some commented-out code has been duplicated from 
//				weapon_flaregun.cpp in order to keep the mod code isolated
//				from the base game.
//
//	Author:		1upD
//
//=============================================================================//

#include "cbase.h"
#include "gamerules.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "decals.h"
#include "soundenvelope.h"
#include "IEffects.h"
#include "engine/IEngineSound.h"

#ifndef CLIENT_DLL
#include "env_flare.h"
#include "props.h"
#include "ai_basenpc.h"
#include "player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CFlaregun C_Flaregun
#endif // !CLIENT_DLL


// Custom convars for flaregun
ConVar	flaregun_primary_velocity("sv_flaregun_primary_velocity", "1500", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	flaregun_secondary_velocity("sv_flaregun_secondary_velocity", "500", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	flaregun_duration_seconds("sv_flaregun_lifetime_seconds", "10", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	flaregun_stop_velocity("sv_flaregun_stop_velocity", "128", FCVAR_REPLICATED | FCVAR_CHEAT );

//---------------------
// Flaregun
//---------------------
class CFlaregun : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(CFlaregun, CBaseHL2MPCombatWeapon);
	DECLARE_NETWORKCLASS();

	virtual bool Reload(void);
	void Precache(void);
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void AttackWithVelocity(float projectileVelocity);
};

IMPLEMENT_NETWORKCLASS_ALIASED( Flaregun, DT_Flaregun )

BEGIN_NETWORK_TABLE( CFlaregun, DT_Flaregun )
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( weapon_flaregun, CFlaregun );
PRECACHE_WEAPON_REGISTER( weapon_flaregun );

#ifndef CLIENT_DLL
// Custom derived class for flare gun projectiles
class CFlareGunProjectile : public CFlare
{
public:
	DECLARE_CLASS(CFlareGunProjectile, CFlare);
	static CFlareGunProjectile *Create(Vector vecOrigin, QAngle vecAngles, CBaseEntity *pOwner, float lifetime);
	void   IgniteOtherIfAllowed(CBaseEntity *pOther);
	void	FlareGunProjectileTouch(CBaseEntity *pOther);
	void	FlareGunProjectileBurnTouch(CBaseEntity *pOther);
};

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
		if (GetAbsVelocity().Length() < flaregun_stop_velocity.GetFloat())
		{
			RemoveSolidFlags(FSOLID_NOT_SOLID);
			AddSolidFlags(FSOLID_TRIGGER);
			SetAbsVelocity(vec3_origin);
			SetMoveType(MOVETYPE_NONE);
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
		// Don't do damage - I want consistent behavior between initial collisions and after landing collisions
		// pOther->TakeDamage(CTakeDamageInfo(this, m_pOwner, 1, (DMG_BULLET | DMG_BURN)));
		m_flNextDamage = gpGlobals->curtime + 1.0f;
		IgniteOtherIfAllowed(pOther);
	}
}

void CFlareGunProjectile::IgniteOtherIfAllowed(CBaseEntity * pOther)
{
	CAI_BaseNPC *pNPC;
	pNPC = dynamic_cast<CAI_BaseNPC*>(pOther);
	if (pNPC) {
		// Don't burn friendly NPCs
		if (pNPC->IsPlayerAlly())
			return;

		// Don't burn boss enemies
		if (FStrEq(STRING(pNPC->m_iClassname), "npc_combinegunship")
		 || FStrEq(STRING(pNPC->m_iClassname), "npc_combinedropship")
		 || FStrEq(STRING(pNPC->m_iClassname), "npc_strider")
		 || FStrEq(STRING(pNPC->m_iClassname), "npc_helicopter")
			)
			return;

		// Burn this NPC
		pNPC->IgniteLifetime(flaregun_duration_seconds.GetFloat());
	}

	// If this is a breakable prop, ignite it!
	CBreakableProp *pBreakable;
	pBreakable = dynamic_cast<CBreakableProp*>(pOther);
	if (pBreakable)
		pBreakable->IgniteLifetime(flaregun_duration_seconds.GetFloat());
}
#endif

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
	AttackWithVelocity(flaregun_primary_velocity.GetFloat());
}

//-----------------------------------------------------------------------------
// Purpose: Secondary attack - launches flares closer to the player
//-----------------------------------------------------------------------------
void CFlaregun::SecondaryAttack( void )
{
	AttackWithVelocity(flaregun_secondary_velocity.GetFloat());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CFlaregun::Reload(void)
{
	bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		WeaponSound(RELOAD);
	}
	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: Fires a flare from a given flaregun with a given velocity
//-----------------------------------------------------------------------------
void CFlaregun::AttackWithVelocity(float projectileVelocity)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	if (m_iClip1 <= 0)
	{
		SendWeaponAnim(ACT_VM_DRYFIRE);
		pOwner->m_flNextAttack = gpGlobals->curtime + SequenceDuration();
		return;
	}

	m_iClip1--;

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	pOwner->m_flNextAttack = gpGlobals->curtime + 1;

#ifndef CLIENT_DLL
	CFlare *pFlare = CFlareGunProjectile::Create(pOwner->Weapon_ShootPosition(), pOwner->EyeAngles(), pOwner, FLARE_DURATION);

	if (pFlare == NULL)
		return;

	Vector forward;
	pOwner->EyeVectors(&forward);
	forward *= projectileVelocity;
	forward += pOwner->GetAbsVelocity(); // Add the player's velocity to the forward vector so that the flare follows the player's motion
	forward.Normalized();

	pFlare->SetAbsVelocity(forward);
	pFlare->SetGravity(1.0f);
	pFlare->SetFriction(0.85f);
	pFlare->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
#endif

	WeaponSound(SINGLE);
}