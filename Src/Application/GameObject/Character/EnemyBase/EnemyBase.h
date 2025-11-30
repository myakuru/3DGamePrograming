#pragma once
#include "../CharacterBase.h"

class Player;
class EnemyHitEffect;

class EnemyBase : public CharacterBase
{
public:
	EnemyBase() = default;
	~EnemyBase() override = default;

	// 攻撃判定更新
	void UpdateAttackCollision(float _radius = 1.f, float _distance = 1.1f,
		int _attackCount = 5, float _attackInterval = 0.3f,
		float _activeBeginSec = 0.0f, float _activeEndSec = 3.0f);

	void ResetAttackCollision();

	// 公開アクセサ
	bool  GetJustAvoidSuccess() const { return m_avoid.justSuccess; }
	void  SetJustAvoidSuccess(bool f) { m_avoid.justSuccess = f; }

	int   GetChargeCount()     const { return m_charge.count; }
	float GetChargeTimer()     const { return m_charge.timer; }
	bool  IsChargeActive()     const { return m_charge.active; }

	bool  IsAttackSetupDone()  const { return m_action.attackSetupDone; }
	bool  IsAtkPlayer()        const { return m_action.isAttack; }
	void  SetAttack(bool f) { m_action.isAttack = f; }

	bool  GetEnableRadialBlur() const { return m_visual.enableRadialBlur; }
	void  SetEnableRadialBlur(bool f) { m_visual.enableRadialBlur = f; }

	int  GetTotalHitCount() const { return m_totalHitCount; }
	void IncrementTotalHitCount() { ++m_totalHitCount; }
	void ResetTotalHitCount() { m_totalHitCount = 0; }


protected:
	// ----- 派生向け制御API（必要な操作だけ許可） -----

	// Avoid
	void MarkJustAvoidSuccess() { m_avoid.justSuccess = true; }
	void ClearJustAvoidSuccess() { m_avoid.justSuccess = false; }

	// Charge 読み取り
	int   ChargeTargetTotal() const { return m_charge.targetTotal; }
	float ChargeInterval()    const { return m_charge.interval; }

	// Charge 操作（開始/終了/内部タイマー更新）
	void StartCharge(int targetTotal, float interval)
	{
		m_charge.count = 0;
		m_charge.timer = 0.f;
		m_charge.active = true;
		m_charge.targetTotal = targetTotal;
		m_charge.interval = interval;
	}
	void StopCharge() { m_charge.active = false; }
	void AdvanceChargeTimer(float dt) { m_charge.timer += dt; }
	bool ChargeReadyToHit() const { return m_charge.timer >= m_charge.interval && m_charge.active; }
	void CommitChargeHit()
	{
		m_charge.count++;
		m_charge.timer = 0.f;
		if (m_charge.count >= m_charge.targetTotal) m_charge.active = false;
	}

	// ActionFlags
	void SetAttackSetupDone(bool v) { m_action.attackSetupDone = v; }

	// VisualState（時間更新のみ許可）
	float GetBlurTime() const { return m_visual.blurTime; }
	void  AddBlurTime(float dt) { m_visual.blurTime += dt; }
	void  ResetBlurTime() { m_visual.blurTime = 1.0f; }

	// 既存の仮想関数
	void Init() override;
	void PostUpdate() override;
	void DrawLit() override;
	void ImGuiInspector() override;
	void JsonInput(const nlohmann::json& _json) override;
	void JsonSave(nlohmann::json& _json) const override;
	void UpdateQuaternion(Math::Vector3& _moveVector) override;

	void SearchHitEffect();

	std::weak_ptr<EnemyHitEffect> GetHitEffect() const { return m_hitEffect; }

private:
	// 内部状態
	struct AvoidState { bool justSuccess = false; };
	struct ChargeState
	{
		int   count = 0;
		float timer = 0.f;
		bool  active = false;
		int   targetTotal = 0;
		float interval = 0.f;
	};
	struct ActionFlags
	{
		bool attackSetupDone = false;
		bool isAttack = false;
	};
	struct VisualState
	{
		bool  enableRadialBlur = false;
		float blurTime = 0.f;
	};

	AvoidState  m_avoid{};
	ChargeState m_charge{};
	ActionFlags m_action{};
	VisualState m_visual{};

	int  m_totalHitCount = 0;
	int  m_getDamage = 0;

	std::weak_ptr<EnemyHitEffect> m_hitEffect;
};