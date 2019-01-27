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

class CAchievementMaplab01Completed : public CBaseAchievement
{
	virtual void Init()
	{
		static const char *szComponents[] =
		{
			"MAPLAB01_1", "MAPLAB01_2", "MAPLAB01_3", "MAPLAB01_4",
			"MAPLAB01_5", "MAPLAB01_6", "MAPLAB01_7", "MAPLAB01_8"
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
			"TESTTUBE01_1", "TESTTUBE01_2", "TESTTUBE01_3", "TESTTUBE01_4",
			"TESTTUBE01_5", "TESTTUBE01_6", "TESTTUBE01_7", "TESTTUBE01_8",
			"TESTTUBE01_10", "TESTTUBE01_11", "TESTTUBE01_12", "TESTTUBE01_13",
			"TESTTUBE01_14", "TESTTUBE01_15"
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




// map logic achievements

// halloween horror
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_1, "MAPLAB01_1", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_2, "MAPLAB01_2", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_3, "MAPLAB01_3", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_4, "MAPLAB01_4", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_5, "MAPLAB01_5", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_6, "MAPLAB01_6", 5); 
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_7, "MAPLAB01_7", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_8, "MAPLAB01_8", 5);

// one room

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


#endif MAPLAB