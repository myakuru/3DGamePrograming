#pragma once
#include <memory>
#include <vector>
#include <string>

class PlayerStateBase;

class PlayerConfig
{
public:
	void InGuiInspector();
	void JsonInput(const nlohmann::json& js);
	void JsonSave() const;
	void CreateStates();
	void ApplyPrototypeParametersTo(PlayerStateBase& runtime);

	//　現在選択されているステート名取得
	const std::string& GetSelectedStateName() const { return m_currentStateName; }

private:
	// コンボ描画専用
	void DrawStateComboImGui();

	std::vector<std::unique_ptr<PlayerStateBase>> m_states;
	std::vector<std::string> m_stateNames;
	int m_selectedStateIndex = -1;
	std::string m_currentStateName = "None";
};