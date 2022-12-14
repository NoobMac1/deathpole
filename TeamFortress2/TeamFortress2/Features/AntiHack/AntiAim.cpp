#include "AntiAim.h"
#include "../Vars.h"

int edgeToEdgeOn = 0;
float spinpoint = 0;
float spinpointf = 0;
float rollpoint = 0;

void CAntiAim::FixMovement(CUserCmd* pCmd, Vec3 vOldAngles, float fOldSideMove, float fOldForwardMove)
{
	Vec3 curAngs = pCmd->viewangles;

	float fDelta = pCmd->viewangles.y - vOldAngles.y;
	float f1, f2;

	if (vOldAngles.y < 0.0f)
		f1 = 360.0f + vOldAngles.y;

	else f1 = vOldAngles.y;

	if (pCmd->viewangles.y < 0.0f)
		f2 = 360.0f + pCmd->viewangles.y;

	else f2 = pCmd->viewangles.y;

	if (f2 < f1)
		fDelta = abs(f2 - f1);

	else fDelta = 360.0f - abs(f1 - f2);

	fDelta = 360.0f - fDelta;

	pCmd->forwardmove = cos(DEG2RAD(fDelta)) * fOldForwardMove + cos(DEG2RAD(fDelta + 90.0f)) * fOldSideMove;
	pCmd->sidemove = sin(DEG2RAD(fDelta)) * fOldForwardMove + sin(DEG2RAD(fDelta + 90.0f)) * fOldSideMove;
}

float edgeDistance(float edgeRayYaw)
{
	// Main ray tracing area
	CGameTrace trace;
	Ray_t ray;
	Vector forward;
	float sp, sy, cp, cy;
	sy = sinf(DEG2RAD(edgeRayYaw)); // yaw
	cy = cosf(DEG2RAD(edgeRayYaw));
	sp = sinf(DEG2RAD(0)); // pitch
	cp = cosf(DEG2RAD(0));
	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
	forward = forward * 300.0f + g_EntityCache.m_pLocal->GetEyePosition();
	ray.Init(g_EntityCache.m_pLocal->GetEyePosition(), forward);
	// trace::g_pFilterNoPlayer to only focus on the enviroment
	CTraceFilterWorldAndPropsOnly Filter = { };
	g_Interfaces.EngineTrace->TraceRay(ray, 0x4200400B, &Filter, &trace);
	// Pythagorean theorem to calculate distance
	float edgeDistance = (sqrt(pow(trace.vStartPos.x - trace.vEndPos.x, 2) + pow(trace.vStartPos.y - trace.vEndPos.y, 2)));
	return edgeDistance;
}

bool findEdge(float edgeOrigYaw)
{
	// distance two vectors and report their combined distances
	float edgeLeftDist = edgeDistance(edgeOrigYaw - 21);
	edgeLeftDist = edgeLeftDist + edgeDistance(edgeOrigYaw - 27);
	float edgeRightDist = edgeDistance(edgeOrigYaw + 21);
	edgeRightDist = edgeRightDist + edgeDistance(edgeOrigYaw + 27);

	// If the distance is too far, then set the distance to max so the angle
	// isnt used
	if (edgeLeftDist >= 260)
		edgeLeftDist = 999999999;
	if (edgeRightDist >= 260)
		edgeRightDist = 999999999;

	// If none of the vectors found a wall, then dont edge
	if (edgeLeftDist == edgeRightDist)
		return false;

	// Depending on the edge, choose a direction to face
	if (edgeRightDist < edgeLeftDist)
	{
		edgeToEdgeOn = 1;
		if ((Vars::AntiHack::AntiAim::Pitch.m_Var == 3) || (Vars::AntiHack::AntiAim::Pitch.m_Var == 1) || (g_GlobalInfo.m_vRealViewAngles.x <= 89.0f  && Vars::AntiHack::AntiAim::Pitch.m_Var == 6) || (g_GlobalInfo.m_vRealViewAngles.x <= 89.0f && Vars::AntiHack::AntiAim::Pitch.m_Var == 5))	// check for real up
			edgeToEdgeOn = 2;
		return true;
	}
	else
	{
		edgeToEdgeOn = 2;
		if ((Vars::AntiHack::AntiAim::Pitch.m_Var == 3) || (Vars::AntiHack::AntiAim::Pitch.m_Var == 1) || (g_GlobalInfo.m_vRealViewAngles.x <= 89.0f && Vars::AntiHack::AntiAim::Pitch.m_Var == 6) || (g_GlobalInfo.m_vRealViewAngles.x <= 89.0f && Vars::AntiHack::AntiAim::Pitch.m_Var == 5))	// check for real up
			edgeToEdgeOn = 1;
		return true;
	}
}

void CAntiAim::Run(CUserCmd* pCmd, bool* pSendPacket)
{
	g_GlobalInfo.m_bAAActive = false;
	g_GlobalInfo.m_vRealViewAngles = g_GlobalInfo.m_vViewAngles;
	g_GlobalInfo.m_vFakeViewAngles = g_GlobalInfo.m_vViewAngles;

	if (!Vars::AntiHack::AntiAim::Active.m_Var)
		return;

	if (const auto& pLocal = g_EntityCache.m_pLocal)
	{
		if (!pLocal->IsAlive()
			|| pLocal->IsTaunting()
			|| pLocal->IsInBumperKart()
			|| pLocal->IsAGhost())
			return;

		if (g_GlobalInfo.m_bAttacking)
			return;

		else
		{
			if (const auto& pWeapon = g_EntityCache.m_pLocalWeapon) {
				if (Utils::IsAttacking(pCmd, pWeapon))
					return;
			}
		}

		bool bPitchSet = true;
		bool bYawSet = true;

		Vec3 vOldAngles = pCmd->viewangles;
		float fOldSideMove = pCmd->sidemove;
		float fOldForwardMove = pCmd->forwardmove;

		Vec3 vAngles = pCmd->viewangles;

		switch (Vars::AntiHack::AntiAim::Pitch.m_Var) {
		case 1: { pCmd->viewangles.x = -89.0f; g_GlobalInfo.m_vRealViewAngles.x = -89.0f; break; }	// up
		case 2: { pCmd->viewangles.x = 89.0f; g_GlobalInfo.m_vRealViewAngles.x = 89.0f; break; }	// down
		case 3: { pCmd->viewangles.x = -271.0f; g_GlobalInfo.m_vRealViewAngles.x = 89.0f; break; }	// fake down
		case 4: { pCmd->viewangles.x = 271.0f; g_GlobalInfo.m_vRealViewAngles.x = -89.0f; break; }	// fake up
		case 5: { pCmd->viewangles.x = Vars::AntiHack::AntiAim::CustomPitch.m_Var; g_GlobalInfo.m_vRealViewAngles.x = Vars::AntiHack::AntiAim::CustomPitch.m_Var; break; }	// custom
		case 6: {
			rollpoint += Vars::AntiHack::AntiAim::RollStep.m_Var;
			while (rollpoint > 89.0f) { rollpoint += -89.0f; }
			while (rollpoint < -89.0f) { rollpoint += 89.0f; } // wrap around
			pCmd->viewangles.x = rollpoint;
			g_GlobalInfo.m_vRealViewAngles.x = rollpoint;
			break;
		}	// roll (fucking useless I think)
		default: { bPitchSet = false; break; }
		}

		if (Vars::AntiHack::AntiAim::YawReal.m_Var == 4 || Vars::AntiHack::AntiAim::YawFake.m_Var == 4) { findEdge(pCmd->viewangles.y); }

		static bool b = false;
		// add edge detection https://github.com/nullworks/cathook/blob/06b3f0a116aded50a9cb7cd550b3e2c819b599c6/src/hacks/AntiAim.cpp#L316
		if (b)
		{
			switch (Vars::AntiHack::AntiAim::YawReal.m_Var) {
			case 1: { pCmd->viewangles.y += 90.0f;  break; }	// left
			case 2: { pCmd->viewangles.y -= 90.0f; break; }		// right
			case 3: { pCmd->viewangles.y += 180.0f; break; }	// backwards
			case 4: { if (edgeToEdgeOn == 1) { pCmd->viewangles.y += 90; } else if (edgeToEdgeOn == 2) { pCmd->viewangles.y -= 90.0f; }; break; }	// edge
			case 5: {
				spinpoint -= Vars::AntiHack::AntiAim::SpinStep.m_Var;
				while (spinpoint > 180.0f) { spinpoint += -360.0f; } while (spinpoint < -180.0f) { spinpoint += 360.0f; }
				pCmd->viewangles.y += spinpoint;
				break;
			}	// spin
			default: { bYawSet = false; break; }
			}

			g_GlobalInfo.m_vRealViewAngles.y = pCmd->viewangles.y;
		}

		else
		{
			switch (Vars::AntiHack::AntiAim::YawFake.m_Var) {
			case 1: { pCmd->viewangles.y += 90.0f; break; }		// left
			case 2: { pCmd->viewangles.y -= 90.0f; break; }		// right
			case 3: { pCmd->viewangles.y += 180.0f; break; }	// backwards
			case 4: { if (edgeToEdgeOn == 1) { pCmd->viewangles.y -= 90; } else if (edgeToEdgeOn == 2) { pCmd->viewangles.y += 90.0f; }; break; }	// edge
			case 5: { 
				spinpointf -= Vars::AntiHack::AntiAim::SpinStep.m_Var;
				while (spinpointf > 180.0f) { spinpointf += -360.0f; } 
				while (spinpointf < -180.0f) { spinpointf += 360.0f; } 
				pCmd->viewangles.y += spinpointf; 
				break; 
			}	// spin

			default: { bYawSet = false; break; }
			}

			g_GlobalInfo.m_vFakeViewAngles.y = pCmd->viewangles.y;
		}

		*pSendPacket = b = !b;
		g_GlobalInfo.m_bAAActive = bPitchSet || bYawSet;
		FixMovement(pCmd, vOldAngles, fOldSideMove, fOldForwardMove);
	}
}