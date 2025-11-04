#pragma once
#include"../Utility/SelectDraw3dModel.h"
class PlayerCamera;
class Collision :public SelectDraw3dModel
{
public:

	static const uint32_t TypeID;

	Collision();
	~Collision() override = default;

private:

	void Init() override;

	std::weak_ptr<PlayerCamera> m_playerCamera;

};