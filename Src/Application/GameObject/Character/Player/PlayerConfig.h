#pragma once
#include "Application/GameObject/Character/Player/Player.h"
#include "MyFramework/State/StateBase/StateBase.h"

class PlayerConfig {
public:
	void InGuiInspector();
	void DrawStateComboImGui();
	void JsonInput(const nlohmann::json& js);
	void JsonSave() const;
	void CreateStates();

	void ApplyPrototypeParametersTo(StateBase<Player>& _runtime);

private:
	
	std::vector<std::unique_ptr<StateBase<Player>>> m_states;
	std::vector<std::string> m_stateNames;
	int m_selectedStateIndex = -1;
	std::string m_currentStateName = "None";
};