#pragma once
#include"../WeaponBase.h"
class WeaponKatanaScabbard;
class Katana : public WeaponBase
{
public:
	// クラスごとに一意なTypeIDを持たせる
	static const uint32_t TypeID;

	Katana() { m_typeID = TypeID; }
	~Katana() override = default;

	std::shared_ptr<KdTrailPolygon> GetTrail() { return m_trailPolygon; }

	bool& EnableTrail() { return m_enableTrail; }

	void SetNowAttackState(bool state) { m_isAttackState = state; }

	void SetShowTrail(bool show) { m_showTrail = show; }

private:

	void Init() override;
	void Update() override;
	void UpdateTrailPolygon();								//　軌跡ポリゴンの更新
	void DrawRimLight() override;
	void UpdateHand();
	void ImGuiInspector() override;
	void JsonSave(nlohmann::json& _json) const override;
	void JsonInput(const nlohmann::json& _json) override;

	std::shared_ptr<KdTrailPolygon> m_trailPolygon; // 軌跡ポリゴン

	std::shared_ptr<KdTexture> m_trailTex = std::make_shared<KdTexture>(); // 軌跡テクスチャ

	bool m_enableTrail = true;

	bool m_isAttackState = false; // 攻撃中かどうか

	Math::Matrix m_trailOffset = Math::Matrix::Identity; // 軌跡ポリゴンのオフセット

	Math::Matrix m_finalMat = Math::Matrix::Identity; // 軌跡ポリゴンの最終行列

	Math::Matrix m_trailRotation = Math::Matrix::Identity; // 軌跡ポリゴンの回転

	Math::Color m_trailColor = { 1.0f,1.0f,1.0f,0.7f };

	float m_trailKatanaPoint = 0.0f;	// 軌跡をつける刀の位置
	Math::Vector3 m_trailScale = { 1.0f,1.0f,1.0f }; // 軌跡の拡大縮小

	bool m_showTrail = false;

};