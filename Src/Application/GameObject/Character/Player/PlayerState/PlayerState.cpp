#include "PlayerState.h"
#include"../../../Weapon/Katana/Katana.h"
#include"../../../Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include"../PlayerState/PlayerState_SpecialAttack/PlayerState_SpecialAttack.h"
#include"../PlayerState/PlayerState_FowardAvoidFast/PlayerState_FowardAvoidFast.h"
#include"../PlayerState/PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include"Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"
#include"Application/Scene/SceneManager.h"
#include"Application/main.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_FullCharge/PlayerState_FullCharge.h"

void PlayerStateBase::StateStart()
{
	// プレイヤー位置
	const Math::Vector3 playerPos = m_player->GetPos();

	// 検索半径（g_focusMaxDistSq と整合）
	constexpr float kSearchRadius = 50.0f;               // = sqrt(50^2)
	constexpr float kSearchRadiusSq = kSearchRadius * kSearchRadius;

	// 1) 既存フォーカスが有効なら採用（距離とボス出現状態も考慮）
	if (m_focusRemainSec > 0.0f)
	{
		if (auto f = m_focusTarget.lock())
		{
			if (!f->IsExpired())
			{
				// 未出現ボスは無視
				if (!(f->GetTypeID() == BossEnemy::TypeID && !SceneManager::Instance().IsBossAppear()))
				{
					const Math::Vector3 fpos = f->GetPos();
					const float distSq = (fpos - playerPos).LengthSquared();
					if (distSq <= kSearchRadiusSq)
					{
						m_nearestEnemy = f;
						m_nearestEnemyPos = fpos;
						m_minDistSq = distSq;
					}
				}
			}
		}
		// 無効なら解除
		if (!m_nearestEnemy)
		{
			m_focusTarget.reset();
			m_focusRemainSec = 0.0f;
		}
	}

	// 再取得
	if (!m_nearestEnemy)
	{
		std::list<std::weak_ptr<KdGameObject>> candidates;
		// 近い敵ものを列挙
		SceneManager::Instance().GetObjectWeakPtrListByTagInSphere
		(
			ObjTag::EnemyLike, playerPos, kSearchRadius, candidates
		);

		// 最も近い敵を探索
		for (auto& w : candidates)
		{
			auto sp = w.lock();
			if (!sp) { continue; }
			
			// 未出現ボスは除外
			if (sp->GetTypeID() == BossEnemy::TypeID && !SceneManager::Instance().IsBossAppear())
			{
				continue;
			}

			const Math::Vector3 epos = sp->GetPos();
			const float distSq = (epos - playerPos).LengthSquared();
			if (distSq < m_minDistSq)
			{
				m_minDistSq = distSq;
				m_nearestEnemyPos = epos;
				m_nearestEnemy = sp;
			}
		}

		// 新規フォーカス確定
		if (m_nearestEnemy)
		{
			m_focusTarget = m_nearestEnemy;
			m_focusRemainSec = m_focusDurationSec;
		}
	}

	// 3) 攻撃方向を決定（敵がいればその方向、いなければ最後の移動方向）
	if (m_nearestEnemy)
	{
		m_attackDirection = m_nearestEnemyPos - playerPos;
		m_attackDirection.y = 0.0f;

		if (m_attackDirection != Math::Vector3::Zero)
		{
			m_attackDirection.Normalize();
			m_player->UpdateQuaternionDirect(m_attackDirection); // カメラ回転なし
		}
	}
	else
	{
		m_attackDirection = m_player->GetLastMoveDirection();
		if (m_attackDirection != Math::Vector3::Zero)
		{
			m_player->UpdateQuaternionDirect(m_attackDirection);
		}
	}

	// 刀の初期フラグ
	if (auto katana = m_player->GetKatana().lock())
	{
		katana->SetNowAttackState(false);
	}

	m_isKeyPressing = false;
	m_time = 0.0f;
	m_animeTime = 0.0f;
	m_maxAnimeTime = 0.0f;

	m_playerData = m_player->GetStatus();
}

void PlayerStateBase::StateUpdate()
{
	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	// フォーカスタイマー更新と自動解除
	if (m_focusRemainSec > 0.0f)
	{
		m_focusRemainSec -= deltaTime;
		if (m_focusRemainSec <= 0.0f)
		{
			m_focusRemainSec = 0.0f;
			m_focusTarget.reset();
		}
		else
		{
			if (auto f = m_focusTarget.lock())
			{
				// 失効条件
				if (f->IsExpired()
					|| (f->GetTypeID() == BossEnemy::TypeID && !SceneManager::Instance().IsBossAppear()))
				{
					m_focusRemainSec = 0.0f;
					m_focusTarget.reset();
				}
			}
			else
			{
				m_focusRemainSec = 0.0f;
			}
		}
	}
}

void PlayerStateBase::StateEnd()
{
	// カタナの取得
	if (auto katana = m_player->GetKatana().lock())
	{
		katana->SetNowAttackState(false);
	}
}

// 刀の位置を右手に追従するように更新
void PlayerStateBase::UpdateKatanaPos()
{
	// 右手のワークノードを取得
	auto rightHandNode = m_player->GetModelWork()->FindWorkNode("Katana");
	// 左手のワークノードを取得
	auto leftHandNode = m_player->GetModelWork()->FindWorkNode("Sheath");

	if (!rightHandNode) return;
	if (!leftHandNode) return;

	// カタナの取得
	auto katana = m_player->GetKatana().lock();
	// 鞘の取得
	auto saya = m_player->GetScabbard().lock();

	if (!katana) return;
	if (!saya) return;

	// プレイヤーに追尾する刀にするためにワークノードとプレイヤーのワールド変換を設定
	katana->SetHandKatanaMatrix(rightHandNode->m_worldTransform);
	saya->SetHandKatanaMatrix(leftHandNode->m_worldTransform);
}

// 刀と鞘の位置が左手に追従するように更新
void PlayerStateBase::UpdateUnsheathed()
{
	// 左手のワークノードを取得
	auto leftHandNode = m_player->GetModelWork()->FindWorkNode("Sheath");

	if (!leftHandNode) return;

	// カタナの取得
	auto katana = m_player->GetKatana().lock();
	// 鞘の取得
	auto saya = m_player->GetScabbard().lock();

	if (!katana) return;
	if (!saya) return;

	// プレイヤーに追尾する刀にするためにワークノードとプレイヤーのワールド変換を設定
	katana->SetHandKatanaMatrix(leftHandNode->m_worldTransform);
	saya->SetHandKatanaMatrix(leftHandNode->m_worldTransform);
}

// 必殺技入力関連
bool PlayerStateBase::UpdateSpecialAttackInput()
{
	if (KeyboardManager::GetInstance().IsKeyJustPressed('Q'))
	{
		if (m_playerData.GetPlayerStatus().specialPoint == m_playerData.GetPlayerStatus().specialPointMax)
		{
			m_playerData.SetPlayerStatus().specialPoint = 0;
			auto specialAttackState = std::make_shared<PlayerState_SpecialAttackCutIn>();
			m_player->ChangeState(specialAttackState);
			return true;
		}
	}

	return false;
}

// 回避入力関連
bool PlayerStateBase::UpdateMoveAvoidInput()
{
	// 回避関係
	{
		const float kShortPressMin = 0.1f;       // 短押し有効開始
		const float kLongPressThreshold = 0.2f;  // 長押し閾値

		float rDuration = KeyboardManager::GetInstance().GetKeyPressDuration(VK_RBUTTON);

		// 押された瞬間
		if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_RBUTTON))
		{
			m_isKeyPressing = true; // 判定開始
		}

		// 長押し判定
		if (m_isKeyPressing &&
			rDuration >= kLongPressThreshold && 
			!KeyboardManager::GetInstance().IsKeyJustReleased(VK_RBUTTON))
		{
			m_isKeyPressing = false;
			auto avoidFast = std::make_shared<PlayerState_FowardAvoidFast>();
			m_player->ChangeState(avoidFast);
			return true;
		}

		// 短押し判定
		if (m_isKeyPressing && 
			KeyboardManager::GetInstance().IsKeyJustReleased(VK_RBUTTON)&&
			!KeyboardManager::GetInstance().IsKeyPressed('W'))
		{

			if (rDuration >= kShortPressMin && rDuration < kLongPressThreshold)
			{
				m_isKeyPressing = false;
				auto backAvoid = std::make_shared<PlayerState_BackWordAvoid>();
				m_player->ChangeState(backAvoid);
				return true;
			}

			// 0.1秒未満なら何もしない
			m_isKeyPressing = false;
		}

		if (m_isKeyPressing &&
			KeyboardManager::GetInstance().IsKeyJustReleased(VK_RBUTTON)&&
			KeyboardManager::GetInstance().IsKeyPressed('W'))
		{
			if (rDuration >= kShortPressMin && rDuration < kLongPressThreshold)
			{
				m_isKeyPressing = false;
				auto backAvoid = std::make_shared<PlayerState_ForwardAvoid>();
				m_player->ChangeState(backAvoid);
				return true;
			}

			// 0.1秒未満なら何もしない
			m_isKeyPressing = false;
		}

	}

	return false;
}

// 刀を鞘に納める入力関連
bool PlayerStateBase::UpdateSheathKatanaInput()
{
	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto sheath = std::make_shared<PlayerState_SheathKatana>();
		m_player->ChangeState(sheath);
		return true;
	}

	return false;
}

// ため攻撃入力関連
bool PlayerStateBase::UpdateChargeAttackInput()
{
	// チャージが残っている場合のみ、長押しでFullChargeへ
	if (KeyboardManager::GetInstance().GetKeyPressDuration(VK_LBUTTON) >= 0.5f)
	{
		auto state = std::make_shared<PlayerState_FullCharge>();
		m_player->ChangeState(state);
		return true;
	}

	return false;
}
