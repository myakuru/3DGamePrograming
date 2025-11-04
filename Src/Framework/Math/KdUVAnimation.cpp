#include "KdUVAnimation.h"

// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// KdUVAnimationData
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####

// アニメーションデータの読み込みcsvデータとして読み込む
// アニメーション名,開始フレーム番号,終了フレーム番号
void KdUVAnimationData::Load(std::string_view fileName)
{
	KdCSVData data;

	if (data.Load(fileName.data()))
	{
		for (int i = 0; ; ++i)
		{
			const std::vector<std::string>& anim = data.GetLine(i);

			if (!anim.size()) { break; }

			AddAnimation(anim[0], atoi(anim[1].c_str()), atoi(anim[2].c_str()));
		}
	}
}

void KdUVAnimationData::AddAnimation(const std::string_view animName, const KdAnimationFrame& data)
{
	m_animations[animName.data()] = std::make_shared<KdAnimationFrame>(data);
}

void KdUVAnimationData::AddAnimation(const std::string_view animName, int start, int end)
{
	m_animations[animName.data()] = std::make_shared<KdAnimationFrame>(start, end);
}

const std::shared_ptr<KdAnimationFrame> KdUVAnimationData::GetAnimation(std::string_view name)
{
	auto dataItr = m_animations.find(name.data());

	if ( dataItr == m_animations.end() ) { return nullptr; }

	return dataItr->second;
}

// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// UVAnimator
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####

// 再生したいアニメーションのセット
void KdUVAnimator::SetAnimation(const std::shared_ptr<KdAnimationFrame>& animData, bool loop, bool restart)
{
	if (!m_spNowAnimation)
	{
		m_spNowAnimation = animData;
	}

	// アニメーションの進捗を初期化しない
	if (!restart)
	{
		float nowProgress = m_nowAnimPos - m_spNowAnimation->m_startFrame;

		m_nowAnimPos = std::min(animData->m_startFrame + nowProgress, static_cast<float>(animData->m_endFrame));
	}
	else
	{
		m_nowAnimPos = static_cast<float>(animData->m_startFrame);
	}

	m_spNowAnimation = animData;

	m_loopAnimation = loop;
}

// アニメーションを進行させる
void KdUVAnimator::AdvanceTime(float speed)
{
	if (!m_spNowAnimation) { return; }

	// アニメーション位置を進める
	m_nowAnimPos += speed;

	// 終了判定
	if (IsAnimationEnd())
	{
		if (m_loopAnimation)
		{
			m_nowAnimPos = static_cast<float>(m_spNowAnimation->m_startFrame);
		}
		else
		{
			// 最後のコマにする
			m_nowAnimPos = static_cast<float>(m_spNowAnimation->m_endFrame) - 0.001f;
		}
	}
}

// アニメーションが終了しているかどうか
bool KdUVAnimator::IsAnimationEnd() const
{
	if (!m_spNowAnimation) { return true; }

	// 終了判定
	if (m_nowAnimPos >= m_spNowAnimation->m_endFrame - 0.001f) { return true; }

	return false;
}
