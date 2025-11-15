#pragma once
#include "../EnemyBase.h"
class EnemyStateBase;
class Player;
class EnemySword;
class EnemyShield;
class AetheriusEnemy :public EnemyBase
{
public:
	// クラスごとに一意なTypeIDを持たせる
	static const uint32_t TypeID;
	AetheriusEnemy() { m_typeID = TypeID; AddTag(ObjTag::EnemyLike); } // タグ付与
	~AetheriusEnemy() override = default;

	void Init() override;
	void Update() override;
	void ChangeState(std::shared_ptr<EnemyStateBase> _state);

	std::weak_ptr<Player>& GetPlayerWeakPtr()
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

	const CharacterData& GetEnemyStatus()
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

		// 時間ウィンドウも初期化
		m_attackActiveTime = 0.0f;
		m_attackActiveBegin = 0.0f;
		m_attackActiveEnd = 3.0f;
	}

	// 敵への累積ヒット回数（全ステート共通）
	int  GetHitCount() const { return m_totalHitCount; }
	void IncrementHitCount() { ++m_totalHitCount; }
	void ResetHitCount() { m_totalHitCount = 0; }

	void SetDissolve(float v)
	{
		// 0～1にクランプ
		if (v < 0.0f) v = 0.0f;
		else if (v > 1.0f) v = 1.0f;
		m_dissever = v;
	}

	float GetDissolve() const
	{
		return m_dissever;
	}

private:

	void StateInit();

	std::weak_ptr<EnemySword> m_wpSword;
	std::weak_ptr<EnemyShield> m_wpShield;

	// ブラーを発生させる時間
	float m_blurTime = 0.0f;

};