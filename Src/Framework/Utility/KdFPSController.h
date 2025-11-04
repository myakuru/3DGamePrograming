#pragma once

//============================================================
// アプリケーションのFPS制御 + 測定
//============================================================
struct KdFPSController
{
	// FPS制御
	int		m_nowfps = 0;		// 現在のFPS値
	int		m_maxFps = 60;		// 最大FPS
	float	m_deltaTime = 0.0f;	// フレーム間の時間差（秒）
	float	m_unscaledDeltaTime = 0.0f;	// フレーム間の時間差（秒）(時間スケールを無視した値)
	float	m_timeScale = 1.0f;	// 時間スケール（1.0f = 通常、0.5f = 半分の速さ）

	void Init();

	void UpdateStartTime();

	void Update();

	void Control();

private:

	void Monitoring();

	DWORD		m_frameStartTime = 0;		// フレームの開始時間

	int			m_fpsCnt = 0;				// FPS計測用カウント
	DWORD		m_fpsMonitorBeginTime = 0;	// FPS計測開始時間
	std::chrono::high_resolution_clock::time_point m_lastTime;
	std::chrono::steady_clock::time_point currentTime;

	const int	kSecond = 1000;				// １秒のミリ秒
};
