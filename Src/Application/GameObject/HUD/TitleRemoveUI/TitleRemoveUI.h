#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class TitleRemoveUI :public SelectDraw2DTexture
{
public:

	static const uint32_t TypeID;

	TitleRemoveUI() { m_typeID = TypeID; }
	~TitleRemoveUI() override = default;

private:

	void Update() override;
	void DrawSprite() override;

};