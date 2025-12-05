#pragma once

#include "Application/GameObject/Utility/SelectDraw3dModel.h"
#include "MyFramework/State/StateManager/StateManager.h"

// 前方宣言（依存の最小化）
class PlayerCamera;
class StateManager;
class CharacterData;
class Collision;
class Player;
class AetheriusEnemy;
class BossEnemy;

class CharacterBase : public SelectDraw3dModel
{
public:
	CharacterBase() = default;
	~CharacterBase() override = default;

	// ===== ライフサイクル =====
	void Init      () override;
	void Update    () override;
	void PostUpdate() override;
	bool ModelLoad (const std::string& _path) override;

	// ===== 移動・回転 =====
	// 移動ベクトルを設定
	void SetIsMoving(Math::Vector3 _move);
	const Math::Vector3& GetMovement() const;
	bool GetIsMoving() const { return m_movement.isMoving; }

	// コリジョン抜けしないように押し出しを適用する関数
	void ApplyPushWithCollision(const Math::Vector3& _rawPush);

	Math::Matrix& GetRotationMatrix();
	void SetRotation(const Math::Quaternion& _rotation);
	Math::Quaternion& GetRotationQuaternion();

	void SetPosition(const Math::Vector3& _position);

	// ムーブベクトルに基づいてクォータニオンを更新
	virtual void UpdateQuaternion(Math::Vector3& _moveVector);
	void UpdateQuaternionDirect(const Math::Vector3& _direction);

	// ===== アニメーション =====
	std::shared_ptr<KdModelWork> GetAnimeModel();
	std::shared_ptr<KdAnimator> GetAnimator();
	void SetAnimeSpeed(float _speed);

	// ===== カメラ =====
	std::weak_ptr<PlayerCamera> GetPlayerCamera() const;

	// ===== アクセサ（戦闘系の共通化） =====
	// 被弾/無敵
	void SetHitCheck(bool _isHit) { m_combat.flags.isHit = _isHit; }
	bool GetHitCheck() const { return m_combat.flags.isHit; }

	void SetInvincible(bool _flag) { m_combat.flags.invincible = _flag; }
	bool GetInvincible() const { return m_combat.flags.invincible; }

	// 攻撃の有効時間ウィンドウ（参照ビュー）
	struct AttackWindowRef { float& elapsed; float& begin; float& end; };
	struct AttackWindowCRef { const float& elapsed; const float& begin; const float& end; };

	AttackWindowRef AttackWindow() { return { m_combat.attackWindow.elapsed, m_combat.attackWindow.begin, m_combat.attackWindow.end }; }
	AttackWindowCRef AttackWindow() const { return { m_combat.attackWindow.elapsed, m_combat.attackWindow.begin, m_combat.attackWindow.end }; }

	// HitStop
	void SetHitStop(float _time) { m_physics.hitStop = _time; }
	float GetHitStop() const { return m_physics.hitStop; }

private:
	// ====== 責務ごとに束ねた内部状態 ======
	struct TransformState
	{
		Math::Matrix     rotationM = Math::Matrix::Identity;     // 回転行列
		Math::Quaternion rotationQ = Math::Quaternion::Identity; // 回転（クォータニオン）
	};

	struct MovementState
	{
		Math::Vector3 movement		= Math::Vector3::Zero;	// 現在の移動ベクトル
		float moveSpeed				= 0.0f;					// 移動速度
		float rotateSpeed			= 0.0f;					// 回転速度
		Math::Vector3 lastDir		= Math::Vector3::Zero;	// 最後に移動した方向ベクトル
		bool isMoving				= false;				// 移動中かどうか
	};

	struct PhysicsState
	{
		float gravity		 = 0.0f;	// 重力加速度
		float gravitySpeed	 = 0.0f;	// 現在の重力速度
		float fixedFrameRate = 0.0f;    // 60fps換算等
		float hitStop = 1.0f;			// ヒットストップ時間
	};

	struct RaycastState
	{
		Math::Vector3 prevPosition{};			// 前フレームのワールド位置
		float forwardRayYOffset		= 0.35f;	// プレイヤー中心(腰程度)の高さ
		float forwardRayMargin		= 0.02f;	// 壁手前で残すマージン
		float forwardRayExtra		= 0.05f;	// 余剰距離(浮動小数ヒット安定用)
		float bumpSphereRadius		= 0.2f;		// 壁めり込み防止球の半径
		float bumpSphereYOffset		= 0.3f;		// プレイヤー中心(腰程度)の高さ
		float collisionMargin		= 1.0f;		// 壁にめり込まないための余白
	};

	struct RenderingState
	{
		Math::Vector3 dissolveColor = Math::Vector3::Zero; // 溶解エフェクトの色
		float         dissolvePower = 1.0f;                // 溶解進行度
	};

	struct RefsState
	{
		std::weak_ptr<PlayerCamera>					playerCamera;		// プレイヤーカメラ
		std::weak_ptr<KdGameObject>					collision;			// 当たり判定オブジェクト
		std::list<std::shared_ptr<KdGameObject>>	effectList;			// エフェクトオブジェクトリスト
		std::list<std::weak_ptr<KdGameObject>>		enemyList;			// エーテリウスエネミーオブジェクトリスト
		std::vector<std::weak_ptr<Collision>>		collisionObjects;	// 当たり判定オブジェクトリスト
		std::vector<std::weak_ptr<Player>>			playerObjects;		// プレイヤーオブジェクトリスト
	};

	struct CombatState
	{
		struct Flags
		{
			bool isHit		= false;	// 被弾判定
			bool invincible = false;	// 無敵判定
		};

		struct AttackWindow
		{
			float elapsed	= 0.0f;		// 攻撃開始からの経過時間
			float begin		= 0.0f;		// 当たり判定が有効になる開始秒
			float end		= 3.0f;		// 当たり判定が無効化される終了秒
		};

		Flags flags{};
		AttackWindow attackWindow{};
	};

	// アニメーション
	std::shared_ptr<KdAnimator>		m_animator = std::make_shared<KdAnimator>();	// アニメーター
	DirectX::BoundingSphere			m_sphere{};										// バウンディングスフィア
	std::shared_ptr<CharacterData>	m_characterData;								// キャラクターデータ

	// ===== ステート管理 =====
	StateManager m_stateManager;

	TransformState	m_transform{};		// 行列関係
	MovementState	m_movement{};		// 移動関係
	PhysicsState	m_physics{};		// 物理関係
	RaycastState	m_raycast{};		// レイキャスト関係
	RenderingState	m_rendering{};		// 描画関係(ディゾルブ)
	RefsState		m_refs{};			// 参照関係
	CombatState		m_combat{};			// 戦闘関係

protected:

	// ===== アクセサ（派生向け） =====
	TransformState& Transform() { return m_transform; }
	const TransformState& Transform() const { return m_transform; }

	MovementState& Movement() { return m_movement; }
	const MovementState& Movement() const { return m_movement; }

	PhysicsState& Physics() { return m_physics; }
	const PhysicsState& Physics() const { return m_physics; }

	RaycastState& Raycast() { return m_raycast; }
	const RaycastState& Raycast() const { return m_raycast; }

	RenderingState& Rendering() { return m_rendering; }
	const RenderingState& Rendering() const { return m_rendering; }

	RefsState& Refs() { return m_refs; }
	const RefsState& Refs() const { return m_refs; }

	CombatState& Combat() { return m_combat; }
	const CombatState& Combat() const { return m_combat; }

	// ===== アクセサ =====
	DirectX::BoundingSphere GetBoundingSphere() const { return m_sphere; }
	std::shared_ptr<KdAnimator> GetAnimatorShared() { return m_animator; }
	std::shared_ptr<CharacterData> GetCharacterData() const { return m_characterData; }

	// ===== ステート管理 =====
	StateManager& GetStateManager() { return m_stateManager; }

};