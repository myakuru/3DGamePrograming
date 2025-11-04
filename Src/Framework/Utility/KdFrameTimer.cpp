#include "KdFrameTimer.h"

void KdFrameTimer::SetSec(float endSec)
{
	m_nowTime = 0.0f;
	m_endTime = 60.0f * endSec;
}