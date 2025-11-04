#include"EnemyShineBlue.h"
#include "../../../Character/Enemy/Enemy.h"
#include"../../../../Scene/SceneManager.h"

const uint32_t EnemyShineBlue::TypeID = KdGameObject::GenerateTypeID();

void EnemyShineBlue::Update()
{
	KdEffekseerManager::GetInstance().Update();


	SceneManager::Instance().GetObjectWeakPtr(m_enemy);
	auto enemy = m_enemy.lock();
	if (!enemy) return;

	// プレイヤーの前方ベクトル
	Math::Vector3 forward = Math::Vector3::TransformNormal(Math::Vector3::Forward, Math::Matrix::CreateFromQuaternion(enemy->GetRotationQuaternion()));
	forward.Normalize();

	m_mWorld = Math::Matrix::CreateScale(m_scale);
	m_mWorld *= Math::Matrix::CreateFromYawPitchRoll
	(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z)
	) * Math::Matrix::CreateFromQuaternion(enemy->GetRotationQuaternion());

	m_mWorld.Translation(m_position + enemy->GetPos() + forward * m_distance);

	EffectUpdate();
}
