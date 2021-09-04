#include "Misc.h"
#include "../Vars.h"

void CMisc::Run(CUserCmd* pCmd)
{
	AutoJump(pCmd);
	AutoStrafe(pCmd);
	NoiseMakerSpam();
	ChatSpam();
	NoPush();
	CheatBypass();
	SpeedHack();
	ConsoleStuff();
	Interp();
}

void CMisc::ConsoleStuff()
{
	ConVar* cpuThrottle = g_Interfaces.CVars->FindVar(_("engine_no_focus_sleep"));
	ConVar* localFootsteps = g_Interfaces.CVars->FindVar(_("mp_footsteps")); //Maybe disables local footsteps? reverse nullcore dll to see if it is a cvar, i'm too lazy
	ConVar* holidayForce = g_Interfaces.CVars->FindVar(_("tf_force_holidays_off")); //Maybe disables holiday events? if so, stops issues with chams
	ConVar* maxYaw = g_Interfaces.CVars->FindVar(_("mp_feetyawrate")); //How many degrees per second that we can turn our feet or upper body.
	maxYaw->SetValue(999999999); //Set it to some high number for pseudo removal of the limit?
	cpuThrottle->SetValue(0);
	holidayForce->SetValue(1);
	localFootsteps->SetValue(0);
}

void CMisc::Interp()
{
	ConVar* interpRatio = g_Interfaces.CVars->FindVar(_("cl_interp_ratio"));
	ConVar* interp = g_Interfaces.CVars->FindVar(_("cl_interpolate"));
	ConVar* interpAmount = g_Interfaces.CVars->FindVar(_("cl_interp"));
	ConVar* interpRatioServer = g_Interfaces.CVars->FindVar(_("sv_client_min_interp_ratio"));
	if (Vars::Misc::DisableInterpolation.m_Var)
	{
		interpRatioServer->SetValue(0);
		interpRatio->SetValue(0);
		interpAmount->SetValue(0);
		interp->SetValue(0);
	}
	else
	{
		interpRatioServer->SetValue(1);
		interpRatio->SetValue(1);
		interpAmount->SetValue(0);
		interp->SetValue(1);
	}
}

void CMisc::NoPush()
{
	ConVar* push = g_Interfaces.CVars->FindVar(_("tf_avoidteammates_pushaway"));
	if (Vars::Misc::NoPush.m_Var)
	{
		push->SetValue(0);
	}
	else
	{
		push->SetValue(1);
	}
}

void CMisc::EdgeJump(CUserCmd* pCmd, const int nOldFlags)
{
	if ((nOldFlags & FL_ONGROUND) && GetAsyncKeyState(Vars::Misc::EdgeJump.m_Var))
	{
		if (const auto& pLocal = g_EntityCache.m_pLocal)
		{
			if (pLocal->IsAlive() && !pLocal->IsOnGround() && !pLocal->IsSwimming())
				pCmd->buttons |= IN_JUMP;
		}
	}
}

void CMisc::SpeedHack()
{
	ConVar* h_framerate = g_Interfaces.CVars->FindVar(_("host_framerate"));
	ConVar* framerate = g_Interfaces.CVars->FindVar(_("fps_max"));
	ConVar* updaterate = g_Interfaces.CVars->FindVar(_("cl_updaterate"));
	ConVar* h_timescale = g_Interfaces.CVars->FindVar(_("host_timescale"));
	ConVar* cheats = g_Interfaces.CVars->FindVar(_("sv_cheats"));
	if (Vars::Misc::CL_Move::SEnabled.m_Var)
	{
		cheats->SetValue(1);
		h_timescale->SetValue(Vars::Misc::CL_Move::SFactor.m_Var);
		h_framerate->SetValue(66 / Vars::Misc::CL_Move::SFactor.m_Var);
		updaterate->SetValue(66 / Vars::Misc::CL_Move::SFactor.m_Var);
		framerate->SetValue(66);
	}
	else
	{
		h_framerate->SetValue(0);
		h_timescale->SetValue(1);
		updaterate->SetValue(1000); //choose a big fucking number lol
		framerate->SetValue(0);
	}
}

void CMisc::CheatBypass()
{
	ConVar* cheats = g_Interfaces.CVars->FindVar(_("sv_cheats"));
	if (Vars::Misc::CheatBypass.m_Var)
	{
		cheats->SetValue(1);
	}
}

void CMisc::AutoJump(CUserCmd *pCmd)
{
	if (const auto &pLocal = g_EntityCache.m_pLocal)
	{
		if (!Vars::Misc::AutoJump.m_Var
			|| !pLocal->IsAlive()
			|| pLocal->IsSwimming()
			|| pLocal->IsInBumperKart()
			|| pLocal->IsAGhost())
			return;

		static bool bJumpState = false;

		if (pCmd->buttons & IN_JUMP)
		{
			if (!bJumpState && !pLocal->IsOnGround())
				pCmd->buttons &= ~IN_JUMP;

			else if (bJumpState)
				bJumpState = false;
		}

		else if (!bJumpState)
			bJumpState = true;
	}
}

void CMisc::AutoStrafe(CUserCmd* pCmd)
{
	if (Vars::Misc::AutoStrafe.m_Var)
	{
		if (const auto& pLocal = g_EntityCache.m_pLocal)
		{
			if (pLocal->IsAlive() && !pLocal->IsSwimming() && !pLocal->IsOnGround() && (pCmd->mousedx > 1 || pCmd->mousedx < -1))
				pCmd->sidemove = pCmd->mousedx > 1 ? 450.f : -450.f;
		}
	}
}

void CMisc::InitSpamKV(void* pKV)
{
	char chCommand[30] = "use_action_slot_item_server";
	typedef int(__cdecl* HashFunc_t)(const char*, bool);

	static DWORD dwHashFunctionLocation = g_Pattern.Find(_(L"client.dll"), _(L"FF 15 ? ? ? ? 83 C4 08 89 06 8B C6"));
	static HashFunc_t SymbForString = (HashFunc_t)* *(PDWORD*)(dwHashFunctionLocation + 0x2);

	int nAddr = 0;
	while (nAddr < 29)
	{
		switch (nAddr)
		{
			case 0:
				*(PDWORD)((DWORD)pKV + nAddr) = SymbForString(chCommand, true);
				break;
			case 16:
				*(PDWORD)((DWORD)pKV + nAddr) = 0x10000;
				break;
			default:
				*(PDWORD)((DWORD)pKV + nAddr) = 0;
				break;
		}

		nAddr += 4;
	}
}

void CMisc::NoiseMakerSpam()
{
	if (!Vars::Misc::NoisemakerSpam.m_Var)
		return;

	if (const auto& pLocal = g_EntityCache.m_pLocal)
	{
		if (pLocal->GetUsingActionSlot())
			return;

		if (pLocal->GetNextNoiseMakerTime() < g_Interfaces.GlobalVars->curtime)
		{
			if (const auto pKV = Utils::InitKeyValue())
			{
				InitSpamKV(pKV);
				g_Interfaces.Engine->ServerCmdKeyValues(pKV);
			}
		}
	}
}

void CMisc::BypassPure()
{
	if (Vars::Misc::BypassPure.m_Var)
	{
		static DWORD dwAddress = 0x0;

		while (!dwAddress)
			dwAddress = g_Pattern.Find(_(L"engine.dll"), _(L"A1 ? ? ? ? 56 33 F6 85 C0"));

		static DWORD* pPure = nullptr;

		while (!pPure)
		{
			if (reinterpret_cast<DWORD**>(dwAddress + 0x01))
				pPure = *reinterpret_cast<DWORD**>(dwAddress + 0x01);
		}

		if (*pPure)
			*pPure = 0x0;
	}
}

std::string GetSpam(const int nIndex) {
	std::string str;

	switch (nIndex)
	{
		case 0: str = XorStr("say deathpole - shitting on ur p2c with superior crimwalk").str(); break;
		case 1: str = XorStr("say deathpole - go cry to senator to fix your retarded cheat").str(); break;
		case 2: str = XorStr("say deathpole - best cheat for speedhack hvh wdym").str(); break;
		case 3: str = XorStr("say deathpole - cry more retard lOl").str(); break;
  	   default: str = XorStr("say deathpole - best cheat made in like an hour").str(); break;
	}

	return str;
}

void CMisc::ChatSpam()
{
	if (!Vars::Misc::ChatSpam.m_Var)
		return;

	float flCurTime = g_Interfaces.Engine->Time(); 
	static float flNextSend = 0.0f;

	if (flCurTime > flNextSend) {
		g_Interfaces.Engine->ClientCmd_Unrestricted(GetSpam(Utils::RandIntSimple(0, 5)).c_str());
		flNextSend = (flCurTime + 3.0f);
	}
}
