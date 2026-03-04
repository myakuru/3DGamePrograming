#pragma once
class EffekseerEffectManager;
class EffekseerEffectBase;

class EffectReference
{
public:

	~EffectReference() = default;

	std::weak_ptr<KdGameObject> AttachObject() const
	{
		return m_wpObject;
	}

	void JsonInput(std::string_view _name, const nlohmann::json& _json);

	// 各オブジェクトのデーターをここでJsonに保存する関数
	void JsonSave(std::string_view _name, nlohmann::json& _json) const;

	// 各オブジェクトのImGuiインスペクターを実装する関数
	void ImGuiInspector(std::string_view _label);

	void FindObjectById(const std::list<std::shared_ptr<KdGameObject>>& _list);

	std::weak_ptr<EffekseerEffectBase> GetEffectBase() const
	{
		return m_wpEffectBase;
	}

private:

	Guid m_guid = {};
	
	std::weak_ptr<KdGameObject> m_wpObject;

	std::weak_ptr<EffekseerEffectManager> m_wpEffectManager;

	std::weak_ptr<EffekseerEffectBase> m_wpEffectBase;

};