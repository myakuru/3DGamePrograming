#pragma once

class KdGameObject;

class KdGameObjectFactory
{
public:

	KdGameObjectFactory() {}
	~KdGameObjectFactory() {}

	void RegisterCreateFunction(const std::string_view, const std::function <std::shared_ptr<KdGameObject>(void)> func);

	template<class T>
	std::shared_ptr<T> CreateGameObject()
	{
		std::shared_ptr<T> spObj = std::make_shared<T>();

		spObj->Init();

		m_objects.push_back(spObj);

		return spObj;
	}

	std::shared_ptr<KdGameObject> CreateGameObject(const std::string_view objName) const;

	const std::list<std::shared_ptr<KdGameObject>>& GetObjects() { return m_objects; }

	void RemoveExpiredObjects();

private:

	// GameObjectのインスタンスリスト
	std::list<std::shared_ptr<KdGameObject>> m_objects;

	// GameObjectの生成関数
	std::unordered_map<std::string_view, std::function<std::shared_ptr<KdGameObject>(void)>> m_createFunctions;
};