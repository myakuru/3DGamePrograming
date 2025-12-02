#pragma once
#include"../Utility/SelectDraw3dPolygon.h"
class PlayerCamera;
class Winner : public SelectDraw3dPolygon
{
public:
	static const uint32_t TypeID;

	Winner() { m_type = TypeID; }
	~Winner() override = default;

	void Init() override;
	void Update() override;
	void DrawLit() override;

	void ImGuiInspector() override;
	void JsonSave(nlohmann::json& _json) const override;
	void JsonInput(const nlohmann::json& _json) override;


private:

	std::weak_ptr<PlayerCamera> m_camera; // カメラへの参照

	float m_distance = 10.0f; // カメラからの距離

};