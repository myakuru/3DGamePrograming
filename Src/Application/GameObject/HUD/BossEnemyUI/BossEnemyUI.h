#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class BossEnemy;
class BossEnemyUI : public SelectDraw2DTexture
{
public:
	static const uint32_t TypeID;
	BossEnemyUI() { m_type = TypeID; }
	~BossEnemyUI() override = default;

private:

	void Init() override;
	void DrawSprite() override;
	void Update() override;

	Math::Rectangle m_srcRect;
	float m_hpRate = 0.0f;

	Math::Rectangle m_rect = { 0,0,1500,300 };

	std::weak_ptr<BossEnemy> m_bossEnemy;
};