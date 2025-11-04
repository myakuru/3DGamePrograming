#pragma once

// UVアニメーションのフレーム情報
struct KdAnimationFrame
{
	KdAnimationFrame(int start, int end) :
		m_startFrame(start),
		// 最後のコマを表示するため+1
		// 終了フレームになった瞬間終了判定になってしまっては、最後のコマが描画される時間が無くなる
		m_endFrame(end + 1) {}

	int m_startFrame = 0;
	int m_endFrame = 0;
};

struct KdUVAnimationData
{
public:

	void Load(std::string_view);

	void AddAnimation(const std::string_view animName, const KdAnimationFrame& data);

	void AddAnimation(const std::string_view animName, int start, int end);

	const std::shared_ptr<KdAnimationFrame> GetAnimation(std::string_view name);

private:

	std::unordered_map<std::string, std::shared_ptr<KdAnimationFrame>> m_animations;
};

class KdUVAnimator
{
public:

	void SetAnimation(const std::shared_ptr<KdAnimationFrame>& animData, bool isLoop = true, bool restart = true);

	// コマアニメーションを進行させる
	// ・speed		… 進行速度 1.0で1フレーム1コマ
	void AdvanceTime(float speed);

	int GetFrame() { return static_cast<int>(m_nowAnimPos); }

	// アニメーションの再生が終わった？
	bool IsAnimationEnd() const;

private:

	float	m_nowAnimPos = 0;	// 現在のアニメーション位置

	std::shared_ptr<KdAnimationFrame> m_spNowAnimation = nullptr;

	bool m_loopAnimation = false;
};
