#pragma once

#include "../../SDK/SDK.h"

class CMisc
{
private:
	void AutoJump(CUserCmd *pCmd);
	void AutoStrafe(CUserCmd* pCmd);
	void StopFast(CUserCmd* pCmd);
	void SpeedHack();
	void ConsoleStuff();
	void Interp();
	void NoiseMakerSpam();
	void InitSpamKV(void* pKV);
	void ChatSpam();
	void CathookIdentify();
	void showhitboxes();
	void should();

public:
	void Run(CUserCmd *pCmd);
	void EdgeJump(CUserCmd* pCmd, const int nOldFlags);
	void BypassPure();
	void AutoRocketJump(CUserCmd *pCmd);
	void NoPush();
	void VoteRevealer(CGameEvent& event) noexcept;
	float currentcommandforwardmove = 0.0f;
	float currentcommandsidemove = 0.0f;
	float currentspeed = 0.0f;
	float currentmaxspeed = 0.0f;
	bool goback = false;
	bool stop = false;
};

inline CMisc g_Misc;