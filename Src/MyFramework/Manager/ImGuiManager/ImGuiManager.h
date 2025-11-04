#pragma once
class TPSCamera;
class FPSCamera;

class ImGuiManager : public KdGameObject
{
public:
	ImGuiManager() = default;
	~ImGuiManager() override = default;

	void Init() override;
	void ImGuiUpdate();

	// ウィンドウ
	void MainMenuBar();
	void ShowGameScene();
	void Hierarchy();
	void ShowInspector();

	// UIパーツ
	std::string ImSelectClass() const;
	void InGuiSceneSelect() const;
	void ImGuiSelectCamera();

	void MakeTreeNode(const std::shared_ptr<KdGameObject>& parentObj);

	// 状態取得
	KdRenderTargetPack GetRenderTargetChanger() { return m_renderTargetChanger; }
	bool GetGameSceneInMouse() const { return m_gameSceneInMouse; }
	Math::Vector2 GetWindowSize() const { return m_windowSize; }
	bool GetShowEffect() const { return m_showEffect; }
	bool GetShowDebugWireFrame() const { return m_showDebugWireFrame; }


	// ツリー描画の共通処理
	void DrawObjectTreeList(std::list<std::shared_ptr<KdGameObject>>& _list, std::weak_ptr<KdGameObject>& _openObject);

private:
	void TreeNode();
	std::shared_ptr<KdCamera> GetActiveCamera();

private:
	std::weak_ptr<TPSCamera> m_tpsCamera;
	std::weak_ptr<FPSCamera> m_fpsCamera;

	std::weak_ptr<KdGameObject> m_openObject;

	std::shared_ptr<KdTexture> m_texture = nullptr;
	KdRenderTargetPack m_renderTargetChanger;

	ImVec2 m_gameSceneSize{};
	ImVec2 m_winSize{};
	float m_width = 0.0f;
	float m_height = 0.0f;

	float offsetX = 0.0f;
	float offsetY = 0.0f;

	Math::Vector2 m_windowSize = Math::Vector2(1920, 1080);

	bool m_gameSceneInMouse = true;

	bool m_showEffect = true;
	bool m_showDebugWireFrame = false;

	std::string m_modelPath = "Asset/";
};