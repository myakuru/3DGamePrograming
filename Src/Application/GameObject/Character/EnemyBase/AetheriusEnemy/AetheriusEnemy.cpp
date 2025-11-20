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
#include"Application/GameObject/Effect/EffekseerEffect/EnemyHitEffect/EnemyHitEffect.h"

const uint32_t AetheriusEnemy::TypeID = GenerateTypeID();

void AetheriusEnemy::Init()
{
	CharacterBase::Init();

	// 初期状態のアニメーション設定
	m_animator->SetAnimation(m_modelWork->GetData()->GetAnimation("Idle"));

	// 当たり判定の設定
	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape("EnemySphere", m_sphere, KdCollider::TypeDamage);
	m_pCollider->RegisterCollisionShape("PlayerSphere", m_sphere, KdCollider::TypeEnemyHit);

	// ステート初期化
	StateInit();

	m_isAtkPlayer = false;
	m_dissever = 0.0f;
	m_invincible = false;
	m_Expired = false;

	// 要変更
	m_characterData->SetCharacterData().hp = 500;
	m_characterData->SetCharacterData().maxHp = 500;
	m_characterData->SetCharacterData().attack = 10;

	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemySword, m_enemySwords);
	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemyShield, m_enemyShields);

	// シーン内の EnemyHitEffect を1つ取得（共有の発射台として使う）
}

void AetheriusEnemy::Update()
{
	// 自分の武器が未割り当て/消滅なら一度だけ取得して所有者に設定する
	CharacterBase::Update();

	for (const auto& w : m_enemySwords)
	{
		if (auto weapon = w.lock())
		{
			weapon->SetOwnerEnemy(std::static_pointer_cast<AetheriusEnemy>(shared_from_this()));
			if (auto rightHandNode = m_modelWork->FindWorkNode("weapon_r"))
			{
				weapon->SetEnemyRightHandMatrix(rightHandNode->m_worldTransform);
				weapon->SetEnemyMatrix(m_mWorld);
			}
		}
	}

	for (const auto& w : m_enemyShields)
	{
		if (auto weapon = w.lock())
		{
			weapon->SetOwnerEnemy(std::static_pointer_cast<AetheriusEnemy>(shared_from_this()));

			if (auto leftHandNode = m_modelWork->FindWorkNode("weapon_l"))
			{
				weapon->SetEnemyLeftHandMatrix(leftHandNode->m_worldTransform);
				weapon->SetEnemyMatrix(m_mWorld);
			}
		}
	}

	SceneManager::Instance().GetObjectWeakPtr(m_hitEffect);

	if (m_isHit)
	{
		m_isHit = false;

		// ヒットエフェクト再生
		if (auto hitEffect = m_hitEffect.lock())
		{
			if (auto me = std::static_pointer_cast<AetheriusEnemy>(GetMyAdls()))
			{
				hitEffect->PlayForEnemy(me);
			}
		}

		// 無敵中なら累積だけリセットして終了
		if (GetInvincible())
		{
			ResetHitCount();
			return;
		}

		// ヒット演出
		m_enableRadialBlur = true;
		m_blurTime = 0.0f;

		// Hitステートへ遷移
		auto spDamageState = std::make_shared<EnemyState_Hit>();
		ChangeState(spDamageState);
		return;
	}

	// --- 毎フレームのブラー管理（ヒット発生とは独立して時間管理する） ---
	if (m_enableRadialBlur)
	{
		m_blurTime += Application::Instance().GetUnscaledDeltaTime();
		if (m_blurTime <= 0.1f) // ブラー持続時間
		{
			KdShaderManager::Instance().m_postProcessShader.SetRadialBlur(0.1f, 2.0f, { 0.5f,0.55f });
			KdShaderManager::Instance().m_postProcessShader.SetEnableRadialBlur(true);
		}
		else
		{
			m_enableRadialBlur = false;
			m_blurTime = 0.0f; 
			KdShaderManager::Instance().m_postProcessShader.SetEnableRadialBlur(false);
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
	if (m_Expired) return;

	m_getDamage = _damage;
	m_characterData->SetCharacterData().hp -= _damage;
	if (m_characterData->GetCharacterData().hp < 0)
	{
		m_characterData->SetCharacterData().hp = 0;
	}

	if (m_characterData->GetCharacterData().hp == 0)
	{
		//　死亡処理
		m_Expired = true;
		m_isHit = false;
		SetInvincible(true); // 不要な追加ヒット抑止

		auto spDeathState = std::make_shared<EnemyState_Death>();
		ChangeState(spDeathState);
		return;
	}
}