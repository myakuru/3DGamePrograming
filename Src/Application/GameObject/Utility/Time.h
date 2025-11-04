#pragma once
#include <chrono>

class Time : public KdGameObject
{
public:
	// シングルトンインスタンスを取得
	static Time& Instance()
	{
		static Time instance;
		return instance;
	}

	// 現在の経過秒数を取得
	float GetElapsedTime() const
	{
		return m_elapsedTime;
	}

	// タイマーをリセット
	void Reset()
	{
		m_startTime = std::chrono::steady_clock::now();
		m_elapsedTime = 0.0f;
		m_isResetDone = true; // リセットが実行されたことを記録

		// スタートタイマー・カウントダウンもリセット
		m_startTimerActive = false;
		m_countdownActive = false;
	}

	// リセットされたかどうかを取得
	bool WasReset() const
	{
		return m_isResetDone;
	}

	// リセットフラグをクリア
	void ClearResetFlag()
	{
		m_isResetDone = false;
	}

	// 更新処理 (毎フレーム呼び出す)
	void Update() override
	{
		auto currentTime = std::chrono::steady_clock::now();
		float elapsedTime = std::chrono::duration<float>(currentTime - m_startTime).count();

		// 秒単位で計算
		m_elapsedTime = elapsedTime;

		// スタートタイマー処理
		if (m_startTimerActive)
		{
			m_startTimerElapsed = elapsedTime - m_startTimerBegin;
		}

		// カウントダウン処理
		if (m_countdownActive)
		{
			m_countdownTimeLeft = m_countdownDuration - (elapsedTime - m_countdownBegin);
			if (m_countdownTimeLeft <= 0.0f)
			{
				m_countdownTimeLeft = 0.0f;
				m_countdownFinished = true;
				m_countdownActive = false;
			}
		}
	}

	// スタートタイマー開始
	void StartTimer()
	{
		m_startTimerBegin = m_elapsedTime;
		m_startTimerElapsed = 0.0f;
		m_startTimerActive = true;
	}

	// カウントダウン開始
	void StartCountdown(float duration)
	{
		m_countdownDuration = duration;
		m_countdownBegin = m_elapsedTime;
		m_countdownTimeLeft = duration;
		m_countdownFinished = false;
		m_countdownActive = true;
	}

	// カウントダウン残り時間取得
	float GetCountdownTimeLeft() const
	{
		return m_countdownTimeLeft;
	}

	// カウントダウンが終了したか
	bool IsCountdownFinished() const
	{
		return m_countdownFinished;
	}

private:
	// コンストラクタ
	Time() : m_startTime(std::chrono::steady_clock::now()), m_elapsedTime(0.0f), m_isResetDone(false),
		m_startTimerActive(false), m_startTimerBegin(0.0f), m_startTimerElapsed(0.0f),
		m_countdownActive(false), m_countdownFinished(false), m_countdownDuration(0.0f), m_countdownBegin(0.0f), m_countdownTimeLeft(0.0f)
	{
	}

	// コピーコンストラクタと代入演算子を削除
	Time(const Time&) = delete;
	Time& operator=(const Time&) = delete;

	// 開始時間
	std::chrono::steady_clock::time_point m_startTime;

	// 経過時間
	float m_elapsedTime;

	// リセットが実行されたかどうかを記録するフラグ
	bool m_isResetDone;

	// スタートタイマー用
	bool  m_startTimerActive;
	float m_startTimerBegin;
	float m_startTimerElapsed;

	// カウントダウン用
	bool  m_countdownActive;
	bool  m_countdownFinished;
	float m_countdownDuration;
	float m_countdownBegin;
	float m_countdownTimeLeft;
};