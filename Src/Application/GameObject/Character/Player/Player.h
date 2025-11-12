#pragma once

#include "../CharacterBase.h"
#include "PlayerConfig.h"

// 前方宣言（依存の最小化）
class Katana;
class WeaponKatanaScabbard;
class PlayerStateBase;
class AetheriusEnemy;
class AfterImage;

class Player : public CharacterBase
{
public:
	// クラスごとに一意なTypeIDを持たせる
	static const uint32_t TypeID;

	Player() { m_typeID = TypeID; AddTag(ObjTag::PlayerLike); }
	~Player() override = default;

	// ライフサイクル
	void Init() override;
	void PreUpdate() override;
	void Update() override;
	void PostUpdate() override;
	void DrawLit() override;
	void DrawRimLight() override;

	// ステート管理
	void StateInit();
	void ChangeState(std::shared_ptr<PlayerStateBase> _state);

	// 入力から移動方向を更新
	void UpdateMoveDirectionFromInput();

	// 攻撃の当たり判定(攻撃半径、攻撃距離、攻撃回数、攻撃間隔、カメラシェイクの強さ、カメラシェイクの時間、当たり判定が有効な開始秒・終了秒)
	// 開始 > 終了なら入れ替え
	void UpdateAttackCollision(float _radius = 10.0f, float _distance = 1.1f,
		int _attackCount = 5, float _attackTimer = 0.3f,
		Math::Vector2 _cameraShakePow = { 0.3f, 0.3f }, float _cameraTime = 0.3f,
		float _activeBeginSec = 0.0f, float _activeEndSec = 3.0f);

	// 当たり判定リセット（CharacterBase の攻撃ウィンドウを使用）
	void ResetAttackCollision()
	{
		m_charge.count = 0;
		m_charge.timer = 0.0f;
		m_charge.active = false;
		m_action.onceEffect = false;

		auto wnd = AttackWindow();
		wnd.elapsed = 0.0f;
		wnd.begin = 0.0f;
		wnd.end = 3.0f;
	}

	// デバッグ/永続化
	void ImGuiInspector() override;
	void JsonInput(const nlohmann::json& _json) override;
	void JsonSave(nlohmann::json& _json) const override;

	// モデルアクセス
	KdModelWork* GetModelWork() { return m_modelWork.get(); }

	// 装備/関連参照
	const std::weak_ptr<Katana>& GetKatana() const { return m_katana; }
	const std::weak_ptr<WeaponKatanaScabbard>& GetScabbard() const { return m_scabbard; }
	const std::list<std::weak_ptr<KdGameObject>>& GetEnemyLike() const { return m_enemyLike; }

	// ステータス系
	const CharacterData& GetStatus() const { return *m_characterData; }
	CharacterData& SetStatus() { return *m_characterData; }

	// 残像
	std::shared_ptr<AfterImage> GetAfterImage() { return m_visual.afterImage; }

	// プロパティ（移動）
	const Math::Vector3& GetMoveDirection() const { return m_movement.movement; }
	Math::Vector3        GetLastMoveDirection() const { return m_movement.lastDir; }
	void                 SetMoveDirection(const Math::Vector3& _moveDirection) { m_movement.movement = _moveDirection; }

	// プロパティ（回避）
	void  SetAvoidFlg(bool _flg) { m_avoid.active = _flg; }
	bool  GetAvoidFlg() const { return m_avoid.active; }
	void  SetAvoidStartTime(float _time) { m_avoid.startTime = _time; }
	float GetAvoidStartTime() const { return m_avoid.startTime; }

	// プロパティ（カメラシェイク）
	const Math::Vector2& GetCameraShakePower() const { return m_cameraShake.power; }
	float                GetCameraShakeTime()  const { return m_cameraShake.time; }

	// プロパティ（被ヒット/無敵/各種フラグ）: CharacterBase へ移譲
	void SetHitCheck(bool _isHit) { CharacterBase::SetHitCheck(_isHit); }
	bool GetHitCheck() const { return CharacterBase::GetHitCheck(); }

	bool GetJustAvoidSuccess() const { return m_avoid.justSuccess; }
	void SetJustAvoidSuccess(bool _flg) { m_avoid.justSuccess = _flg; }

	bool GetInvincible() const { return CharacterBase::GetInvincible(); }
	void SetInvincible(bool _flg) { CharacterBase::SetInvincible(_flg); }

	bool GetUseSkill() const { return m_action.useSkill; }
	bool GetUseSpecial() const { return m_action.useSpecial; }

	void SetAtkPlayer(bool _flg) { m_action.isAtkPlayer = _flg; }
	PlayerConfig& GetPlayerConfig() { return m_playerConfig; }

	// onceEffect
	void SetOnceEffect(bool _v) { m_action.onceEffect = _v; }
	bool GetOnceEffect() const { return m_action.onceEffect; }

	// ダメージ処理
	void TakeDamage(int _damage);

private:
	// 内部移動処理
	void ApplyHorizontalMove(const Math::Vector3& _inputMove, float _deltaTime);
	void ApplyPushWithCollision(const Math::Vector3& _rawPush);
	void ApplyVerticalMove(float _deltaY);

	// ====== 状態グループ ======

	struct AvoidState {
		bool  active = false; // 回避中かどうか
		float startTime = 0.0f;  // 回避開始タイム
		bool  justSuccess = false; // ジャスト回避判定
	};

	struct ChargeState {
		int   count = 0;     // 何回ダメージを与えたか
		float timer = 0.0f;  // 経過時間
		bool  active = false; // 連続攻撃中か
	};

	struct CameraShakeState {
		Math::Vector2 power = Math::Vector2::Zero; // カメラシェイクの強さ
		float         time = 0.0f;                // カメラシェイクの時間
	};

	struct ActionFlags {
		bool isAtkPlayer = false; // プレイヤーと敵が接触したか
		bool useSkill = false; // スキル使用中
		bool useSpecial = false; // スペシャル使用中
		bool onceEffect = false; // 1度きりのエフェクトトリガ
	};

	struct VisualState {
		std::shared_ptr<AfterImage> afterImage; // 残像オブジェクト
		bool rimLightOn = true;                 // リムライトのオン/オフ
	};

	// ====== データ ======
	AvoidState        m_avoid;
	ChargeState       m_charge;
	CameraShakeState  m_cameraShake;
	ActionFlags       m_action;
	VisualState       m_visual;

	float             m_attackBossEnemyRadius = 2.0f; // ボスに対する当たり判定半径
	float             m_unScaledeltaTime = 0.0f; // デフォルトのdeltaTime保存

	PlayerConfig      m_playerConfig;                 // プレイヤーの設定

	// 参照
	std::list<std::weak_ptr<KdGameObject>> m_enemyLike;

	// 装備
	std::weak_ptr<Katana>               m_katana;
	std::weak_ptr<WeaponKatanaScabbard> m_scabbard;
};