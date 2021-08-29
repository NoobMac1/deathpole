#pragma once
#include "../../SDK/SDK.h"

class DTInd
{
public:
	void Run();
	int m_nIndX = 300, m_nIndY = 300;

private:
	bool ShouldRun();
	bool GetDrawPosition(int& x, int& y, CBaseEntity* pEntity);
	void DragInd();
	void DrawPoints(CBaseEntity* pLocal);
	void DrawInd();

	int m_nRadarSize, m_nRadarCorrSize;
	float m_flLocalCos, m_flLocalSin, m_flRange, m_flLocalYaw;
	Vec3 m_vLocalOrigin;
};

inline DTInd g_Ind;