#pragma once
#include "../CameraBase.h"

class FPSCamera : public CameraBase
{
public:

	static const uint32_t TypeID;

	FPSCamera()							{ m_typeID = TypeID; }
	~FPSCamera()			override	{}
	uint32_t GetTypeID() const override { return m_typeID; }

private:

	void Init()				override;
	//void ImGuiInspector()	override;
	void CameraUpdate()	override;
};