#pragma once
#include "../../../Utility/SelectDraw2DTexture.h"
class Player;
class NotChargeUI :public SelectDraw2DTexture
{
public:
	static const uint32_t TypeID;

	NotChargeUI() { m_typeID = TypeID; }
	~NotChargeUI() override = default;


private:

	void Init() override;
	void Update() override;
	void DrawSprite() override;

	std::weak_ptr<Player> m_player;

	std::shared_ptr<KdTexture> m_ChargeTexture;

	bool m_showUI = false;
};