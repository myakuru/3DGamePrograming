#include "AetheriusEnemy.h"

#include"Application/GameObject/Character/Player/Player.h"
#include"Application/Scene/SceneManager.h"
#include"EnemyState/EnemyState_Idle/EnemyState_Idle.h"
#include"EnemyState/EnemyState_Hit/EnemyState_Hit.h"
#include"Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"
#include"Application/GameObject/Weapon/EnemySword/EnemySword.h"
#include"Application/GameObject/Weapon/EnemyShield/EnemyShield.h"
#include"Application/GameObject/Collition/Collition.h"
#include"Application/main.h"
#include"MyFramework/Manager/JsonManager/JsonManager.h"
#include"EnemyState/EnemyState_Dath/EnemyState_Dath.h"
#include"../../../../Data/CharacterData/CharacterData.h"

const uint32_t AetheriusEnemy::TypeID = GenerateTypeID();

void AetheriusEnemy::Init()
{
	CharacterBase::Init();

	// 初期状態のアニメーション設定
	m_animator->SetAnimation(m_modelWork->GetData()->GetAnimation("Idle"));

	// 当たり判定の設定
	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape("EnemySphere", sphere, KdCollider::TypeDamage);
	m_pCollider->RegisterCollisionShape("PlayerSphere", sphere, KdCollider::TypeEnemyHit);

	// ステート初期化
	StateInit();

	m_isAtkPlayer = false;
	m_dissever = 0.0f;
	m_invincible = false;
	m_Expired = false;

	// 要変更
	m_characterData->SetCharacterData().hp = 100;
	m_characterData->SetCharacterData().maxHp = 100;
	m_characterData->SetCharacterData().attack = 10;
}

void AetheriusEnemy::Update()
{
	// 自分の武器が未割り当て/消滅なら一度だけ取得して所有者に設定する
	if (m_wpSword.expired())
	{
		SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemySword, m_object);
		for (auto& w : m_object)
		{
			if (auto weapon = w.lock())
			{
				if (weapon->GetTypeID() == EnemySword::TypeID)
				{
					auto castedEnemySword = std::static_pointer_cast<EnemySword>(weapon);
					if (castedEnemySword->GetOwnerEnemy().expired())
					{
						castedEnemySword->SetOwnerEnemy(std::static_pointer_cast<AetheriusEnemy>(shared_from_this()));
						m_wpSword = castedEnemySword;
						break;
					}
				}
			}
		}
		m_object.clear();
	}

	if (m_wpShield.expired())
	{
		SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemyShield, m_object);
		for (auto& w : m_object)
		{
			if (auto weapon = w.lock())
			{
				if (weapon->GetTypeID() == EnemyShield::TypeID)
				{
					auto castedEnemyShield = std::static_pointer_cast<EnemyShield>(weapon);
					if (castedEnemyShield->GetOwnerEnemy().expired())
					{
						castedEnemyShield->SetOwnerEnemy(std::static_pointer_cast<AetheriusEnemy>(shared_from_this()));
						m_wpShield = castedEnemyShield;
						break;
					}
				}
			}
		}
		m_object.clear();
	}

	CharacterBase::Update();

	// ヒット処理。
	if (m_isHit)
	{
		m_isHit = false;

		if (m_invincible) return;

		// ダメージステートに変更
		auto spDamageState = std::make_shared<EnemyState_Hit>();
		ChangeState(spDamageState);
		return;
	}

	// 自分が所有する剣のみ行列を更新
	if (auto sword = m_wpSword.lock())
	{
		if (auto rightHandNode = m_modelWork->FindWorkNode("weapon_r"); rightHandNode)
		{
			sword->SetEnemyRightHandMatrix(rightHandNode->m_worldTransform);
			sword->SetEnemyMatrix(m_mWorld);
		}
	}

	// 自分が所有する盾のみ行列を更新
	if (auto shield = m_wpShield.lock())
	{
		if (auto leftHandNode = m_modelWork->FindWorkNode("weapon_l"); leftHandNode)
		{
			shield->SetEnemyLeftHandMatrix(leftHandNode->m_worldTransform);
			shield->SetEnemyMatrix(m_mWorld);
		}
	}
}

void AetheriusEnemy::StateInit()
{
	auto idleState = std::make_shared<EnemyState_Idle>();
	ChangeState(idleState);
}

void AetheriusEnemy::ChangeState(std::shared_ptr<EnemyStateBase> _state)
{
	_state->SetEnemy(this);
	m_stateManager.ChangeState(_state);
}

void AetheriusEnemy::Damage(int _damage)
{
	m_getDamage = _damage;
	m_characterData->SetCharacterData().hp -= _damage;
	if (m_characterData->GetCharacterData().hp <= 0)
	{
		m_Expired = true;
	}
}