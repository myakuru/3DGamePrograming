#pragma once
#include "../Utility/SelectDraw2DTexture.h"
class HUDBase :public SelectDraw2DTexture
{
public:

	static const uint32_t TypeID;

	HUDBase() { m_typeID = TypeID; }
	~HUDBase() override = default;

protected:

	void Init() override;
	void Update() override;

};