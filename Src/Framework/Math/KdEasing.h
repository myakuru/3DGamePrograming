#pragma once

struct KdEase
{
	inline float InSine(float progress);
	inline float OutSine(float progress);
	inline float InOutSine(float progress);

	inline float OutBounce(float progress);
};
