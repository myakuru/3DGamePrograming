#include "PlayerState_SheathKatana.h"
#include"../PlayerState_Idle/PlayerState_Idle.h"
#include"../PlayerState_Attack/PlayerState_Attack.h"
#include"../PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include "../PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include"../PlayerState_Skill/PlayerState_Skill.h"
#include"../PlayerState_FullCharge/PlayerState_FullCharge.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_Run/PlayerState_Run.h"

void PlayerState_SheathKatana::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("SheathKatana");
	m_player->GetAnimator()->SetAnimation(anime, 0.9f, false);


	PlayerStateBase::StateStart();

	if (!m_SheathKatanaSound) m_SheathKatanaSound = KdAudioManager::Instance().Play("Asset/Sound/Player/SheathKatana.wav", false);

	m_LButtonkeyInput = false;
}

void PlayerState_SheathKatana::StateUpdate()
{
	m_player->SetAnimeSpeed(120.0f);

	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON)) m_LButtonkeyInput = true;

	// 攻撃入力処理
	if (UpdateAttackInput<PlayerState_Attack>()) return;

	// キーが押されたらRunステートへ
	if (KeyboardManager::GetInstance().IsKeyPressed('W') ||
		KeyboardManager::GetInstance().IsKeyPressed('A') ||
		KeyboardManager::GetInstance().IsKeyPressed('S') ||
		KeyboardManager::GetInstance().IsKeyPressed('D'))
	{
		auto spRunState = std::make_shared<PlayerState_Run>();
		m_player->ChangeState(spRunState);
		return;
	}

	// 回避入力処理
	if (UpdateMoveAvoidInput()) return;

	// 必殺技入力処理
	if (UpdateSpecialAttackInput()) return;

	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto idleState = std::make_shared<PlayerState_Idle>();
		m_player->ChangeState(idleState);
		return;
	}

	// カタナの取得
	auto katana = m_player->GetKatana().lock();
	if (!katana) return;

	float time = m_player->GetAnimator()->GetTime();

	katana->SetShowTrail(false);

	if (time >= 20.0f)
	{
		katana->SetNowAttackState(true);
		UpdateKatanaPos();
	}
	else
	{
		katana->SetNowAttackState(false);
		UpdateUnsheathed();
	}

}

void PlayerState_SheathKatana::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (m_SheathKatanaSound)
	{
		m_SheathKatanaSound->Stop();
		m_SheathKatanaSound.reset();
	}
}
