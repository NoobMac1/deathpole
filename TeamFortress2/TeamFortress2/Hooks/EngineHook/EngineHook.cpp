#include "EngineHook.h"
#include "../../Features/Vars.h"
#include "../../Features/Misc/Misc.h"

void __cdecl EngineHook::CL_Move::Hook(float accumulated_extra_samples, bool bFinalTick)
{
	static auto oClMove = Func.Original<fn>();
	if (!Vars::Misc::CL_Move::Enabled.m_Var) {
		return oClMove(accumulated_extra_samples, bFinalTick);
	}

	auto pLocal = g_EntityCache.m_pLocal;

	//Teleport
	if (Vars::Misc::CL_Move::TeleportKey.m_Var && (GetAsyncKeyState(Vars::Misc::CL_Move::TeleportKey.m_Var)) && g_GlobalInfo.m_nShifted >= Vars::Misc::CL_Move::DoubletapAmt.m_Var) {
		while (g_GlobalInfo.m_nShifted != 0) {
			g_GlobalInfo.m_nShifted--;
			oClMove(accumulated_extra_samples, (g_GlobalInfo.m_nShifted == 1));
		}

		return;
	}

	//Speedhack
	if (Vars::Misc::CL_Move::SEnabled.m_Var)
	{
		int SpeedTicks{ 0 };
		int SpeedTicksDesired = Vars::Misc::CL_Move::SFactor.m_Var;

		while (SpeedTicks < SpeedTicksDesired)
		{
			SpeedTicks++;
			oClMove(accumulated_extra_samples, (SpeedTicks == (SpeedTicksDesired)));
		}
	}


	//Recharge
	if (GetAsyncKeyState(Vars::Misc::CL_Move::RechargeKey.m_Var)) {
		g_GlobalInfo.m_bRecharging = true;
	}
	if (g_GlobalInfo.m_bRecharging && g_GlobalInfo.m_nShifted < Vars::Misc::CL_Move::DoubletapAmt.m_Var) {
		g_GlobalInfo.m_nShifted++;
		g_GlobalInfo.m_nWaitForShift = DT_WAIT_CALLS;	
		return; // Don't move
	}
	else {
		g_GlobalInfo.m_bRecharging = false;
	}

	oClMove(accumulated_extra_samples, (g_GlobalInfo.m_bShouldShift && !g_GlobalInfo.m_nWaitForShift) ? true : bFinalTick);

	if (g_GlobalInfo.m_nWaitForShift) {
		g_GlobalInfo.m_nWaitForShift--;
		return;
	}

	if (g_GlobalInfo.lateUserCmd != nullptr) {
		// Shift if attacking normally
		g_GlobalInfo.m_bShouldShift = g_GlobalInfo.m_bShouldShift ? true : g_GlobalInfo.lateUserCmd->buttons & IN_ATTACK;
	}



	if (!pLocal) {
		return;
	}

	if (g_GlobalInfo.m_bShouldShift) {
		if (
			(Vars::Misc::CL_Move::DTMode.m_Var == 0 && GetAsyncKeyState(Vars::Misc::CL_Move::DoubletapKey.m_Var)) ||	// 0 - On key
			(Vars::Misc::CL_Move::DTMode.m_Var == 1) ||																	// 1 - Always
			(Vars::Misc::CL_Move::DTMode.m_Var == 2 && !GetAsyncKeyState(Vars::Misc::CL_Move::DoubletapKey.m_Var)))		// 2 - Disable on key 
		{
			while (g_GlobalInfo.m_nShifted) {
				oClMove(accumulated_extra_samples, g_GlobalInfo.m_nShifted == 1);
				g_GlobalInfo.m_nShifted--;
			}
		}
		g_Interfaces.Engine->FireEvents(); // immediately queue shifted ticks (probably not an issue but jic)
		g_GlobalInfo.m_bShouldShift = false;
	}
	
}