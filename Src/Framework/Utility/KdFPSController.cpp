#include "KdFPSController.h"

// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// FPSの制御コントローラー
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
void KdFPSController::Init()
{
	m_lastTime = std::chrono::high_resolution_clock::now();
	m_fpsMonitorBeginTime = timeGetTime();
}

void KdFPSController::UpdateStartTime()
{
	m_lastTime = std::chrono::high_resolution_clock::now();
}

void KdFPSController::Update()
{
	Control();

	Monitoring();
}

// FPS制御
void KdFPSController::Control()
{
	currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsedTime = currentTime - m_lastTime;
	m_unscaledDeltaTime = elapsedTime.count();
	m_deltaTime = m_unscaledDeltaTime * m_timeScale;
	m_lastTime = currentTime;
}

// 現在のFPS計測
void KdFPSController::Monitoring()
{
	static int fpsCount = 0;			// 現在のFPS
	static float timeElapsed = 0.0f;	// 経過時間

	fpsCount++;							// FPSカウント
	timeElapsed += m_deltaTime;			// 経過時間を加算

	if(timeElapsed >= 1.0f)				// 1秒経過したら
	{
		m_nowfps = fpsCount;			// FPSを保存
		fpsCount = 0;					// FPSカウントをリセット
		timeElapsed = 0.0f;				// 経過時間をリセット
	}
}
