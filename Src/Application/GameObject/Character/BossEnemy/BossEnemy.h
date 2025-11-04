#pragma once
#include "../CharacterBase.h"
class BossEnemyStateBase;
class Player;
class EnemySword;
class EnemyShield;
class BossEnemy :public CharaBase
{
public:
	// クラスごとに一意なTypeIDを持たせる
	static const uint32_t TypeID;
	BossEnemy() { m_typeID = TypeID; AddTag(ObjTag::EnemyLike); } // タグ付与
	~BossEnemy() override = default;

	// 追加: 行動種別
	enum class ActionType { None, Idle, Run, AttackL, AttackR, Water, WaterFall,Dodge };

	void Init() override;
	void Update() override;
	void DrawLit() override;
	void PostUpdate() override;
	void ImGuiInspector() override;
	void JsonInput(const nlohmann::json& _json) override;
	void JsonSave(nlohmann::json& _json) const override;

	void UpdateQuaternion(Math::Vector3& _moveVector) override;

	void StateInit();
	void ChangeState(std::shared_ptr<BossEnemyStateBase> _state);

	const std::weak_ptr<Player>& GetPlayerWeakPtr() const
	{
		return m_wpPlayer;
	}

	// 攻撃の当たり判定(攻撃半径、攻撃距離、攻撃回数、攻撃間隔、当たり判定の開始秒、終了秒)
	// 開始 > 終了なら入れ替え（クランプはしない）
	void UpdateAttackCollision(float _radius = 1.f, float _distance = 1.1f,
		int _attackCount = 5, float _attackTimer = 0.3f,
		float _activeBeginSec = 0.0f, float _activeEndSec = 3.0f);

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

	struct BossEnemyStatus
	{
		int hp = 100000;			// 体力
		int attack = 10;			// 攻撃力
		int maxHp = 100000;			// 最大体力
	};

	const BossEnemyStatus& GetStatus() { return m_status; }

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

	// 追加: 行動コンテキストAPI
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

	std::weak_ptr<Player> m_wpPlayer;

	bool m_Expired = false;				// 敵を消滅させるかどうか
	bool m_isHit = false;				// ヒット判定
	bool m_isAtkPlayer = false;

	int m_getDamage = 0;				// 受けるダメージ量

	float m_attackRadius = 1.5f;		// 攻撃判定の半径
	float m_attackFrame = 0.0f;			// 攻撃判定フレーム

	std::list<std::weak_ptr<KdGameObject>> m_enemySwords; // 敵の剣
	std::list<std::weak_ptr<KdGameObject>> m_enemyShields; // 敵の盾

	std::list<std::weak_ptr<KdGameObject>> m_player;

	BossEnemyStatus m_status;				// 敵のステータス

	// ジャスト回避成功フラグ
	bool m_justAvoidSuccess = false;

	bool m_hitOnce = false;
	int m_chargeAttackCount = 0;								// 何回ダメージを与えたか
	float m_chargeAttackTimer = 0.0f;							// 経過時間
	bool m_isChargeAttackActive = false;						// 連続攻撃中か

	bool m_invincible = false;					// 無敵判定用
	int m_totalHitCount = 0;					// 累積ヒット回数（無敵判定用）

	// ステート切り替えフラグ
	bool m_stateChange = false;

	// 追加: 行動コンテキスト
	ActionType m_lastAction = ActionType::None;
	float m_meleeCooldown = 0.0f;
	float m_waterCooldown = 0.0f;
	float m_waterFallCooldown = 0.0f;

	// 攻撃の有効時間ウィンドウ（クランプなし）
	float m_attackActiveTime = 0.0f;	// 攻撃開始からの経過時間
	float m_attackActiveBegin = 0.0f;	// 当たり判定が有効になる開始秒
	float m_attackActiveEnd = 3.0f;		// 当たり判定が無効化される終了秒
};