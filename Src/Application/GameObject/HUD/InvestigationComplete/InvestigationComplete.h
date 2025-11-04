#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class InvestigationComplete :public SelectDraw2DTexture
{
public:
	static const uint32_t TypeID;

	InvestigationComplete() { m_typeID = TypeID; }
	~InvestigationComplete() override = default;

private:

	void Init() override;
	void DrawSprite() override;
	void Update() override;

	float m_timer = 0.0f; // タイマー
	bool m_isComplete = false; // 完了フラグ

};