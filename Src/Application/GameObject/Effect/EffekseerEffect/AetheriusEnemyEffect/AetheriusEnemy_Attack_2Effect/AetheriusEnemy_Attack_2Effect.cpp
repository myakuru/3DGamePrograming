#include "AetheriusEnemy_Attack_2Effect.h"
#include "Application/Scene/SceneManager.h"
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/AetheriusEnemy.h"

const uint32_t AetheriusEnemy_Attack_2Effect::TypeID = KdGameObject::GenerateTypeID();

void AetheriusEnemy_Attack_2Effect::Update()
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

void AetheriusEnemy_Attack_2Effect::EffectUpdate()
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

void AetheriusEnemy_Attack_2Effect::PlayForEnemy(const std::shared_ptr<AetheriusEnemy>& enemy)
{
	if (!enemy) return;
	// エネミーの前方ベクトル
	Math::Vector3 forward = Math::Vector3::TransformNormal(
		Math::Vector3::Forward,
		Math::Matrix::CreateFromQuaternion(enemy->GetRotationQuaternion()));

	Math::Vector3 right = Math::Vector3::TransformNormal(
		Math::Vector3::Right,
		Math::Matrix::CreateFromQuaternion(enemy->GetRotationQuaternion()));

	forward.Normalize();
	right.Normalize();

	// ワールド行列
	Math::Matrix mWorld = Math::Matrix::CreateScale(m_scale);
	mWorld *= Math::Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z))
		* Math::Matrix::CreateFromQuaternion(enemy->GetRotationQuaternion());
	mWorld.Translation(m_position + enemy->GetPos() + (forward * m_distance) + (right * m_sideDistance));
	// 毎回スポーン
	if (auto sp = KdEffekseerManager::GetInstance().Play(m_path, mWorld, m_effectSpeed, false, m_effectColor).lock())
	{
		m_effects.emplace_back(sp);
		m_isEffectPlaying = true;
	}
}