#include "CloudEffect.h"

const uint32_t CloudEffect::TypeID = KdGameObject::GenerateTypeID();

void CloudEffect::Init()
{
	EffekseerEffectBase::Init();
	m_once = true;
}

void CloudEffect::EffectUpdate()
{
	if(KeyboardManager::GetInstance().IsKeyJustPressed('R'))
	{
		m_once = true;
	}

	if (m_once)
	{
		KdEffekseerManager::GetInstance().Play(m_path, m_mWorld, m_effectSpeed, true);
		m_once = false;
	}
}

void CloudEffect::Update()
{
	m_mWorld = Math::Matrix::CreateScale(m_scale);
	m_mWorld *= Math::Matrix::CreateFromYawPitchRoll
	(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z)
	);

	m_mWorld.Translation(m_position);

	EffectUpdate();
}

void CloudEffect::DrawEffect()
{
	// Effekseerの描画
	KdShaderManager::Instance().ChangeRasterizerState(KdRasterizerState::CullFront);
	KdEffekseerManager::GetInstance().Draw();
	KdShaderManager::Instance().UndoRasterizerState();
}
