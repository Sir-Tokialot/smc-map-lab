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
DECLARE_ACHIEVEMENT(CAchievementMaplabMenu, ACHIEVEMENT_EVENT_JOIN_SMC, "JOIN_SMC", 5);

class CAchievementMaplab01Completed : public CBaseAchievement
{
protected:

	void Init()
	{
		SetFlags(ACH_LISTEN_MAP_EVENTS | ACH_SAVE_WITH_GAME);
		m_bStoreProgressInSteam = true;
		SetGoal(8);
	}

	// Don't show a notification on every map completion
	virtual bool ShouldShowProgressNotification() { return false; }
};
DECLARE_ACHIEVEMENT(CAchievementMaplab01Completed, ACHIEVEMENT_EVENT_MAPLAB01_ALL, "MAPLAB01_ALL", 5);


class CAchievementMaplabManHack : public CBaseAchievement
{
protected:

	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		m_bStoreProgressInSteam = true;
		SetGoal(50);
		SetMapNameFilter("herecomethehacks"); // SOMEONE didn't follow the rules and made a map that isn't completable so gotta make this achievement instead
	}

	// Listen for this event (event must be defined in :/resource/modevents.res)
	virtual void ListenForEvents()
	{
		ListenForGameEvent("manhack_killed"); 
	}

	void FireGameEvent_Internal(IGameEvent *event)
	{
		if (0 == Q_strcmp(event->GetName(), "manhack_killed"))
		{
			IncrementCount();
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementMaplabManHack, ACHIEVEMENT_EVENT_TESTTUBE01_9, "TESTTUBE01_9", 5);

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