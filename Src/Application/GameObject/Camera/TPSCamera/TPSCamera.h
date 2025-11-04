#pragma once
#include "../CameraBase.h"

class TPSCamera : public CameraBase
{
public:

	static const uint32_t TypeID;

	TPSCamera()							{ m_typeID = TypeID; }
	~TPSCamera()			override = default;

	uint32_t GetTypeID() const override { return m_typeID; }

	void Init()				override;
	void PostUpdate()		override;
	void UpdateResultCamera();

	Math::Matrix GetMatrix() const
	{
		return m_mWorld;
	}

	bool m_resultInitialized = false;
	// Result 固定用
	Math::Vector3 m_resultStartPos{};
	Math::Matrix  m_resultStartRot{};

};