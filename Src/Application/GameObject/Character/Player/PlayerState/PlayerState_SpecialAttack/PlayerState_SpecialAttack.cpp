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
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);

	PlayerStateBase::StateStart();

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

	if (m_animeTime >= 0.6f)
	{
		auto state = std::make_shared<PlayerState_SpecialAttack1>();
		m_player->ChangeState(state);
		return;
	}

	if (m_animeTime >= 0.15f)
	{
		// 当たり判定
		m_player->UpdateAttackCollision(10.0f, 7.0f, 6, 0.2f, { 0.4f, 0.4f }, 0.5f, 0.0f, 1.2f);

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

	if (auto camera = m_player->GetPlayerCamera().lock(); camera)
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
		if (auto camera = m_player->GetPlayerCamera().lock(); camera)
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
		if (auto effect = m_specialAttackEffect.lock(); effect)
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
