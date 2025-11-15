#pragma once

#include "../Utility/SelectDraw3dModel.h"
#include "MyFramework/State/StateManager/StateManager.h"

// 前方宣言（依存の最小化）
class PlayerCamera;
class StateManager;
class CharacterData;

class CharacterBase : public SelectDraw3dModel
{
public:
	CharacterBase() = default;
	~CharacterBase() override = default;

	// ===== ライフサイクル =====
	void Init      () override;
	void Update    () override;
	void PostUpdate() override;
	bool ModelLoad (std::string _path) override;

	// ===== デバッグ/永続化 =====
	void ImGuiInspector() override;
	void JsonInput     (const nlohmann::json& _json) override;
	void JsonSave      (nlohmann::json&       _json) const override;

	// ===== 移動・回転 =====
	// 移動ベクトルを設定
	void SetIsMoving(Math::Vector3 _move);
	const Math::Vector3& GetMovement() const;

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

	// ===== ステート管理 =====
	StateManager m_stateManager;

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

protected:
	// ====== 責務ごとに束ねた内部状態 ======
	struct TransformState
	{
		Math::Matrix     rotationM = Math::Matrix::Identity;     // 回転行列
		Math::Quaternion rotationQ = Math::Quaternion::Identity; // 回転（クォータニオン）
	};

	struct MovementState
	{
		Math::Vector3 movement	= Math::Vector3::Zero;	// 現在の移動ベクトル
		float moveSpeed			= 0.0f;					// 移動速度
		float rotateSpeed		= 0.0f;					// 回転速度
		Math::Vector3 lastDir	= Math::Vector3::Zero;	// 最後に移動した方向ベクトル
	};

	struct PhysicsState
	{
		float gravity		 = 0.0f;	// 重力加速度
		float gravitySpeed	 = 0.0f;	// 現在の重力速度
		float fixedFrameRate = 0.0f;    // 60fps換算等
	};

	struct RaycastState
	{
		Math::Vector3 prevPosition{};			// 前フレームのワールド位置
		float forwardRayYOffset		= 0.35f;	// プレイヤー中心(腰程度)の高さ
		float forwardRayMargin		= 0.02f;	// 壁手前で残すマージン
		float forwardRayExtra		= 0.05f;	// 余剰距離(浮動小数ヒット安定用)
		float bumpSphereRadius		= 0.2f;		// 壁めり込み防止球の半径
		float bumpSphereYOffset		= 0.3f;		// プレイヤー中心(腰程度)の高さ
		float collisionMargin		= 0.01f;	// 壁にめり込まないための余白
	};

	struct RenderingState
	{
		Math::Vector3 dissolveColor = Math::Vector3::Zero; // 溶解エフェクトの色
		float         dissolvePower = 1.0f;                // 溶解進行度
	};

	struct RefsState
	{
		std::weak_ptr<PlayerCamera>					playerCamera;
		std::weak_ptr<KdGameObject>					collision;
		std::list<std::weak_ptr<KdGameObject>>		referencedObjects;
		std::list<std::shared_ptr<KdGameObject>>	effectList;
	};

	struct CombatState
	{
		struct Flags
		{
			bool isHit		= false;
			bool invincible	= false;
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

protected:

	TransformState	m_transform{};		// 行列関係
	MovementState	m_movement{};		// 移動関係
	PhysicsState	m_physics{};		// 物理関係
	RaycastState	m_raycast{};		// レイキャスト関係
	RenderingState	m_rendering{};		// 描画関係(ディゾルブ)
	RefsState		m_refs{};			// 参照関係
	CombatState		m_combat{};			// 戦闘関係

	// アニメーション
	std::shared_ptr<KdAnimator>		m_animator = std::make_shared<KdAnimator>();
	DirectX::BoundingSphere			m_sphere{};
	std::shared_ptr<CharacterData>	m_characterData;
};