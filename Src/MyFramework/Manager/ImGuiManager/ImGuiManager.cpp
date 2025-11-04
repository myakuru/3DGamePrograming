#include "ImGuiManager.h"
#include"../JsonManager/JsonManager.h"
#include"../../../Application/Scene/SceneManager.h"
#include"../../../Application/main.h"
#include"../../../Application/GameObject/Camera/TPSCamera/TPSCamera.h"
#include"../../../Application/GameObject/Camera/FPSCamera/FPSCamera.h"
#include"../../../Application/Scene/BaseScene/BaseScene.h"
#include"../../../MyFramework/RegisterObject/RegisterObject.h"

namespace
{
	// トグルボタン
	inline void ToggleButton(const char* _label, bool& _flg)
	{
		if (ImGui::Button(_label))
		{
			_flg = !_flg;
		}
		ImGui::SameLine();
		ImGui::Text(_flg ? "ON" : "OFF");
	}
}

void ImGuiManager::Init()
{
	Math::Viewport vp;
	KdDirect3D::Instance().CopyViewportInfo(vp);

	m_gameSceneSize = ImVec2(vp.width / 2, vp.height / 2); // ゲームシーンのサイズを設定
	m_gameSceneInMouse = false; // マウスがゲームシーン内にあるかどうか
}

void ImGuiManager::ImGuiUpdate()
{
	// ゲームシーンを表示
	ShowGameScene();

	// 上部のメニューバーを表示
	MainMenuBar();

	// ヒエラルキーウィンドウを表示
	Hierarchy();

	// インスペクターウィンドウを表示
	ShowInspector();

	SceneManager::Instance().DrawImGui();
}

void ImGuiManager::Hierarchy()
{
	if (ImGui::Begin("Hierarchy"))
	{
		std::string name = ImSelectClass();

		if (ImGui::Button("AddObject"))
		{
			JSON_MANAGER.AddJsonObject(name);
		}

		// ツリー構造のオブジェクトを表示
		TreeNode();

	}
	ImGui::End();
}

void ImGuiManager::MainMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		// FileやEditなどのメニュー
		if (ImGui::BeginMenu("File"))
		{
			// メニュー項目
			if (ImGui::MenuItem("ObjectSave"))
			{
				JSON_MANAGER.AllSave();
			}
			ImGui::EndMenu();
		}

		ImGui::SameLine();

		// シーン切り替え
		InGuiSceneSelect();

		ImGui::SameLine(300);

		// エフェクト表示のトグル
		ToggleButton(U8("Effectの表示"), m_showEffect);

		// ワイヤーフレーム表示のトグル
		ImGui::SameLine();
		ToggleButton(U8("ワイヤーフレーム表示"), m_showDebugWireFrame);

		// 画面幅に応じてボタンを配置
		ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f);
		ImGuiSelectCamera();

		ImGui::EndMainMenuBar();
	}
}

void ImGuiManager::DrawObjectTreeList(std::list<std::shared_ptr<KdGameObject>>& _list, std::weak_ptr<KdGameObject>& _openObject)
{
	for (const auto& obj : _list)
	{
		if (!obj) continue;
		// ノード描画は MakeTreeNode に一本化
		MakeTreeNode(obj);
	}
}

void ImGuiManager::TreeNode()
{
	// ヒエラルキーウィンドウの子ウィンドウを作成
	if (ImGui::BeginChild("Objects"))
	{
		ImGui::Separator();


		// 通常オブジェクトリストの描画
		DrawObjectTreeList(SceneManager::Instance().GetObjList(), m_openObject);

		if (auto scene = SceneManager::Instance().GetCurrentScene())
		{
			// フリーカメラオブジェクトリストの描画
			DrawObjectTreeList(scene->GetCameraObjList(), m_openObject);
			// マップオブジェクトリストの描画(カリング対応)
			DrawObjectTreeList(scene->GetMapObjectList(), m_openObject);
		}
	}
	ImGui::EndChild();
}

void ImGuiManager::ShowInspector()
{
	if (!ImGui::Begin("Inspector"))
	{
		ImGui::End();
		return;
	}

	const auto selectObject = SceneManager::Instance().m_selectObject;
	const auto openObject = m_openObject.lock();

	if (openObject && openObject != selectObject)
	{
		openObject->ImGuiInspector();
	}
	else if (selectObject)
	{
		selectObject->ImGuiInspector();
	}
	else
	{
		ImGui::TextUnformatted("No selection");
	}

	ImGui::End();
}

void ImGuiManager::ShowGameScene()
{
	if (ImGui::Begin("Game"))
	{
		if (!SceneManager::Instance().GetCurrentScene())
		{
			ImGui::End();
			return;
		}

		Math::Viewport vp;
		KdDirect3D::Instance().CopyViewportInfo(vp);

		m_winSize = ImGui::GetWindowSize();

		auto texID = (ImTextureID)(KdShaderManager::Instance().m_postProcessShader
			.GetNoiseRenderTargetPack().m_RTTexture->WorkSRView());

		m_width = m_winSize.x;
		m_height = m_width * 9.0f / 16.0f;		// 16:9に固定
		m_gameSceneSize.x = m_width - 20.f;		// 少し余白を取る
		m_gameSceneSize.y = m_height;

		ImGui::Image(texID, { m_gameSceneSize.x ,m_gameSceneSize.y });

		// 画像の矩形取得
		ImVec2 imageMin = ImGui::GetItemRectMin();
		ImVec2 imageMax = ImGui::GetItemRectMax();
		ImVec2 mousePos = ImGui::GetIO().MousePos;
		const bool hovered = ImGui::IsItemHovered();

		// 画像上でクリックされたか判定
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			offsetX = mousePos.x - imageMin.x;
			offsetY = mousePos.y - imageMin.y;

			POINT mouse;
			if (offsetX >= 0 && offsetY >= 0 && offsetX < m_gameSceneSize.x && offsetY < m_gameSceneSize.y)
			{
				float gameX = offsetX * (vp.width / m_gameSceneSize.x);
				float gameY = offsetY * (vp.height / m_gameSceneSize.y);

				mouse.x = static_cast<LONG>(gameX);
				mouse.y = static_cast<LONG>(gameY);

				// コライダーからレイを取得
				KdCollider::RayInfo rayInfo;
				auto spCamera = GetActiveCamera();
				if (spCamera)
				{
					spCamera->GenerateRayInfoFromClientPos(mouse, rayInfo.m_pos, rayInfo.m_dir, rayInfo.m_range);
					rayInfo.m_type = KdCollider::TypeEvent;

					std::list<KdCollider::CollisionResult> results;
					for (const auto& it : SceneManager::Instance().GetMapList())
					{
						it->SelectObjectIntersects(rayInfo, &results);
					}

					for (const auto& it : SceneManager::Instance().GetObjList())
					{
						it->SelectObjectIntersects(rayInfo, &results);
					}

					KdCollider::CollisionResult resultObject;
					float length = rayInfo.m_range;
					for (auto& it : results)
					{
						if (length > (it.m_hitPos - spCamera->GetCameraMatrix().Translation()).Length())
						{
							resultObject = it;
							length = (it.m_hitPos - spCamera->GetCameraMatrix().Translation()).Length();
						}
					}

					if (resultObject.m_resultObject)
					{
						SceneManager::Instance().m_selectObject = resultObject.m_resultObject;
						m_openObject = resultObject.m_resultObject;
					}
				}
			}
		}
		ImGui::End();

		// gameシーン外にマウスが出たらフラグを折る
		m_gameSceneInMouse = hovered;
	}
}

std::string ImGuiManager::ImSelectClass() const
{
	static std::string currentName = "KdGameObject";
	// ImGuiのコンボボックスを作成
	if (ImGui::BeginCombo("##Class", currentName.data()))
	{
		for (const auto& [name, id] : RegisterObject::GetInstance().m_ClassNameToID)
		{
			bool selected = (currentName == name);
			if (ImGui::Selectable(name.c_str(), selected)) currentName = name;
			if (selected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	return currentName;
}

void ImGuiManager::InGuiSceneSelect() const
{
	// シーンの種類を定義するマップを作成＜－こいつ増やせばシーンを増やせる
	std::map<std::string, SceneManager::SceneType, std::less<>> sceneType =
	{
		{"Title",SceneManager::SceneType::Title},
		{"Game",SceneManager::SceneType::Game},
		{"Result",SceneManager::SceneType::Result},
		{"Test",SceneManager::SceneType::Test},
		{"ConstructionSiteStage",SceneManager::SceneType::ConstructionSiteStage},
	};

	// こいつと比較して、シーンが異なるかどうかを確認する
	std::string nowSceneName = SceneManager::Instance().GetCurrentScene()->GetSceneName();

	if (ImGui::Begin("Scene"))
	{
		// シーンを選択するボタンを作成
		for (const auto& [key, value] : sceneType)
		{
			// ボタンと現在のシーン名が異なる場合のみ、シーンを切り替える
			if (ImGui::Button(key.data()) && nowSceneName != key.data())
			{
				SceneManager::Instance().SetNextScene(value);
			}
			ImGui::SameLine();
		}
	}
	ImGui::End();

	ImGui::NewLine(); // レイアウト崩れ防止のため改行
}

void ImGuiManager::ImGuiSelectCamera()
{

	if (SceneManager::Instance().m_sceneCamera)
	{
		if (ImGui::Button(U8("ゲーム終了"), ImVec2(150, 20)))
		{
			SceneManager::Instance().m_sceneCamera = false;
		}
	}
	else
	{
		if (ImGui::Button(U8("ゲーム開始"), ImVec2(150, 20)))
		{
			SceneManager::Instance().m_sceneCamera = true;
		}
	}

}

void ImGuiManager::MakeTreeNode(const std::shared_ptr<KdGameObject>& parentObj)
{
	if (!parentObj) return;

	// 選択中なら最初から開く
	ImGui::SetNextItemOpen(parentObj == m_openObject.lock(), ImGuiCond_Always);

	ImGui::PushID(parentObj.get());

	const bool opened = ImGui::TreeNodeEx(parentObj->GetNameClass().c_str());

	// ヘッダクリックで選択
	if (ImGui::IsItemClicked())
	{
		m_openObject = parentObj;
	}

	// ドラッグ元
	if (ImGui::BeginDragDropSource())
	{
		KdGameObject* raw = parentObj.get();
		ImGui::SetDragDropPayload("Child", &raw, sizeof(raw));
		ImGui::Text("%s", parentObj->GetNameClass().c_str());
		ImGui::EndDragDropSource();
	}

	// ドロップ先
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Child"))
		{
			auto dropped = *static_cast<KdGameObject* const*>(payload->Data);
			if (dropped && dropped != parentObj.get())
			{
				// 共有所有権を復元して親子化
				std::shared_ptr<KdGameObject> childSp;
				try
				{
					childSp = dropped->shared_from_this();
				}
				catch (...)
				{
					childSp.reset();
				}

				if (childSp)
				{
					std::weak_ptr<KdGameObject> w = childSp;
					parentObj->AddChild(w);
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::SameLine(200);
	if (ImGui::SmallButton("Delete"))
	{
		parentObj->SetExpired(true);
	}

	// 子の描画
	if (opened)
	{
		// 子の weak_ptr が切れていたら掃除しつつ描画
		auto& children = parentObj->GetChild();
		for (auto it = children.begin(); it != children.end(); )
		{
			if (auto child = it->lock())
			{
				MakeTreeNode(child);
				++it;
			}
			else
			{
				it = children.erase(it);
			}
		}
		ImGui::TreePop();
	}

	ImGui::Separator();
	ImGui::PopID();
}

std::shared_ptr<KdCamera> ImGuiManager::GetActiveCamera()
{
	if (SceneManager::Instance().m_sceneCamera)
	{
		SceneManager::Instance().GetCameraWeakPtr(m_fpsCamera);
		if (!m_fpsCamera.expired()) return m_fpsCamera.lock()->GetCamera();
	}
	return nullptr;
}
