/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <generated/protocol.h>
#include <generated/client_data.h>
#include <game/client/teecomp.h>

#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include "nameplates.h"
#include "controls.h"

void CNamePlates::RenderNameplate(
	const CNetObj_Character *pPrevChar,
	const CNetObj_Character *pPlayerChar,
	const CNetObj_PlayerInfo *pPlayerInfo,
	int ClientID
	)
{
	float IntraTick = Client()->IntraGameTick();

	vec2 Position = mix(vec2(pPrevChar->m_X, pPrevChar->m_Y), vec2(pPlayerChar->m_X, pPlayerChar->m_Y), IntraTick);

	float FontSize = 18.0f + 20.0f * g_Config.m_ClNameplatesSize / 100.0f;
	// render name plate
	if(m_pClient->m_LocalClientID != ClientID)
	{
		float a = 1;
		if(g_Config.m_ClNameplatesAlways == 0)
			a = clamp(1-powf(distance(m_pClient->m_pControls->m_TargetPos, Position)/200.0f,16.0f), 0.0f, 1.0f);

		char aName[64];
		if(!g_Config.m_TcNameplateScore)
			str_format(aName, sizeof(aName), "%s", g_Config.m_ClShowsocial ? m_pClient->m_aClients[ClientID].m_aName: "");
		else
			str_format(aName, sizeof(aName), "%s (%d)", g_Config.m_ClShowsocial ? m_pClient->m_aClients[ClientID].m_aName: "",
				pPlayerInfo->m_Score);

		CTextCursor Cursor;
		float tw = TextRender()->TextWidth(0, FontSize, aName, -1, -1.0f) + RenderTools()->GetClientIdRectSize(FontSize);
		TextRender()->SetCursor(&Cursor, Position.x-tw/2.0f, Position.y-FontSize-38.0f, FontSize, TEXTFLAG_RENDER);

		TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.5f*a);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, a);
		if(g_Config.m_ClNameplatesTeamcolors && m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS)
		{
			if(CTeecompUtils::UseDefaultTeamColor(m_pClient->m_aClients[ClientID].m_Team, m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team, g_Config))
			{
				// non-teecomp
				if(m_pClient->m_aClients[ClientID].m_Team == TEAM_RED)
					TextRender()->TextColor(1.0f, 0.5f, 0.5f, a);
				else if(m_pClient->m_aClients[ClientID].m_Team == TEAM_BLUE)
					TextRender()->TextColor(0.7f, 0.7f, 1.0f, a);
			}
			else
			{
				// teecomp
				vec3 Col = CTeecompUtils::GetTeamColorSaturatedRGB(m_pClient->m_aClients[ClientID].m_Team, m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team, g_Config);
				TextRender()->TextColor(Col.r, Col.g, Col.b, a);
			}
		}

		const vec4 IdTextColor(0.1f, 0.1f, 0.1f, a);
		vec4 BgIdColor(1.0f, 1.0f, 1.0f, a * 0.5f);
		if(g_Config.m_ClNameplatesTeamcolors && m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS)
		{
			if(m_pClient->m_aClients[ClientID].m_Team == TEAM_RED)
				BgIdColor = vec4(1.0f, 0.5f, 0.5f, a * 0.5f);
			else if(m_pClient->m_aClients[ClientID].m_Team == TEAM_BLUE)
				BgIdColor = vec4(0.7f, 0.7f, 1.0f, a * 0.5f);
		}

		if(a > 0.001f)
		{
			RenderTools()->DrawClientID(TextRender(), &Cursor, ClientID, BgIdColor, IdTextColor);
			TextRender()->TextEx(&Cursor, aName, -1);
		}

		TextRender()->TextColor(1,1,1,1);
		TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.3f);
	}
}

void CNamePlates::OnRender()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		// only render active characters
		if(!m_pClient->m_Snap.m_aCharacters[i].m_Active)
			continue;

		const void *pInfo = Client()->SnapFindItem(IClient::SNAP_CURRENT, NETOBJTYPE_PLAYERINFO, i);

		if(pInfo)
		{
			RenderNameplate(
				&m_pClient->m_Snap.m_aCharacters[i].m_Prev,
				&m_pClient->m_Snap.m_aCharacters[i].m_Cur,
				(const CNetObj_PlayerInfo *)pInfo,
				i);
		}
	}
}
