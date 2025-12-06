#include "PlayerState_SpecialAttack.h"

#include "Application/main.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_SpecialAttack1/PlayerState_SpecialAttack1.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"
#include "Application/Scene/SceneManager.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"
#include "Application/GameObject/Utility/EffectReference.h"

#include "MyFramework/Manager/JsonManager/JsonManager.h"

void PlayerState_SpecialAttack::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("PowerAttack");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);

	PlayerStateBase::StateStart();

	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	// サウンド再生フラグ初期化
	m_playSound = false;
}

void PlayerState_SpecialAttack::StateUpdate()
{
	// アニメーション時間の取得
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();
	}

	if (m_animeTime >= m_stateParameter.changeStateTime)
	{
		// カメラの位置をImGUiで変更する
		if (auto camera = m_player->GetPlayerCamera().lock())
		{
			camera->SetTargetLookAt(m_lastCameraPos);
		}

		auto state = std::make_shared<PlayerState_SpecialAttack1>();
		m_player->ChangeState(state);
		return;
	}

	if (m_animeTime >= 0.15f)
	{
		// エフェクト再生・移動停止（複数）
		for (const auto& ref : m_playerEffects)
		{
			if (auto effect = ref->GetEffectBase().lock())
			{
				effect->PlayForTarget<Player>(std::static_pointer_cast<Player>(m_player->GetMyAdls()));
			}
		}

		// 当たり判定
		m_player->UpdateAttackCollision
		(
			m_stateParameter.attackRadius,
			m_stateParameter.attackDistance,
			m_stateParameter.attackCount,
			m_stateParameter.attackInterval,
			m_stateParameter.cameraShake,
			m_stateParameter.cameraTime,
			m_stateParameter.attackStartTime,
			m_stateParameter.attackEndTime
		);

		if (!m_playSound)
		{
			KdAudioManager::Instance().Play("Asset/Sound/Player/SpecialAttack.WAV", false)->SetVolume(0.5f);
			m_playSound = true;
		}
	}

	Math::Vector3 toEnemyDir = m_nearestEnemyPos - m_player->GetPos();
	
	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		// キャラ前方からヨー角(deg)を計算してカメラ回転に反映
		toEnemyDir.Normalize();
		const float yawRad = std::atan2(toEnemyDir.x, toEnemyDir.z);
		const float yawDeg = DirectX::XMConvertToDegrees(yawRad);
		camera->SetPlayerRotation({ 0.0f, yawDeg , 0.0f });
	}

	m_player->SetIsMoving(m_attackDirection);

	UpdateKatanaPos();
	
	// 移動を止める
	m_player->SetIsMoving(Math::Vector3::Zero);
}

void PlayerState_SpecialAttack::StateEnd()
{
	PlayerStateBase::StateEnd();

	// エフェクト停止（複数）
	for (const auto& ref : m_playerEffects)
	{
		if (auto effect = ref->GetEffectBase().lock())
		{
			effect->StopEffect();
		}
	}
}

void PlayerState_SpecialAttack::ApplyFromConfig(const PlayerStateBase& other)
{
	assert(typeid(other) == typeid(PlayerState_SpecialAttack));
	const auto& p = static_cast<const PlayerState_SpecialAttack&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;

	// 当たり判定設定
	m_stateParameter.attackRadius = p.m_stateParameter.attackRadius;
	m_stateParameter.changeStateTime = p.m_stateParameter.changeStateTime;
	m_stateParameter.attackCount = p.m_stateParameter.attackCount;
	m_stateParameter.attackInterval = p.m_stateParameter.attackInterval;
	m_stateParameter.attackStartTime = p.m_stateParameter.attackStartTime;
	m_stateParameter.attackEndTime = p.m_stateParameter.attackEndTime;
	m_lastCameraPos = p.m_lastCameraPos;
	m_playerEffects = p.m_playerEffects;
}

void PlayerState_SpecialAttack::ExposeParametersImGui()
{
	// 複数エフェクトの編集UI
	ImGui::Text("Effects");
	for (size_t i = 0; i < m_playerEffects.size(); ++i)
	{
		ImGui::PushID(static_cast<int>(i));
		m_playerEffects[i]->ImGuiInspector("PlayerState_SpecialAttack_Effect");
		ImGui::PopID();
	}
	if (ImGui::SmallButton("+")) { m_playerEffects.emplace_back(std::make_shared<EffectReference>()); }
	if (ImGui::SmallButton("-")) { if (!m_playerEffects.empty()) m_playerEffects.pop_back(); }

	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed, 1.0f, 1.0f, 120.0f);
	ImGui::DragFloat(U8("状態遷移時間"), &m_stateParameter.changeStateTime, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat3(U8("最後のカメラ位置"), &m_lastCameraPos.x);

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

void PlayerState_SpecialAttack::LoadParametersJson(const nlohmann::json& js)
{
	// エフェクト
	if (js.contains("PlayerState_SpecialAttack_Effects") && js["PlayerState_SpecialAttack_Effects"].is_array())
	{
		m_playerEffects.clear();
		for (const auto& node : js["PlayerState_SpecialAttack_Effects"])
		{
			auto ref = std::make_shared<EffectReference>();
			ref->JsonInput("Effect", node);
			m_playerEffects.emplace_back(std::move(ref));
		}
	}

	if (!js.contains("PlayerState_SpecialAttack")) return;
	const auto& stateNode = js["PlayerState_SpecialAttack"];
	if (stateNode.contains("Player"))
	{
		const auto& playerNode = stateNode["Player"];
		if (playerNode.contains("blendTime")) m_stateParameter.blendTime = playerNode["blendTime"].get<float>();
		if (playerNode.contains("animationSpeed")) m_stateParameter.animationSpeed = playerNode["animationSpeed"].get<float>();
		if (playerNode.contains("lastCameraPos")) m_lastCameraPos = JSON_MANAGER.JsonToVector(playerNode["lastCameraPos"]);
		if (playerNode.contains("changeStateTime")) m_stateParameter.changeStateTime = playerNode["changeStateTime"].get<float>();

		// 当たり判定設定
		if (playerNode.contains("attackRadius")) m_stateParameter.attackRadius = playerNode["attackRadius"].get<float>();
		if (playerNode.contains("attackDistance")) m_stateParameter.attackDistance = playerNode["attackDistance"].get<float>();
		if (playerNode.contains("attackCount")) m_stateParameter.attackCount = playerNode["attackCount"].get<int>();
		if (playerNode.contains("attackInterval")) m_stateParameter.attackInterval = playerNode["attackInterval"].get<float>();
		if (playerNode.contains("attackStartTime")) m_stateParameter.attackStartTime = playerNode["attackStartTime"].get<float>();
		if (playerNode.contains("attackEndTime")) m_stateParameter.attackEndTime = playerNode["attackEndTime"].get<float>();
	}
}

void PlayerState_SpecialAttack::SaveParametersJson(nlohmann::json& js) const
{
	// エフェクト
	nlohmann::json arr = nlohmann::json::array();
	for (const auto& ref : m_playerEffects)
	{
		nlohmann::json item = nlohmann::json::object();
		ref->JsonSave("Effect", item);
		arr.push_back(item);
	}
	js["PlayerState_SpecialAttack_Effects"] = std::move(arr);

	if (!js.contains("PlayerState_SpecialAttack")) js["PlayerState_SpecialAttack"] = nlohmann::json::object();
	auto& stateNode = js["PlayerState_SpecialAttack"];

	stateNode["Player"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["Player"]["animationSpeed"] = m_stateParameter.animationSpeed;
	stateNode["Player"]["lastCameraPos"] = JSON_MANAGER.VectorToJson(m_lastCameraPos);
	stateNode["Player"]["changeStateTime"] = m_stateParameter.changeStateTime;

	// 当たり判定設定
	stateNode["Player"]["attackRadius"] = m_stateParameter.attackRadius;
	stateNode["Player"]["attackDistance"] = m_stateParameter.attackDistance;
	stateNode["Player"]["attackCount"] = m_stateParameter.attackCount;
	stateNode["Player"]["attackInterval"] = m_stateParameter.attackInterval;
	stateNode["Player"]["attackStartTime"] = m_stateParameter.attackStartTime;
	stateNode["Player"]["attackEndTime"] = m_stateParameter.attackEndTime;
}
