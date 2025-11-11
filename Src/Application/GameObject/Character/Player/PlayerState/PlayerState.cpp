#include "PlayerState.h"
#include"../../../Weapon/Katana/Katana.h"
#include"../../../Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include"../PlayerState/PlayerState_SpecialAttack/PlayerState_SpecialAttack.h"
#include"../PlayerState/PlayerState_FowardAvoidFast/PlayerState_FowardAvoidFast.h"
#include"../PlayerState/PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include"Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"
#include"../../../../Scene/SceneManager.h"
#include"../../../../main.h"

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

	// 2) 再取得が必要な場合のみ、近傍限定で Enemy/BossEnemy を取得
	if (!m_nearestEnemy)
	{
		std::list<std::weak_ptr<KdGameObject>> candidates;
		// 型バケット＋近傍API（SceneManager 側で型別レジストリを使用）
		SceneManager::Instance().GetObjectWeakPtrListByTagInSphere
		(
			ObjTag::EnemyLike, playerPos, kSearchRadius, candidates
		);

		// 最も近い敵を探索
		for (auto& w : candidates)
		{
			if (auto sp = w.lock())
			{
				if (sp->IsExpired()) continue;

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
	if (auto katana = m_player->GetKatana().lock(); katana)
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
	if (auto katana = m_player->GetKatana().lock(); katana)
	{
		katana->SetNowAttackState(false);
	}
}

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
