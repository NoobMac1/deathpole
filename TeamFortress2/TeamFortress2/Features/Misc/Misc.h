#pragma once

#include "../../SDK/SDK.h"

class CMisc
{
private:
	void AutoJump(CUserCmd *pCmd);
	void AutoStrafe(CUserCmd* pCmd);
	void NoiseMakerSpam();
	void InitSpamKV(void* pKV);
	void ChatSpam();

public:
	void Run(CUserCmd *pCmd);
	void EdgeJump(CUserCmd* pCmd, const int nOldFlags);
	void BypassPure();
	void CheatBypass();
	void SpeedHack();
	void NoPush();
};

inline CMisc g_Misc;