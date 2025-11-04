#pragma once
#include "../Utility/SelectDraw2DTexture.h"
class HUDManager : public SelectDraw2DTexture
{
public:

	HUDManager() = default;
	~HUDManager() override = default;

protected:

	void Init() override;
	void Update() override;
	void DrawSprite() override;

};