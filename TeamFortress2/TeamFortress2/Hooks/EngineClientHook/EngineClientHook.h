#pragma once

#include "../../SDK/SDK.h"

namespace EngineClientHook
{
	inline DPHook::VTable Table;

	namespace IsPlayingTimeDemo
	{
		const int index = 78;
		using fn = bool(__thiscall*)(CEngineClient*);
		bool __stdcall Hook();
	}

	namespace RemoveEventDelay
	{
		using fn = bool(__thiscall*)(CEngineClient*);
		float __fastcall Hook();
	}
}