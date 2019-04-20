//========= Copyright TOKI, All rights reserved. ==============================//
//
// Purpose: Maplabs achievements.
//
//=============================================================================//
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
DECLARE_ACHIEVEMENT(CAchievementMaplabMenu, ACHIEVEMENT_EVENT_JOIN_SMC, "JOIN_SMC", 5); // remember to join Map Labs Discord :)

// C stands for component. used to differentiate between achievements and their respective component.

class CAchievementMaplab01Completed : public CBaseAchievement
{
	virtual void Init()
	{
		static const char *szComponents[] =
		{
			"CMAPLAB01_1", "CMAPLAB01_2", "CMAPLAB01_3", "CMAPLAB01_4",
			"CMAPLAB01_5", "CMAPLAB01_6", "CMAPLAB01_7", "CMAPLAB01_8"
		};
		SetFlags(ACH_HAS_COMPONENTS | ACH_LISTEN_COMPONENT_EVENTS | ACH_SAVE_GLOBAL);
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE(szComponents);
		SetComponentPrefix("CMAPLAB01");
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
			"CTESTTUBE01_1", "CTESTTUBE01_2", "CTESTTUBE01_3", "CTESTTUBE01_4",
			"CTESTTUBE01_5", "CTESTTUBE01_6", "CTESTTUBE01_7", "CTESTTUBE01_8",
			"CTESTTUBE01_10", "CTESTTUBE01_11", "CTESTTUBE01_12", "CTESTTUBE01_13",
			"CTESTTUBE01_14", "CTESTTUBE01_15", "CTESTTUBE01_16"
		};
		SetFlags(ACH_HAS_COMPONENTS | ACH_LISTEN_COMPONENT_EVENTS | ACH_SAVE_GLOBAL);
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE(szComponents);
		SetComponentPrefix("CTESTTUBE01");
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
			"CMAPLAB02_1", "CMAPLAB02_2", "CMAPLAB02_3", "CMAPLAB02_4",
			"CMAPLAB02_5", "CMAPLAB02_6", "CMAPLAB02_7"

		};
		SetFlags(ACH_HAS_COMPONENTS | ACH_LISTEN_COMPONENT_EVENTS | ACH_SAVE_GLOBAL);
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE(szComponents);
		SetComponentPrefix("CMAPLAB02");
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
			"CTESTTUBE02_1", "CTESTTUBE02_2", "CTESTTUBE02_3", "CTESTTUBE02_4",
			"CTESTTUBE02_5", "CTESTTUBE02_6", "CTESTTUBE02_7", "CTESTTUBE02_8"
		};
		SetFlags(ACH_HAS_COMPONENTS | ACH_LISTEN_COMPONENT_EVENTS | ACH_SAVE_GLOBAL);
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE(szComponents);
		SetComponentPrefix("CTESTTUBE02");
		SetGoal(m_iNumComponents);
	}
	virtual bool ShouldShowProgressNotification() { return false; }
};
DECLARE_ACHIEVEMENT(CAchievementTesttube02Completed, ACHIEVEMENT_EVENT_TESTTUBE02_ALL, "TESTTUBE02_ALL", 5);


class CAchievementMaplab03Completed : public CBaseAchievement
{

	virtual void Init()
	{
		static const char *szComponents[] =
		{
			"CMAPLAB03_1", "CMAPLAB03_2", "CMAPLAB03_3", "CMAPLAB03_4",
			"CMAPLAB03_5", "CMAPLAB03_6", "CMAPLAB03_7", "CMAPLAB03_8",
			"CMAPLAB03_9", "CMAPLAB03_10", "CMAPLAB03_11", "CMAPLAB03_12",
			"CMAPLAB03_13", "CMAPLAB03_14"
		};
		SetFlags(ACH_HAS_COMPONENTS | ACH_LISTEN_COMPONENT_EVENTS | ACH_SAVE_GLOBAL);
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE(szComponents);
		SetComponentPrefix("CMAPLAB03");
		SetGoal(m_iNumComponents);
	}
	virtual bool ShouldShowProgressNotification() { return false; }
};
DECLARE_ACHIEVEMENT(CAchievementMaplab03Completed, ACHIEVEMENT_EVENT_MAPLAB03_ALL, "MAPLAB03_ALL", 5);

// map logic achievements
//
//
//
// halloween horror
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_MAPLAB01_1, "MAPLAB01_1", 5);
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
//DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_9, "TESTTUBE01_9", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_10, "TESTTUBE01_10", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_11, "TESTTUBE01_11", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_12, "TESTTUBE01_12", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_13, "TESTTUBE01_13", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_14, "TESTTUBE01_14", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_15, "TESTTUBE01_15", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_EVENT_TESTTUBE01_16, "TESTTUBE01_16", 5);
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
//
//
//
// runthinkshootliveville2
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB03_1, "MAPLAB03_1", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB03_2, "MAPLAB03_2", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB03_3, "MAPLAB03_3", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB03_4, "MAPLAB03_4", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB03_5, "MAPLAB03_5", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB03_6, "MAPLAB03_6", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB03_7, "MAPLAB03_7", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB03_8, "MAPLAB03_8", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB03_9, "MAPLAB03_9", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB03_10, "MAPLAB03_10", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB03_11, "MAPLAB03_11", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB03_12, "MAPLAB03_12", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB03_13, "MAPLAB03_13", 5);
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_MAPLAB03_14, "MAPLAB03_14", 5);

#endif MAPLAB