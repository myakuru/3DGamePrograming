#include "RedEnemy.h"
#include "Application/GameObject/Character/Player/Player.h"
#include "Application/Scene/SceneManager.h"
#include "EnemyState/EnemyState_Idle/EnemyState_Idle.h"
#include "EnemyState/EnemyState_Hit/EnemyState_Hit.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"
#include "Application/GameObject/Weapon/EnemySword/EnemySword.h"
#include "Application/GameObject/Weapon/EnemyShield/EnemyShield.h"
#include "Application/GameObject/Collision/Collision.h"
#include "Application/main.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"
#include "EnemyState/EnemyState_Death/EnemyState_Death.h"
#include "Application/Data/CharacterData/CharacterData.h"
#include "Application/GameObject/Effect/EffekseerEffect/EnemyHitEffect/EnemyHitEffect.h"
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/AetheriusEnemyConfig/AetheriusEnemyConfig.h"

const uint32_t RedEnemy::TypeID = GenerateTypeID();

void RedEnemy::Init()
{
	EnemyBase::Init(); // CharacterBase::Init() は EnemyBase 内で呼ばれる

	// 基本ステータス（暫定。Config 反映前の初期値）
	GetCharacterData()->SetCharacterData().hp = 500;
	GetCharacterData()->SetCharacterData().maxHp = 500;
	GetCharacterData()->SetCharacterData().attack = 10;

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

void RedEnemy::Update()
{
	EnemyBase::Update(); // アニメーション・移動など

	// --- 武器の割当（近傍検索） ---
	{
		std::list<std::weak_ptr<KdGameObject>> nearby;

		// 剣：未割当なら近傍で未所有の剣を探して割り当てる
		if (m_wpSword.expired())
		{
			SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemySword, nearby);
			for (const auto& w : nearby)
			{
				if (auto sp = w.lock())
				{
					if (sp->GetTypeID() == EnemySword::TypeID)
					{
						auto sword = std::static_pointer_cast<EnemySword>(sp);
						if (sword->GetOwnerEnemy().expired())
						{
							sword->SetOwnerEnemy(std::static_pointer_cast<RedEnemy>(shared_from_this()));
							m_wpSword = sword;
							break;
						}
					}
				}
			}
			nearby.clear();
		}

		// 盾：未割当なら近傍で未所有の盾を探して割り当てる
		if (m_wpShield.expired())
		{
			SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemyShield, nearby);
			for (const auto& w : nearby)
			{
				if (auto sp = w.lock())
				{
					if (sp->GetTypeID() == EnemyShield::TypeID)
					{
						auto shield = std::static_pointer_cast<EnemyShield>(sp);
						if (shield->GetOwnerEnemy().expired())
						{
							shield->SetOwnerEnemy(std::static_pointer_cast<RedEnemy>(shared_from_this()));
							m_wpShield = shield;
							break;
						}
					}
				}
			}
			nearby.clear();
		}
	}

	// --- 自分が所有する武器だけ行列更新 ---
	if (auto sword = m_wpSword.lock())
	{
		if (auto rightHandNode = m_modelWork ? m_modelWork->FindWorkNode("weapon_r") : nullptr)
		{
			sword->SetEnemyRightHandMatrix(rightHandNode->m_worldTransform);
			sword->SetEnemyMatrix(m_mWorld);
		}
	}
	if (auto shield = m_wpShield.lock())
	{
		if (auto leftHandNode = m_modelWork ? m_modelWork->FindWorkNode("weapon_l") : nullptr)
		{
			shield->SetEnemyLeftHandMatrix(leftHandNode->m_worldTransform);
			shield->SetEnemyMatrix(m_mWorld);
		}
	}


	SearchHitEffect();

	// 被弾処理
	if (GetHitCheck())
	{
		SetHitCheck(false);

		// ヒットエフェクト
		if (auto hitEffect = GetHitEffect().lock())
		{
			if (auto me = std::static_pointer_cast<RedEnemy>(GetMyAdls()))
			{
				hitEffect->PlayForEnemy(me);
			}
		}

		// 演出開始（ラジアルブラー/ヒットストップ）
		SetEnableRadialBlur(true);
		ResetBlurTime();

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

	m_stateMachine.Update();

	// ラジアルブラー制御
	if (GetEnableRadialBlur())
	{
		AddBlurTime(Application::Instance().GetUnscaledDeltaTime());

		// ヒットストップ
		if (GetBlurTime() <= 0.2f)  Physics().hitStop = 0.0f;
		else                        Physics().hitStop = 1.0f;

		if (GetBlurTime() <= 0.3f)
		{
			KdShaderManager::Instance().m_postProcessShader.SetRadialBlur(0.1f, 2.0f, { 0.5f,0.55f });
			KdShaderManager::Instance().m_postProcessShader.SetEnableRadialBlur(true);

			// 微妙な位置揺らぎ
			if (static_cast<int>(std::floor(GetBlurTime())) % 10 == 0)
			{
				Math::Vector3 jitter = 
				{
					KdRandom::GetFloat(-0.1f, 0.1f),
					KdRandom::GetFloat(-0.1f, 0.1f),
					KdRandom::GetFloat(-0.1f, 0.1f)
				};
				m_mWorld.Translation(m_position + jitter * 0.5f);
			}
		}
		else
		{
			SetEnableRadialBlur(false);
			ResetBlurTime();
			m_mWorld.Translation(m_position);
			KdShaderManager::Instance().m_postProcessShader.SetEnableRadialBlur(false);
		}
	}
}

void RedEnemy::StateInit()
{
	m_stateMachine.Start(this);

	auto idleState = std::make_shared<EnemyState_Idle>();
	m_stateMachine.ChangeState(idleState);
}

void RedEnemy::ImGuiInspector()
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

void RedEnemy::JsonInput(const nlohmann::json& _json)
{
	EnemyBase::JsonInput(_json);
	if (m_config)
	{
		m_config->JsonInput(_json);
	}
	if (_json.contains("Expired"))          m_expired = _json["Expired"].get<bool>();
	if (_json.contains("LastDamage"))       m_lastDamageReceived = _json["LastDamage"].get<int>();
}

void RedEnemy::JsonSave(nlohmann::json& _json) const
{
	EnemyBase::JsonSave(_json);
	if (m_config)
	{
		m_config->JsonSave();
	}
	_json["Expired"] = m_expired;
	_json["LastDamage"] = m_lastDamageReceived;
}

void RedEnemy::ChangeState(const std::shared_ptr<EnemyStateBase>& _state)
{
	if (m_config)
	{
		m_config->ApplyPrototypeParametersTo(*_state);
	}
	m_stateMachine.ChangeState(_state);
}

void RedEnemy::Damage(int _damage)
{
	if (m_expired) return;

	m_lastDamageReceived = _damage;

	GetCharacterData()->SetCharacterData().hp -= _damage;
	if (GetCharacterData()->GetCharacterData().hp < 0)
	{
		GetCharacterData()->SetCharacterData().hp = 0;
	}

	if (GetCharacterData()->GetCharacterData().hp == 0)
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

void RedEnemy::SetDissolve(float v)
{
	Rendering().dissolvePower = std::clamp(v, 0.0f, 1.0f);
}