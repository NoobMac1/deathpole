#include "EngineVGuiHook.h"

#include "../../SDK/Includes/icons.h"
#include "../../Features/Menu/Menu.h"
#include "../../Features/SpectatorList/SpectatorList.h"
#include "../../Features/SpyWarning/SpyWarning.h"
#include "../../Features/PlayerArrows/PlayerArrows.h"
#include "../../Features/ESP/ESP.h"
#include "../../Features/Misc/Misc.h"
#include "../../Features/Radar/Radar.h"
#include "../../Features/Keybinds/Keybinds.h"
#include "../../Features/Aimbot/AimbotMelee/AimbotMelee.h"
#include "../../Features/Visuals/Visuals.h"
#include "../../Features/Crits/Crits.h"
#include "../../Features/Aimbot/AimbotProjectile/AimbotProjectile.h"
#include "../../Features/Crits/Crits.h"


void __stdcall EngineVGuiHook::Paint::Hook(int mode)
{

	static auto StartDrawing = reinterpret_cast<void(__thiscall*)(void*)>(g_Pattern.Find(_(L"vguimatsurface.dll"), _(L"55 8B EC 64 A1 ? ? ? ? 6A FF 68 ? ? ? ? 50 64 89 25 ? ? ? ? 83 EC 14")));
	static auto FinishDrawing = reinterpret_cast<void(__thiscall*)(void*)>(g_Pattern.Find(_(L"vguimatsurface.dll"), _(L"55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 64 89 25 ? ? ? ? 51 56 6A 00")));
	static int lastcheck = g_Interfaces.GlobalVars->tickcount;
	static int fps{};
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

				//projektile aimbot line (looks ugly)
				if (!g_GlobalInfo.m_vPredictedPos.IsZero())
				{
					if (Vars::Visuals::AimPosSquare.m_Var) {
						Vec3 vProjAimStart, vProjAimEnd = Vec3(g_ScreenSize.c, g_ScreenSize.h, 0.0f);

						Utils::W2S(g_GlobalInfo.m_vProjAimEnd, vProjAimStart);
						Utils::W2S(g_GlobalInfo.m_vPredictedPos, vProjAimEnd);
						g_Draw.Line(
							vProjAimStart.x,
							vProjAimStart.y,
							vProjAimEnd.x,
							vProjAimEnd.y,
							{ Colors::Target }
						);
					}
				}
				
				//damage logger
				g_notify.Think();

				//rijin style dt ind
				if (Vars::Misc::CL_Move::Enabled.m_Var)
				{
					const auto& pLocal = g_EntityCache.m_pLocal;
					const auto& pWeapon = g_EntityCache.m_pLocalWeapon;
					if (pLocal && pWeapon)
					{
						if (pLocal->GetLifeState() == LIFE_ALIVE)
						{
							const int nY = (g_ScreenSize.h / 2) + 20;

							float ratio = ((float)g_GlobalInfo.m_nShifted / (float)Vars::Misc::CL_Move::DoubletapAmt.m_Var);

							if (ratio > 1) { ratio = 1; }
							else if (ratio < 0) { ratio = 0; } //player changes tick count.

							int xoff = (Vars::Misc::CL_Move::DTBarX.m_Var);
							int yoff = (Vars::Misc::CL_Move::DTBarY.m_Var);
							int xscale = (Vars::Misc::CL_Move::DTBarScaleX.m_Var);
							int yscale = (Vars::Misc::CL_Move::DTBarScaleY.m_Var);

							g_Draw.OutlinedRect(g_ScreenSize.c - (yscale / 2 + 1) + xoff, nY - (xscale / 2 + 1) + yoff, (yscale + 2), (xscale + 2), Colors::DtOutline);
							g_Draw.GradientRect(g_ScreenSize.c - (yscale / 2) + xoff, nY - (xscale / 2) + yoff, (g_ScreenSize.c - (yscale / 2) + xoff + yscale), (nY - (xscale / 2) + yoff + xscale), { Colors::DTStart }, { Colors::DTEnd }, TRUE);
							g_Draw.String(FONT_ESP_COND_OUTLINED, g_ScreenSize.c - (yscale / 2 + 1) + xoff, nY - (xscale / 2 + 1) - 10 + yoff, { 255, 255, 255, 255 }, ALIGN_DEFAULT, _(L"CHARGE"));
							if (g_GlobalInfo.m_nShifted == 0)
							{
								g_Draw.String(FONT_ESP_COND_OUTLINED, (g_ScreenSize.c - (yscale / 2) + xoff + yscale), nY - (xscale / 2 + 1) - 10 + yoff, { 255, 55, 40, 255 }, ALIGN_REVERSE, _(L"NO CHARGE"));
								g_Draw.Rect(g_ScreenSize.c - (yscale / 2) + xoff, nY - (xscale / 2) + yoff, yscale, xscale, { 17, 24, 26, 255 });
							}
							else if (ratio != 1)
							{
								g_Draw.String(FONT_ESP_COND_OUTLINED, (g_ScreenSize.c - (yscale / 2) + xoff + yscale), nY - (xscale / 2 + 1) - 10 + yoff, { 255, 126, 0, 255 }, ALIGN_REVERSE, _(L"CHARGING"));
								g_Draw.Rect(g_ScreenSize.c - (yscale / 2) + (yscale * ratio) + xoff, nY - (xscale / 2) + yoff, yscale - (yscale * ratio), xscale, { 17, 24, 26, 255 });
							}
							else if (!g_GlobalInfo.m_nWaitForShift)
							{
								g_Draw.String(FONT_ESP_COND_OUTLINED, (g_ScreenSize.c - (yscale / 2) + xoff + yscale), nY - (xscale / 2 + 1) - 10 + yoff, { 66, 255, 0, 255 }, ALIGN_REVERSE, _(L"READY"));
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
						g_Draw.OutlinedCircle(g_ScreenSize.w / 2, g_ScreenSize.h / 2, flR, 68, clr);
					}
				}
			};
			OtherDraws();

			// get tickrate.
			int rate = (int)std::round(1.f / g_Interfaces.GlobalVars->interval_per_tick);

			if (g_GlobalInfo.watermarkCounter > 75) {
				g_GlobalInfo.watermarkCounter = 0;
				// get framerate.
				fps = (int)std::round(1.f / g_Interfaces.GlobalVars->frametime);
			} // delay updates so it doesn't spaz
			else { g_GlobalInfo.watermarkCounter++; }

			TCHAR infoBuf[32767];
			DWORD bufCharCount = 32767;
			typedef std::basic_string<TCHAR> tstring;
			GetComputerNameW(infoBuf, &bufCharCount);
			tstring computername = infoBuf;
			std::string computernamestd = std::string(computername.begin(), computername.end());

			std::stringstream watermarktext;
			if (g_Interfaces.Engine->IsInGame()) {
				watermarktext << tfm::format(_("deathpole | %s | rate: %i | fps: %i"), computernamestd, rate-1, fps);
			}
			else {
				watermarktext << tfm::format(_("deathpole | %s | in menu"), computernamestd);
			}

			int wideth, heighth;
			g_Interfaces.Surface->GetTextSize(g_Draw.m_vecFonts[FONT_MENU].dwFont, Utils::ConvertUtf8ToWide(watermarktext.str()).data(), wideth, heighth);

			if (Vars::Visuals::Watermark.m_Var) {
				g_Draw.Rect(g_ScreenSize.w - wideth - 20, 8, wideth + 10, 2, { Colors::DmgLoggerOutline });

				g_Draw.Rect(g_ScreenSize.w - wideth - 20, 10, wideth + 10, heighth + 2, { 44, 49, 56, 100 });

				g_Draw.String(FONT_MENU, g_ScreenSize.w - (wideth / 2) - 15, 10, { 255, 255, 255 ,250 }, ALIGN_CENTERHORIZONTAL, watermarktext.str().c_str());

			}
			//Debug
			{
				
			}
			g_Misc.BypassPure();
			g_ESP.Run();
			g_SpyWarning.Run();
			g_PlayerArrows.Run();
			g_SpectatorList.Run();
			g_Radar.Run();
			g_Keybinds.Run();
			g_Crits.Frame();
			g_Menu.Run();
		}
		FinishDrawing(g_Interfaces.Surface);
	}
}
