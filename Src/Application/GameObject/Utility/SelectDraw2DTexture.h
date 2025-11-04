#pragma once
class SelectDraw2DTexture : public KdGameObject
{
public:
	SelectDraw2DTexture() = default;
	~SelectDraw2DTexture() override = default;

	void DrawSprite() override
	{

		// 現在のビューポートサイズ取得
		Math::Viewport vp;
		KdDirect3D::Instance().CopyViewportInfo(vp);

		// 伸張（Stretch）：XとYを個別にスケーリング（画面サイズにピッタリ）
		const float sx = vp.width / kRefW;
		const float sy = vp.height / kRefH;

		// 等方（Fit）にしたい場合は以下を使う（必要なら切替）
		//const float s  = std::min(sx, sy);
		// Math::Matrix uiScale = Math::Matrix::CreateScale(s, s, 1.0f);

		Math::Matrix uiScale = Math::Matrix::CreateScale(sx, sy, 1.0f);

		// UIスケールをワールド行列に後掛け（平行移動も含め全体をスケール）
		Math::Matrix m = m_mWorld * uiScale;

		KdShaderManager::Instance().m_spriteShader.SetMatrix(m);
		if (m_bDrawTexture)
		{
			KdShaderManager::Instance().m_spriteShader.DrawTex(
				m_texture,
				static_cast<int>(m_position.x), // 1920x1080基準の座標そのまま
				static_cast<int>(m_position.y),
				nullptr,
				&m_color
			);
		}
		KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
	}

	// ImGuiのインスペクターでDrawを変更できるようにする。
	void ImGuiInspector() override
	{
		KdGameObject::ImGuiInspector();

		ImGui::Checkbox("Draw Texture", &m_bDrawTexture);
	}

	// jsonに、DrawFlagsを保存する
	void JsonSave(nlohmann::json& _json) const override
	{
		KdGameObject::JsonSave(_json);
		_json["drawFlags"] = m_bDrawTexture;
	}

	// jsonから、DrawFlagsを読み込む
	void JsonInput(const nlohmann::json& _json) override
	{
		KdGameObject::JsonInput(_json);
		
		if (_json.contains("drawFlags")) m_bDrawTexture = _json["drawFlags"].get<bool>();
	}

protected:

	bool ModelLoad(std::string _path) override
	{
		// .~ 以降の拡張子を識別するために部分文字列を取得
		std::string ext = _path.substr(_path.find_last_of('.') + 1);

		if (ext == "png" || ext == "PNG" || ext == "svg")
		{
			_path = m_path;
			// テクスチャ読み込み
			m_texture->Load(m_path);
			return true;
		}

		return false;
	}

	bool m_bDrawTexture = false;

	// 基準解像度
	const float kRefW = 1920.0f;
	const float kRefH = 1080.0f;

};