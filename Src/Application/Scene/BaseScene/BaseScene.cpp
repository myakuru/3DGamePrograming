#include "BaseScene.h"
#include"../SceneManager.h"
#include"../../GameObject/Camera/CameraBase.h"
#include"../../GameObject/Camera/PlayerCamera/PlayerCamera.h"

#include"../../../MyFramework/Manager/JsonManager/JsonManager.h"
#include"../../../MyFramework/Manager/ImGuiManager/ImGuiManager.h"
#include"../../main.h"

void BaseScene::PreUpdate()
{
	// Updateの前の更新処理
	// オブジェクトリストの整理・・・ 無効なオブジェクトを削除
	auto it = m_objList.begin();

	while (it != m_objList.end())
	{
		if ((*it)->IsExpired())	// IsExpired()・・・ 無効ならtrue
		{
			// 無効なオブジェクトをリストから削除
			it = m_objList.erase(it);
		}
		else
		{
			++it;	// 次の要素へイテレータを進める
		}
	}

	auto mapIt = m_MapObjectList.begin();
	while (mapIt != m_MapObjectList.end())
	{
		if ((*mapIt)->IsExpired())	// IsExpired()・・・ 無効ならtrue
		{
			// 無効なオブジェクトをリストから削除
			mapIt = m_MapObjectList.erase(mapIt);
		}
		else
		{
			++mapIt;	// 次の要素へイテレータを進める
		}
	}

	// バケットの整理（expired を除去）
	CompactTypeBuckets();

	// ↑の後には有効なオブジェクトだけのリストになっている

	for (auto& obj : m_objList)
	{
		obj->PreUpdate();
	}

	for (auto& obj : m_MapObjectList)
	{
		obj->PreUpdate();
	}

	SceneManager::Instance().GetObjectWeakPtr(m_playerCamera);

	if (m_playerCamera.expired()) return;

}

void BaseScene::Update()
{
	if (!SceneManager::Instance().m_sceneCamera)
	{
		// シーン毎のイベント処理
		Event();

		// KdGameObjectを継承した全てのオブジェクトの更新 (ポリモーフィズム)
		for (auto& obj : m_objList)
		{
			obj->Update();
		}
	}
}

void BaseScene::PostUpdate()
{
	if (!SceneManager::Instance().m_sceneCamera)
	{
		for (auto& obj : m_objList)
		{
			obj->PostUpdate();
		}
	}
	else
	{
		for (auto& obj : m_CameraObjList)
		{
			obj->CameraUpdate();
		}
	}
}

void BaseScene::PreDraw()
{
	for (auto& obj : m_objList)
	{
		obj->PreDraw();
	}

	if (SceneManager::Instance().m_sceneCamera)
	{
		for (auto& cameraObj : m_CameraObjList)
		{
			cameraObj->PreDraw();
		}
	}

	auto playerCamera = m_playerCamera.lock();

	if (playerCamera)
	{
		// カメラからフラスタム生成
		DirectX::BoundingFrustum frustum = playerCamera->CreateFrustum();

		int cunter =0;

		// Mapリストからカリング
		for (auto& obj : m_MapObjectList)
		{
			bool result = obj->CheckInScreen(frustum);
			if (result)
			{
				cunter++;
				m_drawObjectList.push_back(obj);
			}
		}
	}
	else
	{
		for (auto& obj : m_MapObjectList)
		{
			m_drawObjectList.push_back(obj);
		}
	}
}

void BaseScene::Draw()
{

	// 光を遮るオブジェクト(不透明な物体や2Dキャラ)
	KdShaderManager::Instance().m_StandardShader.BeginGenerateDepthMapFromLight();
	{
		for (auto& obj : m_objList) obj->GenerateDepthMapFromLight();
		for (auto& obj : m_drawObjectList) obj->GenerateDepthMapFromLight();
	}
	KdShaderManager::Instance().m_StandardShader.EndGenerateDepthMapFromLight();

	// 陰影のないオブジェクト(背景など)
	KdShaderManager::Instance().m_StandardShader.BeginUnLit();
	{
		for (auto& obj : m_objList) obj->DrawUnLit();
		for (auto& obj : m_drawObjectList) obj->DrawUnLit();
	}
	KdShaderManager::Instance().m_StandardShader.EndUnLit();

	// 陰影のあるオブジェクト
	KdShaderManager::Instance().m_StandardShader.BeginLit();
	{
		for (auto& obj : m_objList) obj->DrawLit();
		for (auto& obj : m_drawObjectList) obj->DrawLit();
	}
	KdShaderManager::Instance().m_StandardShader.EndLit();

	// グレースケール
	KdShaderManager::Instance().m_StandardShader.BeginGrayscale();
	{
		for (auto& obj : m_objList) obj->DrawGrayScale();
		for (auto& obj : m_drawObjectList) obj->DrawGrayScale();
	}
	KdShaderManager::Instance().m_StandardShader.EndGrayscale();

	// リムライトオブジェクトの描画
	KdShaderManager::Instance().m_StandardShader.BeginRimLight();
	{
		for (auto& obj : m_objList) obj->DrawRimLight();
		for (auto& obj : m_drawObjectList) obj->DrawRimLight();
	}
	KdShaderManager::Instance().m_StandardShader.EndRimLight();

	// 陰影のないエフェクト
	KdShaderManager::Instance().m_StandardShader.BeginUnLit();
	{
		for (auto& obj : m_objList) obj->DrawEffect();
		for (auto& obj : m_drawObjectList) obj->DrawEffect();
	}
	KdShaderManager::Instance().m_StandardShader.EndUnLit();

	// 光源オブジェクト
	KdShaderManager::Instance().m_postProcessShader.BeginBright();
	{
		for (auto& obj : m_objList) obj->DrawBright();
		for (auto& obj : m_drawObjectList) obj->DrawBright();
	}
	KdShaderManager::Instance().m_postProcessShader.EndBright();

	// Toonシェーダー
	KdShaderManager::Instance().m_StandardShader.BeginToon();
	{
		for (auto& obj : m_drawObjectList) obj->DrawToon();
		for (auto& obj : m_objList) obj->DrawToon();
	}
	KdShaderManager::Instance().m_StandardShader.EndToon();

	// グラデーション
	KdShaderManager::Instance().m_StandardShader.BeginGradient();
	{
		for (auto& obj : m_objList) obj->DrawGradation();
		for (auto& obj : m_drawObjectList) obj->DrawGradation();
	}
	KdShaderManager::Instance().m_StandardShader.EndGradient();

	// エフェクト
	KdShaderManager::Instance().m_StandardShader.BeginEffect();
	{
		for (auto& obj : m_objList) obj->DrawEffect();
		for (auto& obj : m_drawObjectList) obj->DrawEffect();
	}
	KdShaderManager::Instance().m_StandardShader.EndEffect();

	KdEffekseerManager::GetInstance().Draw();

	m_drawObjectList.clear();
}

void BaseScene::DrawSprite()
{
	//2D描画
	KdShaderManager::Instance().m_spriteShader.Begin();
	for (auto& obj : m_objList) obj->DrawSprite();
	KdShaderManager::Instance().m_spriteShader.End();
}

void BaseScene::DrawDebug()
{
	KdShaderManager::Instance().m_StandardShader.BeginUnLit();
	{
		// デバッグ表示がOFFでも、各オブジェクトにFlushさせてバッファをクリアするため常に呼ぶ
		for (auto& obj : m_objList) obj->DrawDebug();
		for (auto& obj : m_MapObjectList) obj->DrawDebug();
	}
	KdShaderManager::Instance().m_StandardShader.EndUnLit();
}

void BaseScene::Event()
{
}

void BaseScene::Init()
{
}

void BaseScene::DrawImGui()
{
	ImGui::Begin("PostProcess");
	{
		ImGui::Text(U8("光の調整"));
		ImGui::DragFloat(U8("ブライトの調整"), &m_brightThreshold,0.1f);
		ImGui::ColorEdit3(U8("霧の色"), &m_fogColor.x);
		ImGui::DragFloat(U8("霧の濃さ"), &m_fogDensity,0.0001f,0.0f,0.1f);

		ImGui::Separator();
		ImGui::Text(U8("高さFOGの調整"));
		ImGui::ColorEdit3(U8("高い位置の霧の色"), &m_highFogColor.x);
		ImGui::DragFloat(U8("Fogの最大値"), &m_highFogHeight,0.1f);
		ImGui::DragFloat(U8("Fogの最小値"), &m_lowFogHeight,0.1f);
		ImGui::DragFloat(U8("Fogの開始距離"), &m_highFogDistance,0.1f);

		ImGui::Separator();
		ImGui::Text(U8("平行光の調整"));
		ImGui::DragFloat3(U8("平行光の方向"), &m_directionalLightDir.x,0.1f);
		ImGui::ColorEdit3(U8("平行光の色"), &m_directionalLightColor.x);

		ImGui::Separator();
		ImGui::Text(U8("環境光の調整"));
		ImGui::ColorEdit4(U8("環境光の色"), &m_anviLightColor.x);

		ImGui::Separator();

		ImGui::Checkbox(U8("霧を使う"), &m_fogEnable);
		ImGui::Checkbox(U8("距離で霧を使う"), &m_fogUseRange);

		ImGui::Separator();
		ImGui::DragFloat2(U8("影の射影行列"), &m_lightingArea.x,0.1f,0.1f);
		ImGui::DragFloat(U8("影の高さ"), &m_dirLightHeight,1.0f,0.1f);
	}
	ImGui::End();

	KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(m_brightThreshold);
	KdShaderManager::Instance().WorkAmbientController().SetFogEnable(m_fogEnable, m_fogUseRange);
	KdShaderManager::Instance().WorkAmbientController().SetDistanceFog({ m_fogColor }, m_fogDensity);
	KdShaderManager::Instance().WorkAmbientController().SetheightFog({ m_highFogColor }, m_highFogHeight, m_lowFogHeight, m_highFogDistance);

	KdShaderManager::Instance().WorkAmbientController().SetDirLight(m_directionalLightDir, m_directionalLightColor);
	KdShaderManager::Instance().WorkAmbientController().SetDirLightShadowArea(m_lightingArea, m_dirLightHeight);

	KdShaderManager::Instance().WorkAmbientController().SetAmbientLight(m_anviLightColor);
}

void BaseScene::JsonInput(const nlohmann::json& _json)
{
	if (_json.contains("m_brightThreshold")) m_brightThreshold = _json["m_brightThreshold"];
	if (_json.contains("m_fogEnable")) m_fogEnable = _json["m_fogEnable"];
	if (_json.contains("m_fogUseRange")) m_fogUseRange = _json["m_fogUseRange"];
	if (_json.contains("m_fogColor")) m_fogColor = JSON_MANAGER.JsonToVector(_json["m_fogColor"]);
	if (_json.contains("m_fogDensity")) m_fogDensity = _json["m_fogDensity"];
	if (_json.contains("m_highFogColor")) m_highFogColor = JSON_MANAGER.JsonToVector(_json["m_highFogColor"]);
	if (_json.contains("m_highFogHeight")) m_highFogHeight = _json["m_highFogHeight"];
	if (_json.contains("m_lowFogHeight")) m_lowFogHeight = _json["m_lowFogHeight"];
	if (_json.contains("m_highFogDistance")) m_highFogDistance = _json["m_highFogDistance"];
	if (_json.contains("m_directionalLightDir")) m_directionalLightDir = JSON_MANAGER.JsonToVector(_json["m_directionalLightDir"]);
	if (_json.contains("m_directionalLightColor")) m_directionalLightColor = JSON_MANAGER.JsonToVector(_json["m_directionalLightColor"]);
	if (_json.contains("m_lightingArea")) m_lightingArea = JSON_MANAGER.JsonToVector2(_json["m_lightingArea"]);
	if (_json.contains("m_dirLightHeight")) m_dirLightHeight = _json["m_dirLightHeight"];
	if (_json.contains("m_anviLightColor")) m_anviLightColor = JSON_MANAGER.JsonToVector4(_json["m_anviLightColor"]);
}

void BaseScene::JsonSave(nlohmann::json& _json) const
{
	_json["m_brightThreshold"] = m_brightThreshold;
	_json["m_fogEnable"] = m_fogEnable;
	_json["m_fogUseRange"] = m_fogUseRange;
	_json["m_fogColor"] = JSON_MANAGER.VectorToJson(m_fogColor);
	_json["m_fogDensity"] = m_fogDensity;
	_json["m_highFogColor"] = JSON_MANAGER.VectorToJson(m_highFogColor);
	_json["m_highFogHeight"] = m_highFogHeight;
	_json["m_lowFogHeight"] = m_lowFogHeight;
	_json["m_highFogDistance"] = m_highFogDistance;
	_json["m_directionalLightDir"] = JSON_MANAGER.VectorToJson(m_directionalLightDir);
	_json["m_directionalLightColor"] = JSON_MANAGER.VectorToJson(m_directionalLightColor);
	_json["m_lightingArea"] = JSON_MANAGER.Vector2ToJson(m_lightingArea);
	_json["m_dirLightHeight"] = m_dirLightHeight;
	_json["m_anviLightColor"] = JSON_MANAGER.Vector4ToJson(m_anviLightColor);
}

void BaseScene::LoadSceneSettingsFromJson(const std::string& filePath)
{
	nlohmann::json json = JSON_MANAGER.JsonDeserialize(filePath);
	if (!json.is_null())
	{
		JsonInput(json);
	}
}

void BaseScene::SaveSceneSettingsToJson(const std::string& filePath) const
{
	nlohmann::json json;
	JsonSave(json);
	JSON_MANAGER.JsonSerialize(json, filePath); // ファイルに保存
}