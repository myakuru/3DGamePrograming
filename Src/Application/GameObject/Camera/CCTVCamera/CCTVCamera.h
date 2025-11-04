#pragma once
#include "../CameraBase.h"

class CCTVCamera : public CameraBase
{
public:
	CCTVCamera()						{}
	~CCTVCamera()			override	{}

	void Init()				override;
	void PostUpdate()		override;

private:
	void UpdateLookAtRotate(const Math::Vector3& targetPos);

};