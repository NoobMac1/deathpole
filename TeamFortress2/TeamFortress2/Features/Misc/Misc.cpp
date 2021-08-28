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
}

void CMisc::NoPush()
{
	ConVar* push = g_Interfaces.CVars->FindVar(_("tf_avoidteammates_pushaway"));
	if (Vars::Misc::NoPush.m_Var)
	{
		push->SetValue(0);
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
	ConVar* updaterate = g_Interfaces.CVars->FindVar(_("cl_updaterate"));
	int update_Rate = updaterate->GetInt();
	ConVar* h_timescale = g_Interfaces.CVars->FindVar(_("host_timescale"));
	ConVar* cheats = g_Interfaces.CVars->FindVar(_("sv_cheats"));
	if (Vars::Misc::CL_Move::SEnabled.m_Var)
	{
		cheats->SetValue(1);
		h_timescale->SetValue(Vars::Misc::CL_Move::SFactor.m_Var);
		h_framerate->SetValue(update_Rate / Vars::Misc::CL_Move::SFactor.m_Var);
		updaterate->SetValue(update_Rate / Vars::Misc::CL_Move::SFactor.m_Var);
	}
	else
	{
		h_framerate->SetValue(0);
		h_timescale->SetValue(1);
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
		case 0: str = XorStr("say deathpole - shitting on ur p2c with superior speedhack method").str(); break;
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
