#include "Collition.h"
#include"../../Scene/SceneManager.h"
#include"../Camera/PlayerCamera/PlayerCamera.h"

const uint32_t Collision::TypeID = KdGameObject::GenerateTypeID();

Collision::Collision()
{
	m_typeID = TypeID;
	AddTag(ObjTag::Collision);
}

void Collision::Init()
{
	SelectDraw3dModel::Init();
	// 描画フラグを初期化
	InitDrawFlags();

	m_pCollider->RegisterCollisionShape("Mesh", std::make_unique<KdModelCollision>(m_model, KdCollider::TypeGround));

	m_pCollider->RegisterCollisionShape("Mesh", std::make_unique<KdModelCollision>(m_model, KdCollider::TypeBump));

	SceneManager::Instance().GetObjectWeakPtr(m_playerCamera);

	if (auto camera = m_playerCamera.lock(); camera)
	{
		camera->RegistHitObject(shared_from_this());
	}
}
