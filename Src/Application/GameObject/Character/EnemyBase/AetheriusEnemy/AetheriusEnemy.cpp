#include "AetheriusEnemy.h"

#include "Application/GameObject/Character/Player/Player.h"
#include "Application/Scene/SceneManager.h"
#include "EnemyState/EnemyState_Idle/EnemyState_Idle.h"
#include "EnemyState/EnemyState_Hit/EnemyState_Hit.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"
#include "Application/GameObject/Weapon/EnemySword/EnemySword.h"
#include "Application/GameObject/Weapon/EnemyShield/EnemyShield.h"
#include "Application/GameObject/Collition/Collition.h"
#include "Application/main.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"
#include "EnemyState/EnemyState_Death/EnemyState_Death.h"
#include "../../../../Data/CharacterData/CharacterData.h"
#include "Application/GameObject/Effect/EffekseerEffect/EnemyHitEffect/EnemyHitEffect.h"
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/AetheriusEnemyConfig.h"

const uint32_t AetheriusEnemy::TypeID = GenerateTypeID();

void AetheriusEnemy::Init()
{
	EnemyBase::Init(); // CharacterBase::Init() は EnemyBase 内で呼ばれる

	// 基本ステータス（暫定。Config 反映前の初期値）
	m_characterData->SetCharacterData().hp = 500;
	m_characterData->SetCharacterData().maxHp = 500;
	m_characterData->SetCharacterData().attack = 10;

	// 武器参照取得
	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemySword, m_enemySwords);
	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemyShield, m_enemyShields);

	// Config
	m_config = std::make_shared<AetheriusEnemyConfig>();
	if (m_config)
	{
		m_config->CreateStates();

		const nlohmann::json cfg = JSON_MANAGER.JsonDeserialize("Json/AetheriusEnemyConfig/AetheriusEnemyConfig");
		if (!cfg.is_null())
		{
			m_config->JsonInput(cfg);
		}
	}

	StateInit();
}

void AetheriusEnemy::Update()
{
	EnemyBase::Update(); // アニメーション・移動など

	// 剣
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

	// 盾
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

	// 被弾処理（旧 m_isHit → CombatState.flags.isHit）
	if (GetHitCheck())
	{
		SetHitCheck(false);

		// ヒットエフェクト
		if (auto hitEffect = m_hitEffect.lock())
		{
			if (auto me = std::static_pointer_cast<AetheriusEnemy>(GetMyAdls()))
			{
				hitEffect->PlayForEnemy(me);
			}
		}

		// 演出開始（ラジアルブラー/ヒットストップ）
		m_visual.enableRadialBlur = true;
		m_visual.blurTime = 0.0f;

		// 無敵中なら累積ヒットリセットのみ
		if (GetInvincible())
		{
			ResetHitCount();
			return;
		}

		// Hit ステートへ
		auto spDamageState = std::make_shared<EnemyState_Hit>();
		ChangeState(spDamageState);
		return;
	}

	// ラジアルブラー制御
	if (m_visual.enableRadialBlur)
	{
		m_visual.blurTime += Application::Instance().GetUnscaledDeltaTime();

		// ヒットストップ
		if (m_visual.blurTime <= 0.1f)  m_physics.hitStop = 0.0f;
		else                            m_physics.hitStop = 1.0f;

		if (m_visual.blurTime <= 0.3f)
		{
			KdShaderManager::Instance().m_postProcessShader.SetRadialBlur(0.1f, 2.0f, { 0.5f,0.55f });
			KdShaderManager::Instance().m_postProcessShader.SetEnableRadialBlur(true);

			// 微妙な位置揺らぎ
			if (static_cast<int>(std::floor(m_visual.blurTime)) % 10 == 0)
			{
				Math::Vector3 jitter = {
					KdRandom::GetFloat(-0.1f, 0.1f),
					KdRandom::GetFloat(-0.1f, 0.1f),
					KdRandom::GetFloat(-0.1f, 0.1f)
				};
				m_mWorld.Translation(m_position + jitter * 0.5f);
			}
		}
		else
		{
			m_visual.enableRadialBlur = false;
			m_visual.blurTime = 0.0f;
			m_mWorld.Translation(m_position);
			KdShaderManager::Instance().m_postProcessShader.SetEnableRadialBlur(false);
		}
	}
}

void AetheriusEnemy::StateInit()
{
	auto idleState = std::make_shared<EnemyState_Idle>();
	ChangeState(idleState);
}

void AetheriusEnemy::ImGuiInspector()
{
	EnemyBase::ImGuiInspector();
	if (m_config)
	{
		m_config->InGuiInspector();
	}

	ImGui::Separator();
	ImGui::Text(U8("AetheriusEnemy 拡張"));
	ImGui::Checkbox(U8("Expired"), &m_expired);
	ImGui::DragInt(U8("LastDamageReceived"), &m_lastDamageReceived);
}

void AetheriusEnemy::JsonInput(const nlohmann::json& _json)
{
	EnemyBase::JsonInput(_json);
	if (m_config)
	{
		m_config->JsonInput(_json);
	}
	if (_json.contains("Expired"))          m_expired = _json["Expired"].get<bool>();
	if (_json.contains("LastDamage"))       m_lastDamageReceived = _json["LastDamage"].get<int>();
}

void AetheriusEnemy::JsonSave(nlohmann::json& _json) const
{
	EnemyBase::JsonSave(_json);
	if (m_config)
	{
		m_config->JsonSave();
	}
	_json["Expired"] = m_expired;
	_json["LastDamage"] = m_lastDamageReceived;
}

void AetheriusEnemy::ChangeState(std::shared_ptr<EnemyStateBase> _state)
{
	_state->SetEnemy(this);
	if (m_config)
	{
		m_config->ApplyPrototypeParametersTo(*_state);
	}
	m_stateManager.ChangeState(_state);
}

void AetheriusEnemy::Damage(int _damage)
{
	if (m_expired) return;

	m_lastDamageReceived = _damage;

	m_characterData->SetCharacterData().hp -= _damage;
	if (m_characterData->GetCharacterData().hp < 0)
	{
		m_characterData->SetCharacterData().hp = 0;
	}

	if (m_characterData->GetCharacterData().hp == 0)
	{
		// 死亡
		m_expired = true;
		SetHitCheck(false);
		SetInvincible(true);

		auto spDeathState = std::make_shared<EnemyState_Death>();
		ChangeState(spDeathState);
		return;
	}
}

void AetheriusEnemy::SetDissolve(float v)
{
	if (v < 0.0f) v = 0.0f;
	else if (v > 1.0f) v = 1.0f;
	m_rendering.dissolvePower = v;
}