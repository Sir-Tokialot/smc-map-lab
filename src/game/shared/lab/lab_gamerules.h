#ifndef LAB_GAMERULES_H
#define LAB_GAMERULES_H
#ifdef _WIN32
#pragma once
#endif

#include "teamplay_gamerules.h"

#ifdef CLIENT_DLL
	#define CLabGameRules C_LabGameRules
	#define CLabGameRulesProxy C_LabGameRulesProxy
#endif

class CLabGameRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CLabGameRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();
};

class CLabGameRules : public CTeamplayRules
{
public:
	DECLARE_CLASS( CLabGameRules, CTeamplayRules );
	DECLARE_NETWORKCLASS_NOBASE();

	CLabGameRules();
	virtual ~CLabGameRules();

#ifdef CLIENT_DLL

#else
	virtual void Think();
#endif
private:
	CNetworkVar(bool, m_bMegaPhysgun);
	
};

inline CLabGameRules* LabGameRules()
{
	return static_cast<CLabGameRules*>(g_pGameRules);
}

#endif // LAB_GAMERULES_H
