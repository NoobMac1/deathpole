#include "hol.h"

enum EHoliday
{
	kHoliday_None = 0,		// must stay at zero for backwards compatibility
	kHoliday_TFBirthday,
	kHoliday_Halloween,
	kHoliday_Christmas,
	kHoliday_CommunityUpdate,
	kHoliday_EOTL,
	kHoliday_Valentines,
	kHoliday_MeetThePyro,
	kHoliday_FullMoon,
	kHoliday_HalloweenOrFullMoon,
	kHoliday_HalloweenOrFullMoonOrValentines,
	kHoliday_AprilFools,
	kHolidayCount,
};

bool TF_IsHolidayActive::hook(int nMode)
{
	typedef bool(*fn)(int);
	return nMode == kHoliday_HalloweenOrFullMoon ? true : func.Original<fn>()(nMode);
	//return true;
}


/*void TF_IsHolidayActive::Init()
{
	fn FN = reinterpret_cast<fn>(g_Pattern.Find(L"client.dll", L"55 8B EC A1 ? ? ? ? 83 78 30 00 74 04 32 C0 5D C3"));
	Hook.Create(FN, Func);
}*/