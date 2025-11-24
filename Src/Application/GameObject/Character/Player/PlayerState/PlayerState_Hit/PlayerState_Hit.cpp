#include "PlayerState_Hit.h"
#include"../PlayerState_Idle/PlayerState_Idle.h"
#include"../PlayerState_Attack/PlayerState_Attack.h"

#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../../main.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"

void PlayerState_Hit::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("Hit");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);

	PlayerStateBase::StateStart();

	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);
}

void PlayerState_Hit::StateUpdate()
{

	// アニメーション時間のデバッグ表示
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();
	}

	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto idleState = std::make_shared<PlayerState_Idle>();
		m_player->ChangeState(idleState);
		return;
	}

	// 回避入力処理
	if (UpdateMoveAvoidInput()) return;

	// 必殺技入力処理
	if (UpdateSpecialAttackInput()) return;

	// Eスキル入力処理
	if (UpdateESkillInput()) return;

	PlayerStateBase::StateUpdate();

	UpdateKatanaPos();

	//0.9
	if (m_animeTime >= m_stateParameter.changeStateTime)
	{
		// 押された瞬間
		if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON)) m_lButtonKeyInput = true; // 判定開始

		// 攻撃入力受付
		if (UpdateAttackInput<PlayerState_Attack>()) return;
	}


	float deltaTime = Application::Instance().GetDeltaTime();

	if (m_time < m_stateParameter.dashSpeedTime)
	{
		m_player->SetIsMoving(m_attackDirection * m_stateParameter.dashSpeed);
		m_time += deltaTime;
	}
	else
	{
		m_player->SetIsMoving(Math::Vector3::Zero);
	}
}

void PlayerState_Hit::StateEnd()
{
	PlayerStateBase::StateEnd();
	m_player->SetHitCheck(false);
}

void PlayerState_Hit::ApplyFromConfig(const PlayerStateBase& other)
{
	assert(typeid(other) == typeid(PlayerState_Hit));
	const auto& p = static_cast<const PlayerState_Hit&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_stateParameter.dashSpeed = p.m_stateParameter.dashSpeed;
	m_stateParameter.dashSpeedTime = p.m_stateParameter.dashSpeedTime;
	m_stateParameter.changeStateTime = p.m_stateParameter.changeStateTime;
}

void PlayerState_Hit::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);
	ImGui::Separator();
	ImGui::DragFloat(U8("ダッシュ速度"), &m_stateParameter.dashSpeed);
	ImGui::DragFloat(U8("ダッシュ速度時間"), &m_stateParameter.dashSpeedTime);
	ImGui::Separator();
	ImGui::DragFloat(U8("状態遷移時間"), &m_stateParameter.changeStateTime);
}

void PlayerState_Hit::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("PlayerState_Hit")) return;
	const auto& stateNode = js["PlayerState_Hit"];
	if (stateNode.contains("Player"))
	{
		const auto& playerNode = stateNode["Player"];
		if (playerNode.contains("blendTime")) m_stateParameter.blendTime = playerNode["blendTime"].get<float>();
		if (playerNode.contains("animationSpeed")) m_stateParameter.animationSpeed = playerNode["animationSpeed"].get<float>();
		if (playerNode.contains("dashSpeed")) m_stateParameter.dashSpeed = playerNode["dashSpeed"].get<float>();
		if (playerNode.contains("dashSpeedTime")) m_stateParameter.dashSpeedTime = playerNode["dashSpeedTime"].get<float>();
		if (playerNode.contains("changeStateTime")) m_stateParameter.changeStateTime = playerNode["changeStateTime"].get<float>();
	}
}

void PlayerState_Hit::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("PlayerState_Hit")) js["PlayerState_Hit"] = nlohmann::json::object();
	auto& stateNode = js["PlayerState_Hit"];

	stateNode["Player"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["Player"]["animationSpeed"] = m_stateParameter.animationSpeed;
	stateNode["Player"]["dashSpeed"] = m_stateParameter.dashSpeed;
	stateNode["Player"]["dashSpeedTime"] = m_stateParameter.dashSpeedTime;
	stateNode["Player"]["changeStateTime"] = m_stateParameter.changeStateTime;
}
