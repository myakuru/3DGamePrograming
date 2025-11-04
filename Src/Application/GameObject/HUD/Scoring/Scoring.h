#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class Scoring :public SelectDraw2DTexture
{
public:
	static const uint32_t TypeID;

	Scoring() { m_typeID = TypeID; }
	~Scoring() override = default;

private:

	void Init() override;
	void Update() override;
	void DrawSprite() override;

	void ImGuiInspector() override;

	void JsonSave(nlohmann::json& _json) const override;
	void JsonInput(const nlohmann::json& _json) override;

	Math::Rectangle m_srcRect = { 0,0,150,50 };	// スコアの表示位置

	int m_displayTime = 0;

	float m_dawnTimer = 0.0f;
	Math::Vector3 m_dawnPos = Math::Vector3::Zero;

};