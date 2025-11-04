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

	bool m_showSkillUI = false;

	std::weak_ptr<Player> m_player;

	std::shared_ptr<KdTexture> m_skillIconTexture = std::make_shared<KdTexture>();

};