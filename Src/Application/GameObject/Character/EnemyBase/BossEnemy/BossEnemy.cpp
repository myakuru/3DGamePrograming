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
#include "Application/Data/CharacterData/CharacterData.h"

#include"Application/GameObject/Effect/EffekseerEffect/EnemyHitEffect/EnemyHitEffect.h"

const uint32_t BossEnemy::TypeID = KdGameObject::GenerateTypeID();

void BossEnemy::Init()
{
	CharacterBase::Init();

	m_modelWork->SetModelData("Asset/Models/Enemy/BossEnemy/BossEnemy.gltf");

	SetDrawFlag("DrawLit", true);

	m_movement.rotateSpeed = 10.0f;

	m_animator->SetAnimation(m_modelWork->GetData()->GetAnimation("Idle"));

	m_pCollider = std::make_unique<KdCollider>();

	m_pCollider->RegisterCollisionShape("EnemySphere", m_sphere, KdCollider::TypeDamage);

	m_pCollider->RegisterCollisionShape("PlayerSphere", m_sphere, KdCollider::TypeEnemyHit);

	StateInit();

	m_position = { -8.0f,101.0f,18.0f };

	m_isAtkPlayer = false;
	m_dissever = 0.0f;

	m_movement.moveSpeed = 0.1f;

	m_invincible = false;
	m_stateChange = false;

	m_lastAction = ActionType::None;
	m_meleeCooldown = 0.0f;
	m_waterCooldown = 0.0f;

	m_characterData->SetCharacterData().hp = 500;
	m_characterData->SetCharacterData().maxHp = 500;
	m_characterData->SetCharacterData().attack = 10;
}

void BossEnemy::Update()
{
	// 球の中心座標と半径を設定
	m_sphere.Center = m_position + Math::Vector3(0.0f, 0.7f, 0.0f); // 敵の位置＋オフセット
	m_sphere.Radius = 0.2f; // 半径0.5

	m_pDebugWire->AddDebugSphere(m_sphere.Center, m_sphere.Radius, kBlueColor);

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

		// ヒットエフェクト再生
		if (auto hitEffect = m_hitEffect.lock())
		{
			if (auto me = std::static_pointer_cast<BossEnemy>(GetMyAdls()))
			{
				hitEffect->PlayForBossEnemy(me);
			}
		}

		// ヒット演出
		m_enableRadialBlur = true;
		m_blurTime = 0.0f;

		// 無敵中なら累積だけリセットして終了
		if (GetInvincible())
		{
			ResetHitCount();
			return;
		}

		// Hitステートへ遷移
		auto spDamageState = std::make_shared<BossEnemyState_Hit>();
		ChangeState(spDamageState);
		return;
	}

	// --- 毎フレームのブラー管理 ---
	if (m_enableRadialBlur)
	{
		m_blurTime += Application::Instance().GetUnscaledDeltaTime();

		if (m_blurTime <= 0.1f)
		{
			m_physics.hitStop = 0.0f; // ヒットストップ時間
		}
		else
		{
			m_physics.hitStop = 1.0f; // ヒットストップ解除
		}

		if (m_blurTime <= 0.3f) // ブラー持続時間
		{
			KdShaderManager::Instance().m_postProcessShader.SetRadialBlur(0.1f, 2.0f, { 0.5f,0.55f });
			KdShaderManager::Instance().m_postProcessShader.SetEnableRadialBlur(true);

			if (static_cast<int>(std::floor(m_blurTime)) % 10 == 0)
			{
				Math::Vector3 jitter = {
					KdRandom::GetFloat(-0.1f, 0.1f),	// X軸揺れ
					KdRandom::GetFloat(-0.1f, 0.1f),	// Y軸揺れ
					KdRandom::GetFloat(-0.1f, 0.1f)		// Z軸揺れ
				};

				Math::Vector3 effective = jitter * 0.5f; // 揺れの強さ調整
				m_mWorld.Translation(m_position + effective);
			}
		}
		else
		{
			m_enableRadialBlur = false;
			m_blurTime = 0.0f;
			m_mWorld.Translation(m_position);

			KdShaderManager::Instance().m_postProcessShader.SetEnableRadialBlur(false);
		}
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
