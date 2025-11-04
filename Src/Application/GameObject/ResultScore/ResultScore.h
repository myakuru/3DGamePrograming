#pragma once
#include"../Utility/SelectDraw3dModel.h"
class TPSCamera;
class ResultScore :public SelectDraw3dModel
{
public:
	static const uint32_t TypeID;
	ResultScore() { m_typeID = TypeID; };
	~ResultScore() override = default;

	uint32_t GetTypeID() const override { return m_typeID; }

private:

	void Init() override;
	void Update() override;
	void DrawGradation() override;

	std::shared_ptr<KdModelData> m_model_S = std::make_shared<KdModelData>();
	std::shared_ptr<KdModelData> m_model_A = std::make_shared<KdModelData>();
	std::shared_ptr<KdModelData> m_model_B = std::make_shared<KdModelData>();
	std::shared_ptr<KdModelData> m_model_X = std::make_shared<KdModelData>();

	std::weak_ptr<TPSCamera> m_camera; // カメラへの参照

	float m_selfRotateY = 0.0f;

	float m_distance = 0.0f; // カメラからの距離

	Math::Color m_gradientColor = { 1.0f,1.0f,0.0f,1.0f }; // グラデーションの色
	Math::Matrix cameraRot = Math::Matrix::Identity; // カメラの回転行列

	float m_time = 0.0f; // 時間計測用

	void ImGuiInspector() override;
	void JsonSave(nlohmann::json& _json) const override;
	void JsonInput(const nlohmann::json& _json) override;


};