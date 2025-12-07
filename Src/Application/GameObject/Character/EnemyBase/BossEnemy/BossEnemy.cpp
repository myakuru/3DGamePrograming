#include "BossEnemy.h"
#include "Application/GameObject/Character/Player/Player.h"
#include "Application/Scene/SceneManager.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"
#include "Application/GameObject/Weapon/EnemySword/EnemySword.h"
#include "Application/GameObject/Weapon/EnemyShield/EnemyShield.h"
#include "Application/GameObject/Collition/Collition.h"
#include "Application/main.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"
#include "BossEnemyState/BossEnemyState_Enter/BossEnemyState_Enter.h"
#include "BossEnemyState/BossEnemyState_Hit/BossEnemyState_Hit.h"
#include "BossEnemyState/BossEnemyState_Dodge/BossEnemyState_Dodge.h"
#include "Application/Data/CharacterData/CharacterData.h"
#include "Application/GameObject/Effect/EffekseerEffect/EnemyHitEffect/EnemyHitEffect.h"
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyConfig/BossEnemyConfig.h"
#include "BossEnemyState/BossEnemyAI.h"

const uint32_t BossEnemy::TypeID = KdGameObject::GenerateTypeID();

void BossEnemy::Init()
{
	EnemyBase::Init(); // CharacterBase::Init() 呼び出し済み

	if (!SceneManager::Instance().IsBossAppear()) return;

	m_modelWork->SetModelData("Asset/Models/Enemy/BossEnemy/BossEnemy.gltf");

	Movement().rotateSpeed = 10.0f;
	Movement().moveSpeed = 0.1f;

	GetAnimatorShared()->SetAnimation(m_modelWork->GetData()->GetAnimation("Idle"));

	m_position = { -8.0f, 101.0f, 18.0f };

	m_lastAction = ActionType::None;

	StateInit();

	// ステータス初期値（暫定）
	GetCharacterData()->SetCharacterData().hp = 500;
	GetCharacterData()->SetCharacterData().maxHp = 500;
	GetCharacterData()->SetCharacterData().attack = 10;

	// Config 読み込み
	m_bossEnemyConfig = std::make_shared<BossEnemyConfig>();
	if (m_bossEnemyConfig)
	{
		m_bossEnemyConfig->CreateStates();
		const nlohmann::json cfg = JSON_MANAGER.JsonDeserialize("Json/BossEnemyConfig/BossEnemyConfig");
		if (!cfg.is_null()) m_bossEnemyConfig->JsonInput(cfg);
	}

	// AI
	m_bossEnemyAI = std::make_shared<BossEnemyAI>();
	if (m_bossEnemyAI)
	{
		const nlohmann::json ai = JSON_MANAGER.JsonDeserialize("Json/BossEnemyConfig/BossEnemyAI/BossEnemyAI");
		if (!ai.is_null()) m_bossEnemyAI->JsonInput(ai);
	}
}

void BossEnemy::Update()
{
	if (!SceneManager::Instance().IsBossAppear()) return;

	float unscaledDt = Application::Instance().GetUnscaledDeltaTime();
	float dt = Application::Instance().GetDeltaTime();

	// ゲームクリアで強制終了
	if (SceneManager::Instance().m_gameClear) m_expired = true;

	// 死亡後のディゾルブ進行
	if (m_expired)
	{
		if (Rendering().dissolvePower < 1.0f)
		{
			Rendering().dissolvePower += 2.0f * unscaledDt;
			if (Rendering().dissolvePower > 1.0f) Rendering().dissolvePower = 1.0f;
		}
	}

	// クールダウン
	TickCooldowns(dt);

	EnemyBase::Update(); // アニメ・移動等

	SearchHitEffect();

	// 被弾処理
	if (GetHitCheck())
	{
		SetHitCheck(false);

		// エフェクト
		if (auto hitEffect = GetHitEffect().lock())
		{
			if (auto me = std::static_pointer_cast<BossEnemy>(GetMyAdls()))
			{
				hitEffect->PlayForBossEnemy(me);
			}
		}

		// ブラー開始（既に有効ならリセットしない）
		if (!GetEnableRadialBlur())
		{
			SetEnableRadialBlur(true);
			ResetBlurTime();
		}

		// 無敵なら累積だけリセット
		if (GetInvincible())
		{
			ResetHitCount();
			return;
		}

		// Hitステートへ
		auto spDamageState = std::make_shared<BossEnemyState_Hit>();
		ChangeState(spDamageState);
		return;
	}

	// ブラー制御
	if (GetEnableRadialBlur())
	{
		AddBlurTime(unscaledDt);

		// ヒットストップ
		if (GetBlurTime() <= 0.1f) Physics().hitStop = 0.0f;
		else                       Physics().hitStop = 1.0f;

		if (GetBlurTime() <= 0.3f)
		{
			KdShaderManager::Instance().m_postProcessShader.SetRadialBlur(0.1f, 2.0f, { 0.5f,0.55f });
			KdShaderManager::Instance().m_postProcessShader.SetEnableRadialBlur(true);

			if (static_cast<int>(std::floor(GetBlurTime())) % 10 == 0)
			{
				Math::Vector3 jitter{
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

	// HP 半分以下で回避ステートへ（1回だけ）
	/*if (GetCharacterData()->GetCharacterData().hp <= (GetCharacterData()->GetCharacterData().maxHp / 2)
		&& m_lastAction != ActionType::Dodge)
	{
		auto dodgeState = std::make_shared<BossEnemyState_Dodge>();
		ChangeState(dodgeState);
		return;
	}*/
}

void BossEnemy::DrawLit()
{
	if (!SceneManager::Instance().IsBossAppear()) return;
	SelectDraw3dModel::DrawLit();
}

void BossEnemy::StateInit()
{
	auto enterState = std::make_shared<BossEnemyState_Enter>();
	ChangeState(enterState);
}

void BossEnemy::ChangeState(std::shared_ptr<BossEnemyStateBase> _state)
{
	_state->SetBossEnemy(this);

	Physics().hitStop = 1.0f;
	ResetBlurTime();

	if (m_bossEnemyConfig)
	{
		m_bossEnemyConfig->ApplyPrototypeParametersTo(*_state);
	}
	GetStateManager().ChangeState(_state);
}

void BossEnemy::Damage(int _damage)
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
		m_expired = true;
		SetInvincible(true);
		SetHitCheck(false);
		// 死亡後の追加演出が必要ならここにステート遷移を追加
	}
}

void BossEnemy::ImGuiInspector()
{
	EnemyBase::ImGuiInspector();
	if (m_bossEnemyConfig)
	{
		m_bossEnemyConfig->InGuiInspector();
	}

	ImGui::Separator();
	ImGui::Text(U8("BossEnemy 拡張"));
	ImGui::Text(U8("LastAction")); ImGui::SameLine();
	ImGui::TextColored({ 1,1,0,1 }, std::to_string(static_cast<int>(m_lastAction)).c_str());
	ImGui::DragFloat(U8("MeleeCD"), &m_meleeCooldown, 0.01f, 0.0f);
	ImGui::DragFloat(U8("WaterCD"), &m_waterCooldown, 0.01f, 0.0f);
	ImGui::DragFloat(U8("WaterFallCD"), &m_waterFallCooldown, 0.01f, 0.0f);
	ImGui::Checkbox(U8("Expired"), &m_expired);
	ImGui::DragInt(U8("LastDamageReceived"), &m_lastDamageReceived);
}

void BossEnemy::JsonInput(const nlohmann::json& _json)
{
	EnemyBase::JsonInput(_json);
	if (m_bossEnemyConfig) m_bossEnemyConfig->JsonInput(_json);

	if (_json.contains("Boss_LastAction"))        m_lastAction = static_cast<ActionType>(_json["Boss_LastAction"].get<int>());
	if (_json.contains("Boss_MeleeCooldown"))     m_meleeCooldown = _json["Boss_MeleeCooldown"].get<float>();
	if (_json.contains("Boss_WaterCooldown"))     m_waterCooldown = _json["Boss_WaterCooldown"].get<float>();
	if (_json.contains("Boss_WaterFallCooldown")) m_waterFallCooldown = _json["Boss_WaterFallCooldown"].get<float>();
	if (_json.contains("Boss_Expired"))           m_expired = _json["Boss_Expired"].get<bool>();
	if (_json.contains("Boss_LastDamage"))        m_lastDamageReceived = _json["Boss_LastDamage"].get<int>();
}

void BossEnemy::JsonSave(nlohmann::json& _json) const
{
	EnemyBase::JsonSave(_json);
	if (m_bossEnemyConfig) m_bossEnemyConfig->JsonSave();
	if (m_bossEnemyAI)     m_bossEnemyAI->JsonSave();

	_json["Boss_LastAction"] = static_cast<int>(m_lastAction);
	_json["Boss_MeleeCooldown"] = m_meleeCooldown;
	_json["Boss_WaterCooldown"] = m_waterCooldown;
	_json["Boss_WaterFallCooldown"] = m_waterFallCooldown;
	_json["Boss_Expired"] = m_expired;
	_json["Boss_LastDamage"] = m_lastDamageReceived;
}

void BossEnemy::SetDissolve(float v)
{
	if (v < 0.0f) v = 0.0f;
	else if (v > 1.0f) v = 1.0f;
	Rendering().dissolvePower = v;
}