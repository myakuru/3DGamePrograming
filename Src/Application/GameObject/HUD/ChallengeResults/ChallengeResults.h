#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class ChallengeResults :public SelectDraw2DTexture
{
public:
	static const uint32_t TypeID;

	ChallengeResults() { m_typeID = TypeID; }
	~ChallengeResults() override = default;

private:

	void Init() override;
	void DrawSprite() override;
	void Update() override;

	float m_timer = 0.0f; // タイマー
	bool m_isComplete = false; // 完了フラグ

};