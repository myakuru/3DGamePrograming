#pragma once
#include"../CameraBase.h"
class Player;
class PlayerCameraState;
class PlayerCamera :public CameraBase
{
public:

	static const uint32_t TypeID;

	PlayerCamera() { m_typeID = TypeID; }
	~PlayerCamera() override = default;

	uint32_t GetTypeID() const override { return m_typeID; }

	void Init() override;
	void PostUpdate() override;
	void UpdateWinnerCamera();
	void NewUpdateIntroCamera();
	void ImGuiInspector()	override;
	void JsonSave(nlohmann::json& _json) const override;
	void JsonInput(const nlohmann::json& _json) override;
	DirectX::BoundingFrustum CreateFrustum() const override;

	void DebugDraw(DirectX::BoundingFrustum _frustum);

	// 強さ、時間
	void StartShake(Math::Vector2 _power, float time)
	{
		m_shakePower = _power;
		m_shakeTime = time;
	}

	Math::Vector3 GetCameraPos() const { return m_cameraPos; }

	// カメラのターゲット位置を設定デフォルトは(0.0f,1.0f,-3.5f)
		// 衝突(遮蔽)中は適用しないことで、プレイヤーステート側の移動と干渉しないようにする
	void SetTargetLookAt(const Math::Vector3& target)
	{
		if (m_isBlocked) return;      // 当たり中は無視
		m_followRate = target;
	}


	// カメラのターゲットの回転を設定
	void SetTargetRotation(const Math::Vector3& rot)
	{
		m_degree = rot;
	}

	// カメラの距離のスムージング係数を設定
	void SetDistanceSmooth(float smooth) { m_dhistanceSmooth = smooth; }

	// カメラの回転のスムージング係数を設定
	void SetRotationSmooth(float smooth) { m_rotationSmooth = smooth; }

	void StateInit();
	void ChangeState(std::shared_ptr<PlayerCameraState> _state);


private:

	void UpdateCameraRayCast(const Math::Vector3& _anchor);

	void UpdateCameraRayCast();


	Math::Vector3 m_targetLookAt = Math::Vector3::Zero;
	Math::Vector3 m_introCamPos = Math::Vector3::Zero;	// Introカメラの位置
	Math::Vector3 m_cameraPos = Math::Vector3::Zero; // カメラの現在位置

	std::shared_ptr<Player> m_spTarget = nullptr; // カメラのターゲット

	// どこまでターゲットを追従するか
	Math::Vector3 m_followRate = Math::Vector3::Zero;

	// カメラシェイク用変数
	float m_shakeTime = 0.0f;

	Math::Vector2 m_shakePower = Math::Vector2::Zero; // シェイクの強さ

	float m_dhistanceSmooth = 0.0f; // カメラ距離のスムージング係数
	float m_rotationSmooth = 0.0f; // カメラ回転のスムージング係数
	float m_introTimer = 0.0f;	// Introカメラのタイマー
	float m_fov = 60.0f;		// 視野角

	// --- カメラ衝突補正用 追加メンバ ---
	float m_currentCamDistance = -1.0f;      // 現在補正後の使用距離
	float m_camHitSmoothIn = 18.0f;      // 障害物に近づく(距離を縮める)ときの収束速度
	float m_camHitSmoothOut = 6.0f;       // 障害物から離れる(距離を伸ばす)ときの収束速度
	float m_obstacleMargin = 0.30f;      // 壁とのマージン
	float m_minCamDistance = 0.60f;      // これ以下には詰めない(プレイヤーを貫通しない距離)
	float m_prevHitDist = -1.0f;      // 前フレームのヒット距離(ノイズ抑制)
	float m_hitDistSmoothing = 0.0f;       // 内部平滑化(0:なし～1:即時)

	// 衝突前の理想カメラ位置（毎フレーム生成、衝突で変更しない）
	Math::Vector3 m_desiredCameraPos = Math::Vector3::Zero;

	// 追加フィールド
	Math::Vector3 m_effectiveLookAt = Math::Vector3::Zero;
	// ヒット解除後に理想へ戻す速度（好みに調整）
	float m_effectiveRecoverSpeed = 6.0f;

	bool  m_isBlocked = false; // カメラが障害物に当たっているかどうか

	// 衝突後にすぐ伸ばさないための伸張ディレイ
	float m_expandDelayTime = 0.10f;
	float m_expandDelayTimer = 0.0f;

	// FOVのラープ用
	float m_fovLerpSpeed = 0.0f;

	Math::Vector2 m_fovShake = Math::Vector2::Zero;
	Math::Vector2 m_fovShakeTarget = { 60.0f,0.0f };

	float m_time = 0.0f;
	Math::Vector3 m_endFollow = { 0.0f, 1.0f, -1.7f };
	Math::Vector3 m_startFollow = { 0.0f, 3.0f, -10.0f };

	bool  m_inited = false;
	float m_startYaw = 0.0f;

};