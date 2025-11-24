#include "PlayerState_SpecialAttack.h"

#include"../../../../../main.h"
#include"../PlayerState_SpecialAttack1/PlayerState_SpecialAttack1.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/SpecialAttack/SpecialAttack.h"

void PlayerState_SpecialAttack::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("PowerAttack");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);

	PlayerStateBase::StateStart();

	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	// 攻撃時はtrueにする
	for (const auto& katanaWeak : m_player->GetKatanas())
	{
		if (auto katana = katanaWeak.lock())
		{
			katana->SetNowAttackState(true);
		}
	}

	SceneManager::Instance().GetObjectWeakPtr(m_specialAttackEffect);

	// アニメーション再生速度を変更
	m_player->SetAnimeSpeed(60.0f);

	if (auto camera = m_player->GetPlayerCamera().lock(); camera)
	{
		camera->SetTargetLookAt({ 0.0f,1.0f,-3.5f });
	}

	// 当たり判定リセット
	m_player->ResetAttackCollision();
	m_playSound = false;
}

void PlayerState_SpecialAttack::StateUpdate()
{

	// アニメーション時間のデバッグ表示
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();
	}

	if (m_animeTime >= m_stateParameter.changeStateTime)
	{
		auto state = std::make_shared<PlayerState_SpecialAttack1>();
		m_player->ChangeState(state);
		return;
	}

	if (m_animeTime >= 0.15f)
	{
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


	Math::Vector3 moveDir = m_player->GetMovement();

	// 攻撃中の移動方向で回転を更新
	if (m_player->GetMovement() != Math::Vector3::Zero)
	{
		moveDir.y = 0.0f;
		moveDir.Normalize();
		m_player->UpdateQuaternionDirect(moveDir);
	}

	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		// キャラ前方からヨー角(deg)を計算してカメラ回転に反映
		if (moveDir != Math::Vector3::Zero)
		{
			moveDir.Normalize();
			m_yawRad = std::atan2(moveDir.x, moveDir.z);
			m_yawDeg = DirectX::XMConvertToDegrees(m_yawRad);
			camera->SetTargetRotation({ 10.0f, m_yawDeg , 0.0f });
		}
	}

	if (m_animeTime >= 0.55f)
	{
		if (auto camera = m_player->GetPlayerCamera().lock())
		{
			camera->SetTargetLookAt({ 0.0f,1.0f,-4.0f });
			camera->SetTargetRotation({ -10.0f, m_yawDeg , 0.0f });
		}
	}

	m_player->SetIsMoving(m_attackDirection);

	UpdateKatanaPos();

	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	if (m_time >= 1.0f)
	{
		if (auto effect = m_specialAttackEffect.lock())
		{
			effect->SetPlayEffect(true);
		}
	}
	
	// 移動を止める
	m_player->SetIsMoving(Math::Vector3::Zero);
}

void PlayerState_SpecialAttack::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		// カメラと、プレイヤーの距離を近づける（らーぷ）
		camera->SetTargetLookAt({ 0.0f,1.0f,-3.5f });
	}

	if (auto effect = m_specialAttackEffect.lock())
	{
		effect->SetPlayEffect(false);
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
}

void PlayerState_SpecialAttack::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed, 1.0f, 1.0f, 120.0f);
	ImGui::DragFloat(U8("状態遷移時間"), &m_stateParameter.changeStateTime, 0.01f, 0.0f, 10.0f);

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
	if (!js.contains("PlayerState_JustAvoidAttack")) return;
	const auto& stateNode = js["PlayerState_JustAvoidAttack"];
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

void PlayerState_SpecialAttack::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("PlayerState_JustAvoidAttack")) js["PlayerState_JustAvoidAttack"] = nlohmann::json::object();
	auto& stateNode = js["PlayerState_JustAvoidAttack"];

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
