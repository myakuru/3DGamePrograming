#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class Player;
class NumBer :public SelectDraw2DTexture
{
public:
	static const uint32_t TypeID;

	NumBer() { m_typeID = TypeID; }
	~NumBer() override = default;


private:

	void Init() override;
	void Update() override;
	void DrawSprite() override;

	int m_displayTime = 0;
	bool m_isIncreasing = false;

	std::weak_ptr<Player> m_player;
};