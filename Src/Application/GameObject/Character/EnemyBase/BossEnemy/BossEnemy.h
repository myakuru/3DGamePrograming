#pragma once
#include "../EnemyBase.h"

class BossEnemyStateBase;
class Player;
class EnemySword;
class EnemyShield;
class BossEnemyConfig;
class BossEnemyAI;

class BossEnemy : public EnemyBase
{
public:
	static const uint32_t TypeID;
	BossEnemy() { m_typeID = TypeID; AddTag(ObjTag::EnemyLike); }
	~BossEnemy() override = default;

	enum class ActionType { None, Idle, Run, AttackL, AttackR, Water, WaterFall, Dodge };

	void Init() override;
	void Update() override;
	void DrawLit() override;

	void StateInit();
	void ChangeState(std::shared_ptr<BossEnemyStateBase> _state);

	// ダメージ
	void Damage(int _damage);

	int  GetDamage() const { return m_lastDamageReceived; }

	const CharacterData& GetStatus() const { return *GetCharacterData(); }

	// 累積ヒット回数（Baseの公開関数経由）
	int  GetHitCount() const { return GetTotalHitCount(); }
	void IncrementHitCount() { IncrementTotalHitCount(); }
	void ResetHitCount() { ResetTotalHitCount(); }

	// 行動コンテキスト
	void SetLastAction(ActionType t) { m_lastAction = t; }
	ActionType GetLastAction() const { return m_lastAction; }

	void SetMeleeCooldown(float sec) { m_meleeCooldown = std::max(m_meleeCooldown, sec); }
	void SetWaterCooldown(float sec) { m_waterCooldown = std::max(m_waterCooldown, sec); }
	void SetWaterFallCooldown(float sec) { m_waterFallCooldown = std::max(m_waterFallCooldown, sec); }

	float GetMeleeCooldown() const { return m_meleeCooldown; }
	float GetWaterCooldown() const { return m_waterCooldown; }
	float GetWaterFallCooldown() const { return m_waterFallCooldown; }

	void TickCooldowns(float dt)
	{
		m_meleeCooldown = std::max(0.0f, m_meleeCooldown - dt);
		m_waterCooldown = std::max(0.0f, m_waterCooldown - dt);
		m_waterFallCooldown = std::max(0.0f, m_waterFallCooldown - dt);
	}

	std::vector<std::weak_ptr<Player>>& GetPlayerList() { return m_player; }

	std::shared_ptr<BossEnemyAI> GetBossEnemyAI() { return m_bossEnemyAI; }

	// ディゾルブ
	void  SetDissolve(float v);
	float GetDissolve() const { return Rendering().dissolvePower; }

private:
	void ImGuiInspector() override;
	void JsonInput(const nlohmann::json& _json) override;
	void JsonSave(nlohmann::json& _json) const override;

	std::vector<std::weak_ptr<Player>> m_player;

	std::shared_ptr<BossEnemyConfig> m_bossEnemyConfig;
	std::shared_ptr<BossEnemyAI>     m_bossEnemyAI;

	StateManager<BossEnemy> m_stateMachine;

	ActionType m_lastAction = ActionType::None;
	float      m_meleeCooldown = 0.0f;
	float      m_waterCooldown = 0.0f;
	float      m_waterFallCooldown = 0.0f;

	bool m_expired = false;
	int  m_lastDamageReceived = 0;
};