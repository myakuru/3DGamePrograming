#include "BossEnemy.h"
#include"Application/GameObject/Character/Player/Player.h"
#include"Application/Scene/SceneManager.h"
#include"Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"
#include"Application/GameObject/Weapon/EnemySword/EnemySword.h"
#include"Application/GameObject/Weapon/EnemyShield/EnemyShield.h"
#include"Application/GameObject/Collition/Collition.h"
#include"Application/main.h"
#include"MyFramework/Manager/JsonManager/JsonManager.h"
#include"BossEnemyState/BossEnemyState_Enter/BossEnemyState_Enter.h"
#include"BossEnemyState/BossEnemyState_Hit/BossEnemyState_Hit.h"
#include"../BossEnemy/BossEnemyState/BossEnemyState_Dodge/BossEnemyState_Dodge.h"
#include "Application\Data\CharacterData\CharacterData.h"

const uint32_t BossEnemy::TypeID = KdGameObject::GenerateTypeID();

void BossEnemy::Init()
{
	CharacterBase::Init();

	m_modelWork->SetModelData("Asset/Models/Enemy/BossEnemy/BossEnemy.gltf");

	SetDrawFlag("DrawLit", true);

	m_rotateSpeed = 10.0f;

	m_animator->SetAnimation(m_modelWork->GetData()->GetAnimation("Idle"));

	m_pCollider = std::make_unique<KdCollider>();

	m_pCollider->RegisterCollisionShape("EnemySphere", sphere, KdCollider::TypeDamage);

	m_pCollider->RegisterCollisionShape("PlayerSphere", sphere, KdCollider::TypeEnemyHit);

	StateInit();

	m_position = { 15.0f,9.8f,-58.0f };

	m_isAtkPlayer = false;
	m_dissever = 0.0f;

	m_moveSpeed = 0.1f;

	m_invincible = false;
	m_stateChange = false;

	m_lastAction = ActionType::None;
	m_meleeCooldown = 0.0f;
	m_waterCooldown = 0.0f;
}

void BossEnemy::Update()
{
	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemySword, m_enemySwords);
	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemyShield, m_enemyShields);

	// 球の中心座標と半径を設定
	sphere.Center = m_position + Math::Vector3(0.0f, 0.7f, 0.0f); // 敵の位置＋オフセット
	sphere.Radius = 0.2f; // 半径0.5

	m_pDebugWire->AddDebugSphere(sphere.Center, sphere.Radius, kBlueColor);

	SceneManager::Instance().GetObjectWeakPtr(m_wpPlayer);

	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	if (SceneManager::Instance().m_gameClear == true)
	{
		m_isExpired = true;
	}

	if (m_Expired)
	{

		if (m_dissever < 1.0f)
		{
			m_dissever += 2.0f * deltaTime;
		}
		else
		{
			m_dissever = 1.0f;
			m_isExpired = true;
		}
	}

	// クールダウン処理
	TickCooldowns(Application::Instance().GetDeltaTime());

	CharacterBase::Update();

	// ヒット処理。
	if (m_isHit)
	{
		m_isHit = false;

		if (m_invincible) return;

		// ダメージステートに変更
		auto spDamageState = std::make_shared<BossEnemyState_Hit>();
		ChangeState(spDamageState);
		return;
	}

	// ダメージが半分以下になったら攻撃状態を遷移
	if (m_characterData->GetCharacterData().hp <= m_characterData->GetCharacterData().maxHp / 2 && !m_stateChange)
	{
		auto state = std::make_shared<BossEnemyState_Dodge>();
		ChangeState(state);
		return;
	}
}

void BossEnemy::StateInit()
{
	auto spIdleState = std::make_shared<BossEnemyState_Enter>();
	ChangeState(spIdleState);
}

void BossEnemy::ChangeState(std::shared_ptr<BossEnemyStateBase> _state)
{
	_state->SetBossEnemy(this);
	m_stateManager.ChangeState(_state);
}

void BossEnemy::Damage(int _damage)
{
	m_getDamage = _damage;
	m_characterData->SetCharacterData().hp -= _damage;
	if (m_characterData->GetCharacterData().hp <= 0)
	{
		m_Expired = true;
	}
}
