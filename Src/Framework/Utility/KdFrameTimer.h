#pragma once

struct KdFrameTimer
{
	void Update(float speedRate = 1.0f)
	{
		m_nowTime = std::clamp(m_nowTime + speedRate, 0.0f, m_endTime);
	}

	void SetFrame(float endFrame)
	{
		m_nowTime = 0.0f;
		m_endTime = endFrame;
	}

	void SetSec(float endSec);

	void Restart() { m_nowTime = 0.0f; }

	bool IsEnd() { return (m_nowTime >= m_endTime); }
	float GetProgress() { return (m_endTime <= 0.0f) ? 1.0f : (m_nowTime / m_endTime); }

	float GetNowFrame() const { return m_nowTime; }

private:
	float m_nowTime = 0.0f;		// 経過フレーム数 ゲームがスローになったり巻き戻りの可能性も考えて浮動小数点を採用
	float m_endTime = 0.0f;		// 終了フレーム数
};