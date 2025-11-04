#include "Map.h"
#include"../../Scene/SceneManager.h"

const uint32_t Map::TypeID = KdGameObject::GenerateTypeID();

Map::Map()
{
	m_typeID = TypeID;
}

void Map::DrawLit()
{
	SelectDraw3dModel::DrawLit();
}

void Map::DrawGrayScale()
{
	if (!SceneManager::Instance().GetDrawGrayScale()) return;

	SelectDraw3dModel::DrawGrayScale();

}
