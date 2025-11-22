#include "PlayerConfig.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState.h"

void PlayerConfig::InGuiInspector(const std::vector<std::unique_ptr<PlayerStateBase>>& states)
{
	ImGui::Text(U8("Player Common"));
	ImGui::DragFloat(U8("MoveSpeed"), &commonMoveSpeed, 0.01f, 0.0f, 50.0f);
	ImGui::Separator();
	for (auto& st : states)
	{
		st->ExposeParametersImGui();
		ImGui::Separator();
	}
}

void PlayerConfig::JsonInput(const nlohmann::json& js, const std::vector<std::unique_ptr<PlayerStateBase>>& states)
{
	for (auto& st : states) st->LoadParametersJson(js);
}

void PlayerConfig::JsonSave(nlohmann::json& js, const std::vector<std::unique_ptr<PlayerStateBase>>& states) const
{
	js["Player"]["MoveSpeed"] = commonMoveSpeed;
	for (auto& st : states) st->SaveParametersJson(js);
}
