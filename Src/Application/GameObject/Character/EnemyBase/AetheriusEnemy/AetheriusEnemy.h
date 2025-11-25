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
	// クラスごとに一意なTypeID
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

	const CharacterData& GetEnemyStatus() const { return *m_characterData; }

	// 累積ヒット回数（旧インターフェース維持）
	int  GetHitCount() const { return m_totalHitCount; }
	void IncrementHitCount() { ++m_totalHitCount; }
	void ResetHitCount() { m_totalHitCount = 0; }

	// ディゾルブ
	void  SetDissolve(float v);
	float GetDissolve() const { return m_rendering.dissolvePower; }

private:
	void StateInit();

	void ImGuiInspector() override;
	void JsonInput(const nlohmann::json& _json) override;
	void JsonSave(nlohmann::json& _json) const override;

	std::shared_ptr<AetheriusEnemyConfig> m_config;

	std::vector<std::weak_ptr<EnemySword>>  m_enemySwords;
	std::vector<std::weak_ptr<EnemyShield>> m_enemyShields;

	// 死亡フラグ
	bool m_expired = false;
	// 直近受けたダメージ
	int  m_lastDamageReceived = 0;
};