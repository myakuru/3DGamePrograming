#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class TitleRemoveUI :public SelectDraw2DTexture
{
public:

	static const uint32_t TypeID;

	TitleRemoveUI() { m_typeID = TypeID; }
	~TitleRemoveUI() override = default;


private:

	bool m_hit = false;

	Math::Rectangle m_rect = { 0,0,1500,300 };

	Math::Matrix m_finalScaleMat = Math::Matrix::Identity;

	void Update() override;
	void DrawSprite() override;

};