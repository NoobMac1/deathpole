#include "EngineVGuiHook.h"

#include "../../SDK/Includes/icons.h"
#include "../../Features/Menu/Menu.h"
#include "../../Features/SpectatorList/SpectatorList.h"
#include "../../Features/SpyWarning/SpyWarning.h"
#include "../../Features/ESP/ESP.h"
#include "../../Features/Misc/Misc.h"
#include "../../Features/Radar/Radar.h"
#include "../../Features/Aimbot/AimbotMelee/AimbotMelee.h"
#include "../../Features/Aimbot/AimbotProjectile/AimbotProjectile.h"

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
					{
						static const int size = 10;
						Vec3 vecScreen = Vec3();
						Vec3 vecStart = Vec3();
						Utils::W2S(g_GlobalInfo.m_vPredictedPos, vecStart);
						if (Utils::W2S(g_GlobalInfo.m_vPosition, vecScreen))
						{
							g_Draw.Line(
								static_cast<int>(vecScreen.x),
								static_cast<int>(vecScreen.y),
								static_cast<int>(vecStart.x),
								static_cast<int>(vecStart.y),
								{ 255, 255, 255, 255 });

							g_Draw.OutlinedRect(
								static_cast<int>(vecStart.x - (size / 2)),
								static_cast<int>(vecStart.y - (size / 2)),
								size, size,
								{ 255, 113, 35, 255 });
						}
					}
				}

				//watermark
				auto nci = g_Interfaces.Engine->GetNetChannelInfo(); int ping = (nci->GetLatency(FLOW_OUTGOING) * 1000);
				g_Draw.String(FONT_DEBUG, 10, (g_ScreenSize.h / 100) * 15, { 255, 255, 255, 255 }, ALIGN_DEFAULT, _(L"deathpole"));
				g_Draw.String(FONT_DEBUG, 10, (g_ScreenSize.h / 100) * 17, { 255, 255, 255, 255 }, ALIGN_DEFAULT, _(L"ping: %d"), ping);

				//Tickbase info
				if (Vars::Misc::CL_Move::Enabled.m_Var)
				{
					const auto& pLocal = g_EntityCache.m_pLocal;
					const auto& pWeapon = g_EntityCache.m_pLocalWeapon;
					if (pLocal && pWeapon)
					{
						if (pLocal->GetLifeState() == LIFE_ALIVE)
						{
							const int nY = (g_ScreenSize.h / 2) + 20;

							int MAXNEW = MAX_NEW_COMMANDS;
							int nClass = pLocal->GetClassNum();
							if (nClass == CLASS_HEAVY) {
								MAXNEW = MAX_NEW_COMMANDS_HEAVY;
							}

							float charged = (MAXNEW - g_GlobalInfo.m_nShifted);
							float ratio = (charged / MAXNEW);
							if (ratio > 1) { ratio = 1; }
							if (ratio < 0) { ratio = 0; } //player is playing as heavy, charges, changes to scout, maxnew changes to a value lower than heavies maxnew, becomes negative, slightly annoying

							int xoff = (Vars::Misc::CL_Move::DTBarX.m_Var);
							int yoff = (Vars::Misc::CL_Move::DTBarY.m_Var);
							int xscale = (Vars::Misc::CL_Move::DTBarScaleX.m_Var);
							int yscale = (Vars::Misc::CL_Move::DTBarScaleY.m_Var);

							g_Draw.OutlinedRect(g_ScreenSize.c - (yscale / 2 + 1) + xoff, nY - (xscale / 2 + 1) + yoff, (yscale + 2), (xscale + 2), Colors::TicksOutline);
							g_Draw.GradientRect(g_ScreenSize.c - (yscale / 2) + xoff, nY - (xscale / 2) + yoff, (g_ScreenSize.c - (yscale / 2) + xoff + yscale), (nY - (xscale / 2) + yoff + xscale), { 62, 81, 221, 255 }, { 148, 246, 255, 255 }, TRUE);
							g_Draw.String(FONT_ESP_COND_OUTLINED, g_ScreenSize.c - (yscale / 2 + 1) + xoff, nY - (xscale / 2 + 1) - 10 + yoff, { 255, 255, 255, 255 }, ALIGN_DEFAULT, _(L"CHARGE"));
							if (ratio == 0)
							{
								g_Draw.String(FONT_ESP_COND_OUTLINED, (g_ScreenSize.c - (yscale / 2) + xoff + yscale), nY - (xscale / 2 + 1) - 10 + yoff, { 255, 55, 40, 255 }, ALIGN_REVERSE, _(L"FLAT"));
								g_Draw.Rect(g_ScreenSize.c - (yscale / 2) + xoff, nY - (xscale / 2) + yoff, yscale, xscale, { 17, 24, 26, 255 });
							}
							else if (ratio != 1)
							{
								g_Draw.String(FONT_ESP_COND_OUTLINED, (g_ScreenSize.c - (yscale / 2) + xoff + yscale), nY - (xscale / 2 + 1) - 10 + yoff, { 255, 126, 0, 255 }, ALIGN_REVERSE, _(L"CHARGING"));
								g_Draw.Rect(g_ScreenSize.c - (yscale/2) + (yscale * ratio) + xoff, nY - (xscale/2) + yoff, yscale - (yscale * ratio), xscale, { 17, 24, 26, 255 });				
							}
							else if (!g_GlobalInfo.m_nWaitForShift)
							{
								g_Draw.String(FONT_ESP_COND_OUTLINED, (g_ScreenSize.c - (yscale / 2) + xoff + yscale), nY - (xscale / 2 + 1) - 10 + yoff, { 66, 255, 0, 255 }, ALIGN_REVERSE, _(L"DT READY"));
							}
							else
							{
								g_Draw.String(FONT_ESP_COND_OUTLINED, (g_ScreenSize.c - (yscale / 2) + xoff + yscale), nY - (xscale / 2 + 1) - 10 + yoff, { 255, 46, 46, 255 }, ALIGN_REVERSE, _(L"DT IMPOSSIBLE"));
							}
						}
					}
				}

				//Current Active Aimbot FOV
				if (Vars::Visuals::AimFOVAlpha.m_Var && g_GlobalInfo.m_flCurAimFOV)
				{
					if (const auto& pLocal = g_EntityCache.m_pLocal)
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