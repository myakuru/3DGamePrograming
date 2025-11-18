#include"EnemyHitEffect.h"
#include"Application/Scene/SceneManager.h"
#include"Application/GameObject/Character/EnemyBase/AetheriusEnemy/AetheriusEnemy.h"

const uint32_t EnemyHitEffect::TypeID = KdGameObject::GenerateTypeID();

void EnemyHitEffect::Init()
{
	EffekseerEffectBase::Init();
}

void EnemyHitEffect::Update()
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

void EnemyHitEffect::EffectUpdate()
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

void EnemyHitEffect::PlayForEnemy(const std::shared_ptr<AetheriusEnemy>& enemy)
{
	if (!enemy) return;

	// エネミーの前方ベクトル
	Math::Vector3 forward = Math::Vector3::TransformNormal(
		Math::Vector3::Forward,
		Math::Matrix::CreateFromQuaternion(enemy->GetRotationQuaternion()));
	forward.Normalize();

	// ワールド行列
	Math::Matrix mWorld = Math::Matrix::CreateScale(m_scale);
	mWorld *= Math::Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z))
		* Math::Matrix::CreateFromQuaternion(enemy->GetRotationQuaternion());

	mWorld.Translation(m_position + enemy->GetPos() + forward * m_distance);

	// 毎回スポーン
	if (auto sp = KdEffekseerManager::GetInstance().Play(m_path, mWorld, m_effectSpeed, false, m_effectColor).lock())
	{
		m_effects.emplace_back(sp);
		m_isEffectPlaying = true;
	}
}