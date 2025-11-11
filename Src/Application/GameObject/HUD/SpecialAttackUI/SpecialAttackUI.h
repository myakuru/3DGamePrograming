#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class Player;
class SpecialAttackUI :public SelectDraw2DTexture
{
public:

	static const uint32_t TypeID;

	SpecialAttackUI() { m_typeID = TypeID; }
	~SpecialAttackUI() override = default;

private:

	void Init() override;
	void Update() override;
	void DrawSprite() override;

	bool m_showSpecialAttackUI = false;

	std::weak_ptr<Player> m_player;

	std::shared_ptr<KdTexture> m_specialAttackIconTexture = std::make_shared<KdTexture>();

	float m_specialCountRate = 0.0f;

	Math::Rectangle m_rect = { 0,0,100,100 };
	Math::Rectangle m_srcRect = { 0,0,100,100 };

	float m_timer = 0.0f;

};