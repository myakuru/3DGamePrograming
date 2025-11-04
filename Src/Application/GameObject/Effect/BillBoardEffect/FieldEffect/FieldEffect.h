#pragma once
#include"../../../Utility/SelectDraw3dPolygon.h"
class PlayerCamera;
class FieldEffect : public SelectDraw3dPolygon
{
public:
	static const uint32_t TypeID;

	FieldEffect() { m_typeID = TypeID; }
	~FieldEffect() override = default;

private:

	void Init() override;
	void Update() override;
	void DrawBright() override;

	struct PolyInfo 
	{
		Math::Vector3 pos;
		Math::Vector3 scale;
	};
	std::vector<PolyInfo> m_polys;

	Math::Matrix m_finalMat = Math::Matrix::Identity; // 最終行列

	std::weak_ptr<PlayerCamera> m_camera;
};