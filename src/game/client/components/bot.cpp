/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/vmath.h>

#include <iterator>
#include <algorithm>

#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/collision.h>
#include <game/client/gameclient.h>
#include <game/gamecore.h>
#include <game/client/animstate.h>
#include <game/client/ui.h>
#include <game/client/render.h>

#include "bot.h"
#include "console.h"
#include "controls.h"
#include "players.h"

void CBot::OnConsoleInit()
{
	Console()->Register("+permhook", "", CFGFLAG_CLIENT, ConPermHook, this, "PermHook");
}

void CBot::ConPermHook(IConsole::IResult *pResult, void *pUserData) {
    CBot *pSelf = (CBot *)pUserData;
    pSelf->GameClient()->m_pControls->m_InputData.m_Hook = true;
}

CBot::CBot()
{
    //BOT FİRE
    //BOT HOOK
    //cache
}

void CBot::OnReset(){
    g_Config.m_ClAimBot = false;
}

void CBot::OnRender()
{
    if(!g_Config.m_ClAimBot) return;
    
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        CNetObj_Character PrevChar = m_pClient->m_Snap.m_aCharacters[i].m_Prev;
        CNetObj_Character CurChar = m_pClient->m_Snap.m_aCharacters[i].m_Cur;

        CGameClient::CClientData cData = m_pClient->m_aClients[i];
                
        bool OtherTeam;

        int LocalTeam = m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team;

        if (LocalTeam == TEAM_SPECTATORS)
            OtherTeam = false;
        else
            OtherTeam = m_pClient->m_aClients[i].m_Team != LocalTeam;

        bool Spec = m_pClient->m_Snap.m_SpecInfo.m_Active;

        if(!m_pClient->m_Snap.m_aCharacters[i].m_Active || cData.m_Team == TEAM_SPECTATORS || m_pClient->m_LocalClientID == i || Spec/* || (str_comp(gamemod, "zCatch") == 0 && cData.m_Team == m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team)*/)
        {
            continue;
        }

        float IntraTick = Client()->IntraGameTick();

        vec2 Position = mix(vec2(PrevChar.m_X, PrevChar.m_Y), vec2(CurChar.m_X, CurChar.m_Y), IntraTick);
        vec2 Vel = mix(vec2(PrevChar.m_VelX/256.0f, PrevChar.m_VelY/256.0f), vec2(CurChar.m_VelX/256.0f, CurChar.m_VelY/256.0f), IntraTick);

        CNetObj_Character LocalPrevChar = m_pClient->m_Snap.m_aCharacters[m_pClient->m_LocalClientID].m_Prev;
        CNetObj_Character LocalCurChar = m_pClient->m_Snap.m_aCharacters[m_pClient->m_LocalClientID].m_Cur;

        vec2 m_Vel = mix(vec2(LocalPrevChar.m_VelX/256.0f, LocalPrevChar.m_VelY/256.0f), vec2(LocalCurChar.m_VelX/256.0f, LocalCurChar.m_VelY/256.0f), IntraTick);
        
        bool GotAirJump = CurChar.m_Jumped&2?0:1;
        bool Stationary = CurChar.m_VelX <= 1 && CurChar.m_VelX >= -1;
        bool InAir = !Collision()->CheckPoint(CurChar.m_X, CurChar.m_Y+16);
        bool WantOtherDir = (CurChar.m_Direction == -1 && Vel.x > 0) || (CurChar.m_Direction == 1 && Vel.x < 0);
        
        vec2 m_Pos = m_pClient->m_LocalCharacterPos;
        
        vec2 enemyPos = vec2(160 * (Position.x - m_Pos.x) / sqrt((Position.x - m_Pos.x)*(Position.x - m_Pos.x) + (Position.y - m_Pos.y)*(Position.y - m_Pos.y)), 160 * (Position.y - m_Pos.y) / sqrt((Position.x - m_Pos.x)*(Position.x - m_Pos.x) + (Position.y - m_Pos.y)*(Position.y - m_Pos.y)));
        
        /*if (m_pClient->m_pControls->m_InputData.m_Fire) {
            enemyPos = GetGrenadeAngle(m_Pos, enemyPos) + m_Pos;
        }*/

        vec2 Direction = direction(angle(enemyPos));

        vec2 OldPos = m_Pos + Direction * 28.0f * 1.5f;
        vec2 NewPos = OldPos;
        vec2 finishPos = m_Pos + Direction * (m_pClient->m_Tuning.m_HookLength-42.0f);

        bool doBreak = false;
        int Hit = 0;

        do {
            OldPos = NewPos;
            NewPos = OldPos + Direction * m_pClient->m_Tuning.m_HookFireSpeed;

            if (distance(m_Pos, NewPos) > m_pClient->m_Tuning.m_HookLength)
            {
                NewPos = m_Pos + normalize(NewPos-m_Pos) * m_pClient->m_Tuning.m_HookLength;
                doBreak = true;
            }

            int teleNr = 0;
            Hit = Collision()->IntersectLineTeleHook(OldPos, NewPos, &finishPos, 0x0, &teleNr);
                
            if(Hit)
                break;

            NewPos.x = round_to_int(NewPos.x);
            NewPos.y = round_to_int(NewPos.y);

            if (OldPos == NewPos)
                break;

            Direction.x = round_to_int(Direction.x*256.0f) / 256.0f;
            Direction.y = round_to_int(Direction.y*256.0f) / 256.0f;
        } while (!doBreak);

        if (Hit || OtherTeam) continue;

        //bool exists = std::find(std::begin(ids), std::end(ids), i) != std::end(ids);

        if(distance(m_Pos, Position) <= m_pClient->m_Tuning.m_HookLength*2/* && !exists*/)
        {
            if (length(m_Vel*50) >= g_Config.m_ClAimBotLimit/2) {
                m_pClient->m_pControls->m_MousePos = enemyPos;
                /*if (length(m_Vel*50) >= 100) {
                    m_pClient->m_pControls->m_InputData.m_Hook = true;
                }*/
            }
            //ids[i] = i;
        /*} else if (exists) {
            std::remove(std::begin(ids), std::end(ids), i);*/
        }
    }
}

vec2 CBot::GetGrenadeAngle(vec2 m_StartPos, vec2 m_ToShoot)
{
	if(m_ToShoot == vec2(0, 0))
	{
		return vec2(0, 0);
	}
	vec2 m_Direction;
    float Curvature = m_pClient->m_Tuning.m_GrenadeCurvature;
	m_Direction.x = (m_ToShoot.x - m_StartPos.x);
	m_Direction.y = (m_ToShoot.y - m_StartPos.y - 32*Curvature);
	return m_Direction;
}
