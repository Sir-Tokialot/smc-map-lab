//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Maplab achievements.
//
//=============================================================================
#include "cbase.h"

#ifdef CLIENT_DLL
#include "achievementmgr.h"
#include "baseachievement.h"

// Create achievement manager object
CAchievementMgr AchievementMgr;

class CAchievementMaplabMenu : public CBaseAchievement
{
protected:

	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		m_bStoreProgressInSteam = true;
		SetGoal(1);
	}

	// Listen for this event (event must be defined in :/resource/modevents.res)
	virtual void ListenForEvents()
	{
		ListenForGameEvent("joined_smc");
	}

	void FireGameEvent_Internal(IGameEvent *event)
	{
		if (0 == Q_strcmp(event->GetName(), "joined_smc"))
		{
			IncrementCount();
		}
	}
};
DECLARE_ACHIEVEMENT(CAchievementMaplabMenu, ACHIEVEMENT_EVENT_JOIN_SMC, "JOIN_SMC", 5); // remember to join SMC :)

// remember!!!! components are to be the same as the map's name with a _c affixed at the end!!!!
//
class CAchievementMaplab01Completed : public CBaseAchievement
{
	virtual void Init()
	{
		static const char *szComponents[] =
		{
			"MAPLAB01_1_c", "MAPLAB01_2_c", "MAPLAB01_3_c", "MAPLAB01_4_c",
			"MAPLAB01_5_c", "MAPLAB01_6_c", "MAPLAB01_7_c", "MAPLAB01_8_c"
		};
		SetFlags(ACH_HAS_COMPONENTS | ACH_LISTEN_COMPONENT_EVENTS | ACH_SAVE_GLOBAL);
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE(szComponents);
		SetComponentPrefix("MAPLAB01");
		SetGoal(m_iNumComponents);
	}

	// don't show progress notifications for this achievement, it's distracting
	virtual bool ShouldShowProgressNotification() { return false; }
};
DECLARE_ACHIEVEMENT(CAchievementMaplab01Completed, ACHIEVEMENT_EVENT_MAPLAB01_ALL, "MAPLAB01_ALL", 5);

class CAchievementTesttube01Completed : public CBaseAchievement
{
	virtual void Init()
	{
		static const char *szComponents[] =
		{
			"TESTTUBE01_1_c", "TESTTUBE01_2_c", "TESTTUBE01_3_c", "TESTTUBE01_4_c",
			"TESTTUBE01_5_c", "TESTTUBE01_6_c", "TESTTUBE01_7_c", "TESTTUBE01_8_c",
			"TESTTUBE01_10_c", "TESTTUBE01_11_c", "TESTTUBE01_12_c", "TESTTUBE01_13_c",
			"TESTTUBE01_14_c", "TESTTUBE01_15_c"
		};
		SetFlags(ACH_HAS_COMPONENTS | ACH_LISTEN_COMPONENT_EVENTS | ACH_SAVE_GLOBAL);
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE(szComponents);
		SetComponentPrefix("TESTTUBE01");
		SetGoal(m_iNumComponents);
	}

	// don't show progress notifications for this achievement, it's distracting
	virtual bool ShouldShowProgressNotification() { return false; }
};
DECLARE_ACHIEVEMENT(CAchievementTesttube01Completed, ACHIEVEMENT_EVENT_TESTTUBE01_ALL, "TESTTUBE01_ALL", 5);

class CAchievementMaplab02Completed : public CBaseAchievement
{

	virtual void Init()
	{
		static const char *szComponents[] =
		{
			"MAPLAB02_1_c", "MAPLAB02_2_c", "MAPLAB02_3_c", "MAPLAB02_4_c",
			"MAPLAB02_5_c", "MAPLAB02_6_c", "MAPLAB02_7_c"

		};
		SetFlags(ACH_HAS_COMPONENTS | ACH_LISTEN_COMPONENT_EVENTS | ACH_SAVE_GLOBAL);
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE(szComponents);
		SetComponentPrefix("MAPLAB02");
		SetGoal(m_iNumComponents);
	}

	// don't show progress notifications for this achievement, it's distracting
	virtual bool ShouldShowProgressNotification() { return false; }
};
DECLARE_ACHIEVEMENT(CAchievementMaplab02Completed, ACHIEVEMENT_EVENT_MAPLAB02_ALL, "MAPLAB02_ALL", 5);

class CAchievementTesttube02Completed : public CBaseAchievement
{

	virtual void Init()
	{
		static const char *szComponents[] =
		{
			"TESTTUBE02_1_c", "TESTTUBE02_2_c", "TESTTUBE02_3_c", "TESTTUBE02_4_c",
			"TESTTUBE02_5_c", "TESTTUBE02_6_c", "TESTTUBE02_7_c", "TESTTUBE02_8_c"
		};
		SetFlags(ACH_HAS_COMPONENTS | ACH_LISTEN_COMPONENT_EVENTS | ACH_SAVE_GLOBAL);
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE(szComponents);
		SetComponentPrefix("TESTTUBE02");
		SetGoal(m_iNumComponents);
	}
	virtual bool ShouldShowProgressNotification() { return false; }
};
DECLARE_ACHIEVEMENT(CAchievementTesttube02Completed, ACHIEVEMENT_EVENT_TESTTUBE02_ALL, "TESTTUBE02_ALL", 5);

// map logic achievements
//
//
//
// halloween horror
DECLARE_MAP_EVENT_ACHIEVEMENT( MAPLAB01_1, "MAPLAB01_1", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_2, "MAPLAB01_2", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_3, "MAPLAB01_3", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_4, "MAPLAB01_4", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_5, "MAPLAB01_5", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_6, "MAPLAB01_6", 5); 
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_7, "MAPLAB01_7", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_8, "MAPLAB01_8", 5);
//
// one room
//
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_1, "TESTTUBE01_1", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_2, "TESTTUBE01_2", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_3, "TESTTUBE01_3", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_4, "TESTTUBE01_4", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_5, "TESTTUBE01_5", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_6, "TESTTUBE01_6", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_7, "TESTTUBE01_7", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_8, "TESTTUBE01_8", 5);
// map9 achievement not called by map logic
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_10, "TESTTUBE01_10", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_11, "TESTTUBE01_11", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_12, "TESTTUBE01_12", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_13, "TESTTUBE01_13", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_14, "TESTTUBE01_14", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_15, "TESTTUBE01_15", 5);
//
// episode one
//
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB02_1, "MAPLAB02_1", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB02_2, "MAPLAB02_2", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB02_3, "MAPLAB02_3", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB02_4, "MAPLAB02_4", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB02_5, "MAPLAB02_5", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB02_6, "MAPLAB02_6", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB02_7, "MAPLAB02_7", 5);
//
// companion piece
//
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_TESTTUBE02_1, "TESTTUBE02_1", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_TESTTUBE02_2, "TESTTUBE02_2", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_TESTTUBE02_3, "TESTTUBE02_3", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_TESTTUBE02_4, "TESTTUBE02_4", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_TESTTUBE02_5, "TESTTUBE02_5", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_TESTTUBE02_6, "TESTTUBE02_6", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_TESTTUBE02_7, "TESTTUBE02_7", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_TESTTUBE02_8, "TESTTUBE02_8", 5);
#endif MAPLAB