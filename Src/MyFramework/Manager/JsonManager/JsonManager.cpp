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
		// ゲームオブジェクトに追加
		AddJsonObject(it["Name"], it);
	}
}

void JsonManager::AllSave() const
{
	// jsonの配列の生成
	nlohmann::json json = nlohmann::json::array();

	for (auto& it : SceneManager::Instance().GetObjList())
	{
		nlohmann::json jsonObj;
		it->JsonSave(jsonObj);
		json.push_back(jsonObj);
	}
	for (auto& camera : SceneManager::Instance().GetCurrentScene()->GetCameraObjList())
	{
		nlohmann::json jsonObj;
		camera->JsonSave(jsonObj);
		json.push_back(jsonObj);
	}
	for (auto& mapList : SceneManager::Instance().GetMapList())
	{
		nlohmann::json jsonObj;
		mapList->JsonSave(jsonObj);
		json.push_back(jsonObj);
	}
	std::string nowScene = SceneManager::Instance().GetCurrentScene()->GetSceneName();

	SceneManager::Instance().GetCurrentScene()->SaveSceneSettingsToJson("Json/" + nowScene + "PostProcess");
	JsonSerialize(json, "Json/" + nowScene);
}

std::shared_ptr<KdGameObject> JsonManager::AddJsonObject(const std::string& _className, const nlohmann::json& _json) const
{
	static int count = 0; // デバッグ用のカウント

	const auto& classMap = RegisterObject::GetInstance().m_ClassNameToID;
	auto it = classMap.find(_className);
	if (it != classMap.end())
	{
		uint32_t classKey = it->second;
		auto& regObj = RegisterObject::GetInstance().GetRegisterObject();
		auto found = regObj.find(classKey);
		if (found != regObj.end())
		{
			// バリューにインスタンスを生成するラムダ式入ってるからそれを呼び出す
			std::shared_ptr<KdGameObject> obj = found->second();

			// 各オブジェクトを,jsonファイルから読み込む
			if (!_json.is_null()) obj->JsonInput(_json);

			// もしカメラだったら、シーンに追加しない
			if (_className == "class FPSCamera")
			{
				SceneManager::Instance().GetCurrentScene()->AddCameraObject(obj);

				KdDebugGUI::Instance().AddLog(U8("FPSCameraを追加しました\n"));
			}
			else if (_className == "class Map")
			{
				SceneManager::Instance().GetCurrentScene()->AddMapObject(obj);
				count++;
				KdDebugGUI::Instance().AddLog(U8("マップオブジェクトを追加しました : %d\n"), count);
			}
			else
			{
				count++; // デバッグ用のカウントアップ

				SceneManager::Instance().AddObject(obj); // シーンに追加
				KdDebugGUI::Instance().AddLog(U8("Jsonからオブジェクトを追加しました : %d \n"), count);
			}

			obj->Init();

			return obj;
		}
	}
	// 見つからなかった場合は,nullptrを返す
	return nullptr;
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
		{"y",_vec.y },
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
