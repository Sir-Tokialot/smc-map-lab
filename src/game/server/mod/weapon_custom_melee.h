//=//=============================================================================//
//
// Purpose: Melee weapon for custom NPCs. Its model can be overridden.
//
//	Author: 1upD
//
//=============================================================================//
#include "cbase.h"
#include "basehlcombatweapon_shared.h"
#include "weapon_crowbar.h"


class CWeaponCustomMelee : public CWeaponCrowbar
{
	DECLARE_CLASS(CWeaponCustomMelee, CWeaponCrowbar);

	const char *GetWorldModel() const { return m_iszWeaponModelName.ToCStr(); }
	void SetPickupTouch(void) {	/* do nothing */ }
public:
	string_t m_iszWeaponModelName;

	DECLARE_DATADESC();

};

BEGIN_DATADESC(CWeaponCustomMelee)
DEFINE_FIELD(m_iszWeaponModelName, FIELD_STRING)
END_DATADESC()
