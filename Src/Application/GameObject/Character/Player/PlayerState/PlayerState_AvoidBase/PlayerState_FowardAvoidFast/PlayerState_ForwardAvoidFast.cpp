#include "PlayerState_ForwardAvoidFast.h"
#include "Application/main.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"

void PlayerState_ForwardAvoidFast::JsonInput(const nlohmann::json& _js)
{
	if (!_js.contains("PlayerState_ForwardAvoidFast")) return;
	const auto& stateNode = _js["PlayerState_ForwardAvoidFast"];
	if (stateNode.contains("Player"))
	{
		const auto& playerNode = stateNode["Player"];
		if (playerNode.contains("blendTime")) m_params.blendTime = playerNode["blendTime"].get<float>();
		if (playerNode.contains("animationSpeed")) m_params.animationSpeed = playerNode["animationSpeed"].get<float>();
		if (playerNode.contains("dashSpeed")) m_params.dashSpeed = playerNode["dashSpeed"].get<float>();
		if (playerNode.contains("dashSpeedTime")) m_params.dashSpeedTime = playerNode["dashSpeedTime"].get<float>();
		if (playerNode.contains("m_cameraTargetOffset")) m_startCameraOffset = JSON_MANAGER.JsonToVector(playerNode["m_cameraTargetOffset"]);
		if (playerNode.contains("m_cameraBossTargetOffset")) m_startBossCameraOffset = JSON_MANAGER.JsonToVector(playerNode["m_cameraBossTargetOffset"]);
		if (playerNode.contains("m_justAvoidCameraOffset")) m_justAvoidCameraOffset = JSON_MANAGER.JsonToVector(playerNode["m_justAvoidCameraOffset"]);

		// 残像設定
		if (_js.contains("afterImageMax")) m_params.afterImageMax = playerNode["afterImageMax"].get<int>();
		if (_js.contains("afterImageInterval")) m_params.afterImageInterval = playerNode["afterImageInterval"].get<float>();
		if (_js.contains("afterImageColor")) m_params.afterImageColor = JSON_MANAGER.JsonToVector4(playerNode["afterImageColor"]);
	}
}

void PlayerState_ForwardAvoidFast::JsonSave(nlohmann::json& _js) const
{
	if (!_js.contains("PlayerState_ForwardAvoidFast")) _js["PlayerState_ForwardAvoidFast"] = nlohmann::json::object();
	auto& stateNode = _js["PlayerState_ForwardAvoidFast"];

	stateNode["Player"]["blendTime"] = m_params.blendTime;
	stateNode["Player"]["animationSpeed"] = m_params.animationSpeed;
	stateNode["Player"]["dashSpeed"] = m_params.dashSpeed;
	stateNode["Player"]["dashSpeedTime"] = m_params.dashSpeedTime;
	stateNode["Player"]["m_cameraTargetOffset"] = JSON_MANAGER.VectorToJson(m_startCameraOffset);
	stateNode["Player"]["m_cameraBossTargetOffset"] = JSON_MANAGER.VectorToJson(m_startBossCameraOffset);
	stateNode["Player"]["m_justAvoidCameraOffset"] = JSON_MANAGER.VectorToJson(m_justAvoidCameraOffset);
	// 残像設定
	stateNode["Player"]["afterImageMax"] = m_params.afterImageMax;
	stateNode["Player"]["afterImageInterval"] = m_params.afterImageInterval;
	stateNode["Player"]["afterImageColor"] = JSON_MANAGER.Vector4ToJson(m_params.afterImageColor);
}
