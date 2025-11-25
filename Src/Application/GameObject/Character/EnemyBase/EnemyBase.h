#pragma once
#include "../CharacterBase.h"

class Player;
class EnemyHitEffect;

class EnemyBase : public CharacterBase
{
public:
	EnemyBase() = default;
	~EnemyBase() override = default;

	// 攻撃判定更新（攻撃半径、距離、ヒット回数、ヒット間隔、開始秒・終了秒）
	void UpdateAttackCollision(float _radius = 1.f, float _distance = 1.1f,
		int _attackCount = 5, float _attackInterval = 0.3f,
		float _activeBeginSec = 0.0f, float _activeEndSec = 3.0f);

	// 攻撃ウィンドウ + 連続攻撃状態リセット
	void ResetAttackCollision();

	// 状態アクセサ（派生から利用しやすくする）
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

protected:
	void Init() override;
	void PostUpdate() override;
	void DrawLit() override;
	void ImGuiInspector() override;
	void JsonInput(const nlohmann::json& _json) override;
	void JsonSave(nlohmann::json& _json) const override;
	void UpdateQuaternion(Math::Vector3& _moveVector) override;

	// ====== 内部状態 ======
	struct AvoidState
	{
		bool justSuccess = false;	// ジャスト回避成功
	};

	struct ChargeState
	{
		int   count = 0;			// 累計ヒット数
		float timer = 0.0f;			// インターバル経過
		bool  active = false;		// 多段攻撃処理中
		int   targetTotal = 0;		// 目標ヒット回数
		float interval = 0.0f;		// 1ヒットごとの間隔
	};

	struct ActionFlags
	{
		bool attackSetupDone = false;	// 攻撃ウィンドウ初期化済
		bool isAttack = false;		// プレイヤーへ攻撃中フラグ
	};

	struct VisualState
	{
		bool  enableRadialBlur = false;	// 放射状ブラー有効フラグ
		float blurTime = 0.0f;			// ブラー時間管理用
	};

	AvoidState  m_avoid{};
	ChargeState m_charge{};
	ActionFlags m_action{};
	VisualState m_visual{};

	int   m_totalHitCount = 0;		// 無敵判定などに使う累積ヒット
	int   m_getDamage = 0;			// 受けたダメージ蓄積（未使用なら削除可）

	std::weak_ptr<EnemyHitEffect> m_hitEffect;
};
