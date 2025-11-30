#pragma once

#include "../CharacterBase.h"

// 前方宣言
class Katana;
class WeaponKatanaScabbard;
class AetheriusEnemy;
class AfterImage;
class PlayerConfig;
class PlayerStateBase;

class Player : public CharacterBase
{
	// ====== 状態グループ ======

	struct AvoidState
	{
		float startTime			= 0.0f;		// 回避開始タイム
		bool  active			= false;	// 回避中かどうか
		bool  justSuccess		= false;	// ジャスト回避判定
		bool  justAvoidAttack	= false;	// ジャスト回避攻撃判定
	};

	struct ChargeState
	{
		int   count		= 0;		// 何回ダメージを与えたか
		float timer		= 0.0f;		// 経過時間
		bool  active	= false;	// 連続攻撃中か
	};

	struct CameraShakeState
	{
		Math::Vector2 power	= Math::Vector2::Zero;	// カメラシェイクの強さ
		float         time	= 0.0f;					// カメラシェイクの時間
	};

	struct ActionFlags
	{
		bool isAtkPlayer = false;	// プレイヤーと敵が接触したか
		bool useSkill	 = false;	// スキル使用中
		bool useSpecial	 = false;	// スペシャル使用中
		bool onceEffect	 = false;	// 1度きりのエフェクトトリガ
		bool guardBreak	 = false;	// ガードブレイク状態か
	};

	struct VisualState
	{
		std::shared_ptr<AfterImage> afterImage = nullptr; // 残像オブジェクト
		bool                        rimLightOn = true;    // リムライトのオン/オフ
	};

public:
	// クラスごとに一意なTypeIDを持たせる
	static const uint32_t TypeID;

	Player();
	~Player() override;

	// ライフサイクル
	void Init        () override;
	void PreUpdate   () override;
	void Update      () override;
	void PostUpdate  () override;
	void DrawLit     () override;
	void DrawRimLight() override;
	void CollisionUpdate();

	// ステート管理
	void StateInit();
	void ChangeState(std::shared_ptr<PlayerStateBase> _state);

	// 入力から移動方向を更新
	void UpdateMoveDirectionFromInput();

	// 攻撃の当たり判定(攻撃半径、攻撃距離、攻撃回数、攻撃間隔、カメラシェイクの強さ、カメラシェイクの時間、当たり判定が有効な開始秒・終了秒)
	// 開始 > 終了なら入れ替え
	void UpdateAttackCollision(float         _radius         = 10.0f          , float _distance     = 1.1f ,
							   int           _attackCount    = 5              , float _attackTimer  = 0.3f ,
		                       Math::Vector2 _cameraShakePow = { 0.3f, 0.3f } , float _cameraTime   = 0.3f ,
							   float         _activeBeginSec = 0.0f           , float _activeEndSec = 3.0f);

	// 当たり判定リセット（CharacterBase の攻撃ウィンドウを使用）
	void ResetAttackCollision()
	{
		m_charge.count      = 0;
		m_charge.timer      = 0.0f;
		m_charge.active     = false;
		m_action.onceEffect = false;

		auto wnd    = AttackWindow();
		wnd.elapsed = 0.0f;
		wnd.begin   = 0.0f;
		wnd.end     = 3.0f;
	}

	// デバッグ
	void ImGuiInspector() override;
	void JsonInput(const nlohmann::json& _json) override;
	void JsonSave(nlohmann::json& _json) const override;

	// モデルアクセス
	KdModelWork* GetModelWork() { return m_modelWork.get(); }

	// 装備/関連参照
	const std::list<std::weak_ptr<KdGameObject>>& GetEnemyLike() const { return m_enemyLike; }

	// ステータス系
	const CharacterData& GetStatus() const { return *GetCharacterData(); }
	CharacterData& SetStatus() { return *GetCharacterData(); }

	// 残像
	std::shared_ptr<AfterImage> GetAfterImage() { return m_visual.afterImage; }

	// プロパティ（移動）
	const Math::Vector3& GetMoveDirection() const { return Movement().movement; }
	Math::Vector3        GetLastMoveDirection() const { return Movement().lastDir; }
	void                 SetMoveDirection(const Math::Vector3& _moveDirection) { Movement().movement = _moveDirection; }

	// プロパティ（回避）
	void  SetAvoidFlg(bool _flg) { m_avoid.active = _flg; }
	bool  GetAvoidFlg() const { return m_avoid.active; }
	void  SetAvoidStartTime(float _time) { m_avoid.startTime = _time; }
	float GetAvoidStartTime() const { return m_avoid.startTime; }

	// プロパティ（カメラシェイク）
	const Math::Vector2& GetCameraShakePower() const { return m_cameraShake.power; }
	float                GetCameraShakeTime()  const { return m_cameraShake.time; }

	// プロパティ（被ヒット/無敵/各種フラグ）: CharacterBase へ移譲

	bool GetJustAvoidSuccess() const { return m_avoid.justSuccess; }
	void SetJustAvoidSuccess(bool _flg) { m_avoid.justSuccess = _flg; }

	void SetJustAvoidAttackSuccess(bool _flg) { m_avoid.justAvoidAttack = _flg; }

	bool GetUseSkill() const { return m_action.useSkill; }
	bool GetUseSpecial() const { return m_action.useSpecial; }

	void SetAtkPlayer(bool _flg) { m_action.isAtkPlayer = _flg; }

	// onceEffect
	void SetOnceEffect(bool _v) { m_action.onceEffect = _v; }
	bool GetOnceEffect() const { return m_action.onceEffect; }

	void SetGuardBreak(bool _v) { m_action.guardBreak = _v; }
	bool GetGuardBreak() const { return m_action.guardBreak; }

	// 右手に刀を持っているか
	bool IsRightHanded() const { return m_isRightHanded; }
	void SetRightHanded(bool _isRight) { m_isRightHanded = _isRight; }

	// ダメージ処理
	void TakeDamage(int _damage) const;

	// カタナ取得
	std::vector <std::weak_ptr<Katana>> GetKatanas() const { return m_katana; }
	std::vector <std::weak_ptr<WeaponKatanaScabbard>> GetKatanaSheaths() const { return m_sheaths; }

	// 最後にヒットさせた敵の記録/取得/クリア
	void SetLastHitEnemy(const std::shared_ptr<KdGameObject>& e) { m_lastHitEnemy = e; }
	std::weak_ptr<KdGameObject> GetLastHitEnemy() const { return m_lastHitEnemy; }
	void ClearLastHitEnemy() { m_lastHitEnemy.reset(); }

private:

	// 内部移動処理
	void ApplyHorizontalMove   (const Math::Vector3& _inputMove, float _deltaTime);
	void ApplyVerticalMove     (float                _deltaY);

	// ====== データ ======
	Player::AvoidState        m_avoid       = {};	// 回避状態
	Player::ChargeState       m_charge      = {};	// 連続攻撃状態
	Player::CameraShakeState  m_cameraShake = {};	// カメラシェイク状態
	Player::ActionFlags       m_action      = {};	// 各種アクションフラグ
	Player::VisualState       m_visual      = {};	// 残像関係

	float		m_attackBossEnemyRadius = 2.0f;		// ボスに対する当たり判定半径
	float		m_unScaledeltaTime = 0.0f;			// デフォルトのdeltaTime保存
	bool 		m_isRightHanded = false;				// 右手に刀を持っているか

	std::shared_ptr<PlayerConfig>  m_playerConfig;	// プレイヤーの設定

	// 参照
	std::list<std::weak_ptr<KdGameObject>> m_enemyLike;
	std::weak_ptr<KdGameObject> m_lastHitEnemy;

	// 装備
	std::vector <std::weak_ptr<Katana>> m_katana;
	std::vector <std::weak_ptr<WeaponKatanaScabbard>> m_sheaths;

	Math::Vector3 tmp = Math::Vector3(10.0f, 9.0f, 1.0f);
};