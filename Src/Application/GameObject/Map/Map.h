#pragma once
#include"../Utility/SelectDraw3dModel.h"
class Map :public SelectDraw3dModel
{
public:

	static const uint32_t TypeID;

	Map();
	~Map() override = default;

	uint32_t GetTypeID() const override { return m_typeID; }

private:

	void DrawLit() override;
	void DrawGrayScale() override;

};