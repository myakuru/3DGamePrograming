#include "KdEasing.h"

inline float KdEase::InSine(float progress)
{
	return 1 - cos((progress * DirectX::XM_PI) * 0.5f);
}

inline float KdEase::OutSine(float progress)
{
	return sin((progress * DirectX::XM_PI) * 0.5f);
}

inline float KdEase::InOutSine(float progress)
{
	return -(cos(DirectX::XM_PI * progress) - 1.0f) * 0.5f;
}

inline float KdEase::OutBounce(float progress)
{
	const float n1 = 7.5625f;
	const float d1 = 2.75f;

	if (progress < 1.0f / d1)
	{
		return n1 * progress * progress;
	}
	else if (progress < 2.0f / d1)
	{
		return n1 * (progress -= 1.5f / d1) * progress + 0.75f;
	}
	else if (progress < 2.5f / d1)
	{
		return n1 * (progress -= 2.25f / d1) * progress + 0.9375f;
	}
	else
	{
		return n1 * (progress -= 2.625f / d1) * progress + 0.984375f;
	}
}
