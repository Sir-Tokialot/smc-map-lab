//=//=============================================================================//
//
// Purpose: A malevolent being from a parallel universe which at one point
//		may have been human.		
//
//		npc_shadow_walker is designed to be reusable as a generic horror 
//		game style npc. Its model and sound files may be configured through
//		the hammer editor using keyfields.
//
//	Author: 1upD
//
//=============================================================================//
#include "cbase.h"
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "entitylist.h"
#include "activitylist.h"
#include "ai_basenpc.h"
#include "ai_blended_movement.h"
#include "ai_behavior_actbusy.h"
#include "engine/IEngineSound.h"
#include "basehlcombatweapon_shared.h"
#include "ai_squadslot.h"
#include "weapon_crowbar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
// schedules
//=========================================================
enum
{
	SCHED_MELEE_ATTACK_NOINTERRUPT = LAST_SHARED_SCHEDULE,
	SCHED_HIDE,

	LAST_SHADOW_WALKER_SCHED
};

//=========================================================
//=========================================================
typedef CAI_BlendingHost< CAI_BehaviorHost<CAI_BaseNPC> > CAI_CustomNPCBase;

class CNPC_ShadowWalker : public CAI_CustomNPCBase
{
	DECLARE_CLASS( CNPC_ShadowWalker, CAI_CustomNPCBase);

public:
	void	Precache( void );
	void	Spawn( void );
	Class_T Classify( void );
	virtual int				SelectFailSchedule(int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode);
	virtual int 			SelectScheduleRetrieveItem();
	virtual int 			SelectScheduleWander();
	virtual int 			SelectSchedule();
	virtual int				SelectIdleSchedule();
	virtual int				SelectAlertSchedule();
	virtual int				SelectCombatSchedule();
	virtual float			GetSequenceGroundSpeed(CStudioHdr *pStudioHdr, int iSequence);
	virtual Activity				NPC_TranslateActivity(Activity eNewActivity);
	virtual int TranslateSchedule(int scheduleType);

	// Sounds
	virtual void		PlaySound(string_t soundname, bool optional);
	virtual void		DeathSound(const CTakeDamageInfo &info) { PlaySound(m_iszDeathSound, true); }
	virtual void		AlertSound(void) { PlaySound(m_iszAlertSound, false); };
	virtual void		IdleSound(void) { PlaySound(m_iszIdleSound, false); };
	virtual void		PainSound(const CTakeDamageInfo &info) { PlaySound(m_iszPainSound, true); };
	virtual void		FearSound(void) { PlaySound(m_iszFearSound, false); };
	virtual void		LostEnemySound(void) { PlaySound(m_iszLostEnemySound, false); };
	virtual void		FoundEnemySound(void);

	void			Activate();
	void			FixupWeapon();

	// Inputs
	virtual void InputSetSpeedModifier(inputdata_t &inputdata);
	virtual void InputEnableOpenDoors(inputdata_t &inputdata);
	virtual void InputDisableOpenDoors(inputdata_t &inputdata);
	virtual void InputEnablePickupWeapons(inputdata_t &inputdata);
	virtual void InputDisablePickupWeapons(inputdata_t &inputdata);

	DECLARE_DATADESC();

	string_t m_iszWeaponModelName;			// Path/filename of model to override weapon model.

	string_t m_iszFearSound;			// Path/filename of WAV file to play.
	string_t m_iszDeathSound;			// Path/filename of WAV file to play.
	string_t m_iszIdleSound;			// Path/filename of WAV file to play.
	string_t m_iszPainSound;			// Path/filename of WAV file to play.
	string_t m_iszAlertSound;			// Path/filename of WAV file to play.
	string_t m_iszLostEnemySound;		// Path/filename of WAV file to play.
	string_t m_iszFoundEnemySound;		// Path/filename of WAV file to play.

	DEFINE_CUSTOM_AI;

private:
	bool		HasRangedWeapon();
	void		PrecacheNPCSoundScript(string_t* SoundName, string_t defaultSoundName);


	bool		m_bUseBothSquadSlots;	// If true use two squad slots, if false use one squad slot
	bool		m_bCannotOpenDoors;		// If true, this NPC cannot open doors. The condition is reversed because originally it could.
	bool		m_bCanPickupWeapons;			// If true, this NPC is able to pick up weapons off of the ground just like npc_citizen.
	bool		m_bWanderToggle;		// Boolean to toggle wandering / standing every think cycle
	float		m_flNextSoundTime;		// Next time at which this NPC is allowed to play an NPC sound
	float		m_flNextFoundEnemySoundTime;	// Next time at which this NPC is allowed to play an NPC sound
	float		m_flSpeedModifier;		// Modifier to apply to move distance
};


LINK_ENTITY_TO_CLASS( npc_shadow_walker, CNPC_ShadowWalker );
//IMPLEMENT_CUSTOM_AI( npc_citizen,CNPC_ShadowWalker );


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CNPC_ShadowWalker)
	DEFINE_KEYFIELD(m_iszWeaponModelName, FIELD_STRING, "WeaponModel"),
	DEFINE_KEYFIELD(m_iHealth, FIELD_INTEGER, "Health"),
	DEFINE_KEYFIELD(m_iszFearSound, FIELD_SOUNDNAME, "FearSound"),
	DEFINE_KEYFIELD(m_iszDeathSound, FIELD_SOUNDNAME, "DeathSound"),
	DEFINE_KEYFIELD(m_iszIdleSound, FIELD_SOUNDNAME, "IdleSound"),
	DEFINE_KEYFIELD(m_iszPainSound, FIELD_SOUNDNAME, "PainSound"),
	DEFINE_KEYFIELD(m_iszAlertSound, FIELD_SOUNDNAME, "AlertSound"),
	DEFINE_KEYFIELD(m_iszLostEnemySound, FIELD_SOUNDNAME, "LostEnemySound"),
	DEFINE_KEYFIELD(m_iszFoundEnemySound, FIELD_SOUNDNAME, "FoundEnemySound"),
	DEFINE_KEYFIELD(m_bUseBothSquadSlots, FIELD_BOOLEAN, "UseBothSquadSlots"),
	DEFINE_KEYFIELD(m_bCannotOpenDoors, FIELD_BOOLEAN, "CannotOpenDoors"),
	DEFINE_KEYFIELD(m_bCanPickupWeapons, FIELD_BOOLEAN, "CanPickupWeapons"),

	DEFINE_FIELD(m_bWanderToggle, FIELD_BOOLEAN),
	DEFINE_FIELD(m_flNextSoundTime, FIELD_TIME),
	DEFINE_FIELD(m_flNextFoundEnemySoundTime, FIELD_TIME),
	DEFINE_FIELD(m_flSpeedModifier, FIELD_TIME),

	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetSpeedModifier", InputSetSpeedModifier),
	DEFINE_INPUTFUNC(FIELD_VOID, "EnableOpenDoors", InputEnableOpenDoors),
	DEFINE_INPUTFUNC(FIELD_VOID, "DisableOpenDoors", InputDisableOpenDoors),
	DEFINE_INPUTFUNC(FIELD_VOID, "EnablePickupWeapons", InputEnablePickupWeapons),
	DEFINE_INPUTFUNC(FIELD_VOID, "DisablePickupWeapons", InputDisablePickupWeapons)
END_DATADESC()


AI_BEGIN_CUSTOM_NPC(npc_shadow_walker, CNPC_ShadowWalker)
//=========================================================
// > Melee_Attack_NoInterrupt
//=========================================================
DEFINE_SCHEDULE
(
	SCHED_MELEE_ATTACK_NOINTERRUPT,

	"	Tasks"
	"		TASK_STOP_MOVING		0"
	"		TASK_FACE_ENEMY			0"
	"		TASK_ANNOUNCE_ATTACK	1"	// 1 = primary attack
	"		TASK_MELEE_ATTACK1		0"
	""
	"	Interrupts"
	"		COND_ENEMY_DEAD"
	"		COND_ENEMY_OCCLUDED"
);

//=========================================================
// 	SCHED_HIDE
//=========================================================
DEFINE_SCHEDULE
(
	SCHED_HIDE,

	"	Tasks"
	"		TASK_SET_FAIL_SCHEDULE		SCHEDULE:SCHED_COMBAT_FACE"
	"		TASK_STOP_MOVING			0"
	"		TASK_FIND_COVER_FROM_ENEMY	0"
	"		TASK_RUN_PATH				0"
	"		TASK_WAIT_FOR_MOVEMENT		0"
	"		TASK_REMEMBER				MEMORY:INCOVER"
	"		TASK_FACE_ENEMY				0"
	""
	"	Interrupts"
	"		COND_HEAR_DANGER"
	"		COND_NEW_ENEMY"
	"		COND_ENEMY_DEAD"
);
AI_END_CUSTOM_NPC()



//---------------------------------------------------------
// Constants
//---------------------------------------------------------
const float MIN_TIME_NEXT_SOUND = 0.5f;
const float MAX_TIME_NEXT_SOUND = 1.0f;
const float MIN_TIME_NEXT_FOUNDENEMY_SOUND = 2.0f;
const float MAX_TIME_NEXT_FOUNDENEMY_SOUND = 5.0f;

//-----------------------------------------------------------------------------
// Purpose: Inner class for default weapon
// TODO: Merge this with the Matt weapon in npc_citizen
//-----------------------------------------------------------------------------
class CWeaponCustomMelee : public CWeaponCrowbar
{
	DECLARE_CLASS(CWeaponCustomMelee, CWeaponCrowbar);

	const char *GetWorldModel() const { return GetModelName().ToCStr(); }
	void SetPickupTouch(void) {	/* do nothing */ }
};

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::Precache( void )
{
	// If no model name is supplied, use the default Shadow Walker model
	if (!GetModelName())
	{
		SetModelName(MAKE_STRING("models/monster/subject.mdl"));
	}

	if (&m_iszWeaponModelName && m_iszWeaponModelName != MAKE_STRING("")) {
		PrecacheModel(STRING(m_iszWeaponModelName));
	}
	else {
		PrecacheModel("models/props_canal/mattpipe.mdl");
	}

	PrecacheModel(STRING(GetModelName()));
	PrecacheNPCSoundScript(&m_iszFearSound, MAKE_STRING("NPC_ShadowWalker.Fear"));
	PrecacheNPCSoundScript(&m_iszIdleSound, MAKE_STRING("NPC_ShadowWalker.Idle"));
	PrecacheNPCSoundScript(&m_iszAlertSound, MAKE_STRING("NPC_ShadowWalker.Alert"));
	PrecacheNPCSoundScript(&m_iszPainSound, MAKE_STRING("NPC_ShadowWalker.Pain"));
	PrecacheNPCSoundScript(&m_iszLostEnemySound, MAKE_STRING("NPC_ShadowWalker.LostEnemy"));
	PrecacheNPCSoundScript(&m_iszFoundEnemySound, MAKE_STRING("NPC_ShadowWalker.FoundEnemy"));
	PrecacheNPCSoundScript(&m_iszDeathSound, MAKE_STRING("NPC_ShadowWalker.Death"));

	m_bWanderToggle = false;

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::Spawn( void )
{
	Precache();

	SetModel(STRING(GetModelName()));
	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	SetBloodColor( BLOOD_COLOR_RED );
	
	// If the health has not been set through Hammer, use a default health value of 75
	if (m_iHealth < 1) 
	{
		m_iHealth = 75;
	}

	m_flFieldOfView		= 0.5;
	m_flNextSoundTime = gpGlobals->curtime;
	m_flNextFoundEnemySoundTime = gpGlobals->curtime;
	m_NPCState			= NPC_STATE_NONE;
	m_flSpeedModifier = 1.0f;

	CapabilitiesClear();

	if (!HasSpawnFlags(SF_NPC_START_EFFICIENT))
	{
		CapabilitiesAdd(bits_CAP_TURN_HEAD);
		CapabilitiesAdd(bits_CAP_SQUAD);
		CapabilitiesAdd(bits_CAP_USE_WEAPONS | bits_CAP_AIM_GUN | bits_CAP_MOVE_SHOOT);
		CapabilitiesAdd(bits_CAP_WEAPON_MELEE_ATTACK1 || bits_CAP_WEAPON_MELEE_ATTACK2);
		CapabilitiesAdd(bits_CAP_INNATE_MELEE_ATTACK1 || bits_CAP_INNATE_MELEE_ATTACK2);
		CapabilitiesAdd(bits_CAP_DUCK);
		CapabilitiesAdd(bits_CAP_USE_SHOT_REGULATOR);

		if (!m_bCannotOpenDoors) {
			CapabilitiesAdd(bits_CAP_DOORS_GROUP);
		}
	}

	CapabilitiesAdd(bits_CAP_MOVE_GROUND);
	SetMoveType(MOVETYPE_STEP);

	NPCInit();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::FixupWeapon()
{
	// If no weapons supplied, give a crowbar
	CBaseCombatWeapon *pWeapon = GetActiveWeapon();
	if (pWeapon == NULL) {
		pWeapon = (CBaseCombatWeapon *)CREATE_UNSAVED_ENTITY(CWeaponCustomMelee, "weapon_crowbar");

		// Apply weapon model override
		if (&m_iszWeaponModelName && m_iszWeaponModelName != MAKE_STRING("")) {
			pWeapon->SetModel(STRING(m_iszWeaponModelName));
		}
		else {
			pWeapon->SetModel("models/props_canal/mattpipe.mdl");
		}

		DispatchSpawn(pWeapon);
		Weapon_Equip(pWeapon);
	}


}

void CNPC_ShadowWalker::Activate()
{
	BaseClass::Activate();
	FixupWeapon();
}


//-----------------------------------------------------------------------------
// Purpose: Choose a schedule after schedule failed
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectFailSchedule(int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode)
{
	switch (failedSchedule)
	{
	case SCHED_NEW_WEAPON:
		// If failed trying to pick up a weapon, try again in one second. This is because other AI code
		// has put this off for 10 seconds under the assumption that the citizen would be able to 
		// pick up the weapon that they found. 
		m_flNextWeaponSearchTime = gpGlobals->curtime + 1.0f;
		break;
	case SCHED_TAKE_COVER_FROM_ENEMY:
		// I can't take cover, so I need to run away!
		return SCHED_RUN_FROM_ENEMY;
	case SCHED_CHASE_ENEMY:
		// I can't run towards the enemy, so I will just run randomly!
		return SCHED_CHASE_ENEMY_FAILED;
	case SCHED_RUN_FROM_ENEMY:
		// I can't run away, so I will just run randomly!
		return SCHED_RUN_RANDOM;
	}

	return BaseClass::SelectFailSchedule(failedSchedule, failedTask, taskFailCode);
}

//-----------------------------------------------------------------------------
// Purpose: Select a schedule to retrieve better weapons if they are available.
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectScheduleRetrieveItem()
{
	if (m_bCanPickupWeapons && HasCondition(COND_BETTER_WEAPON_AVAILABLE))
	{
		CBaseHLCombatWeapon *pWeapon = dynamic_cast<CBaseHLCombatWeapon *>(Weapon_FindUsable(WEAPON_SEARCH_DELTA));
		if (pWeapon)
		{
			m_flNextWeaponSearchTime = gpGlobals->curtime + 10.0;
			// Now lock the weapon for several seconds while we go to pick it up.
			pWeapon->Lock(10.0, this);
			SetTarget(pWeapon);
			return SCHED_NEW_WEAPON;
		}
	}

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: Select a schedule to retrieve better weapons if they are available.
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectScheduleWander()
{
	m_bWanderToggle = !m_bWanderToggle;
	if (m_bWanderToggle) {
		return SCHED_IDLE_WANDER;
	}
	else {
		return SCHED_NONE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Select a schedule to execute based on conditions. 
// This is the most critical AI method.
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectSchedule()
{
	switch (m_NPCState)
	{
	case NPC_STATE_IDLE:
		AssertMsgOnce(GetEnemy() == NULL, "NPC has enemy but is not in combat state?");
		return SelectIdleSchedule();

	case NPC_STATE_ALERT:
		AssertMsgOnce(GetEnemy() == NULL, "NPC has enemy but is not in combat state?");
		return SelectAlertSchedule();

	case NPC_STATE_COMBAT:
		return SelectCombatSchedule();

	default:
		return BaseClass::SelectSchedule();
	}
}

//-----------------------------------------------------------------------------
// Idle schedule selection
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectIdleSchedule()
{
	int nSched = SelectFlinchSchedule();
	if (nSched != SCHED_NONE)
		return nSched;

	if (HasCondition(COND_HEAR_DANGER) ||
		HasCondition(COND_HEAR_COMBAT) ||
		HasCondition(COND_HEAR_WORLD) ||
		HasCondition(COND_HEAR_BULLET_IMPACT) ||
		HasCondition(COND_HEAR_PLAYER))
	{
		// Investigate sound source
		return SCHED_INVESTIGATE_SOUND;
	}

	nSched = SelectScheduleRetrieveItem();
	if (nSched != SCHED_NONE)
		return nSched;

	// no valid route! Wander instead
	if (GetNavigator()->GetGoalType() == GOALTYPE_NONE) {
		nSched = SelectScheduleWander();
		if (nSched != SCHED_NONE)
			return nSched;
		return SCHED_IDLE_STAND;
	}

	// valid route. Get moving
	return SCHED_IDLE_WALK;
}

//-----------------------------------------------------------------------------
// Alert schedule selection
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectAlertSchedule()
{
	// Per default base NPC, check flinch schedule first
	int nSched = SelectFlinchSchedule();
	if (nSched != SCHED_NONE)
		return nSched;

	// Scan around for new enemies
	if (HasCondition(COND_ENEMY_DEAD) && SelectWeightedSequence(ACT_VICTORY_DANCE) != ACTIVITY_NOT_AVAILABLE)
		return SCHED_ALERT_SCAN;

	if (HasCondition(COND_HEAR_DANGER) ||
		HasCondition(COND_HEAR_PLAYER) ||
		HasCondition(COND_HEAR_WORLD) ||
		HasCondition(COND_HEAR_BULLET_IMPACT) ||
		HasCondition(COND_HEAR_COMBAT))
	{
		// Investigate sound source
		AlertSound();
		return SCHED_INVESTIGATE_SOUND;
	}

	nSched = SelectScheduleRetrieveItem();
	if (nSched != SCHED_NONE)
		return nSched;

	// no valid route! Wander instead
	if (GetNavigator()->GetGoalType() == GOALTYPE_NONE) {
		nSched = SelectScheduleWander();
		if (nSched != SCHED_NONE)
			return nSched;
		return SCHED_IDLE_STAND;
	}

	// valid route. Get moving
	return SCHED_ALERT_WALK;
}

//-----------------------------------------------------------------------------
// Combat schedule selection
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectCombatSchedule()
{
	// Check flinch first
	int nSched = SelectFlinchSchedule();
	if (nSched != SCHED_NONE)
		return nSched;

	// Check enemy death
	if (HasCondition(COND_ENEMY_DEAD))
	{
		// clear the current (dead) enemy and try to find another.
		SetEnemy(NULL);

		if (ChooseEnemy())
		{
			FoundEnemySound();

			ClearCondition(COND_ENEMY_DEAD);
			return SelectSchedule();
		}

		SetState(NPC_STATE_ALERT);
		return SelectSchedule();
	}

	// Can any enemies see me?
	bool bEnemyCanSeeMe = HasCondition(COND_SEE_ENEMY) && HasCondition(COND_ENEMY_FACING_ME) && HasCondition(COND_HAVE_ENEMY_LOS);


	// If I'm scared of this enemy and he's looking at me, run away
	if ((IRelationType(GetEnemy()) == D_FR) && bEnemyCanSeeMe)
	{
			FearSound();
			return SCHED_RUN_FROM_ENEMY;
	}

	// If in a squad, only one or two shadow walkers can chase the player. This is configurable through Hammer.
	bool bCanChase = true;
	if (bEnemyCanSeeMe && m_bUseBothSquadSlots) {
		bCanChase = OccupyStrategySlotRange(SQUAD_SLOT_ATTACK1, SQUAD_SLOT_ATTACK2);
	}
	else if (bEnemyCanSeeMe){
		bCanChase = OccupyStrategySlot(SQUAD_SLOT_ATTACK1);
	}

	// If I'm not allowed to chase this enemy of this enemy and he's looking at me, set up an ambush
	if (!bCanChase)
	{
		FearSound();
		return SCHED_HIDE;
		
	}

	// Reloading conditions are necessary just in case for some reason somebody gives the Shadow Walker a gun
	if (HasRangedWeapon() && (HasCondition(COND_LOW_PRIMARY_AMMO) || HasCondition(COND_NO_PRIMARY_AMMO)))
	{
		return SCHED_HIDE_AND_RELOAD;
	}

	// Can we see the enemy?
	if (!HasCondition(COND_SEE_ENEMY))
	{
		// Chase!
		return SCHED_CHASE_ENEMY;
	}

	if (HasCondition(COND_TOO_CLOSE_TO_ATTACK))
		return SCHED_BACK_AWAY_FROM_ENEMY;


	// we can see the enemy
	if (HasCondition(COND_CAN_MELEE_ATTACK1)) {
			return SCHED_MELEE_ATTACK1;
	}

	if (HasCondition(COND_CAN_MELEE_ATTACK2)) {
			return SCHED_MELEE_ATTACK2;
	}

	if (HasRangedWeapon() && GetShotRegulator()->IsInRestInterval())
	{
		if (HasCondition(COND_CAN_RANGE_ATTACK1))
			return SCHED_COMBAT_FACE;
	}

	if (HasRangedWeapon() && HasCondition(COND_CAN_RANGE_ATTACK1))
	{
		if (OccupyStrategySlotRange(SQUAD_SLOT_ATTACK1, SQUAD_SLOT_ATTACK2))
			return SCHED_RANGE_ATTACK1;
		return SCHED_RUN_FROM_ENEMY;
	}

	if (HasRangedWeapon() && HasCondition(COND_CAN_RANGE_ATTACK2))
		return SCHED_RANGE_ATTACK2;


	if (HasCondition(COND_NOT_FACING_ATTACK))
		return SCHED_COMBAT_FACE;

	if (!HasCondition(COND_CAN_RANGE_ATTACK1) && !HasCondition(COND_CAN_MELEE_ATTACK1))
	{
		// if we can see enemy but can't use either attack type, we must need to get closer to enemy
		if (HasRangedWeapon())
			return SCHED_MOVE_TO_WEAPON_RANGE;

		return SCHED_CHASE_ENEMY;
	}

	DevWarning(2, "No suitable combat schedule!\n");
	return SCHED_FAIL;
}

bool CNPC_ShadowWalker::HasRangedWeapon()
{
	CBaseCombatWeapon *pWeapon = GetActiveWeapon();

	if (pWeapon)
		return !(FClassnameIs(pWeapon, "weapon_crowbar") || FClassnameIs(pWeapon, "weapon_stunstick"));

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Override base class schedules
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
	case SCHED_MELEE_ATTACK1:
		return SCHED_MELEE_ATTACK_NOINTERRUPT;
	case SCHED_IDLE_WANDER: // We want idle wandering to be interruptible - patrol walk is a better schedule
		return SCHED_PATROL_WALK;
	}

	return BaseClass::TranslateSchedule(scheduleType);
}

//-----------------------------------------------------------------------------
// Purpose: Override base class activiites
//-----------------------------------------------------------------------------
Activity CNPC_ShadowWalker::NPC_TranslateActivity(Activity activity)
{
	switch (activity) {
	case ACT_RUN_AIM_SHOTGUN:
		return ACT_RUN_AIM_RIFLE;
	case ACT_WALK_AIM_SHOTGUN:
		return ACT_WALK_AIM_RIFLE;
	case ACT_IDLE_ANGRY_SHOTGUN:
		return ACT_IDLE_ANGRY_SMG1;
	case ACT_RANGE_ATTACK_SHOTGUN_LOW:
		return ACT_RANGE_ATTACK_SMG1_LOW;
	case ACT_IDLE_MELEE:
	case ACT_IDLE_ANGRY_MELEE:  // If the walker has a melee weapon but is in an idle state, don't raise the weapon
		if (m_NPCState == NPC_STATE_IDLE)
			return ACT_IDLE_SUITCASE;
	default:
		return BaseClass::NPC_TranslateActivity(activity);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play sound when an enemy is spotted. This sound has a separate
//	timer from other sounds to prevent looping if the NPC gets caught
//	in a 'found enemy' condition.
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::FoundEnemySound(void)
{
	if (gpGlobals->curtime > m_flNextFoundEnemySoundTime)
	{
		m_flNextFoundEnemySoundTime = gpGlobals->curtime + random->RandomFloat(MIN_TIME_NEXT_FOUNDENEMY_SOUND, MAX_TIME_NEXT_FOUNDENEMY_SOUND);
		PlaySound(m_iszFoundEnemySound, true);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Play NPC soundscript
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::PlaySound(string_t soundname, bool required /*= false */)
{
	// TODO: Check if silent
	if (required || gpGlobals->curtime > m_flNextSoundTime)
	{
		m_flNextSoundTime = gpGlobals->curtime + random->RandomFloat(MIN_TIME_NEXT_SOUND, MAX_TIME_NEXT_SOUND);
		//CPASAttenuationFilter filter2(this, STRING(soundname));
		EmitSound(STRING(soundname));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Assign a default soundscript if none is provided, then precache
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::PrecacheNPCSoundScript(string_t * SoundName, string_t defaultSoundName) 
{
	if (!SoundName) {
		*SoundName = defaultSoundName;
	}
	PrecacheScriptSound(STRING(*SoundName));
}

//-----------------------------------------------------------------------------
// Purpose: Get movement speed, multipled by modifier
//-----------------------------------------------------------------------------
float CNPC_ShadowWalker::GetSequenceGroundSpeed(CStudioHdr *pStudioHdr, int iSequence)
{
	float t = SequenceDuration(pStudioHdr, iSequence);

	if (t > 0)
	{
		return (GetSequenceMoveDist(pStudioHdr, iSequence) * m_flSpeedModifier / t);
	}
	else
	{
		return 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Hammer input to change the speed of the NPC
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::InputSetSpeedModifier(inputdata_t &inputdata)
{
	this->m_flSpeedModifier = inputdata.value.Float();
}

//-----------------------------------------------------------------------------
// Purpose: Hammer input to enable opening doors
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::InputEnableOpenDoors(inputdata_t &inputdata)
{
	m_bCannotOpenDoors = false;
	if (!HasSpawnFlags(SF_NPC_START_EFFICIENT))
	{
		CapabilitiesAdd(bits_CAP_DOORS_GROUP);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Hammer input to enable opening doors
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::InputDisableOpenDoors(inputdata_t &inputdata)
{
	m_bCannotOpenDoors = true;
	if (!HasSpawnFlags(SF_NPC_START_EFFICIENT))
	{
		CapabilitiesRemove(bits_CAP_DOORS_GROUP);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Hammer input to enable weapon pickup behavior
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::InputEnablePickupWeapons(inputdata_t &inputdata)
{
	m_bCanPickupWeapons = true;
}

//-----------------------------------------------------------------------------
// Purpose: Hammer input to enable weapon pickup behavior
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::InputDisablePickupWeapons(inputdata_t &inputdata)
{
	m_bCanPickupWeapons = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_ShadowWalker::Classify( void )
{
	return	CLASS_ZOMBIE;
}
