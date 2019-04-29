// By Dot Operator, 2019
// Lets the mapper give the player wall-climb abilities,
// but keeps it a cheat so the player can't
// give it to themselves without foregoing achievements.
#include "cbase.h"

static ConVar has_wallclimb("maplab_wallclimb", 0, FCVAR_CHEAT, "Turns wall climbing on or off.");

class CPlayerWallclimb : public CLogicalEntity {
public:
	DECLARE_CLASS(CPlayerWallclimb, CLogicalEntity);
	DECLARE_DATADESC();

	CPlayerWallclimb() {
		has_wallclimb.SetValue(1);
	}

	// Inputs
	void InputToggle(inputdata_t &inputData);
	void InputTurnOn(inputdata_t &inputData);
	void InputTurnOff(inputdata_t &inputData);

	bool m_bTurnedOn = true;
private:
	inline void RefreshCVar() {
		has_wallclimb.SetValue(m_bTurnedOn ? 1 : 0);
	};
};

LINK_ENTITY_TO_CLASS(player_wallclimb, CPlayerWallclimb);

BEGIN_DATADESC(CPlayerWallclimb)

DEFINE_FIELD(m_bTurnedOn, FIELD_BOOLEAN),

DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),
DEFINE_INPUTFUNC(FIELD_VOID, "TurnOn", InputTurnOn),
DEFINE_INPUTFUNC(FIELD_VOID, "TurnOff", InputTurnOff),

END_DATADESC()

void CPlayerWallclimb::InputToggle(inputdata_t & inputData)
{
	m_bTurnedOn = !m_bTurnedOn;
	RefreshCVar();
}

void CPlayerWallclimb::InputTurnOn(inputdata_t & inputData)
{
	m_bTurnedOn = true;
	RefreshCVar();
}

void CPlayerWallclimb::InputTurnOff(inputdata_t & inputData)
{
	m_bTurnedOn = false;
	RefreshCVar();
}