#pragma once
#include"../Utility/SelectDraw3dModel.h"
class AetheriusEnemy;
class WeaponBase : public SelectDraw3dModel
{
public:

	static const uint32_t TypeID;

	WeaponBase() { m_typeID = TypeID; } // 修正: m_type -> m_typeID
	~WeaponBase() override = default;

	// 刀の行列はPlayerで設定される
	virtual void SetKatanaMatrix(const Math::Matrix& _matrix) { m_swordData.m_weaponBonesMatrix = _matrix; }
	virtual void SetPlayerMatrix(const Math::Matrix& _matrix) { m_swordData.m_playerWorldMatrix = _matrix; }

	virtual void SetHandKatanaMatrix(const Math::Matrix& _matrix) { m_swordHandData.m_weaponBonesMatrix = _matrix; }
	virtual void SetPlayerHandMatrix(const Math::Matrix& _matrix) { m_swordHandData.m_playerWorldMatrix = _matrix; }

	virtual const Math::Matrix& GetKatanaMatrix() const { return m_swordData.m_weaponMatrix; }
	virtual const Math::Matrix& GetKatanaRotation() const { return m_swordHandData.m_weaponRotationMatrix; }

	virtual void SetEnemyRightHandMatrix(const Math::Matrix& _matrix) { m_enemyRightSwordData.m_weaponBonesMatrix = _matrix; }
	virtual void SetEnemyLeftHandMatrix(const Math::Matrix& _matrix) { m_enemyLeftSwordData.m_weaponBonesMatrix = _matrix; }
	virtual void SetEnemyMatrix(const Math::Matrix& _matrix) { m_enemyRightSwordData.m_enemyWorldMatrix = _matrix; }
	virtual const Math::Matrix& GetEnemyRightHandMatrix() const { return m_enemyRightSwordData.m_weaponMatrix; }

	void SetOwnerEnemy(const std::weak_ptr<AetheriusEnemy>& owner) { m_ownerEnemy = owner; }
	const std::weak_ptr<AetheriusEnemy>& GetOwnerEnemy() const { return m_ownerEnemy; }

protected:

	void Init() override;
	void DrawToon() override;
	void DrawLit() override;
	void Update() override;
	void ImGuiInspector() override;
	void JsonSave(nlohmann::json& _json) const override;
	void JsonInput(const nlohmann::json& _json) override;

	// 回転行列ユーティリティ（deg -> YawPitchRoll）
	static Math::Matrix ToRotationMatrix(const Math::Vector3& deg);

	// 刀の構造体
	struct SwordData
	{
		Math::Matrix m_weaponMatrix = Math::Matrix::Identity;			// 刀の行列
		Math::Matrix m_weaponBonesMatrix = Math::Matrix::Identity;		// 刀の位置行列
		Math::Matrix m_playerWorldMatrix = Math::Matrix::Identity;		// プレイヤーの位置行列
		Math::Matrix m_enemyWorldMatrix = Math::Matrix::Identity;		// 敵の位置行列
		Math::Matrix m_weaponRotationMatrix = Math::Matrix::Identity;	// 刀の回転行列
		Math::Matrix m_weaponScaleMatrix = Math::Matrix::Identity;		// 刀の拡大縮小行列
		Math::Vector3 m_weaponDeg = Math::Vector3::Zero;				// 刀の角度(deg)
		Math::Vector3 m_scale = { 1.0f,1.0f,1.0f };						// 刀の拡大縮小
	};

	// 合成ユーティリティ
	// trsSrc: 回転・スケールを持つデータ(角度・スケール)
	// attachSrc: ボーン行列を持つデータ(ボーン行列)
	// offset: ローカルオフセット
	// ownerWorld: プレイヤーや敵のワールド行列
	Math::Matrix ComposeWeaponMatrix(const SwordData& trsSrc,
		const SwordData& attachSrc,
		const Math::Vector3& offset,
		const Math::Matrix& ownerWorld) const;

	SwordData m_swordData;												// 刀のデータ
	SwordData m_swordHandData;
	SwordData m_enemyRightSwordData;									// 敵が持つ刀のデータ
	SwordData m_enemyLeftSwordData;										// 敵が持つ刀のデータ

	Math::Vector3 m_katanaOffset = Math::Vector3::Zero;
	Math::Vector3 m_katanaHandOffset = Math::Vector3::Zero;				// 手に持つ刀のオフセット

	/// <summary>
	/// リムライト関連
	/// </summary>
	float m_rimLightPower = 2.0f; // リムライトの強さ
	Math::Vector3 m_rimLightColor = { 20.0f,30.0f,30.0f }; // リムライトの色
	float m_rimLightUVOffsetSpeed = 0.5f; // リムライトのUVスクロール速度
	float m_rimLightUVOffset = 0.0f; // リムライトのUVオフセット

	std::weak_ptr<AetheriusEnemy> m_ownerEnemy;									// この武器を持っている敵

};