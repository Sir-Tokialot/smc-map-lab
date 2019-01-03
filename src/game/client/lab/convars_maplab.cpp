//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Maplab ConVars
//
//=============================================================================

#include "cbase.h"
#include "convar.h"
#include "steam/steam_api.h"

void OpenAchievements_f()
{
	if (steamapicontext && steamapicontext->SteamFriends())
		steamapicontext->SteamFriends()->ActivateGameOverlay("Achievements");
	else
		DevMsg("Missing Steam API context, failed to execute: maplab_openachievements\n");
}
ConCommand maplab_openachievements("maplab_openachievements", OpenAchievements_f, "Open Maplab Achievements in Steam Overlay.", 0);


void OpenInvite_f()
{
	if (steamapicontext && steamapicontext->SteamFriends())
		steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage("https://discord.me/sourcemoddingcommunity");
	IGameEvent *event = gameeventmanager->CreateEvent("joined_smc");
	if (event)
		gameeventmanager->FireEvent(event);
	else
		DevMsg("Missing Steam API context, failed to execute: maplab_openinvite\n");
}
ConCommand maplab_openinvite("maplab_openinvite", OpenInvite_f, "Open the SMC Discord invite link.", 0);