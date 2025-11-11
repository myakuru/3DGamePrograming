#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class NormalUI :public SelectDraw2DTexture
{
public:

	static const uint32_t TypeID;

	NormalUI();
	~NormalUI() override = default;


private:

	void Init() override;
	void DrawSprite() override;
	void Update() override;
};