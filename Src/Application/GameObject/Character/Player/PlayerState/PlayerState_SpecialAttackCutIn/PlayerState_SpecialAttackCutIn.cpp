#include "PlayerState_SpecialAttackCutIn.h"
#include"../PlayerState_SpecialAttack/PlayerState_SpecialAttack.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"
#include"../../../../Weapon/Katana/Katana.h"

#include"../../../../../main.h"

void PlayerState_SpecialAttackCutIn::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("CutIn");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);

	PlayerStateBase::StateStart();

	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	// カメラの位置を変更
	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt(m_cameraCutInOffset);
		camera->SetRotationSmooth(m_cameraStartRotationSmooth);
		camera->SetDistanceSmooth(m_cameraStartDistanceSmooth);
	}

	// 無敵状態にする
	m_player->SetInvincible(true);

}

void PlayerState_SpecialAttackCutIn::StateUpdate()
{
	// アニメーション時間のデバッグ表示
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();
	}

	Math::Vector3 toEnemyDir = m_nearestEnemyPos - m_player->GetPos();
	
	if (toEnemyDir != Math::Vector3::Zero)
	{
		toEnemyDir.y = 0.0f;
		toEnemyDir.Normalize();
		m_player->UpdateQuaternionDirect(toEnemyDir);
	}

	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		// キャラ前方からヨー角(deg)を計算してカメラ回転に反映

		toEnemyDir.Normalize();
		const float yawRad = std::atan2(-toEnemyDir.x, -toEnemyDir.z);
		const float yawDeg = DirectX::XMConvertToDegrees(yawRad);
		camera->SetPlayerRotation({ 0.0f, yawDeg , 0.0f });
	}

	if (m_animeTime >= 0.5f)
	{
		if (auto camera = m_player->GetPlayerCamera().lock())
		{
			camera->SetTargetLookAt(m_cameraCutInOffset);
		}
	}

	m_player->SetIsMoving(m_attackDirection * m_stateParameter.moveSpeed);

	UpdateKatanaPos();

	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		// 必殺技状態へ
		auto state = std::make_shared<PlayerState_SpecialAttack>();
		m_player->ChangeState(state);
		return;
	}
}

void PlayerState_SpecialAttackCutIn::StateEnd()
{
	PlayerStateBase::StateEnd();
	// カメラの位置を変更
	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		camera->SetRotationSmooth(m_cameraRotationSmooth);
		camera->SetDistanceSmooth(m_cameraDistanceSmooth);
	}
}

void PlayerState_SpecialAttackCutIn::ApplyFromConfig(const PlayerStateBase& other)
{
	assert(typeid(other) == typeid(PlayerState_SpecialAttackCutIn));
	const auto& p = static_cast<const PlayerState_SpecialAttackCutIn&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_cameraStartRotationSmooth = p.m_cameraStartRotationSmooth;
	m_cameraStartDistanceSmooth = p.m_cameraStartDistanceSmooth;
	m_stateParameter.moveSpeed = p.m_stateParameter.moveSpeed;
	m_cameraCutInOffset = p.m_cameraCutInOffset;
	m_cameraCutInRotation = p.m_cameraCutInRotation;
}

void PlayerState_SpecialAttackCutIn::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);
	ImGui::DragFloat(U8("カメラ回転スムーズ"), &m_cameraStartRotationSmooth);
	ImGui::DragFloat(U8("カメラ距離スムーズ"), &m_cameraStartDistanceSmooth);
	ImGui::DragFloat(U8("移動速度"), &m_stateParameter.moveSpeed.x);
	ImGui::DragFloat3(U8("カメラカットイン時オフセット"), &m_cameraCutInOffset.x);
	ImGui::DragFloat3(U8("カメラカットイン時回転"), &m_cameraCutInRotation.x);
}

void PlayerState_SpecialAttackCutIn::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("PlayerState_JustAvoidAttack")) return;
	const auto& stateNode = js["PlayerState_JustAvoidAttack"];
	if (stateNode.contains("Player"))
	{
		const auto& playerNode = stateNode["Player"];
		if (playerNode.contains("blendTime")) m_stateParameter.blendTime = playerNode["blendTime"].get<float>();
		if (playerNode.contains("animationSpeed")) m_stateParameter.animationSpeed = playerNode["animationSpeed"].get<float>();
		if (playerNode.contains("cameraStartRotationSmooth")) m_cameraStartRotationSmooth = playerNode["cameraStartRotationSmooth"].get<float>();
		if (playerNode.contains("cameraStartDistanceSmooth")) m_cameraStartDistanceSmooth = playerNode["cameraStartDistanceSmooth"].get<float>();
		if (playerNode.contains("moveSpeed")) m_stateParameter.moveSpeed = JSON_MANAGER.JsonToVector(playerNode["moveSpeed"]);
		if (playerNode.contains("cameraCutInOffset")) m_cameraCutInOffset = JSON_MANAGER.JsonToVector(playerNode["cameraCutInOffset"]);
		if (playerNode.contains("cameraCutInRotation")) m_cameraCutInRotation = JSON_MANAGER.JsonToVector(playerNode["cameraCutInRotation"]);
	}
}

void PlayerState_SpecialAttackCutIn::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("PlayerState_JustAvoidAttack")) js["PlayerState_JustAvoidAttack"] = nlohmann::json::object();
	auto& stateNode = js["PlayerState_JustAvoidAttack"];

	stateNode["Player"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["Player"]["animationSpeed"] = m_stateParameter.animationSpeed;
	stateNode["Player"]["cameraStartRotationSmooth"] = m_cameraStartRotationSmooth;
	stateNode["Player"]["cameraStartDistanceSmooth"] = m_cameraStartDistanceSmooth;
	stateNode["Player"]["moveSpeed"] = JSON_MANAGER.VectorToJson(m_stateParameter.moveSpeed);
	stateNode["Player"]["cameraCutInOffset"] = JSON_MANAGER.VectorToJson(m_cameraCutInOffset);
	stateNode["Player"]["cameraCutInRotation"] = JSON_MANAGER.VectorToJson(m_cameraCutInRotation);
}
