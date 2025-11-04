#pragma once
#include"../../../Utility/SelectDraw3dPolygon.h"
class PlayerCamera;
class Player;
class MapGard : public SelectDraw3dPolygon
{
public:
	static const uint32_t TypeID;

	MapGard() { m_typeID = TypeID; }
	~MapGard() override = default;

private:

	void Init() override;
	void Update() override;
	void DrawToon() override;

	std::weak_ptr<PlayerCamera> m_camera;
	std::weak_ptr<Player> m_player;

	float m_playerDistance = 0.0f;

	Math::Vector3 m_offset = Math::Vector3::Zero;

};