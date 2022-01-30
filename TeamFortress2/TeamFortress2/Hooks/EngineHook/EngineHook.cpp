#include "EngineHook.h"
#include "../../Features/Vars.h"
#include "../../Features/Misc/Misc.h"

void __cdecl EngineHook::CL_Move::Hook(float accumulated_extra_samples, bool bFinalTick)
{
	auto pLocal = g_EntityCache.m_pLocal;

	static auto oClMove = Func.Original<fn>();
	if (!Vars::Misc::CL_Move::Enabled.m_Var) {
		return oClMove(accumulated_extra_samples, bFinalTick);
	}

	// drop em all
	if (Vars::Misc::CL_Move::TeleportKey.m_Var && (GetAsyncKeyState(Vars::Misc::CL_Move::TeleportKey.m_Var)) && g_GlobalInfo.m_nShifted >= Vars::Misc::CL_Move::DoubletapAmt.m_Var) {
		while (g_GlobalInfo.m_nShifted != 0) {
			g_GlobalInfo.m_nShifted--;
			oClMove(accumulated_extra_samples, (g_GlobalInfo.m_nShifted == 1));
		}

		return;
	}

	// my pSpeedhack
	if (Vars::Misc::CL_Move::SEnabled.m_Var)
	{
		int SpeedTicks{ 0 };
		int SpeedTicksDesired = Vars::Misc::CL_Move::SFactor.m_Var;
		g_GlobalInfo.m_nShifted = 0; // dont let it fuck up our indicators LOL

		while (SpeedTicks < SpeedTicksDesired)
		{
			SpeedTicks++;
			oClMove(accumulated_extra_samples, (SpeedTicks == (SpeedTicksDesired)));
		}
	}


	// recharge
	if (GetAsyncKeyState(Vars::Misc::CL_Move::RechargeKey.m_Var)) {
		g_GlobalInfo.m_bRecharging = true;
	}
	if (g_GlobalInfo.m_bRecharging && g_GlobalInfo.m_nShifted < Vars::Misc::CL_Move::DoubletapAmt.m_Var) {
		g_GlobalInfo.m_nShifted++;
		g_GlobalInfo.m_nWaitForShift = DT_WAIT_CALLS;
		return;
	}
	else {
		g_GlobalInfo.m_bRecharging = false;
	}

	// commenting this out makes u unable to join servers???? WHAT THE FUCK DOES IT DO
	oClMove(accumulated_extra_samples, (g_GlobalInfo.m_bShouldShift && !g_GlobalInfo.m_nWaitForShift) ? true : bFinalTick);

	if (g_GlobalInfo.m_nWaitForShift) {
		g_GlobalInfo.m_nWaitForShift--;
		return;
	}

	if (g_GlobalInfo.lateUserCmd != nullptr) {
		// Shift if attacking normally
		g_GlobalInfo.m_bShouldShift = g_GlobalInfo.m_bShouldShift ? true : g_GlobalInfo.lateUserCmd->buttons & IN_ATTACK;
	}

	// are we alive
	if (!pLocal) {
		return;
	}

	// doobletap
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
		g_GlobalInfo.m_bShouldShift = false;
	}
	
}

void __cdecl EngineHook::CL_SendMove::Hook(void* ecx, void* edx)
{
	byte data[4000];

	const int nextcommandnr = g_Interfaces.ClientState->lastoutgoingcommand + g_Interfaces.ClientState->chokedcommands + 1;

	CLC_Move moveMsg;
	moveMsg.m_DataOut.StartWriting(data, sizeof(data));
	moveMsg.m_nNewCommands = std::clamp(1 + g_Interfaces.ClientState->chokedcommands, 0, 15);
	const int extraCommands = g_Interfaces.ClientState->chokedcommands + 1 - moveMsg.m_nNewCommands;
	const int backupCommands = std::max(2, extraCommands);
	moveMsg.m_nBackupCommands = std::clamp(backupCommands, 0, 7);

	const int numcmds = moveMsg.m_nNewCommands + moveMsg.m_nBackupCommands;

	int from = -1;
	bool bOK = true;
	for (int to = nextcommandnr - numcmds + 1; to <= nextcommandnr; to++) {
		const bool isnewcmd = to >= nextcommandnr - moveMsg.m_nNewCommands + 1;
		bOK = bOK && g_Interfaces.Client->WriteUsercmdDeltaToBuffer(&moveMsg.m_DataOut, from, to, isnewcmd);
		from = to;
	}

	if (bOK) {
		if (extraCommands)
			g_Interfaces.ClientState->m_NetChannel->m_nChokedPackets -= extraCommands;
		GetVFunc<bool(__thiscall*)(PVOID, INetMessage* msg, bool, bool)>(g_Interfaces.ClientState->m_NetChannel, 37)(g_Interfaces.ClientState->m_NetChannel, &moveMsg, false, false);
	}
	//static auto originalFn = Func.Original<fn>();
	//return originalFn(ecx, edx);
}