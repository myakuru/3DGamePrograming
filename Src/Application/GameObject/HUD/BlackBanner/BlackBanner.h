#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class BlackBanner :public SelectDraw2DTexture
{
public:

	static const uint32_t TypeID;

	BlackBanner() {m_type = TypeID;}
	~BlackBanner() override = default;

private:

	void Init() override;
	void Update() override;
	void DrawSprite() override;



	bool m_intro = false;

};