#pragma once
#include"../CameraBase.h"

class Player;
class PlayerCameraState;
class PlayerCameraConfig;

class PlayerCamera : public CameraBase
{
public:
	static const uint32_t TypeID;

	PlayerCamera() { m_typeID = TypeID; AddTag(ObjTag::PlayerCamera); }
	~PlayerCamera() override = default;

	uint32_t GetTypeID() const override { return m_typeID; }

	void Init() override;
	void PostUpdate() override;
	void ImGuiInspector() override;
	void JsonSave(nlohmann::json& _json) const override;
	void JsonInput(const nlohmann::json& _json) override;

	// カメラシェイク開始（強さ・時間）
	void StartShake(Math::Vector2 _power, float time)
	{
		m_shakeState.shakePower = _power;
		m_shakeState.shakeTime = time;
	}

	Math::Vector3 GetCameraPos() const { return m_lookState.cameraPos; }

	// ターゲット位置設定（遮蔽中は無視）
	void SetTargetLookAt(const Math::Vector3& target)
	{
		if (m_collisionState.isBlocked) return;
		m_lookState.followRate = target;
	}

	// 回転設定
	void SetPlayerRotation(const Math::Vector3& rot)
	{
		m_degree = rot;
	}

	const Math::Vector3& GetPlayerRotation() const 
	{
		return m_degree;
	}

	// スムージング設定
	void SetDistanceSmooth(float smooth) { m_lookState.distanceSmooth = smooth; }
	void SetRotationSmooth(float smooth) { m_lookState.rotationSmooth = smooth; }

	void StateInit();
	void ChangeState(std::shared_ptr<PlayerCameraState> _state);

	//========================
	// 状態データ構造体
	//========================
	struct LookPlayerStateData
	{
		Math::Vector3 targetLookAt = Math::Vector3::Zero;
		Math::Vector3 followRate = Math::Vector3::Zero;
		Math::Vector3 cameraPos = Math::Vector3::Zero;
		float         distanceSmooth = 0.0f;
		float         rotationSmooth = 0.0f;
		bool          isBlocked = false; // 衝突状態のミラー（Collision側と同期運用）
	};

	struct IntroStateData
	{
		Math::Vector3 introCamPos = Math::Vector3::Zero;	// イントロカメラ位置
		float         introTimer = 0.0f;					// イントロ経過時間
		Math::Vector3 startFollow = { 0.0f, 3.0f, -10.0f };	// 開始時の追従オフセット
		Math::Vector3 endFollow = { 0.0f, 1.0f, -1.7f };	// 終了時の追従オフセット
		bool          inited = false;						// 初期化済みフラグ
		float         startYaw = 0.0f;						// イントロ開始時Yaw
	};

	struct WinnerStateData
	{
		float         time = 0.0f;	// ウィナー状態経過時間
	};

	struct CollisionStateData
	{
		float         currentCamDistance = -1.0f;
		float         camHitSmoothIn = 18.0f;
		float         camHitSmoothOut = 6.0f;
		float         obstacleMargin = 0.30f;
		float         minCamDistance = 0.60f;
		float         prevHitDist = -1.0f;
		float         hitDistSmoothing = 0.0f;
		Math::Vector3 desiredCameraPos = Math::Vector3::Zero;
		Math::Vector3 effectiveLookAt = Math::Vector3::Zero;
		float         effectiveRecoverSpeed = 6.0f;
		bool          isBlocked = false;
		float         expandDelayTime = 0.10f;
		float         expandDelayTimer = 0.0f;
	};

	struct FovStateData
	{
		float         fov = 60.0f;
		float         fovLerpSpeed = 0.0f;
		Math::Vector2 fovShake = Math::Vector2::Zero;
		Math::Vector2 fovShakeTarget = { 60.0f, 0.0f };
	};

	struct ShakeStateData
	{
		float         shakeTime = 0.0f;
		Math::Vector2 shakePower = Math::Vector2::Zero;
	};

	// イントロカメラ設定
	struct IntroConfig
	{
		float startYawDeg = 145.0f;							// イントロ回転開始Yaw（初期化時に設定される目標値）
		float endYawDeg = 320.0f;							// イントロ回転終了Yaw
		Math::Vector3 startFollow = { 0.0f, 1.0f, -0.5f };	// イントロ開始時の追従オフセット
		Math::Vector3 endFollow = { 0.0f, 1.0f, -1.5f };	// イントロ終了時の追従オフセット
		float afterEndFollowZ = -2.5f;						// イントロ終了後の余韻でズームアウトするZ
		float holdSeconds = 1.0f;							// イントロ終了地点での待機秒数
		float yawSpeedDegPerSec = 60.0f;					// イントロ回転速度（deg/sec）
	};

	//========================
	// 状態アクセサ
	//========================
	LookPlayerStateData& LookState()      noexcept { return m_lookState; }
	IntroStateData& IntroState()     noexcept { return m_introState; }
	WinnerStateData& WinnerState()    noexcept { return m_winnerState; }
	CollisionStateData& CollisionState() noexcept { return m_collisionState; }
	FovStateData& FovState()       noexcept { return m_fovState; }
	ShakeStateData& ShakeState()     noexcept { return m_shakeState; }
	IntroConfig& IntroConfigRef() { return m_introConfig; }

	const LookPlayerStateData& LookState() const      noexcept { return m_lookState; }
	const IntroStateData& IntroState() const     noexcept { return m_introState; }
	const WinnerStateData& WinnerState() const    noexcept { return m_winnerState; }
	const CollisionStateData& CollisionState() const noexcept { return m_collisionState; }
	const FovStateData& FovState() const       noexcept { return m_fovState; }
	const ShakeStateData& ShakeState() const     noexcept { return m_shakeState; }
	const IntroConfig& GetIntroConfig() const { return m_introConfig; }

private:
	void UpdateCameraRayCast();

	//========================
	// 状態ストレージ
	//========================
	LookPlayerStateData    m_lookState;
	IntroStateData         m_introState;
	WinnerStateData        m_winnerState;
	CollisionStateData     m_collisionState;
	FovStateData           m_fovState;
	ShakeStateData         m_shakeState;
	IntroConfig m_introConfig;

	// プレイヤー参照
	std::shared_ptr<Player> m_spTarget = nullptr;

	// ステートコンフィグ
	std::shared_ptr<PlayerCameraConfig> m_playerCameraConfig = nullptr;

	// 一度だけ処理フラグ
	bool m_oneceFlag = false;
};