#pragma once
#include"../Utility/SelectDraw3dModel.h"
class SkySphere:public SelectDraw3dModel
{
public:
	static const uint32_t TypeID;
	SkySphere() { m_typeID = TypeID; };
	~SkySphere() override = default;

	uint32_t GetTypeID() const override { return m_typeID; }
	void Init() override;

};