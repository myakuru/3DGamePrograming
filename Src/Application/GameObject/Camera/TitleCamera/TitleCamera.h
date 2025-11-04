#pragma once
#include "../CameraBase.h"
class TitleCamera : public CameraBase
{
public:

	static const uint32_t TypeID;
	TitleCamera() { m_typeID = TypeID; }
	~TitleCamera() override = default;

private:

	void Init()				override;
	void PostUpdate()		override;
	void ImGuiInspector()	override;
	void JsonSave(nlohmann::json& _json) const override;
	void JsonInput(const nlohmann::json& _json) override;

	Math::Vector3 m_startPos = { 0.0f,0.0f,0.0f };
	Math::Vector3 m_endPos = { 0.0f,0.0f,-0.3f };

	float m_time = 0.0f;
	float m_moveSpeed = 0.1f;

};