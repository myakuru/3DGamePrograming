#include "HUDBase.h"

const uint32_t HUDBase::TypeID = KdGameObject::GenerateTypeID();

void HUDBase::Init()
{
	SelectDraw2DTexture::Init();
}

void HUDBase::Update()
{
	SelectDraw2DTexture::Update();
}
