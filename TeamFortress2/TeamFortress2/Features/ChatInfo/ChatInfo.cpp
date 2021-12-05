#include "ChatInfo.h"

#include "../Vars.h"
#include "../Misc/Misc.h"
#include "../Crits/Crits.h"

int attackStringW;
int attackStringH;

#define GET_PLAYER_USERID(userid) g_Interfaces.EntityList->GetClientEntity(g_Interfaces.Engine->GetPlayerForUserID(userid))
#define GET_INDEX_USERID(userid) g_Interfaces.Engine->GetPlayerForUserID(userid)

void CChatInfo::Event(CGameEvent* pEvent, const FNV1A_t uNameHash) {
	if (!g_Interfaces.Engine->IsConnected() || !g_Interfaces.Engine->IsInGame())
		return;

	static std::string clr({ '\x7', '0', 'D', '9', '2', 'F', 'F' });

	if (const auto pLocal = g_EntityCache.m_pLocal) {
		if (Vars::Misc::VoteRevealer.m_Var && uNameHash == FNV1A::HashConst("vote_started")) {
			const auto initiator = pEvent->GetInt("initiator");
			const auto target = pEvent->GetString("param1"); // im almost certain this is the param for the targets name 

			auto pEntity = g_Interfaces.EntityList->GetClientEntity(initiator);
			bool ourteam = (pEntity->GetTeamNum() == pLocal->GetTeamNum());

			PlayerInfo_t pii;
			g_Interfaces.Engine->GetPlayerInfo(initiator, &pii);

			g_Interfaces.ClientMode->m_pChatElement->ChatPrintf(0, tfm::format("%s[dp]%sVote Initiator: \x3%s", clr, std::string(ourteam ? " " : " [enemy] "), pii.name).c_str());
			if (pEvent->GetString("issue") == "#TF_vote_kick_player_other") { // if someone is calling a vote to like do something else why would we want to print the vote target
				g_Interfaces.ClientMode->m_pChatElement->ChatPrintf(0, tfm::format("%s[dp]%sVote Target: \x3%s", clr, std::string(ourteam ? " " : " [enemy] "), target).c_str());
			}
		}
		if (Vars::Misc::VoteRevealer.m_Var && uNameHash == FNV1A::HashConst("vote_cast")) {
			const auto pEntity = g_Interfaces.EntityList->GetClientEntity(pEvent->GetInt("entityid"));
			if (pEntity == pLocal) { return; }
			if (pEntity && pEntity->IsPlayer()) {
				const bool bVotedYes = pEvent->GetInt("vote_option") == 0;
				PlayerInfo_t pi;
				g_Interfaces.Engine->GetPlayerInfo(pEntity->GetIndex(), &pi);
				// TODO: Add colors and possibly OTHER TEAM? 
				std::string voteString;
				//g_Interfaces.ClientMode->m_pChatElement->ChatPrintf(0, tfm::format("%s[dp] \x3%s voted %s", clr, pi.name, bVotedYes ? "Yes" : "No").c_str());
				if (pEntity->GetTeamNum() != pLocal->GetTeamNum())
				{
					voteString = "[dp] [enemy] " + std::string(pi.name) + " voted " + std::string(bVotedYes ? "Yes" : "No");
				}
				else
				{
					voteString = "[dp] " + std::string(pi.name) + " voted " + std::string(bVotedYes ? "Yes" : "No");
				}
				std::wstring wvoteString = std::wstring(voteString.begin(), voteString.end());
				g_notify.Add(voteString);

				if (Vars::Misc::VotesInChat.m_Var) {
					g_Interfaces.Engine->ClientCmd_Unrestricted(tfm::format("say_party [deathpole] \"%s voted %s\"", pi.name, bVotedYes ? "Yes" : "No").c_str());
				}
			}
		}
		if (Vars::Visuals::ChatInfo.m_Var) {
			if (uNameHash == FNV1A::HashConst("player_changeclass")) {
				if (const auto& pEntity = g_Interfaces.EntityList->GetClientEntity(g_Interfaces.Engine->GetPlayerForUserID(pEvent->GetInt("userid")))) {
					if (pEntity == pLocal) { return; }
					int nIndex = pEntity->GetIndex();

					PlayerInfo_t pi;
					g_Interfaces.Engine->GetPlayerInfo(nIndex, &pi);

					std::string classString;

					if (pEntity->GetTeamNum() != pLocal->GetTeamNum())
					{
						classString = "[dp] [enemy] " + std::string(pi.name) + " is now a " + std::string(Utils::GetClassByIndex(pEvent->GetInt("class"))).c_str();
					}
					else
					{
						classString = "[dp] " + std::string(pi.name) + " is now a " + std::string(Utils::GetClassByIndex(pEvent->GetInt("class"))).c_str();
					}
					
					std::wstring wclassString = std::wstring(classString.begin(), classString.end());
					g_notify.Add(classString);

					//g_Interfaces.ClientMode->m_pChatElement->ChatPrintf(nIndex, tfm::format("%s[dp] \x3%s\x1 is now a \x3%s\x1!", clr, pi.name, Utils::GetClassByIndex(pEvent->GetInt("class"))).c_str());
				}
			}

			if (uNameHash == FNV1A::HashConst("player_connect")) {
				g_Interfaces.ClientMode->m_pChatElement->ChatPrintf(GET_INDEX_USERID(pEvent->GetInt(_("userid"))), tfm::format("\x3%s\x1 connected. (%s)", pEvent->GetString("name"), pEvent->GetString("address")).c_str());
			}
		}

		if (Vars::Visuals::damageLogger.m_Var && uNameHash == FNV1A::HashConst("player_hurt")) {
			if (const auto pEntity = g_Interfaces.EntityList->GetClientEntity(g_Interfaces.Engine->GetPlayerForUserID(pEvent->GetInt("userid")))) {
				if (pEntity == pLocal) { return; }
				const auto nAttacker = pEvent->GetInt("attacker");
				const auto nHealth = pEvent->GetInt("health");
				const auto nDamage = pEvent->GetInt("damageamount");
				const auto bCrit = pEvent->GetBool("crit");
				const int  nIndex = pEntity->GetIndex();

				PlayerInfo_t pi;

				{
					g_Interfaces.Engine->GetPlayerInfo(g_Interfaces.Engine->GetLocalPlayer(), &pi);
					if (nAttacker != pi.userID)
						return;
				}

				g_Interfaces.Engine->GetPlayerInfo(nIndex, &pi);

				g_Crits.RoundDamage += nDamage;

				if (bCrit) {
					g_Crits.CritDamage += (float)nDamage;
				}

				const auto maxHealth = pEntity->GetMaxHealth();
				std::string attackString = "[dp] You hit " + std::string(pi.name) + " for " + std::to_string(nDamage) + (bCrit ? " (crit) " : " ") + "(" + std::to_string(nHealth) + "/" + std::to_string(maxHealth) + ")";

				std::wstring wattackString = std::wstring(attackString.begin(), attackString.end());
				const wchar_t* wcattackString = wattackString.c_str();
				g_Interfaces.Surface->GetTextSize(g_Draw.m_vecFonts[FONT_INDICATORS].dwFont, wcattackString, attackStringW, attackStringH);

				if (Vars::Visuals::damageLogger.m_Var == 1 && Vars::Visuals::ChatInfo.m_Var)
					g_Interfaces.ClientMode->m_pChatElement->ChatPrintf(0, tfm::format("%s[dp]\x3 %s", clr, attackString.c_str()).c_str());

				if (Vars::Visuals::damageLogger.m_Var == 2)
					g_notify.Add(attackString);
			}
		}

		if (Vars::Aimbot::Global::showHitboxes.m_Var && uNameHash == FNV1A::HashConst("player_hurt")){
			if (Vars::Aimbot::Global::clearPreviousHitbox.m_Var) { g_Interfaces.DebugOverlay->ClearAllOverlays(); }
			auto time = Vars::Aimbot::Global::hitboxTime.m_Var;
			auto colour = Colors::Hitbox;
			auto pEntity = g_Interfaces.EntityList->GetClientEntity(g_Interfaces.Engine->GetPlayerForUserID(pEvent->GetInt("userid")));
			const auto nAttacker = g_Interfaces.EntityList->GetClientEntity(g_Interfaces.Engine->GetPlayerForUserID(pEvent->GetInt("attacker")));
			if (pEntity == pLocal) { return; }; if (pLocal != nAttacker) { return; }
			const model_t* model;
			studiohdr_t* hdr;
			mstudiohitboxset_t* set;
			mstudiobbox_t* bbox;
			Vec3 mins{}, maxs{}, origin{};
			Vec3 angle;

			model = pEntity->GetModel();
			hdr = g_Interfaces.ModelInfo->GetStudioModel(model);
			set = hdr->GetHitboxSet(pEntity->GetHitboxSet());

			for (int i{}; i < set->numhitboxes; ++i) {
				bbox = set->hitbox(i);
				if (!bbox)
					continue;

				//nigga balls
				matrix3x4 rot_matrix;
				Math::AngleMatrix(bbox->angle, rot_matrix);

				matrix3x4 matrix;
				matrix3x4 boneees[128];
				pEntity->SetupBones(boneees, 128, BONE_USED_BY_ANYTHING, g_Interfaces.GlobalVars->curtime);
				Math::ConcatTransforms(boneees[bbox->bone], rot_matrix, matrix);

				Vec3 bbox_angle;
				Math::MatrixAngles(matrix, bbox_angle);

				Vec3 matrix_origin;
				Math::GetMatrixOrigin(matrix, matrix_origin);

				g_Interfaces.DebugOverlay->AddBoxOverlay(matrix_origin, bbox->bbmin, bbox->bbmax, bbox_angle, colour.r, colour.g, colour.b, colour.a, time);
			}
		}

		if (uNameHash == FNV1A::HashConst("achievement_earned")) {
			const int player = pEvent->GetInt("player", 0xDEAD);
			const int achievement = pEvent->GetInt("achievement", 0xDEAD);

			// 0xCA7 is an identify and mark request.
			// 0xCA8 is a mark request.

			PlayerInfo_t info;
			if (g_Interfaces.Engine->GetPlayerInfo(player, &info) && (achievement == 0xCA7 || achievement == 0xCA8) && pLocal->GetIndex() != player) {
				if (m_known_bots.find(info.friendsID) == m_known_bots.end()) {
					g_notify.Add(tfm::format("%s is a bot!", info.name));
					if (Vars::Visuals::ChatInfo.m_Var)
						g_Interfaces.ClientMode->m_pChatElement->ChatPrintf(player, tfm::format("%s[dp] \x3 %s\x1 is a bot!", clr, info.name).c_str());

					{ // marked by other bots. r.i.p cl_drawline :(
						// this will be detected by fedoraware and lmaobox easily.
						// use 0xCA7 if you want to make more bots do the thing,
						// most only care about being marked.
						if (Vars::Misc::BeCat.m_Var) {
							KeyValues* kv = new KeyValues("AchievementEarned");
							kv->SetInt("achievementID", 0xCA8);
							g_Interfaces.Engine->ServerCmdKeyValues(kv);
						}
					}

					m_known_bots[info.friendsID] = true;
				}
			}
		}
	}
}