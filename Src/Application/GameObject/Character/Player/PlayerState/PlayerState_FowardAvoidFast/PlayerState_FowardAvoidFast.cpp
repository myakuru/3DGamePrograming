#include "PlayerState_FowardAvoidFast.h"
#include"../../../../../main.h"
#include"../PlayerState_RunEnd/PlayerState_RunEnd.h"
#include"../PlayerState_AvoidAttack/PlayerState_AvoidAttack.h"

#include"../PlayerState_JustAvoidAttack/PlayerState_JustAvoidAttack.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"
#include"../../../Enemy/Enemy.h"
#include"../../../BossEnemy/BossEnemy.h"

#include"../PlayerState_Skill/PlayerState_Skill.h"
#include"../../../../../Scene/SceneManager.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"

void PlayerState_FowardAvoidFast::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("AvoidForward");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);

	m_player->SetAvoidFlg(true);
	m_player->SetAvoidStartTime(Application::Instance().GetDeltaTime()); // 現在の時間を記録

	m_player->AddAfterImage(true, 5, 4.0f, Math::Color(0.0f, 1.0f, 1.0f, 0.3f));

	m_player->SetAnimeSpeed(120.0f);

	// 敵との当たり判定を無効化
	m_player->SetAtkPlayer(true);

	m_afterImagePlayed = false;
	m_justAvoided = false;

	KdAudioManager::Instance().Play("Asset/Sound/Player/Dash.WAV", false)->SetVolume(1.0f);

}

void PlayerState_FowardAvoidFast::StateUpdate()
{
	// 途中で敵のジャスト回避成功フラグが立ったら残像発生
	if (!m_afterImagePlayed)
	{
		std::list<std::weak_ptr<KdGameObject>> nearEnemies;

		SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemyLike, nearEnemies);

		for (const auto& wk : nearEnemies)
		{
			if (auto obj = wk.lock())
			{
				bool just = false;
				if (obj->GetTypeID() == Enemy::TypeID)
				{
					auto e = std::static_pointer_cast<Enemy>(obj);
					just = e->GetJustAvoidSuccess();
					if (just) e->SetJustAvoidSuccess(false);
				}
				else if (obj->GetTypeID() == BossEnemy::TypeID)
				{
					auto b = std::static_pointer_cast<BossEnemy>(obj);
					just = b->GetJustAvoidSuccess();
					if (just) b->SetJustAvoidSuccess(false);
				}
				if (just)
				{
					// ...以降は既存処理
					m_player->AddAfterImage(true, 5, 1.0f, Math::Color(0.0f, 1.0f, 1.0f, 0.5f), 0.7f);
					m_justAvoided = true;
					m_afterImagePlayed = true;
					m_player->SetJustAvoidSuccess(true);
					KdAudioManager::Instance().Play("Asset/Sound/Player/SlowMotion.WAV", false)->SetVolume(1.0f);
					if (auto bgm = SceneManager::Instance().GetGameSound()) { bgm->SetPitch(-1.0f); }
					const auto& justCfg = m_player->GetPlayerConfig().GetJustAvoidParam();
					Application::Instance().SetFpsScale(justCfg.m_slowMoScale);
					SceneManager::Instance().SetDrawGrayScale(justCfg.m_useGrayScale);
					break;
				}
			}
		}
	}

	// 前方ベクトルを取得
	Math::Vector3 forward = Math::Vector3::TransformNormal(Math::Vector3::Forward, Math::Matrix::CreateFromQuaternion(m_player->GetRotationQuaternion()));
	forward.Normalize();

	if (m_justAvoided)
	{
		if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
		{
			m_justAvoided = false;
			auto state = std::make_shared<PlayerState_JustAvoidAttack>();
			m_player->ChangeState(state);
			return;
		}
	}
	else
	{
		// 回避中に攻撃ボタンが押されたら回避攻撃へ移行
		if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
		{
			m_justAvoided = false;
			auto state = std::make_shared<PlayerState_AvoidAttack>();
			m_player->ChangeState(state);
			return;
		}

		// Eキー先行入力の予約
		if (KeyboardManager::GetInstance().IsKeyJustPressed('E'))
		{
			if (m_playerData.GetPlayerStatus().skillPoint >= 30)
			{
				m_playerData.SetPlayerStatus().skillPoint -= 30;
				auto state = std::make_shared<PlayerState_Skill>();
				m_player->ChangeState(state);
				return;
			}
		}

		if (KeyboardManager::GetInstance().IsKeyJustPressed('Q'))
		{
			if (m_playerData.GetPlayerStatus().specialPoint == m_playerData.GetPlayerStatus().specialPointMax)
			{
				m_playerData.SetPlayerStatus().specialPoint = 0;
				auto specialAttackState = std::make_shared<PlayerState_SpecialAttackCutIn>();
				m_player->ChangeState(specialAttackState);
				return;
			}
		}
	}

	// アニメーションが終了したらIdleへ移行
	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		// スローモーション解除（ここを終点にする）
		Application::Instance().SetFpsScale(1.f);
		SceneManager::Instance().SetDrawGrayScale(false);
		m_justAvoided = false;

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

	m_player->AddAfterImage();

	m_player->SetAvoidFlg(false);
	m_player->SetAvoidStartTime(0.0f); // 現在の時間を記録

	// 敵との当たり判定を有効化
	m_player->SetAtkPlayer(false);
}
