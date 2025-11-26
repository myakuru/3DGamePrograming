#pragma once
#include "../EnemyBase.h"

class EnemyStateBase;
class Player;
class EnemySword;
class EnemyShield;
class AetheriusEnemyConfig;

class AetheriusEnemy : public EnemyBase
{
public:
	static const uint32_t TypeID;
	AetheriusEnemy() { m_typeID = TypeID; AddTag(ObjTag::EnemyLike); }
	~AetheriusEnemy() override = default;

	void Init() override;
	void Update() override;

	// ステート遷移
	void ChangeState(std::shared_ptr<EnemyStateBase> _state);

	// ダメージ処理
	void Damage(int _damage);

	int GetDamage() const { return m_lastDamageReceived; }

	const CharacterData& GetEnemyStatus() const { return *GetCharacterData(); }

	// 累積ヒット回数（Baseの公開関数経由）
	int  GetHitCount() const { return GetTotalHitCount(); }
	void IncrementHitCount() { IncrementTotalHitCount(); }
	void ResetHitCount() { ResetTotalHitCount(); }

	// ディゾルブ
	void  SetDissolve(float v);
	float GetDissolve() const { return Rendering().dissolvePower; }

private:
	void StateInit();

	void ImGuiInspector() override;
	void JsonInput(const nlohmann::json& _json) override;
	void JsonSave(nlohmann::json& _json) const override;

	std::shared_ptr<AetheriusEnemyConfig> m_config;			// ステート構成情報

	std::vector<std::weak_ptr<EnemySword>>  m_enemySwords;	// 武器参照
	std::vector<std::weak_ptr<EnemyShield>> m_enemyShields;	// 武器参照

	bool m_expired = false;
	int  m_lastDamageReceived = 0;
};