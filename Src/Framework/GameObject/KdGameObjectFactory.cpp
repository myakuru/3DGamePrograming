#include "KdGameObjectFactory.h"

void KdGameObjectFactory::RegisterCreateFunction(const std::string_view str, const std::function<std::shared_ptr<KdGameObject>(void)> func)
{
	m_createFunctions[str.data()] = func;
}

std::shared_ptr<KdGameObject> KdGameObjectFactory::CreateGameObject(const std::string_view objName) const
{
	auto creater = m_createFunctions.find(objName);

	if (creater == m_createFunctions.end())
	{
		assert(0 && "GameObjectFactoryに未登録のゲームオブジェクトクラスです");

		return nullptr;
	}

	return  creater->second();
}

// リスト内の最適化（寿命の尽きたオブジェクトの解放）
void KdGameObjectFactory::RemoveExpiredObjects()
{
	for (auto objIter = m_objects.begin(); objIter != m_objects.end();)
	{
		if (!(*objIter)->IsExpired())
		{
			objIter = m_objects.erase(objIter);

			continue;
		}

		++objIter;
	}
}