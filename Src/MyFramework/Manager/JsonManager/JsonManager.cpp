#include "JsonManager.h"
#include"../../../Application/Scene/SceneManager.h"
#include"../../../Application/Scene/BaseScene/BaseScene.h"
#include"../../../MyFramework/RegisterObject/RegisterObject.h"

void JsonManager::JsonToObj() const
{
	auto name = SceneManager::Instance().GetCurrentScene()->GetSceneName();
	nlohmann::json json = JsonDeserialize("Json/" + name);
	SceneManager::Instance().GetCurrentScene()->LoadSceneSettingsFromJson("Json/" + name + "PostProcess");

	for (auto& it : json)
	{
		// Class があればそれを使う。なければ Name を識別子として後方互換。
		const std::string classKey =
			(it.contains("Class") && it["Class"].is_string())
			? it["Class"].get<std::string>()
			: it["Name"].get<std::string>();

		AddJsonObject(classKey, it);
	}
}

void JsonManager::AllSave() const
{
	// jsonの配列の生成
	nlohmann::json json = nlohmann::json::array();

	for (const auto& it : SceneManager::Instance().GetObjList())
	{
		// 子(親を持つ)はルートに保存しない
		if (it->GetParent().lock()) continue;

		nlohmann::json jsonObj;
		it->JsonSave(jsonObj);
		json.push_back(jsonObj);
	}

	for (const auto& camera : SceneManager::Instance().GetCurrentScene()->GetCameraObjList())
	{
		if (camera->GetParent().lock()) continue;

		nlohmann::json jsonObj;
		camera->JsonSave(jsonObj);
		json.push_back(jsonObj);
	}

	std::string nowScene = SceneManager::Instance().GetCurrentScene()->GetSceneName();

	SceneManager::Instance().GetCurrentScene()->SaveSceneSettingsToJson("Json/" + nowScene + "PostProcess");
	JsonSerialize(json, "Json/" + nowScene);
}

std::shared_ptr<KdGameObject> JsonManager::AddJsonObject(const std::string& _className, const nlohmann::json& _json, bool _addToScene) const
{
	const auto& classMap = RegisterObject::GetInstance().GetClassNameToID();
	auto it = classMap.find(_className);
	if (it == classMap.end()) return nullptr;

	uint32_t classKey = it->second;
	auto& regObj = RegisterObject::GetInstance().GetRegisterObject();
	auto found = regObj.find(classKey);
	if (found == regObj.end()) return nullptr;

	std::shared_ptr<KdGameObject> obj = found->second();

	if (!_json.is_null()) obj->JsonInput(_json);

	// 子の処理
	if (_json.contains("child") && _json["child"].is_array())
	{
		for (auto& childJson : _json["child"])
		{
			if (!(childJson.contains("Class") || childJson.contains("Name")))
			{
				KdDebugGUI::Instance().AddLog(U8("子オブジェクトにClass/Nameがありません\n"));
				continue;
			}

			const std::string childClassName =
				(childJson.contains("Class") && childJson["Class"].is_string())
				? childJson["Class"].get<std::string>()
				: childJson["Name"].get<std::string>();

			const bool addChildToScene = childJson.value("addToScene", true);

			auto childObj = AddJsonObject(childClassName, childJson, addChildToScene);
			if (childObj) obj->AddChild(childObj);
			else KdDebugGUI::Instance().AddLog(U8("子オブジェクト生成失敗: %s\n"), childClassName.data());
		}
	}

	// Listsへの追加
	if (_addToScene)
	{
		if (_className == "class FPSCamera")
		{
			SceneManager::Instance().GetCurrentScene()->AddCameraObject(obj);
			KdDebugGUI::Instance().AddLog(U8("FPSCameraを追加しました\n"));
		}
		else
		{
			SceneManager::Instance().AddObject(obj);
			KdDebugGUI::Instance().AddLog(U8("Jsonからオブジェクトを追加しました\n"));
		}
	}

	obj->Init();
	return obj;
}

void JsonManager::JsonSerialize(const nlohmann::json& _json, const std::string& _path) const
{
	std::ofstream outPut((_path + ".json").data());

	// 開かれてない場合リターン
	if (!outPut.is_open()) return;
	outPut << _json.dump(2);	// jsonにフォーマットする(2スペース)
	outPut.close();

}

nlohmann::json JsonManager::JsonDeserialize(const std::string& _path)const
{
	std::ifstream input((_path + ".json").data());
	nlohmann::json json;

	if (!input.is_open())return json;
	input >> json;
	input.close();

	return json;
}

Math::Vector3 JsonManager::JsonToVector(const nlohmann::json& _json) const
{
	return Math::Vector3
	{
		_json["x"],
		_json["y"],
		_json["z"]
	};
}

nlohmann::json JsonManager::VectorToJson(const Math::Vector3& _vec) const
{
	return nlohmann::json
	{
		{"x", _vec.x },
		{"y", _vec.y },
		{"z", _vec.z}
	};
}

Math::Vector2 JsonManager::JsonToVector2(const nlohmann::json& _json) const
{
	return Math::Vector2
	{
		_json["x"],
		_json["y"]
	};
}

nlohmann::json JsonManager::Vector2ToJson(const Math::Vector2& _vec) const
{
	return nlohmann::json
	{
		{"x", _vec.x },
		{"y",_vec.y }
	};
}

Math::Vector4 JsonManager::JsonToVector4(const nlohmann::json& _json) const
{
	return Math::Vector4
	{
		_json["x"],
		_json["y"],
		_json["z"],
		_json["w"]
	};
}

nlohmann::json JsonManager::Vector4ToJson(const Math::Vector4& _vec) const
{
	return nlohmann::json
	{
		{"x", _vec.x },
		{"y",_vec.y },
		{"z", _vec.z},
		{"w", _vec.w}
	};
}
