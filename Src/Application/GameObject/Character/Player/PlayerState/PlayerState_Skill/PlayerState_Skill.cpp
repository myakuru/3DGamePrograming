#include "PlayerState_Skill.h"
#include"../../../../../main.h"
#include"../PlayerState_Run/PlayerState_Run.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/ESkillEffect/ESkillEffect.h"
	
void PlayerState_Skill::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("Eskill");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);
	

	PlayerStateBase::StateStart();

	SceneManager::Instance().GetObjectWeakPtr(m_effect);

	KdAudioManager::Instance().Play("Asset/Sound/Player/Eskill.WAV", false)->SetVolume(1.0f);

	// 敵との当たり判定を無効化
	m_player->SetAtkPlayer(true);

	m_attackParam.m_dashTimer = 0.0f;

	// 残像の設定
	m_player->AddAfterImage(true, 10, 0.05f, { 0.0f,0.5f,1.0f,0.5f }, 0.5f);


	if (m_playerData.GetPlayerStatus().chargeCount < 3)
	{
		m_playerData.SetPlayerStatus().chargeCount++;
	}
}

void PlayerState_Skill::StateUpdate()
{
	m_player->SetAnimeSpeed(100.0f);

	// 攻撃中の移動方向で回転を更新
	if (m_player->GetMovement() != Math::Vector3::Zero)
	{
		auto dir = m_player->GetMovement();
		dir.y = 0.0f;
		dir.Normalize();
		m_player->UpdateQuaternionDirect(dir);
	}

	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto sheathState = std::make_shared<PlayerState_SheathKatana>();
		m_player->ChangeState(sheathState);
		return;
	}

	UpdateKatanaPos();

	// 当たり判定有効時間: 最初の0.5秒のみ
	m_player->UpdateAttackCollision(8.0f, 1.0f, 5, 0.1f, { 0.3f, 0.3f }, 0.3f);

	float deltaTime = Application::Instance().GetDeltaTime();
	if (m_attackParam.m_dashTimer < 0.1f)
	{
		float dashSpeed = 5.0f;
		m_player->SetIsMoving(m_attackDirection * dashSpeed);
		m_attackParam.m_dashTimer += deltaTime;
	}
	else
	{
		// 移動を止める
		m_player->SetIsMoving(Math::Vector3::Zero);
		if (auto effect = m_effect.lock(); effect)
		{
			effect->SetPlayEffect(true);
		}
	}

}

void PlayerState_Skill::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto effect = m_effect.lock(); effect)
	{
		effect->SetPlayEffect(false);
	}

	// 敵との当たり判定をもとに戻す
	m_player->SetAtkPlayer(false);

	// 残像のリセット
	m_player->AddAfterImage(false);
}
