#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class ScoreBackBar : public SelectDraw2DTexture
{
public:


	static const uint32_t TypeID;
	ScoreBackBar() { m_typeID = TypeID; }
	~ScoreBackBar() override = default;

private:

	void DrawSprite() override;
	void Update() override;

	void ImGuiInspector() override;

	void JsonSave(nlohmann::json& _json) const override;
	void JsonInput(const nlohmann::json& _json) override;

	float m_dawnTimer = 0.0f;
	Math::Vector3 m_dawnPos = Math::Vector3::Zero;


};