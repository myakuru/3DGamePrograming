#include "EffectReference.h"
#include "Application/Scene/SceneManager.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectManager.h"

void EffectReference::JsonInput(std::string_view _name, const nlohmann::json& _json)
{
	std::string value;

	if (_json.contains(_name)) value = _json[_name.data()].get<std::string>();
	m_guid.FromString(value);

	auto objList = SceneManager::Instance().GetCurrentScene()->GetObjList();
	FindObjectById(objList);

	m_wpEffectBase = std::static_pointer_cast<EffekseerEffectBase>(m_wpObject.lock());

}

void EffectReference::JsonSave(std::string_view _name, nlohmann::json& _json) const
{
	_json[_name.data()] = m_guid.ToString();
}

void EffectReference::ImGuiInspector(std::string_view _label)
{
	std::string preview = "None";

	if (auto spObj = m_wpObject.lock())
	{
		preview = spObj->GetName();
	}

	SceneManager::Instance().GetObjectWeakPtr(m_wpEffectManager);

	auto effectManager = m_wpEffectManager.lock();
	if (!effectManager) return;

	if (ImGui::BeginCombo(_label.data(), preview.data()))
	{
		for (const auto& effekseerEffect : effectManager->m_effectObjList)
		{
			if (auto obj = effekseerEffect.lock())
			{
				if (ImGui::MenuItem(obj->GetName().c_str()))
				{

					m_wpObject = effekseerEffect;
					m_wpEffectBase = effekseerEffect;
					m_guid = obj->GetGUID();
				}
			}
		}

		ImGui::EndCombo();
	}

}

void EffectReference::FindObjectById(const std::list<std::shared_ptr<KdGameObject>>& _list)
{
	for (auto& obj : _list)
	{
		if (obj->GetGUID().GetUUID() == m_guid.GetUUID())
		{
			m_wpObject = obj;
			return;
		}

		FindObjectById(obj->GetChild());

		if(m_wpObject.lock())return;
	}
}


