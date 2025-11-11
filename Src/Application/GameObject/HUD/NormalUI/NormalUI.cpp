#include "NormalUI.h"
#include "../../../Scene/SceneManager.h"
#include"../../../Scene/BaseScene/BaseScene.h"

const uint32_t NormalUI::TypeID = KdGameObject::GenerateTypeID();
NormalUI::NormalUI()
{
	m_typeID = TypeID;
}

void NormalUI::Init()
{
	SelectDraw2DTexture::Init();
}

void NormalUI::DrawSprite()
{
	if (SceneManager::Instance().IsIntroCamera()) return;
	SelectDraw2DTexture::DrawSprite();
}

void NormalUI::Update()
{
	SelectDraw2DTexture::Update();
}