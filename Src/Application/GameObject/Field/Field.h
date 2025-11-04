#pragma once
#include"../Utility/SelectDraw3dModel.h"
class Field :public SelectDraw3dModel
{
public:

	static const uint32_t TypeID;

	Field()
	{
		m_typeID = TypeID;
	}
	~Field() override = default;

private:

	void DrawLit() override;
	void DrawGrayScale() override;


	int m_RasterizerState = 0;

};