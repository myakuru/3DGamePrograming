#pragma once
#include"../Utility/SelectDraw3dModel.h"
class TPSCamera;
class ResultScore :public SelectDraw3dModel
{
public:
	static const uint32_t TypeID;

	ResultScore() { m_typeID = TypeID; };
	~ResultScore() override = default;

private:

	void Init() override;
	void Update() override;
	void DrawUnLit() override;

	std::shared_ptr<KdModelData> m_model_S = std::make_shared<KdModelData>();
	std::shared_ptr<KdModelData> m_model_A = std::make_shared<KdModelData>();
	std::shared_ptr<KdModelData> m_model_B = std::make_shared<KdModelData>();
	std::shared_ptr<KdModelData> m_model_X = std::make_shared<KdModelData>();

	std::weak_ptr<TPSCamera> m_camera; // カメラへの参照

	float m_distance = 0.0f;	// カメラからの距離
	float m_time = 0.0f;		// 時間計測用
	const float m_showTime = 5.0f; // 表示開始時間

	const float m_startDistance = 20.0f; // 開始時の距離

	void ImGuiInspector() override;
	void JsonSave(nlohmann::json& _json) const override;
	void JsonInput(const nlohmann::json& _json) override;

	Math::Vector4 m_S_color = Math::Vector4::Zero;
	Math::Vector4 m_A_color = Math::Vector4::Zero;
	Math::Vector4 m_B_color = Math::Vector4::Zero;
	Math::Vector4 m_X_color = Math::Vector4::Zero;


};