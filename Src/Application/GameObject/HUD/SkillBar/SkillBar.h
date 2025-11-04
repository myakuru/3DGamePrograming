#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class Player;
class SkillBar : public SelectDraw2DTexture
{
public:
	static const uint32_t TypeID;
	SkillBar() { m_type = TypeID; }
	~SkillBar() override = default;

private:

	void DrawSprite() override;
	void Update() override;


	float m_hpRate = 0.0f;

	Math::Rectangle m_rect = { 0,0,500,100 };

	std::weak_ptr<Player> m_player;

	float m_timer = 0.0f;

};