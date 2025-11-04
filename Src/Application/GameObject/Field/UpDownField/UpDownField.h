#pragma once
#include"../../Utility/SelectDraw3dModel.h"
class UpDownField :public SelectDraw3dModel
{
public:

	static const uint32_t TypeID;

	UpDownField()
	{
		m_typeID = TypeID;
	}
	~UpDownField() override = default;

private:

	void DrawLit() override;
	void DrawGrayScale() override;

	void Update() override;

	// ImGuiのインスペクターで変更できるようにする。
	void ImGuiInspector() override;
	void JsonSave(nlohmann::json& _json) const override;
	void JsonInput(const nlohmann::json& _json) override;

	float m_time = 0.0f;
	float m_min = 0.0f;
	float m_max = 0.0f;
	float m_speed = 0.0f;

	int m_RasterizerState = 0;

};