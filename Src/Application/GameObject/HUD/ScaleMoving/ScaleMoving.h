#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class ScaleMoving :public SelectDraw2DTexture
{
public:
	static const uint32_t TypeID;

	ScaleMoving() { m_typeID = TypeID; }
	~ScaleMoving() override = default;

private:

	void Update() override;
	void DrawSprite() override;

	void ImGuiInspector() override;
	void JsonSave(nlohmann::json& _json) const override;
	void JsonInput(const nlohmann::json& _json) override;

	int m_displayTime = 0;
	bool m_isIncreasing = false;

	float m_dawnTimer = 0.0f;
	Math::Vector3 m_dawnPos = Math::Vector3::Zero;

	float m_time = 0;		// -1.0f ~ 1.0fをとる値
	float m_alpha = 0;

	float m_min = 0.0f;
	float m_max = 0.0f;

	// バー用のテクスチャ最大幅（ピクセル）
	float m_fullWidth = 500.0f;

	// 右端の移動範囲（1920x1080基準のスクリーン座標）
	float m_rightXStart = 300.0f;   // 残り時間=最大のときの右端X
	float m_rightXEnd = 1620.0f;  // 残り時間=0のときの右端X

	Math::Rectangle m_rect = { 0,0,500,100 };
};