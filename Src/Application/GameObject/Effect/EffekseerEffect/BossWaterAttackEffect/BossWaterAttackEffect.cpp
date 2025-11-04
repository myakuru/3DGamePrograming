#include "BossWaterAttackEffect.h"
#include"../../../../Scene/SceneManager.h"
#include"../../../Character/BossEnemy/BossEnemy.h"

const uint32_t BossWaterAttackEffect::TypeID = KdGameObject::GenerateTypeID();

void BossWaterAttackEffect::Update()
{
	KdEffekseerManager::GetInstance().Update();

	SceneManager::Instance().GetObjectWeakPtr(m_boss);

	auto boss = m_boss.lock();
	if (!boss) return;

	// プレイヤーの前方ベクトル
	Math::Vector3 forward = Math::Vector3::TransformNormal(Math::Vector3::Forward, Math::Matrix::CreateFromQuaternion(boss->GetRotationQuaternion()));
	forward.Normalize();

	m_mWorld = Math::Matrix::CreateScale(m_scale);
	m_mWorld *= Math::Matrix::CreateFromYawPitchRoll
	(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z)
	) * Math::Matrix::CreateFromQuaternion(boss->GetRotationQuaternion());

	m_mWorld.Translation(m_position + boss->GetPos() + forward * m_distance);

	EffectUpdate();
}
