#include "EngineVGuiHook.h"

#include "../../SDK/Includes/icons.h"
#include "../../Features/Menu/Menu.h"
#include "../../Features/SpectatorList/SpectatorList.h"
#include "../../Features/SpyWarning/SpyWarning.h"
#include "../../Features/ESP/ESP.h"
#include "../../Features/Misc/Misc.h"
#include "../../Features/Radar/Radar.h"
#include "../../Features/Aimbot/AimbotMelee/AimbotMelee.h"

void __stdcall EngineVGuiHook::Paint::Hook(int mode)
{
	static auto StartDrawing = reinterpret_cast<void(__thiscall*)(void*)>(g_Pattern.Find(_(L"vguimatsurface.dll"), _(L"55 8B EC 64 A1 ? ? ? ? 6A FF 68 ? ? ? ? 50 64 89 25 ? ? ? ? 83 EC 14")));
	static auto FinishDrawing = reinterpret_cast<void(__thiscall*)(void*)>(g_Pattern.Find(_(L"vguimatsurface.dll"), _(L"55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 64 89 25 ? ? ? ? 51 56 6A 00")));

	if (!g_ScreenSize.w || !g_ScreenSize.h)
		g_ScreenSize.Update();

	//HACK: for some reason we need to do this
	{
		static bool bInitIcons = false;

		if (!bInitIcons)
		{
			for (int nIndex = 0; nIndex < ICONS::TEXTURE_AMOUNT; nIndex++)
			{
				ICONS::ID[nIndex] = -1;
				g_Draw.Texture(-200, 0, 18, 18, Colors::White, nIndex);
			}

			bInitIcons = true;
		}
	}

	Table.Original<fn>(index)(g_Interfaces.EngineVGui, mode);

	if (mode & PAINT_UIPANELS)
	{
		//Update W2S
		{
			CViewSetup ViewSetup = {};

			if (g_Interfaces.Client->GetPlayerView(ViewSetup)) {
				VMatrix WorldToView = {}, ViewToProjection = {}, WorldToPixels = {};
				g_Interfaces.RenderView->GetMatricesForView(ViewSetup, &WorldToView, &ViewToProjection, &g_GlobalInfo.m_WorldToProjection, &WorldToPixels);
			}
		}

		StartDrawing(g_Interfaces.Surface);
		{
			auto OtherDraws = [&]() -> void
			{
				if (g_Interfaces.EngineVGui->IsGameUIVisible())
					return;

				//Projectile Aim's Predicted Position
				if (!g_GlobalInfo.m_vPredictedPos.IsZero())
				{
					if (Vars::Visuals::CrosshairAimPos.m_Var ? g_GlobalInfo.m_vAimPos.IsZero() : true)
					{
						static const int size = 10;
						Vec3 vecScreen = Vec3();

						if (Utils::W2S(g_GlobalInfo.m_vPredictedPos, vecScreen))
						{
							g_Draw.OutlinedRect(
								static_cast<int>(vecScreen.x - (size / 2)),
								static_cast<int>(vecScreen.y - (size / 2)),
								size, size,
								{ 0, 255, 0, 255 });

							g_Draw.OutlinedRect(
								static_cast<int>(vecScreen.x - (size / 2)) - 1,
								static_cast<int>(vecScreen.y - (size / 2)) - 1,
								size + 2, size + 2,
								Colors::OutlineESP);

							g_Draw.OutlinedRect(
								static_cast<int>(vecScreen.x - (size / 2)) + 1,
								static_cast<int>(vecScreen.y - (size / 2)) + 1,
								size - 2, size - 2,
								Colors::OutlineESP);
						}
					}
				}

				//cheat tag
				{
					g_Draw.String(FONT_DEBUG, 10, (g_ScreenSize.h/100) * 15, { 255, 255, 255, 255 }, ALIGN_DEFAULT, _(L"Death Pole"));
				}

				//Tickbase info
				if (Vars::Misc::CL_Move::Enabled.m_Var)
				{
					const auto& pLocal = g_EntityCache.m_pLocal;
					const auto& pWeapon = g_EntityCache.m_pLocalWeapon;

					if (pLocal && pWeapon)
					{
						if (pLocal->GetLifeState() == LIFE_ALIVE && !Vars::Visuals::ThirdPerson.m_Var)
						{
							const int nY = (g_ScreenSize.h / 2) + 20;

							float charged = (DT_WAIT_CALLS - g_GlobalInfo.m_nWaitForShift);
							float ratio = (charged / DT_WAIT_CALLS);

							g_Draw.OutlinedRect(g_ScreenSize.c - 53, nY - 8, 106, 16, Colors::TicksOutline);
							g_Draw.String(FONT_MISC, g_ScreenSize.c - 52, nY - 20, { 255, 255, 255, 255 }, ALIGN_DEFAULT, _(L"CHARGE"));
							if (g_GlobalInfo.m_nShifted)
							{
								g_Draw.Rect(g_ScreenSize.c - 52, nY - 7, 104, 14, { 17, 24, 26, 255 });
							}
							else if (!g_GlobalInfo.m_nShifted && g_GlobalInfo.m_nWaitForShift)
							{
								g_Draw.Rect(g_ScreenSize.c - 52 + (104 * ratio), nY - 7, 104 - (104 * ratio), 14, { 17, 24, 26, 255 });
								g_Draw.GradientRect(g_ScreenSize.c - 52, nY - 7, g_ScreenSize.c - 52 + (104 * ratio), nY + 7, { 62, 81, 221, 255 }, Colors::Ticks, TRUE);
							}
							else if (pWeapon->GetWeaponID() != TF_WEAPON_COMPOUND_BOW || pWeapon->GetWeaponID() != TF_WEAPON_CLEAVER || pWeapon->GetWeaponID() != TF_WEAPON_ROCKETLAUNCHER || pWeapon->GetWeaponID() != TF_WEAPON_SNIPERRIFLE || pWeapon->GetWeaponID() != TF_WEAPON_PIPEBOMBLAUNCHER)
							{
								g_Draw.GradientRect(g_ScreenSize.c - 52, nY - 7, g_ScreenSize.c + 52, nY + 7, { 62, 81, 221, 255 }, Colors::Ticks, TRUE);
							}
						}
					}
				}
				//Current Active Aimbot FOV
				if (Vars::Visuals::AimFOVAlpha.m_Var && g_GlobalInfo.m_flCurAimFOV)
				{
					if (const auto &pLocal = g_EntityCache.m_pLocal)
					{
						float flFOV = static_cast<float>(Vars::Visuals::FieldOfView.m_Var);
						float flR = tanf(DEG2RAD(g_GlobalInfo.m_flCurAimFOV) / 2.0f)
							/ tanf(DEG2RAD((pLocal->IsScoped() && !Vars::Visuals::RemoveZoom.m_Var) ? 30.0f : flFOV) / 2.0f) * g_ScreenSize.w;
						Color_t clr = Colors::FOVCircle;
						clr.a = static_cast<byte>(Vars::Visuals::AimFOVAlpha.m_Var);
						g_Draw.OutlinedCircle(g_ScreenSize.w / 2, g_ScreenSize.h / 2, flR, 68, clr);
					}
				}
			};
			OtherDraws();

			//Debug
			{

			}
			
			g_Misc.BypassPure();
			g_ESP.Run();
			g_SpyWarning.Run();
			g_SpectatorList.Run();
			g_Radar.Run();
			g_Menu.Run();
		}	
		FinishDrawing(g_Interfaces.Surface);
	}
}
