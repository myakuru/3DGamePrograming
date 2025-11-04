#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class BossEnemyBlackBarUI : public SelectDraw2DTexture
{
public:

	static const uint32_t TypeID;

	BossEnemyBlackBarUI() { m_typeID = TypeID; }
	~BossEnemyBlackBarUI() override = default;

private:

	void Update() override;

};