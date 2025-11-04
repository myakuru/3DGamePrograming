#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class Player;
class SpecialMove :public SelectDraw2DTexture
{
public:
	static const uint32_t TypeID;

	SpecialMove() { m_typeID = TypeID; }
	~SpecialMove() override = default;


private:

	void Init() override;
	void Update() override;
	void DrawSprite() override;

	int m_displayTime = 0;
	bool m_isIncreasing = false;

	std::weak_ptr<Player> m_player;

	float m_timer = 0.0f;

	bool m_limitPointFlag = false;

};