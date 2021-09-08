#pragma once

#include "Main/BaseEntity/BaseEntity.h"
#include "Main/BaseCombatWeapon/BaseCombatWeapon.h"
#include "Main/BaseObject/BaseObject.h"
#include "Main/DrawUtils/DrawUtils.h"
#include "Main/EntityCache/EntityCache.h"
#include "Main/GlobalInfo/GlobalInfo.h"
#include "Main/ConVars/ConVars.h"
#include "Main/KeyValues/KeyValues.h"
#include "Main/TraceFilters/TraceFilters.h"

#define TICK_INTERVAL		( g_Interfaces.GlobalVars->interval_per_tick )
#define TIME_TO_TICKS( dt )	( static_cast<int>( 0.5f + static_cast<float>(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )	( TICK_INTERVAL * ( t ) )
#define GetKey(vKey) (Utils::IsGameWindowInFocus() && GetAsyncKeyState(vKey))

//I for some reason have to include this here, if I don't then one steam header goes apeshit full of errors
#include "../Utils/CRC/CRC.h"

#pragma warning (disable : 6385)
#pragma warning (disable : 26451)
#pragma warning (disable : 4305)
#pragma warning (disable : 4172)

struct ShaderStencilState_t
{
	bool                        m_bEnable;
	StencilOperation_t          m_FailOp;
	StencilOperation_t          m_ZFailOp;
	StencilOperation_t          m_PassOp;
	StencilComparisonFunction_t m_CompareFunc;
	int                         m_nReferenceValue;
	uint32                      m_nTestMask;
	uint32                      m_nWriteMask;

	ShaderStencilState_t();
	void SetStencilState(IMatRenderContext *pRenderContext);
};

inline ShaderStencilState_t::ShaderStencilState_t()
{
	m_bEnable = false;
	m_PassOp = m_FailOp = m_ZFailOp = STENCILOPERATION_KEEP;
	m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
	m_nReferenceValue = 0;
	m_nTestMask = m_nWriteMask = 0xFFFFFFFF;
}

inline void ShaderStencilState_t::SetStencilState(IMatRenderContext *pRenderContext)
{
	pRenderContext->SetStencilEnable(m_bEnable);
	pRenderContext->SetStencilFailOperation(m_FailOp);
	pRenderContext->SetStencilZFailOperation(m_ZFailOp);
	pRenderContext->SetStencilPassOperation(m_PassOp);
	pRenderContext->SetStencilCompareFunction(m_CompareFunc);
	pRenderContext->SetStencilReferenceValue(m_nReferenceValue);
	pRenderContext->SetStencilTestMask(m_nTestMask);
	pRenderContext->SetStencilWriteMask(m_nWriteMask);
}

namespace Colors
{
	inline Color_t White =					{ 255, 255, 255, 255 };
	inline Color_t OutlineESP =				{ 0, 0, 0, 220 };
	inline Color_t Cond =					{ 254, 202, 87, 255 };
	inline Color_t Target =					{ 240, 147, 43, 255 };
	inline Color_t Invuln =					{ 120, 111, 166, 255 };
	inline Color_t Cloak =					{ 165, 177, 194, 255 };
	inline Color_t Friend =					{ 32, 191, 107, 255 };
	inline Color_t Overheal =				{ 84, 160, 255, 255 };
	inline Color_t Health =					{ 0, 230, 64, 255 };
	inline Color_t Ammo =					{ 191, 191, 191, 255 };
	inline Color_t UberColor =				{ 224, 86, 253, 255 };
	inline Color_t TeamRed =				{ 255, 100, 87, 255 };
	inline Color_t TeamBlu =				{ 30, 144, 255, 255 };
	inline Color_t Hands =					{ 30, 144, 255, 255 };
	inline Color_t HandsOverlay =			{ 255, 127, 0, 255 };
	inline Color_t Weapon =					{ 30, 144, 255, 255 };
	inline Color_t WeaponOverlay =			{ 255, 127, 0, 255 };
	inline Color_t WorldModulation =		{ 255, 255, 255, 255 };
	inline Color_t SkyModulation =			{ 255, 255, 255, 255 };
	inline Color_t StaticPropModulation =	{ 255, 255, 255, 255 };
	inline Color_t FOVCircle =				{ 255, 255, 255, 255 };
	inline Color_t Bones =					{ 255, 255, 255, 255 };
	inline Color_t BulletTracer =			{ 84, 160, 255, 255 };
}

namespace Utils
{
	__inline IMaterial *CreateMaterial(const char *szVars)
	{
		static int nCreatedMats = 0;
		char szOut[512];
		sprintf_s(szOut, sizeof(szOut), _("SEO_material%i.vmt"), nCreatedMats++);

		char szMaterial[512];
		sprintf_s(szMaterial, sizeof(szMaterial), szVars);

		KeyValues *pVals = new KeyValues;
		g_KeyValUtils.Initialize(pVals, (char *)szOut);
		g_KeyValUtils.LoadFromBuffer(pVals, szOut, szMaterial);

		IMaterial *pCreated = g_Interfaces.MatSystem->Create(szOut, pVals);
		pCreated->IncrementReferenceCount();

		return pCreated;
	}

	__inline void *CreateKeyVals(const char *szVars)
	{
		static int nCreatedKeyVals = 0;
		char szOut[512];
		sprintf_s(szOut, sizeof(szOut), _("SEO_keyvals%i.vmt"), nCreatedKeyVals++);

		KeyValues *pVals = new KeyValues;
		g_KeyValUtils.Initialize(pVals, (char *)szOut);
		g_KeyValUtils.LoadFromBuffer(pVals, szOut, szVars);

		return pVals;
	}

	__inline bool W2S(const Vec3 &vOrigin, Vec3 &m_vScreen)
	{
		const matrix3x4 &worldToScreen = g_GlobalInfo.m_WorldToProjection.As3x4();

		float w = worldToScreen[3][0] * vOrigin[0] + worldToScreen[3][1] * vOrigin[1] + worldToScreen[3][2] * vOrigin[2] + worldToScreen[3][3];
		m_vScreen.z = 0;

		if (w > 0.001)
		{
			float fl1DBw = 1 / w;
			m_vScreen.x = (g_ScreenSize.w / 2) + (0.5 * ((worldToScreen[0][0] * vOrigin[0] + worldToScreen[0][1] * vOrigin[1] + worldToScreen[0][2] * vOrigin[2] + worldToScreen[0][3]) * fl1DBw) * g_ScreenSize.w + 0.5);
			m_vScreen.y = (g_ScreenSize.h / 2) - (0.5 * ((worldToScreen[1][0] * vOrigin[0] + worldToScreen[1][1] * vOrigin[1] + worldToScreen[1][2] * vOrigin[2] + worldToScreen[1][3]) * fl1DBw) * g_ScreenSize.h + 0.5);
			return true;
		}

		return false;
	}

	__inline Color_t Rainbow()
	{
		return
		{
			static_cast<byte>(floor(sin(g_Interfaces.GlobalVars->curtime + 0.0f) * 127.0f + 128.0f)),
			static_cast<byte>(floor(sin(g_Interfaces.GlobalVars->curtime + 2.0f) * 127.0f + 128.0f)),
			static_cast<byte>(floor(sin(g_Interfaces.GlobalVars->curtime + 4.0f) * 127.0f + 128.0f)),
			255
		};
	};

	__inline void GetProjectileFireSetup(CBaseEntity *pPlayer, const Vec3 &vViewAngles, Vec3 vOffset, Vec3 *vSrc)
	{
		if (g_ConVars.cl_flipviewmodels->GetInt())
			vOffset.y *= -1.0f;

		Vec3 vecForward = Vec3(), vecRight = Vec3(), vecUp = Vec3();
		Math::AngleVectors(vViewAngles, &vecForward, &vecRight, &vecUp);

		*vSrc = pPlayer->GetShootPos() + (vecForward * vOffset.x) + (vecRight * vOffset.y) + (vecUp * vOffset.z);
	}

	__inline bool IsGameWindowInFocus()
	{
		static HWND hwGame = 0;

		while (!hwGame) {
			hwGame = WinAPI::FindWindowW(0, _(L"Team Fortress 2"));
			return false;
		}

		return (WinAPI::GetForegroundWindow() == hwGame);
	}

	__inline void* InitKeyValue()
	{
		typedef PDWORD(__cdecl* Init_t)(int);
		static DWORD dwInitLocation = g_Pattern.Find(_(L"client.dll"), _(L"E8 ? ? ? ? 83 C4 14 85 C0 74 10 68")) + 0x1;
		static DWORD dwInit = ((*(PDWORD)(dwInitLocation)) + dwInitLocation + 4);
		static Init_t InitKeyValues = (Init_t)dwInit;

		return InitKeyValues(32);
	}

	__inline Color_t GetTeamColor(int nTeamNum)
	{
		switch (nTeamNum)
		{
			case 2: return Colors::TeamRed;
			case 3: return Colors::TeamBlu;
			default: return Colors::White;
		}
	}

	__inline Color_t GetEntityDrawColor(CBaseEntity* pEntity)
	{
		Color_t out = GetTeamColor(pEntity->GetTeamNum());

		if (pEntity->IsPlayer())
		{
			if (g_EntityCache.Friends[pEntity->GetIndex()] || pEntity == g_EntityCache.m_pLocal)
				out = Colors::Friend;

			if (pEntity->IsCloaked())
				out = Colors::Cloak;

			if (!pEntity->IsVulnerable())
				out = Colors::Invuln;
		}

		if (pEntity->GetIndex() == g_GlobalInfo.m_nCurrentTargetIdx)
			out = Colors::Target;

		return out;
	}

	__inline const char* GetClassByIndex(const int nClass)
	{
		static const char* szClasses[] = { "unknown", "scout", "sniper", "soldier", "demoman",
										   "medic",   "heavy", "pyro",   "spy",     "engineer" };

		return (nClass < 10 && nClass > 0) ? szClasses[nClass] : szClasses[0];
	}

	__inline Color_t GetHealthColor(int nHP, int nMAXHP)
	{
		nHP = std::max(0, std::min(nHP, nMAXHP));
		int r = std::min((510 * (nMAXHP - nHP)) / nMAXHP, 200);
		int g = std::min((510 * nHP) / nMAXHP, 200);
		return { static_cast<byte>(r), static_cast<byte>(g), 0, 255 };
	}

	__inline bool IsOnScreen(CBaseEntity* pLocal, CBaseEntity* pEntity)
	{
		Vec3 WSC = pEntity->GetWorldSpaceCenter();

		if (WSC.DistTo(pLocal->GetWorldSpaceCenter()) > 300.0f)
		{
			Vec3 vScreen = {};

			if (Utils::W2S(pEntity->GetWorldSpaceCenter(), vScreen))
			{
				if (vScreen.x < -400
					|| vScreen.x > g_ScreenSize.w + 400
					|| vScreen.y < -400
					|| vScreen.y > g_ScreenSize.h + 400)
					return false;
			}

			else return false;
		}

		return true;
	}

	__inline void TraceHull(const Vec3 &vecStart, const Vec3 &vecEnd, const Vec3 &vecHullMin, const Vec3 &vecHullMax,
		unsigned int nMask, CTraceFilter *pFilter, CGameTrace *pTrace)
	{
		Ray_t ray;
		ray.Init(vecStart, vecEnd, vecHullMin, vecHullMax);
		g_Interfaces.EngineTrace->TraceRay(ray, nMask, pFilter, pTrace);
	}

	__inline void Trace(const Vec3 &vecStart, const Vec3 &vecEnd, unsigned int nMask, CTraceFilter *pFilter, CGameTrace *pTrace)
	{
		Ray_t ray;
		ray.Init(vecStart, vecEnd);
		g_Interfaces.EngineTrace->TraceRay(ray, nMask, pFilter, pTrace);
	}

	__inline int RandInt(int min, int max)
	{
		//This allows us to reach closer to true randoms generated
		//I don't think we need to update the seed more than once
		static const unsigned nSeed = std::chrono::system_clock::now().time_since_epoch().count();

		std::default_random_engine gen(nSeed);
		std::uniform_int_distribution<int> distr(min, max);
		return distr(gen);
	}

	__inline int RandIntSimple(int min, int max)
	{
		std::random_device rd; std::mt19937 gen(rd()); std::uniform_int_distribution<> distr(min, max);
		return distr(gen);
	}

	__inline void FixMovement(CUserCmd *pCmd, const Vec3 &vecTargetAngle)
	{
		Vec3 vecMove(pCmd->forwardmove, pCmd->sidemove, pCmd->upmove);
		Vec3 vecMoveAng = Vec3();

		Math::VectorAngles(vecMove, vecMoveAng);

		float fSpeed = Math::FastSqrt(vecMove.x * vecMove.x + vecMove.y * vecMove.y);
		float fYaw = DEG2RAD(vecTargetAngle.y - pCmd->viewangles.y + vecMoveAng.y);

		pCmd->forwardmove = (cos(fYaw) * fSpeed);
		pCmd->sidemove = (sin(fYaw) * fSpeed);
	}

	__inline std::wstring ConvertUtf8ToWide(const std::string_view& str)
	{
		int count = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.length(), NULL, 0);
		std::wstring wstr(count, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.data(), str.length(), &wstr[0], count);
		return wstr;
	}

	__inline float ATTRIB_HOOK_FLOAT(float base_value, const char *search_string, CBaseEntity *ent, void *buffer, bool is_global_const_string)
	{
		static auto fn = reinterpret_cast<float(__cdecl *)(float, const char *, CBaseEntity *, void *, bool)>(g_Pattern.Find(_(L"client.dll"),
			_(L"55 8B EC 83 EC 0C 8B 0D ? ? ? ? 53 56 57 33 F6 33 FF 89 75 F4 89 7D F8 8B 41 08 85 C0 74 38")));

		return fn(base_value, search_string, ent, buffer, is_global_const_string);
	}

	__inline void RemovePEH(HINSTANCE hinstDLL)
	{
		DWORD dwProtect = 0x0;
		auto pImageDOS = reinterpret_cast<PIMAGE_DOS_HEADER>(hinstDLL);

		if (pImageDOS->e_magic == IMAGE_DOS_SIGNATURE)
		{
			auto pImageNT = reinterpret_cast<PIMAGE_NT_HEADERS>(pImageDOS + pImageDOS->e_lfanew);
			const auto SizeOfPE = pImageNT->FileHeader.SizeOfOptionalHeader;

			if (pImageNT->Signature == IMAGE_NT_SIGNATURE && SizeOfPE)
			{
				const auto HeaderSize = static_cast<size_t>(SizeOfPE);
				VirtualProtect(reinterpret_cast<LPVOID>(hinstDLL), HeaderSize, PAGE_EXECUTE_READWRITE, &dwProtect);
				RtlZeroMemory(reinterpret_cast<LPVOID>(hinstDLL), HeaderSize);
				VirtualProtect(reinterpret_cast<LPVOID>(hinstDLL), HeaderSize, dwProtect, &dwProtect);
			}
		}
	}

	__inline int SeedFileLineHash(int seedvalue, const char *sharedname, int additionalSeed)
	{
		CRC32_t retval;

		CRC32_Init(&retval);

		CRC32_ProcessBuffer(&retval, (void *)&seedvalue, sizeof(int));
		CRC32_ProcessBuffer(&retval, (void *)&additionalSeed, sizeof(int));
		CRC32_ProcessBuffer(&retval, (void *)sharedname, strlen(sharedname));

		CRC32_Final(&retval);

		return (int)(retval);
	}

	__inline int SharedRandomInt(unsigned iseed, const char *sharedname, int iMinVal, int iMaxVal, int additionalSeed)
	{
		int seed = SeedFileLineHash(iseed, sharedname, additionalSeed);
		g_Interfaces.UniformRandomStream->SetSeed(seed);
		return g_Interfaces.UniformRandomStream->RandomInt(iMinVal, iMaxVal);
	}

	__inline void RandomSeed(int iSeed)
	{
		static auto RandomSeedFn = reinterpret_cast<void(__cdecl *)(int)>(reinterpret_cast<DWORD>(WinAPI::GetProcessAddr(reinterpret_cast<DWORD>(GetModuleHandleW(XorStr(L"vstdlib.dll").c_str())), XorStr("RandomSeed").c_str())));
		RandomSeedFn(iSeed);
	}

	__inline bool VisPos(CBaseEntity *pSkip, CBaseEntity *pEntity, const Vec3 &from, const Vec3 &to)
	{
		CGameTrace trace = {};
		CTraceFilterHitscan filter = {};
		filter.pSkip = pSkip;
		Trace(from, to, (MASK_SHOT | CONTENTS_GRATE), &filter, &trace);
		return ((trace.entity && trace.entity == pEntity) || trace.flFraction > 0.99f);
	}

	__inline bool VisPosHitboxId(CBaseEntity *pSkip, CBaseEntity *pEntity, const Vec3 &from, const Vec3 &to, int nHitbox)
	{
		CGameTrace trace = {};
		CTraceFilterHitscan filter = {};
		filter.pSkip = pSkip;
		Trace(from, to, (MASK_SHOT | CONTENTS_GRATE), &filter, &trace);
		return (trace.entity && trace.entity == pEntity && trace.hitbox == nHitbox);
	}

	__inline bool VisPosHitboxIdOut(CBaseEntity *pSkip, CBaseEntity *pEntity, const Vec3 &from, const Vec3 &to, int &nHitboxOut)
	{
		CGameTrace trace = {};
		CTraceFilterHitscan filter = {};
		filter.pSkip = pSkip;
		Trace(from, to, (MASK_SHOT | CONTENTS_GRATE), &filter, &trace);

		if (trace.entity && trace.entity == pEntity) {
			nHitboxOut = trace.hitbox;
			return true;
		}

		return false;
	}

	__inline bool VisPosFraction(CBaseEntity *pSkip, const Vec3 &from, const Vec3 &to)
	{
		CGameTrace trace = {};
		CTraceFilterHitscan filter = {};
		filter.pSkip = pSkip;
		Trace(from, to, (MASK_SHOT | CONTENTS_GRATE), &filter, &trace);
		return (trace.flFraction > 0.99f);
	}

	__inline EWeaponType GetWeaponType(CBaseCombatWeapon *pWeapon)
	{
		if (!pWeapon)
			return EWeaponType::UNKNOWN;

		if (pWeapon->GetSlot() == EWeaponSlots::SLOT_MELEE)
			return EWeaponType::MELEE;

		switch (pWeapon->GetWeaponID())
		{
			case TF_WEAPON_ROCKETLAUNCHER:
			case 109:
			case TF_WEAPON_GRENADELAUNCHER:
			case TF_WEAPON_FLAREGUN:
			case TF_WEAPON_COMPOUND_BOW:
			case TF_WEAPON_DIRECTHIT:
			case TF_WEAPON_CROSSBOW:
			case TF_WEAPON_PARTICLE_CANNON:
			case TF_WEAPON_DRG_POMSON:
			case TF_WEAPON_RAYGUN_REVENGE:
			case TF_WEAPON_CANNON:
			case TF_WEAPON_SYRINGEGUN_MEDIC:
			case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
			case TF_WEAPON_FLAMETHROWER:
			{
				return EWeaponType::PROJECTILE;
			}

			case TF_WEAPON_PIPEBOMBLAUNCHER://dragon's fury
			{
				//broken Idk
				return EWeaponType::UNKNOWN;
			}

			default:
			{
				int nDamageType = pWeapon->GetDamageType();

				if (nDamageType & DMG_BULLET || nDamageType && DMG_BUCKSHOT)
					return EWeaponType::HITSCAN;

				break;
			}
		}

		return EWeaponType::UNKNOWN;
	}

	__inline uintptr_t GetVirtual(void* pBaseClass, unsigned int nIndex)
	{ 
		return static_cast<uintptr_t>((*static_cast<int**>(pBaseClass))[nIndex]);
	}

	__inline bool IsAttacking(CUserCmd *pCmd, CBaseCombatWeapon *pWeapon)
	{
		if (pWeapon->GetSlot() == SLOT_MELEE)
		{
			if (pWeapon->GetWeaponID() == TF_WEAPON_KNIFE)
				return ((pCmd->buttons & IN_ATTACK) && g_GlobalInfo.m_bWeaponCanAttack);

			else return fabs(pWeapon->GetSmackTime() - g_Interfaces.GlobalVars->curtime) < g_Interfaces.GlobalVars->interval_per_tick * 2.0f;
		}

		else
		{
			if (g_GlobalInfo.m_nCurItemDefIndex == Soldier_m_TheBeggarsBazooka)
			{
				static bool bLoading = false;

				if (pWeapon->GetClip1() > 0)
					bLoading = true;

				if (!(pCmd->buttons & IN_ATTACK) && bLoading) {
					bLoading = false;
					return true;
				}
			}

			else
			{
				int ID = pWeapon->GetWeaponID();

				if (ID == TF_WEAPON_COMPOUND_BOW || ID == TF_WEAPON_PIPEBOMBLAUNCHER)
				{
					static bool bCharging = false;

					if (pWeapon->GetChargeBeginTime() > 0.0f)
						bCharging = true;

					if (!(pCmd->buttons & IN_ATTACK) && bCharging) {
						bCharging = false;
						return true;
					}
				}

				else if (ID == TF_WEAPON_JAR || ID == TF_WEAPON_JAR_MILK || ID == 107/*pyro's gas thing*/)
				{
					static float flThrowTime = 0.0f;

					if ((pCmd->buttons & IN_ATTACK) && g_GlobalInfo.m_bWeaponCanAttack && !flThrowTime)
						flThrowTime = g_Interfaces.GlobalVars->curtime + 0.16f;

					if (flThrowTime && g_Interfaces.GlobalVars->curtime >= flThrowTime) {
						flThrowTime = 0.0f;
						return true;
					}
				}

				else
				{
					if (!g_GlobalInfo.m_bWeaponCanAttack && (pCmd->buttons & IN_ATTACK))
					{
						if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN)
							pCmd->buttons |= IN_ATTACK2;

						pCmd->buttons &= ~IN_ATTACK;
					}

					if ((pCmd->buttons & IN_ATTACK) && g_GlobalInfo.m_bWeaponCanAttack)
						return true;
				}
			}
		}

		return false;
	}
}