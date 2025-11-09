#pragma once
#include "../EnemyBase.h"
class BossEnemyStateBase;
class Player;
class EnemySword;
class EnemyShield;
class BossEnemy :public EnemyBase
{
public:
	// クラスごとに一意なTypeIDを持たせる
	static const uint32_t TypeID;
	BossEnemy() { m_typeID = TypeID; AddTag(ObjTag::EnemyLike); } // タグ付与
	~BossEnemy() override = default;

	// 行動種別
	enum class ActionType { None, Idle, Run, AttackL, AttackR, Water, WaterFall,Dodge };

	void Init() override;
	void Update() override;


	void StateInit();
	void ChangeState(std::shared_ptr<BossEnemyStateBase> _state);

	const std::weak_ptr<Player>& GetPlayerWeakPtr() const
	{
		return m_wpPlayer;
	}

	// ダメージを受ける
	void Damage(int _damage);

	void HitCheck(bool _isHit)
	{
		m_isAtkPlayer = _isHit;
	}

	bool GetHitCheck() const
	{
		return m_isAtkPlayer;
	}

	// Enemyがダメージ受けたときのセッター
	bool EnemyHit() const
	{
		return m_isHit;
	}

	void SetEnemyHit(bool _hit)
	{
		m_isHit = _hit;
	}
	int GetDamage() const
	{
		return m_getDamage;
	}

	const CharacterData& GetStatus()
	{
		return *m_characterData;
	}

	// 回避成功フラグの取得
	bool GetJustAvoidSuccess() const { return m_justAvoidSuccess; }
	void SetJustAvoidSuccess(bool flag) { m_justAvoidSuccess = flag; }

	// 当たり判定リセット
	void ResetAttackCollision()
	{
		m_chargeAttackCount = 0;
		m_chargeAttackTimer = 0.0f;
		m_isChargeAttackActive = false;
		m_hitOnce = false;
	}

	// Hit状態への遷移しなくなる。
	void SetInvincible(bool _flag) { m_invincible = _flag; }

	bool GetInvincible() const { return m_invincible; }

	// 敵への累積ヒット回数（全ステート共通）
	int  GetHitCount() const { return m_totalHitCount; }
	void IncrementHitCount() { ++m_totalHitCount; }
	void ResetHitCount() { m_totalHitCount = 0; }

	// ステート切り替えフラグの取得
	void SetStateChange(bool flag) { m_stateChange = flag; }

	// 行動コンテキストAPI
	void SetLastAction(ActionType t) { m_lastAction = t; }
	ActionType GetLastAction() const { return m_lastAction; }

	void SetMeleeCooldown(float sec) { m_meleeCooldown = std::max(m_meleeCooldown, sec); }
	float GetMeleeCooldown() const { return m_meleeCooldown; }

	void SetWaterCooldown(float sec) { m_waterCooldown = std::max(m_waterCooldown, sec); }
	float GetWaterCooldown() const { return m_waterCooldown; }

	void SetWaterFallCooldown(float sec) { m_waterFallCooldown = std::max(m_waterFallCooldown, sec); }
	float GetWaterFallCooldown() const { return m_waterFallCooldown; }

	void TickCooldowns(float dt)
	{
		m_meleeCooldown = std::max(0.0f, m_meleeCooldown - dt);
		m_waterCooldown = std::max(0.0f, m_waterCooldown - dt);
		m_waterFallCooldown = std::max(0.0f, m_waterFallCooldown - dt);
	}

private:

	std::list<std::weak_ptr<KdGameObject>> m_enemySwords; // 敵の剣
	std::list<std::weak_ptr<KdGameObject>> m_enemyShields; // 敵の盾

	std::list<std::weak_ptr<KdGameObject>> m_player;

	// ステート切り替えフラグ
	bool m_stateChange = false;

	// 追加: 行動コンテキスト
	ActionType m_lastAction = ActionType::None;
	float m_meleeCooldown = 0.0f;
	float m_waterCooldown = 0.0f;
	float m_waterFallCooldown = 0.0f;
};