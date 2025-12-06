#include "EnemyEffect.h"
#include "Application/Scene/SceneManager.h"

void EnemyEffect::Update()
{
	// エフェクサの更新
	KdEffekseerManager::GetInstance().Update();

	if (SceneManager::Instance().m_gameClear)
	{
		KdEffekseerManager::GetInstance().StopEffect(m_path);
		m_effects.clear();
		StopEffect();
	}

	// 再生済みのクリーンアップ
	EffectUpdate();
}

void EnemyEffect::EffectUpdate()
{
	m_isEffectPlaying = false;

	for (auto it = m_effects.begin(); it != m_effects.end();)
	{
		if (auto effect = it->lock())
		{
			const bool playing = effect->IsPlaying();
			m_isEffectPlaying = m_isEffectPlaying || playing;

			if (!playing) it = m_effects.erase(it);
			else          ++it;
		}
		else
		{
			it = m_effects.erase(it);
		}
	}
}
