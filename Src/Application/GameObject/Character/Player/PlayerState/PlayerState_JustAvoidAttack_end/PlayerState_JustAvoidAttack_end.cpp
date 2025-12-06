#include "PlayerState_JustAvoidAttack_end.h"
#include"../../../../../main.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../PlayerState_Run/PlayerState_Run.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"
#include "Application/GameObject/Utility/EffectReference.h"

#include"../PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include"../PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"

#include"Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"

void PlayerState_JustAvoidAttack_end::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("JustAvoidAttack");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	PlayerStateBase::StateStart();

	// 敵との当たり判定を無効化
	m_player->SetAtkPlayer(true);

	// アニメーション速度を変更
	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	// 複数エフェクト再生
	for (const auto& ref : m_playerEffects)
	{
		if (auto effect = ref->GetEffectBase().lock()) effect->SetPlayEffect(true);
	}

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	SceneManager::Instance().GetObjectWeakPtr(m_bossEnemy);
}

void PlayerState_JustAvoidAttack_end::StateUpdate()
{
	// 被ヒット判定無効化
	m_player->SetHitCheck(false);

	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<PlayerState_SheathKatana>();
		m_player->ChangeState(state);
		return;
	}

	// 当たり判定有効時間: 最初の0.5秒のみ
	m_player->UpdateAttackCollision(
		m_stateParameter.attackRadius,
		m_stateParameter.attackDistance,
		m_stateParameter.attackCount,
		m_stateParameter.attackInterval,
		m_stateParameter.cameraShake,
		m_stateParameter.cameraTime,
		m_stateParameter.attackStartTime,
		m_stateParameter.attackEndTime
	);

	UpdateKatanaPos();

	// 攻撃中の移動方向で回転を更新
	if (m_player->GetMovement() != Math::Vector3::Zero)
	{
		Math::Vector3 moveDir = m_player->GetMovement();
		moveDir.y = 0.0f;
		moveDir.Normalize();
		m_player->UpdateQuaternionDirect(moveDir);
	}

	// 索敵範囲もとに戻す
	m_searchEnemyRadius = DefaultSearchEnemyRadius;

	// 目標到達 → 停止して向きのみ合わせる
	m_player->SetIsMoving(Math::Vector3::Zero);

	// 最後に Base 側の StateUpdate を呼び出すことで、フォーカス/方向の追従が反映されます。
	PlayerStateBase::StateUpdate();
}

void PlayerState_JustAvoidAttack_end::StateEnd()
{
	PlayerStateBase::StateEnd();

	// 敵との当たり判定を有効化（押し出し処理を元に戻す）
	m_player->SetAtkPlayer(false);

	// 無敵解除
	m_player->SetInvincible(false);

	// スローモーション解除（ここを終点にする）
	Application::Instance().SetFpsScale(1.f);
	SceneManager::Instance().SetDrawGrayScale(false);

	// ジャスト回避フラグを戻す
	m_player->SetJustAvoidSuccess(false);
	m_player->SetJustAvoidAttackSuccess(false);

	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		if (auto bossEnemy = m_bossEnemy.lock())
		{
			camera->SetTargetLookAt(m_cameraBossTargetOffset);
		}
		else
		{
			camera->SetTargetLookAt(m_cameraTargetOffset);
		}
	}

	// 複数エフェクト停止
	for (const auto& ref : m_playerEffects)
	{
		if (auto effect = ref->GetEffectBase().lock()) effect->SetPlayEffect(false);
	}

	m_player->SetGuardBreak(false);

	// ゲームのメインサウンドのピッチを元に戻す
	if (auto bgm = SceneManager::Instance().GetGameSound())
	{
		bgm->SetPitch(0.0f);
	}
}

void PlayerState_JustAvoidAttack_end::ApplyFromConfig(const PlayerStateBase& other)
{
	assert(typeid(other) == typeid(PlayerState_JustAvoidAttack_end));
	const auto& p = static_cast<const PlayerState_JustAvoidAttack_end&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;

	// 当たり判定設定
	m_stateParameter.attackRadius = p.m_stateParameter.attackRadius;
	m_stateParameter.attackDistance = p.m_stateParameter.attackDistance;
	m_stateParameter.attackCount = p.m_stateParameter.attackCount;
	m_stateParameter.attackInterval = p.m_stateParameter.attackInterval;
	m_stateParameter.attackStartTime = p.m_stateParameter.attackStartTime;
	m_stateParameter.attackEndTime = p.m_stateParameter.attackEndTime;
	m_playerEffects = p.m_playerEffects;
}

void PlayerState_JustAvoidAttack_end::ExposeParametersImGui()
{
	// 複数エフェクトの編集UI
	ImGui::Text("Effects");
	for (size_t i = 0; i < m_playerEffects.size(); ++i)
	{
		ImGui::PushID(static_cast<int>(i));
		m_playerEffects[i]->ImGuiInspector("PlayerState_JustAvoidAttack_end_Effect");
		ImGui::PopID();
	}
	if (ImGui::SmallButton("+")) { m_playerEffects.emplace_back(std::make_shared<EffectReference>()); }
	if (ImGui::SmallButton("-")) { if (!m_playerEffects.empty()) m_playerEffects.pop_back(); }

	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);

	const float kLabelWidth = 160.0f;
	const float kItemWidth = 180.0f;

	// 当たり判定
	if (ImGui::CollapsingHeader(U8("当たり判定"), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::BeginTable("tbl_hit", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, kLabelWidth);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

			// 半径
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("当たり判定の半径"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderFloat("##attackRadius", &m_stateParameter.attackRadius, 0.0f, 10.0f, "%.2f");

			// 距離
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("当たり判定の距離"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderFloat("##attackDistance", &m_stateParameter.attackDistance, 0.0f, 10.0f, "%.2f");

			// 回数
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("当たり判定の回数"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderInt("##attackCount", &m_stateParameter.attackCount, 0, 10);

			// 間隔
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("当たり判定の間隔(秒)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderFloat("##attackInterval", &m_stateParameter.attackInterval, 0.0f, 1.0f, "%.03f");

			// 開始/終了時間（同時編集）
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("当たり判定の時間範囲(秒)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth * 1.3f);
			ImGui::DragFloatRange2("##attackTimeRange", &m_stateParameter.attackStartTime, &m_stateParameter.attackEndTime, 0.01f, 0.0f, 10.0f,
				U8("開始: %.02f"), U8("終了: %.02f"));
			if (m_stateParameter.attackEndTime < m_stateParameter.attackStartTime) m_stateParameter.attackEndTime = m_stateParameter.attackStartTime;

			ImGui::EndTable();
		}
	}
}

void PlayerState_JustAvoidAttack_end::LoadParametersJson(const nlohmann::json& js)
{
	// エフェクト
	if (js.contains("PlayerState_JustAvoidAttack_end_Effects") && js["PlayerState_JustAvoidAttack_end_Effects"].is_array())
	{
		m_playerEffects.clear();
		for (const auto& node : js["PlayerState_JustAvoidAttack_end_Effects"])
		{
			auto ref = std::make_shared<EffectReference>();
			ref->JsonInput("Effect", node);
			m_playerEffects.emplace_back(std::move(ref));
		}
	}

	if (!js.contains("PlayerState_JustAvoidAttack_end")) return;
	const auto& stateNode = js["PlayerState_JustAvoidAttack_end"];
	if (stateNode.contains("Player"))
	{
		const auto& playerNode = stateNode["Player"];
		if (playerNode.contains("blendTime")) m_stateParameter.blendTime = playerNode["blendTime"].get<float>();
		if (playerNode.contains("animationSpeed")) m_stateParameter.animationSpeed = playerNode["animationSpeed"].get<float>();

		// 当たり判定設定
		if (playerNode.contains("attackRadius")) m_stateParameter.attackRadius = playerNode["attackRadius"].get<float>();
		if (playerNode.contains("attackDistance")) m_stateParameter.attackDistance = playerNode["attackDistance"].get<float>();
		if (playerNode.contains("attackCount")) m_stateParameter.attackCount = playerNode["attackCount"].get<int>();
		if (playerNode.contains("attackInterval")) m_stateParameter.attackInterval = playerNode["attackInterval"].get<float>();
		if (playerNode.contains("attackStartTime")) m_stateParameter.attackStartTime = playerNode["attackStartTime"].get<float>();
		if (playerNode.contains("attackEndTime")) m_stateParameter.attackEndTime = playerNode["attackEndTime"].get<float>();
	}
}

void PlayerState_JustAvoidAttack_end::SaveParametersJson(nlohmann::json& js) const
{
	// エフェクト
	nlohmann::json arr = nlohmann::json::array();
	for (const auto& ref : m_playerEffects)
	{
		nlohmann::json item = nlohmann::json::object();
		ref->JsonSave("Effect", item);
		arr.push_back(item);
	}
	js["PlayerState_JustAvoidAttack_end_Effects"] = std::move(arr);

	if (!js.contains("PlayerState_JustAvoidAttack_end")) js["PlayerState_JustAvoidAttack_end"] = nlohmann::json::object();
	auto& stateNode = js["PlayerState_JustAvoidAttack_end"];

	stateNode["Player"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["Player"]["animationSpeed"] = m_stateParameter.animationSpeed;

	// 当たり判定設定
	stateNode["Player"]["attackRadius"] = m_stateParameter.attackRadius;
	stateNode["Player"]["attackDistance"] = m_stateParameter.attackDistance;
	stateNode["Player"]["attackCount"] = m_stateParameter.attackCount;
	stateNode["Player"]["attackInterval"] = m_stateParameter.attackInterval;
	stateNode["Player"]["attackStartTime"] = m_stateParameter.attackStartTime;
	stateNode["Player"]["attackEndTime"] = m_stateParameter.attackEndTime;
}
