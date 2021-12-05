#pragma once
#include "../Hooks.h"
#include "../../SDK/SDK.h"

namespace RemoveDelay {
	inline SEOHook::Func Func;
	using fn = void(__fastcall*)(void*, void*);
	float __fastcall Hook(void* ecx, void* edx);
	void Init();
}