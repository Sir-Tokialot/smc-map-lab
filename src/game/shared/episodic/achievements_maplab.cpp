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
DECLARE_ACHIEVEMENT(CAchievementMaplabMenu, ACHIEVEMENT_JOIN_SMC, "JOIN_SMC", 5);

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
DECLARE_ACHIEVEMENT(CAchievementMaplab01Completed, ACHIEVEMENT_MAPLAB01_ALL, "MAPLAB01_ALL", 5);

// map logic achievements

DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_MAPLAB01_1, "MAPLAB01_1", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_MAPLAB01_2, "MAPLAB01_2", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_MAPLAB01_3, "MAPLAB01_3", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_MAPLAB01_4, "MAPLAB01_4", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_MAPLAB01_5, "MAPLAB01_5", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_MAPLAB01_6, "MAPLAB01_6", 5); 
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_MAPLAB01_7, "MAPLAB01_7", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_MAPLAB01_8, "MAPLAB01_8", 5);


#endif CLIENT_DLL