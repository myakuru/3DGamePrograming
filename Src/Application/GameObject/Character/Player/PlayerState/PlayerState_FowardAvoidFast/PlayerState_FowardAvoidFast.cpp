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
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_FullCharge/PlayerState_FullCharge.h"

void PlayerState_FowardAvoidFast::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("AvoidForward");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);

	m_player->SetAvoidFlg(true);
	m_player->SetAvoidStartTime(Application::Instance().GetDeltaTime()); // 現在の時間を記録

	m_player->SetAnimeSpeed(120.0f);

	m_player->GetAfterImage()->AddAfterImage(true, 5, 4.0f, Math::Color(0.0f, 1.0f, 1.0f, 0.5f));

	// 敵との当たり判定を無効化
	m_player->SetAtkPlayer(true);

	m_afterImagePlayed = false;
	m_justAvoided = false;

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

		m_justAvoided = true;
		m_afterImagePlayed = true;

		// 残像
		m_player->GetAfterImage()->AddAfterImage(true, 5, 1.0f, Math::Color(0.0f, 1.0f, 1.0f, 0.5f));

		const auto& justCfg = m_player->GetPlayerConfig().GetJustAvoidParam();
		Application::Instance().SetFpsScale(justCfg.m_slowMoScale);
		SceneManager::Instance().SetDrawGrayScale(justCfg.m_useGrayScale);
	}
	else
	{
		// スローモーション解除（ここを終点にする）
		Application::Instance().SetFpsScale(1.f);
		SceneManager::Instance().SetDrawGrayScale(false);

		// 回避中に攻撃ボタンが押されたら回避攻撃へ移行
		if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
		{
			m_player->SetJustAvoidSuccess(false);
			auto state = std::make_shared<PlayerState_AvoidAttack>();
			m_player->ChangeState(state);
			return;
		}

		// 必殺技入力処理
		if (UpdateSpecialAttackInput()) return;
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

	float dashSpeed = 2.0f;

	m_player->SetIsMoving(forward * dashSpeed);
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
