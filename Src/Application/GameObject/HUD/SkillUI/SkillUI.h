#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class Player;
class SkillUI :public SelectDraw2DTexture
{
public:

	static const uint32_t TypeID;

	SkillUI() { m_typeID = TypeID; }
	~SkillUI() override = default;

private:

	void Init() override;
	void Update() override;
	void DrawSprite() override;

	std::weak_ptr<Player> m_player;

	std::shared_ptr<KdTexture> m_skillIconTexture = std::make_shared<KdTexture>();

	float m_ESkillCountRate = 0.0f;

	Math::Rectangle m_rect = { 0,0,100,100 };
	Math::Rectangle m_srcRect = { 0,0,100,100 };

	float m_timer = 0.0f;

	bool m_showESkillUI = false;

};