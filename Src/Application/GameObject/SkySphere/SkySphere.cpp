#include "SkySphere.h"

// Idの初期化
const uint32_t SkySphere::TypeID = KdGameObject::GenerateTypeID();

void SkySphere::Init()
{
	KdGameObject::Init();
	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape("ModelMesh", m_model, KdCollider::TypeEvent);
}
