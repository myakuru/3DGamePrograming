#pragma once
class SelectDraw3dModel:public KdGameObject
{
public:

	void GenerateDepthMapFromLight() override
	{
		if (m_bGenerateDepthMapFromLight) KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld, m_color);
		if (m_bGenerateDepthMapFromLight) KdShaderManager::Instance().m_StandardShader.DrawModel(*m_modelWork, m_mWorld, m_color);
	}

	void DrawLit() override
	{
		if(m_bDrawLit) KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld, m_color);
		if (m_bDrawLit) KdShaderManager::Instance().m_StandardShader.DrawModel(*m_modelWork, m_mWorld, m_color);
	}

	void DrawUnLit() override
	{
		if(m_bDrawUnLit) KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld, m_color);
		if (m_bDrawUnLit) KdShaderManager::Instance().m_StandardShader.DrawModel(*m_modelWork, m_mWorld, m_color);
	}

	void DrawBright() override
	{ 
		if (m_bDrawBright)KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld, m_color);
		if (m_bDrawBright)KdShaderManager::Instance().m_StandardShader.DrawModel(*m_modelWork, m_mWorld, m_color);
	}

	void DrawToon() override
	{
		if (m_bDrawToon) KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld, m_color);
		if (m_bDrawToon) KdShaderManager::Instance().m_StandardShader.DrawModel(*m_modelWork, m_mWorld, m_color);
	}

	void DrawGradation() override
	{
		if (m_bDrawGradation) KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld, m_color);
		if (m_bDrawGradation) KdShaderManager::Instance().m_StandardShader.DrawModel(*m_modelWork, m_mWorld, m_color);
	}

	void DrawGrayScale() override
	{
		if (m_bDrawGrayScale) KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld, m_color);
		if (m_bDrawGrayScale) KdShaderManager::Instance().m_StandardShader.DrawModel(*m_modelWork, m_mWorld, m_color);
	}

	// ImGuiのインスペクターでDrawを変更できるようにする。
	void ImGuiInspector() override
	{
		KdGameObject::ImGuiInspector();
		ImGui::BeginTable("DrawFlags", 2); // 2列
		for (const auto& [name, flg] : m_drawFlg)
		{
			ImGui::TableNextColumn();
			ImGui::Checkbox(name.data(), flg);
		}
		ImGui::EndTable();
	}

	// jsonに、DrawFlagsを保存する
	void JsonSave(nlohmann::json& _json) const override
	{
		KdGameObject::JsonSave(_json);

		nlohmann::json flagsJson;
		for (const auto& [name, ptr] : m_drawFlg)
		{
			flagsJson[name] = *ptr;
		}
		_json["drawFlags"] = flagsJson;
	}

	// jsonから、DrawFlagsを読み込む
	void JsonInput(const nlohmann::json& _json) override
	{
		KdGameObject::JsonInput(_json);

		if (_json.contains("drawFlags"))
		{
			const auto& flagsJson = _json["drawFlags"];
			for (const auto& [name, ptr] : m_drawFlg)
			{
				if (flagsJson.contains(name)) *ptr = flagsJson[name].get<bool>();
			}
		}
	}

	// マップフラグを初期化する
	void InitDrawFlags()
	{
		m_bGenerateDepthMapFromLight = false;
		m_bDrawLit = false;
		m_bDrawUnLit = false;
		m_bDrawBright = false;
		m_bDrawToon = false;
		m_bDrawGradation = false;
		m_bDrawGrayScale = false;
	}

	// drawFllgを外部でセットする
	void SetDrawFlag(const std::string& name, bool flg)
	{
		if (m_drawFlg.find(name) != m_drawFlg.end())
		{
			*m_drawFlg[name] = flg;
		}
	}

protected:

	bool m_bGenerateDepthMapFromLight = false;	// ライトからの深度マップを生成するかどうか
	bool m_bDrawLit = false;					// ライトの影響を受ける描画を行うかどうか
	bool m_bDrawUnLit = false;					// ライトの影響を受けない描画を行うかどうか
	bool m_bDrawBright = false;					// ブライト描画を行うかどうか
	bool m_bDrawToon = false;					// トゥーンシェーダーで描画するかどうか
	bool m_bDrawGradation = false;				// グラデーション描画を行うかどうか
	bool m_bDrawGrayScale = false;				// グレースケール描画を行うかどうか

	std::map<std::string, bool*, std::less<void>> m_drawFlg =
	{
			{"GenerateDepthMapFromLight", &m_bGenerateDepthMapFromLight},
			{"DrawLit", &m_bDrawLit},
			{"DrawUnLit", &m_bDrawUnLit},
			{"DrawBright", &m_bDrawBright},
			{"DrawToon", &m_bDrawToon},
			{"DrawGradation", &m_bDrawGradation},
			{"DrawGrayScale", &m_bDrawGrayScale}
	};
};