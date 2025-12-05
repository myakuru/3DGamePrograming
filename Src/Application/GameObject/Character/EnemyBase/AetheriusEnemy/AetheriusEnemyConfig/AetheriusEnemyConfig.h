#pragma once
class EnemyStateBase;

class AetheriusEnemyConfig
{
public:
	void InGuiInspector();
	void JsonInput(const nlohmann::json& js);
	void JsonSave() const;
	void CreateStates();

	// ImGuiで編集した変数を実行時反映させるための関数
	void ApplyPrototypeParametersTo(EnemyStateBase& runtime);

	//　現在選択されているステート名取得
	const std::string& GetSelectedStateName() const { return m_currentStateName; }

private:
	// コンボ描画専用
	void DrawStateComboImGui();

	std::vector<std::unique_ptr<EnemyStateBase>> m_states;
	std::vector<std::string> m_stateNames;
	int m_selectedStateIndex = -1;
	std::string m_currentStateName = "None";
};