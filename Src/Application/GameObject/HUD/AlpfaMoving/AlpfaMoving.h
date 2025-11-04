#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class AlpfaMoving :public SelectDraw2DTexture
{
public:
	static const uint32_t TypeID;

	AlpfaMoving(){ m_typeID = TypeID; }
	~AlpfaMoving() override = default;

	void Init() override;
	void Update() override;
	void DrawSprite() override;

private:

	float m_time = 0;		// -1.0f ~ 1.0fをとる値
	float m_alpha = 0;

	float m_min = 0.0f;
	float m_max = 0.0f;

};