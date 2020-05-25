/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_BOT_H
#define GAME_CLIENT_COMPONENTS_BOT_H

#include <game/client/component.h>
#include <base/vmath.h>

class CBot : public CComponent
{
public:
    CBot();
    //int ids [64];
    void OnRender();
    void OnReset();
    static void ConPermHook(IConsole::IResult *pResult, void *pUserData);
    vec2 GetGrenadeAngle(vec2 m_startPos, vec2 m_ToShoot);
    virtual void OnConsoleInit();
};

#endif  
