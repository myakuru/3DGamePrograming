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
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);

	m_player->SetAvoidFlg(true);

	SceneManager::Instance().GetObjectWeakPtr(m_bossEnemy);

	if (auto camera = m_player->GetPlayerCamera().lock(); camera)
	{
		if (auto bossEnemy = m_bossEnemy.lock(); bossEnemy)
		{
			camera->SetTargetLookAt({ 0.0f,1.0f,-6.5f });
		}
		else
		{
			camera->SetTargetLookAt({ 0.0f, 1.0f, -4.5f });
		}
	}

	m_time = 0.0f;

	// アニメーション速度を変更
	m_player->SetAnimeSpeed(120.0f);

	// 回避時の処理
	m_player->SetAvoidStartTime(0.0f);

	m_afterImagePlayed = false;
	m_justAvoided = false;

	KdAudioManager::Instance().Play("Asset/Sound/Player/BackWardAvoid.WAV", false)->SetVolume(1.0f);

}

void PlayerState_BackWordAvoid::StateUpdate()
{

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

		m_justAvoided = true;
		m_afterImagePlayed = true;

		// 残像
		m_player->GetAfterImage()->AddAfterImage(true, 5, 1.0f, Math::Color(0.0f, 1.0f, 1.0f, 0.5f));

		const auto& justCfg = m_player->GetPlayerConfig().GetJustAvoidParam();
		Application::Instance().SetFpsScale(justCfg.m_slowMoScale);
		SceneManager::Instance().SetDrawGrayScale(justCfg.m_useGrayScale);
	}

	// 前方ベクトルを取得
	Math::Vector3 forward = Math::Vector3::TransformNormal(Math::Vector3::Forward, Math::Matrix::CreateFromQuaternion(m_player->GetRotationQuaternion()));
	forward.Normalize();

	if (m_player->GetJustAvoidSuccess())
	{
		if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
		{
			auto state = std::make_shared<PlayerState_JustAvoidAttack>();
			m_player->ChangeState(state);
			return;
		}
	}
	else
	{
		m_player->SetJustAvoidSuccess(false);

		// スローモーション解除（ここを終点にする）
		Application::Instance().SetFpsScale(1.f);
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

		auto idleState = std::make_shared<PlayerState_Idle>();
		m_player->ChangeState(idleState);
		return;
	}

	PlayerStateBase::StateUpdate();

	// 刀は鞘の中にある状態
	UpdateUnsheathed();

	// 回避中の移動処理
	if (m_time < 0.3f)
	{
		const float dashSpeed = -0.9f;
		m_player->SetIsMoving(forward * dashSpeed);
	}
	else
	{
		m_player->SetIsMoving(Math::Vector3::Zero);
	}

}

void PlayerState_BackWordAvoid::StateEnd()
{

	PlayerStateBase::StateEnd();

	m_player->SetAvoidFlg(false);
	m_player->SetAvoidStartTime(0.0f); // 現在の時間を記録

	m_player->SetHitCheck(false); // 被ヒット判定解除

	m_player->GetAfterImage()->AddAfterImage();

	// 敵との当たり判定を無効化解除（押し出し処理を元に戻す）
	m_player->SetAtkPlayer(false);
		
}
