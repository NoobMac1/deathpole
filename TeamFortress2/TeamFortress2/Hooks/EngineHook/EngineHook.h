#pragma once

#include "../Hooks.h"
#include "../../SDK/SDK.h"
#include "../ClientModeHook/ClientModeHook.h"
namespace EngineHook
{
	namespace CL_Move
	{
		inline DPHook::Func Func;

		using fn = void(__cdecl*)(float, bool);
		void __cdecl Hook(float accumulated_extra_samples, bool bFinalTick);
	}

	namespace CL_SendMove
	{
		inline DPHook::Func Func;

		using fn = void(__cdecl*)(void*, void*);
		void __cdecl Hook(void* ecx, void* edx);
	}
}