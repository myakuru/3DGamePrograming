#include "PlayerState_FowardAvoidFast.h"
#include"../../../../../main.h"
#include"../PlayerState_RunEnd/PlayerState_RunEnd.h"
#include"../PlayerState_AvoidAttack/PlayerState_AvoidAttack.h"

#include"../PlayerState_JustAvoidAttack/PlayerState_JustAvoidAttack.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"
#include"Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"

#include"../PlayerState_Skill/PlayerState_Skill.h"
#include"../../../../../Scene/SceneManager.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"

#include"Application/GameObject/Character/EnemyBase/AetheriusEnemy/AetheriusEnemy.h"
#include"Application/GameObject/Character/AfterImage/AfterImage.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_Attack/PlayerState_Attack.h"

void PlayerState_FowardAvoidFast::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("AvoidForward");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);

	m_player->SetAvoidFlg(true);
	m_player->SetAvoidStartTime(Application::Instance().GetDeltaTime()); // 現在の時間を記録

	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	// 敵との当たり判定を無効化
	m_player->SetAtkPlayer(true);

	// 残像エフェクト開始
	m_player->GetAfterImage()->AddAfterImage
	(
		true,
		m_stateParameter.afterImageMax,
		m_stateParameter.afterImageInterval,
		m_stateParameter.afterImageColor
	);

	m_afterImagePlayed = false;

	KdAudioManager::Instance().Play("Asset/Sound/Player/Dash.WAV", false)->SetVolume(1.0f);

}

void PlayerState_FowardAvoidFast::StateUpdate()
{

	// 前方ベクトルを取得
	Math::Vector3 forward = Math::Vector3::TransformNormal(Math::Vector3::Forward, Math::Matrix::CreateFromQuaternion(m_player->GetRotationQuaternion()));
	forward.Normalize();

	// ジャスト回避成立時の演出を一度だけ
	if (!m_afterImagePlayed && m_player->GetJustAvoidSuccess())
	{
		KdAudioManager::Instance().Play("Asset/Sound/Player/SlowMotion.WAV", false)->SetVolume(1.0f);

		if (auto bgm = SceneManager::Instance().GetGameSound())
		{
			bgm->SetPitch(-1.0f);
		}

		m_afterImagePlayed = true;

		Application::Instance().SetFpsScale(0.1f);
		SceneManager::Instance().SetDrawGrayScale(true);
	}
	else
	{
		// スローモーション解除（ここを終点にする）
		Application::Instance().SetFpsScale(1.f);
		SceneManager::Instance().SetDrawGrayScale(false);

		// 回避中に攻撃ボタンが押されたら回避攻撃へ移行
		if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
		{
			m_player->SetJustAvoidSuccess(true);
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

		// スローモーション解除（ここを終点にする）
		Application::Instance().SetFpsScale(1.f);
		SceneManager::Instance().SetDrawGrayScale(false);

		// ゲームのメインサウンドのピッチを下げる
		if (auto bgm = SceneManager::Instance().GetGameSound())
		{
			bgm->SetPitch(0.0f);
		}

		auto idleState = std::make_shared<PlayerState_RunEnd>();
		m_player->ChangeState(idleState);
		return;
	}

	PlayerStateBase::StateUpdate();

	// 刀は鞘の中にある状態
	UpdateUnsheathed();

	m_player->SetIsMoving(forward * m_moveSpeed);
}

void PlayerState_FowardAvoidFast::StateEnd()
{
	PlayerStateBase::StateEnd();

	m_player->GetAfterImage()->AddAfterImage();

	m_player->SetHitCheck(false); // 被ヒット判定解除

	m_player->SetAvoidFlg(false);
	m_player->SetAvoidStartTime(0.0f); // 現在の時間を記録

	// 敵との当たり判定を有効化
	m_player->SetAtkPlayer(false);
}

void PlayerState_FowardAvoidFast::ApplyFromConfig(const PlayerStateBase& other)
{
	assert(typeid(other) == typeid(PlayerState_FowardAvoidFast));
	const auto& p = static_cast<const PlayerState_FowardAvoidFast&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;  // 構造体一括コピー
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_moveSpeed = p.m_moveSpeed;

	// 残像設定
	m_stateParameter.afterImageMax = p.m_stateParameter.afterImageMax;
	m_stateParameter.afterImageInterval = p.m_stateParameter.afterImageInterval;
	m_stateParameter.afterImageColor = p.m_stateParameter.afterImageColor;
}

void PlayerState_FowardAvoidFast::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);
	ImGui::DragFloat(U8("移動速度"), &m_moveSpeed);
	ImGui::Separator();
	// 残像設定
	ImGui::DragInt(U8("残像最大数"), &m_stateParameter.afterImageMax, 1.0f, 1, 20);
	ImGui::DragFloat(U8("残像生成間隔"), &m_stateParameter.afterImageInterval, 0.01f, 0.01f, 1.0f);
	ImGui::ColorEdit4(U8("残像色"), &m_stateParameter.afterImageColor.x);
}

void PlayerState_FowardAvoidFast::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("PlayerState_FowardAvoidFast")) return;
	const auto& stateNode = js["PlayerState_FowardAvoidFast"];
	if (stateNode.contains("Player"))
	{
		const auto& playerNode = stateNode["Player"];
		if (playerNode.contains("blendTime")) m_stateParameter.blendTime = playerNode["blendTime"].get<float>();
		if (playerNode.contains("animationSpeed")) m_stateParameter.animationSpeed = playerNode["animationSpeed"].get<float>();
		// 移動速度
		if (playerNode.contains("moveSpeed")) m_moveSpeed = playerNode["moveSpeed"].get<float>();

		// 残像設定
		if (playerNode.contains("afterImageMax")) m_stateParameter.afterImageMax = playerNode["afterImageMax"].get<int>();
		if (playerNode.contains("afterImageInterval")) m_stateParameter.afterImageInterval = playerNode["afterImageInterval"].get<float>();
		if (playerNode.contains("afterImageColor")) m_stateParameter.afterImageColor = JSON_MANAGER.JsonToVector4(playerNode["afterImageColor"]);
	}
}

void PlayerState_FowardAvoidFast::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("PlayerState_FowardAvoidFast")) js["PlayerState_FowardAvoidFast"] = nlohmann::json::object();
	auto& stateNode = js["PlayerState_FowardAvoidFast"];

	stateNode["Player"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["Player"]["animationSpeed"] = m_stateParameter.animationSpeed;
	// 移動速度
	stateNode["Player"]["moveSpeed"] = m_moveSpeed;

	// 残像設定
	stateNode["Player"]["afterImageMax"] = m_stateParameter.afterImageMax;
	stateNode["Player"]["afterImageInterval"] = m_stateParameter.afterImageInterval;
	stateNode["Player"]["afterImageColor"] = JSON_MANAGER.Vector4ToJson(m_stateParameter.afterImageColor);
}
