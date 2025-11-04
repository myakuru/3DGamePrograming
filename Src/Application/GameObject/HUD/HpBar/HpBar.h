#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class Player;
class HpBar : public SelectDraw2DTexture
{
public:
	static const uint32_t TypeID;
	HpBar() { m_type = TypeID; }
	~HpBar() override = default;

private:

	void Init() override;
	void DrawSprite() override;
	void Update() override;

	void ImGuiInspector() override;

	Math::Rectangle m_srcRect;
	float m_hpRate = 0.0f;

	Math::Rectangle m_rect = { 0,0,1500,300 };

	std::weak_ptr<Player> m_player;
};