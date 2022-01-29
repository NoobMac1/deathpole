#pragma once
#include "..\..\SDK\SDK.h"

namespace TF_IsHolidayActive {
	bool hook(int);
	inline DWORD target;

	inline DPHook::Func func;
}