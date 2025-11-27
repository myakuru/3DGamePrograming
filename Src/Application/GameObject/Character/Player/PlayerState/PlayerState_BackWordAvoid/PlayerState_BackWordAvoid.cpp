#include "PlayerState_BackWordAvoid.h"
#include"../../../../../main.h"
#include"../../../../Weapon/Katana/Katana.h"

#include"../PlayerState_Idle/PlayerState_Idle.h"
#include"../PlayerState_Run/PlayerState_Run.h"
#include"../../../../Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include"../PlayerState_AvoidAttack/PlayerState_AvoidAttack.h"
#include"Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"

#include"../PlayerState_Skill/PlayerState_Skill.h"
#include"../../../../../Scene/SceneManager.h"
#include"../PlayerState_JustAvoidAttack/PlayerState_JustAvoidAttack.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"

#include"Application/GameObject/Character/EnemyBase/AetheriusEnemy/AetheriusEnemy.h"
#include"Application/GameObject/Character/AfterImage/AfterImage.h"	

void PlayerState_BackWordAvoid::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("AvoidBackward");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);

	m_player->SetAvoidFlg(true);

	SceneManager::Instance().GetObjectWeakPtr(m_bossEnemy);

	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		if (auto bossEnemy = m_bossEnemy.lock())
		{
			camera->SetTargetLookAt(m_startBossCameraOffset);
		}
		else
		{
			camera->SetTargetLookAt(m_startCameraOffset);
		}
	}

	m_time = 0.0f;

	// アニメーション速度を変更
	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	// 回避時の処理
	m_player->SetAvoidStartTime(0.0f);

	m_afterImagePlayed = false;

	KdAudioManager::Instance().Play("Asset/Sound/Player/BackWardAvoid.WAV", false)->SetVolume(1.0f);

}

void PlayerState_BackWordAvoid::StateUpdate()
{

	PlayerStateBase::StateUpdate();

	float deltaTime = Application::Instance().GetUnscaledDeltaTime();
	m_time += deltaTime;

	// 経過時間を Player クラスに伝える
	m_player->SetAvoidStartTime(m_time);

	// ジャスト回避成立時の演出を一度だけ
	if (!m_afterImagePlayed && m_player->GetJustAvoidSuccess())
	{
		KdAudioManager::Instance().Play("Asset/Sound/Player/SlowMotion.WAV", false)->SetVolume(1.0f);

		if (auto bgm = SceneManager::Instance().GetGameSound())
		{
			bgm->SetPitch(-1.0f);
		}

		m_afterImagePlayed = true;

		if (auto camera = m_player->GetPlayerCamera().lock())
		{
			camera->SetTargetLookAt(m_justAvoidCameraOffset);
		}

		// 残像
		m_player->GetAfterImage()->AddAfterImage
		(
			true,
			m_stateParameter.afterImageMax,
			m_stateParameter.afterImageInterval,
			m_stateParameter.afterImageColor
		);

		// 無敵化(アニメーションが終了するまで)
		m_player->SetInvincible(true);
		// 被ヒット判定解除
		m_player->SetHitCheck(false);

		Application::Instance().SetFpsScale(0.1f);
		SceneManager::Instance().SetDrawGrayScale(true);
	}

	// 前方ベクトルを取得
	Math::Vector3 forward = Math::Vector3::TransformNormal(Math::Vector3::Forward, Math::Matrix::CreateFromQuaternion(m_player->GetRotationQuaternion()));
	forward.Normalize();

	if (m_player->GetJustAvoidSuccess())
	{
		if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
		{
			m_player->SetJustAvoidAttackSuccess(true);
			auto state = std::make_shared<PlayerState_JustAvoidAttack>();
			m_player->ChangeState(state);
			return;
		}
	}
	else
	{
		m_player->SetJustAvoidSuccess(false);

		// スローモーション解除（ここを終点にする）
		Application::Instance().SetFpsScale(1.0f);
		SceneManager::Instance().SetDrawGrayScale(false);

		// 回避中に攻撃ボタンが押されたら回避攻撃へ移行
		if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
		{
			auto state = std::make_shared<PlayerState_AvoidAttack>();
			m_player->ChangeState(state);
			return;
		}

		// 必殺技入力処理
		if (UpdateSpecialAttackInput()) return;

		// Eスキル入力処理
		if (UpdateESkillInput()) return;
	}

	// アニメーションが終了したらIdleへ移行
	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		// ジャスト回避フラグを戻す
		m_player->SetJustAvoidSuccess(false);

		m_player->SetJustAvoidAttackSuccess(false);

		if (auto bgm = SceneManager::Instance().GetGameSound())
		{
			bgm->SetPitch(0.0f);
		}

		// スローモーション解除（ここを終点にする）
		Application::Instance().SetFpsScale(1.f);
		SceneManager::Instance().SetDrawGrayScale(false);

		auto idleState = std::make_shared<PlayerState_Idle>();
		m_player->ChangeState(idleState);
		return;
	}

	// 刀は鞘の中にある状態
	UpdateUnsheathed();

	// 回避中の移動処理
	if (m_time < m_stateParameter.dashSpeedTime)
	{
		m_player->SetIsMoving(forward * m_stateParameter.dashSpeed);
	}
	else
	{
		m_player->SetIsMoving(Math::Vector3::Zero);
	}

}

void PlayerState_BackWordAvoid::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt(m_cameraTargetOffset);
	}

	m_player->SetAvoidFlg(false);
	m_player->SetAvoidStartTime(0.0f); // 現在の時間を記録

	m_player->SetHitCheck(false); // 被ヒット判定解除

	m_player->GetAfterImage()->AddAfterImage();

	// 敵との当たり判定を無効化解除（押し出し処理を元に戻す）
	m_player->SetAtkPlayer(false);

	m_player->SetInvincible(false);
		
}

void PlayerState_BackWordAvoid::ApplyFromConfig(const PlayerStateBase& other)
{
	assert(typeid(other) == typeid(PlayerState_BackWordAvoid));
	const auto& p = static_cast<const PlayerState_BackWordAvoid&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_stateParameter.dashSpeed = p.m_stateParameter.dashSpeed;
	m_stateParameter.dashSpeedTime = p.m_stateParameter.dashSpeedTime;
	m_startCameraOffset = p.m_startCameraOffset;
	m_startBossCameraOffset = p.m_startBossCameraOffset;
	m_justAvoidCameraOffset = p.m_justAvoidCameraOffset;

	// 残像設定
	m_stateParameter.afterImageMax = p.m_stateParameter.afterImageMax;
	m_stateParameter.afterImageInterval = p.m_stateParameter.afterImageInterval;
	m_stateParameter.afterImageColor = p.m_stateParameter.afterImageColor;
}

void PlayerState_BackWordAvoid::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);
	ImGui::Separator();
	ImGui::DragFloat(U8("ダッシュ速度"), &m_stateParameter.dashSpeed);
	ImGui::DragFloat(U8("ダッシュ速度時間"), &m_stateParameter.dashSpeedTime);
	ImGui::Separator();
	ImGui::DragFloat3(U8("カメラ注視点オフセット"), &m_startCameraOffset.x);
	ImGui::DragFloat3(U8("ボス戦時カメラ注視点オフセット"), &m_startBossCameraOffset.x);
	ImGui::DragFloat3(U8("ジャスト回避成功時カメラ注視点オフセット"), &m_justAvoidCameraOffset.x);
	ImGui::Separator();
	// 残像設定
	ImGui::DragInt(U8("残像最大数"), &m_stateParameter.afterImageMax, 1.0f, 1, 20);
	ImGui::DragFloat(U8("残像生成間隔"), &m_stateParameter.afterImageInterval, 0.01f, 0.01f, 1.0f);
	ImGui::ColorEdit4(U8("残像色"), &m_stateParameter.afterImageColor.x);

}

void PlayerState_BackWordAvoid::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("PlayerState_BackWordAvoid")) return;
	const auto& stateNode = js["PlayerState_BackWordAvoid"];
	if (stateNode.contains("Player"))
	{
		const auto& playerNode = stateNode["Player"];
		if (playerNode.contains("blendTime")) m_stateParameter.blendTime = playerNode["blendTime"].get<float>();
		if (playerNode.contains("animationSpeed")) m_stateParameter.animationSpeed = playerNode["animationSpeed"].get<float>();
		if (playerNode.contains("dashSpeed")) m_stateParameter.dashSpeed = playerNode["dashSpeed"].get<float>();
		if (playerNode.contains("dashSpeedTime")) m_stateParameter.dashSpeedTime = playerNode["dashSpeedTime"].get<float>();
		if (playerNode.contains("m_cameraTargetOffset")) m_startCameraOffset = JSON_MANAGER.JsonToVector(playerNode["m_cameraTargetOffset"]);
		if (playerNode.contains("m_cameraBossTargetOffset")) m_startBossCameraOffset = JSON_MANAGER.JsonToVector(playerNode["m_cameraBossTargetOffset"]);
		if (playerNode.contains("m_justAvoidCameraOffset")) m_justAvoidCameraOffset = JSON_MANAGER.JsonToVector(playerNode["m_justAvoidCameraOffset"]);

		// 残像設定
		if (playerNode.contains("afterImageMax")) m_stateParameter.afterImageMax = playerNode["afterImageMax"].get<int>();
		if (playerNode.contains("afterImageInterval")) m_stateParameter.afterImageInterval = playerNode["afterImageInterval"].get<float>();
		if (playerNode.contains("afterImageColor")) m_stateParameter.afterImageColor = JSON_MANAGER.JsonToVector4(playerNode["afterImageColor"]);

	}
}

void PlayerState_BackWordAvoid::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("PlayerState_BackWordAvoid")) js["PlayerState_BackWordAvoid"] = nlohmann::json::object();
	auto& stateNode = js["PlayerState_BackWordAvoid"];

	stateNode["Player"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["Player"]["animationSpeed"] = m_stateParameter.animationSpeed;
	stateNode["Player"]["dashSpeed"] = m_stateParameter.dashSpeed;
	stateNode["Player"]["dashSpeedTime"] = m_stateParameter.dashSpeedTime;
	stateNode["Player"]["m_cameraTargetOffset"] = JSON_MANAGER.VectorToJson(m_startCameraOffset);
	stateNode["Player"]["m_cameraBossTargetOffset"] = JSON_MANAGER.VectorToJson(m_startBossCameraOffset);
	stateNode["Player"]["m_justAvoidCameraOffset"] = JSON_MANAGER.VectorToJson(m_justAvoidCameraOffset);
	// 残像設定
	stateNode["Player"]["afterImageMax"] = m_stateParameter.afterImageMax;
	stateNode["Player"]["afterImageInterval"] = m_stateParameter.afterImageInterval;
	stateNode["Player"]["afterImageColor"] = JSON_MANAGER.Vector4ToJson(m_stateParameter.afterImageColor);

}
