#include "PlayerState_SpecialAttackCutIn.h"
#include"../PlayerState_SpecialAttack/PlayerState_SpecialAttack.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"
#include"../../../../Weapon/Katana/Katana.h"

#include"../../../../../main.h"

void PlayerState_SpecialAttackCutIn::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("CutIn");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);

	PlayerStateBase::StateStart();

	m_player->SetAnimeSpeed(60.0f);

	// カメラの位置を変更
	if (auto camera = m_player->GetPlayerCamera().lock(); camera)
	{
		camera->SetTargetLookAt({ 0.0f,0.6f,-1.7f });
		camera->SetRotationSmooth(5.0f);
		camera->SetDistanceSmooth(5.0f);
	}

	// 左手に刀をセットする
	if (auto katana = m_player->GetKatana().lock(); katana)
	{
		katana->SetNowAttackState(true);
	}

	// 無敵状態にする
	m_player->SetInvincible(true);

	m_time = 0.0f;

}

void PlayerState_SpecialAttackCutIn::StateUpdate()
{
	// アニメーション時間のデバッグ表示
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();

		m_maxAnimeTime = m_player->GetAnimator()->GetMaxAnimationTime();

		if (m_animeTime > m_maxAnimeTime)
		{
			KdDebugGUI::Instance().AddLog(U8("Attackアニメ時間: %f"), m_animeTime);
			KdDebugGUI::Instance().AddLog("\n");
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
			const float yawRad = std::atan2(-moveDir.x, -moveDir.z);
			const float yawDeg = DirectX::XMConvertToDegrees(yawRad);
			camera->SetTargetRotation({ 10.0f, yawDeg , 5.0f });
		}
	}

	if (m_animeTime >= 0.6)
	{
		if (auto camera = m_player->GetPlayerCamera().lock(); camera)
		{
			camera->SetTargetLookAt({ 0.0f,1.0f,-0.7f });
		}
	}


	m_player->SetIsMoving(m_attackDirection * 0.01f);

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
	if (auto camera = m_player->GetPlayerCamera().lock(); camera)
	{
		camera->SetTargetLookAt({ 0.f,1.f,-4.0f });
		camera->SetTargetRotation({ 0.0f,90.0f,0.0f });

		camera->SetRotationSmooth(8.0f);
		camera->SetDistanceSmooth(8.0f);
	}
}
