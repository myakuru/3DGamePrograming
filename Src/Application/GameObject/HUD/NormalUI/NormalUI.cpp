#include "NormalUI.h"
#include "../../../Scene/SceneManager.h"
#include"../../../Scene/BaseScene/BaseScene.h"

const uint32_t NormalUI::TypeID = KdGameObject::GenerateTypeID();
NormalUI::NormalUI()
{
	m_typeID = TypeID;
}

void NormalUI::SetSvgPath(const std::string& path)
{
	m_svgPath = path;
	m_rasterizedW = m_rasterizedH = 0; // 次回描画時に作り直す
}

void NormalUI::SetTargetSizePx(uint32_t w, uint32_t h)
{
	m_targetW = w;
	m_targetH = h;
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