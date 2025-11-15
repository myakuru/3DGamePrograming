#include"PlayerState_ChargeAttack.h"
#include"../../../../../main.h"

#include"../PlayerState_ChargeAttack1/PlayerState_ChargeAttack1.h"
#include"../PlayerState_ChargeAttack2/PlayerState_ChaegeAttack2.h"
#include"../PlayerState_ChargeAttackMax/PlayerState_ChargeAttackMax.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"

#include"../../../../Effect/EffekseerEffect/ShineEffectBlue/ShineEffectBlue.h"
#include"../../../../Effect/EffekseerEffect/GroundFreezes/GroundFreezes.h"

void PlayerState_ChargeAttack::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("ChargeAttack0");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);

	PlayerStateBase::StateStart();

	m_chargeTime = 0.0f;
	m_isCharging = false;

	m_time = 0.0f;

	if (auto camera = m_player->GetPlayerCamera().lock(); camera)
	{
		camera->SetTargetLookAt({ 0.f,1.f,-2.0f });
	}

	SceneManager::Instance().GetObjectWeakPtr(m_shineEffect);
	SceneManager::Instance().GetObjectWeakPtr(m_groundFreezes);

	if (auto effect = m_shineEffect.lock(); effect)
	{
		effect->SetPlayEffect(true);
	}

	if (auto effect = m_groundFreezes.lock(); effect)
	{
		effect->SetPlayEffect(true);
	}

	// アニメーション速度を変更
	m_player->SetAnimeSpeed(80.0f);

	KdAudioManager::Instance().Play("Asset/Sound/Player/Charge.WAV", false)->SetVolume(1.0f);

}

void PlayerState_ChargeAttack::StateUpdate()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	// 刀は鞘の中にある状態
	UpdateUnsheathed();

	// 攻撃中の移動方向で回転を更新
	if (m_player->GetMovement() != Math::Vector3::Zero)
	{
		Math::Vector3 moveDir = m_player->GetMovement();
		moveDir.y = 0.0f;
		moveDir.Normalize();
		m_player->UpdateQuaternionDirect(moveDir);
	}

	
	m_player->SetIsMoving(m_attackDirection);
		
	// 移動を止める
	m_player->SetIsMoving(Math::Vector3::Zero);

	if (!m_player->GetAnimator()->IsAnimationEnd()) return;

	{
		// =========================
		// 左ボタン押下 長押し / 短押し判定
		// =========================
		const float kShortPressThreshold = 0.1f; // 短押し閾値
		const float kLongPressThreshold = 0.2f; // 長押し閾値

		const float duration = KeyboardManager::GetInstance().GetKeyPressDuration(VK_LBUTTON);

		// ステート突入前から既に押されていたケースも拾う
		if (!m_isKeyPressing && duration > 0.0f)
		{
			m_isKeyPressing = true;
		}

		// Chargeカウントが2つ以上溜まっている。
		if (duration >= kLongPressThreshold)
		{
			m_isKeyPressing = false;
			auto next = std::make_shared<PlayerState_ChargeAttack1>();
			m_player->ChangeState(next);
			return;
		}

		// Chargeカウントが１つ溜まった状態で攻撃した
		if (duration >= kShortPressThreshold || duration <= 0.0f)
		{
			m_isKeyPressing = false; // 判定終了

			if (m_playerData.GetPlayerStatus().chargeCount >= 0)
			{
				auto next = std::make_shared<PlayerState_ChaegeAttack2>();
				m_player->ChangeState(next);
				return;
			}

		}
	}

}

void PlayerState_ChargeAttack::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto camera = m_player->GetPlayerCamera().lock(); camera)
	{
		camera->SetTargetLookAt({ 0.f,1.f,-2.5f });
	}

	if (auto effect = m_shineEffect.lock(); effect)
	{
		effect->SetPlayEffect(false);
	}

	if (auto effect = m_groundFreezes.lock(); effect)
	{
		effect->SetPlayEffect(false);
	}
}