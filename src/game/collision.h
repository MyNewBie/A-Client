/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

#include <base/vmath.h>

class CCollision
{
	class CTile *m_pTiles;
	int m_Width;
	int m_Height;
	class CLayers *m_pLayers;

	bool IsTile(int x, int y, int Flag=COLFLAG_SOLID) const;
	int GetTile(int x, int y) const;

public:
	enum
	{
		COLFLAG_SOLID=1,
		COLFLAG_DEATH=2,
		COLFLAG_NOHOOK=4,
		COLFLAG_MAXGAME=7,
		COLFLAG_FREEZE=8,

		RACECHECK_TILES_MAIN=1,
		RACECHECK_TILES_STOP=2,
		RACECHECK_TELE=4,
		RACECHECK_SPEEDUP=8,
	};

	CCollision();
	void Init(class CLayers *pLayers);
	bool CheckPoint(float x, float y, int Flag=COLFLAG_SOLID) const { return IsTile(round_to_int(x), round_to_int(y), Flag); }
	bool CheckPoint(vec2 Pos, int Flag=COLFLAG_SOLID) const { return CheckPoint(Pos.x, Pos.y, Flag); }
	int GetCollisionAt(float x, float y) const { return GetTile(round_to_int(x), round_to_int(y)); }
	int GetWidth() const { return m_Width; };
	int GetHeight() const { return m_Height; };
	int IntersectLine(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision) const;
    int IntersectLineTeleHook(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision, int *pTeleNr);
	void MovePoint(vec2 *pInoutPos, vec2 *pInoutVel, float Elasticity, int *pBounces) const;
	void MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, vec2 Size, float Elasticity, bool *pDeath=0) const;
	bool TestBox(vec2 Pos, vec2 Size, int Flag=COLFLAG_SOLID) const;
    vec2 ClosestPointOnLine(vec2 s, vec2 e, vec2 v) { return closest_point_on_line(s, e, v); }
    
    int GetPureMapIndex(float x, float y);
	int GetPureMapIndex(vec2 Pos) { return GetPureMapIndex(Pos.x, Pos.y); }

    int IsSolid(int x, int y);
	bool IsThrough(int x, int y, int xoff, int yoff, vec2 pos0, vec2 pos1);
	bool IsHookBlocker(int x, int y, vec2 pos0, vec2 pos1);
    class CTile *m_pFront;
};

void ThroughOffset(vec2 Pos0, vec2 Pos1, int *Ox, int *Oy);

#endif
