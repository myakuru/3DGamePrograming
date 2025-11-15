#include "CharacterBase.h"
#include"Application/main.h"
#include"Application/Scene/SceneManager.h"
#include"Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"
#include"Application/GameObject/Collition/Collition.h"
#include"Application/Scene/BaseScene/BaseScene.h"
#include"Application/Data/CharacterData/CharacterData.h"

void CharacterBase::Init()
{
	KdGameObject::Init();
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
	ModelLoad(m_path);

	m_characterData = std::make_shared<CharacterData>();
}

std::shared_ptr<KdModelWork> CharacterBase::GetAnimeModel()
{
	return m_modelWork;
}

std::shared_ptr<KdAnimator> CharacterBase::GetAnimator()
{
	return m_animator;
}

void CharacterBase::SetAnimeSpeed(float _speed)
{
	m_physics.fixedFrameRate = _speed;
}

std::weak_ptr<PlayerCamera> CharacterBase::GetPlayerCamera() const
{
	return m_refs.playerCamera;
}

void CharacterBase::UpdateQuaternion(Math::Vector3& _moveVector)
{
	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	if (_moveVector == Math::Vector3::Zero) return;

	SceneManager::Instance().GetObjectWeakPtr(m_refs.playerCamera);

	if (auto camera = m_refs.playerCamera.lock(); camera)
	{
		// カメラのY軸回転と移動ベクトルをかけ合わせて、WASDから入力された値に基づく方向を計算
		_moveVector = Math::Vector3::TransformNormal(_moveVector, camera->GetRotationYMatrix());
	}

	_moveVector.Normalize();

	// Yaw角を計算
	float targetYaw = atan2(-_moveVector.x, -_moveVector.z);

	// Yaw角からクォータニオンを生成
	Math::Quaternion targetRotation = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::Up, targetYaw);

	// クォータニオンの内積を計算
	m_rotation = Math::Quaternion::Slerp(m_rotation, targetRotation, deltaTime * m_movement.rotateSpeed);
}

void CharacterBase::UpdateQuaternionDirect(const Math::Vector3& direction)
{
	float deltaTime = Application::Instance().GetUnscaledDeltaTime();
	if (direction == Math::Vector3::Zero) return;

	Math::Vector3 dir = direction;
	dir.Normalize();

	float targetYaw = atan2(-dir.x, -dir.z);

	Math::Quaternion targetRotation = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::Up, targetYaw);
	
	m_rotation = Math::Quaternion::Slerp(m_rotation, targetRotation, deltaTime * m_movement.rotateSpeed);
}

void CharacterBase::Update()
{
	KdGameObject::Update();

	if (SceneManager::Instance().GetCurrentScene()->GetSceneName() == "Title") return;

	float deltaTime = Application::Instance().GetDeltaTime();

	m_animator->AdvanceTime(m_modelWork->WorkNodes(), m_physics.fixedFrameRate * deltaTime);
	m_modelWork->CalcNodeMatrices();

	// 重力更新
	m_physics.gravity += m_physics.gravitySpeed * deltaTime;

	// 水平
	m_position.x += m_movement.movement.x * m_movement.moveSpeed * m_physics.fixedFrameRate * deltaTime;
	m_position.z += m_movement.movement.z * m_movement.moveSpeed * m_physics.fixedFrameRate * deltaTime;
	// 垂直
	m_position.y += m_physics.gravity;

	m_stateManager.Update();

	Math::Matrix scale = Math::Matrix::CreateScale(m_scale);
	Math::Matrix quaternion = Math::Matrix::CreateFromQuaternion(m_rotation);
	Math::Matrix translation = Math::Matrix::CreateTranslation(m_position);
	m_mWorld = scale * quaternion * translation;
}

void CharacterBase::PostUpdate()
{
	// ====================================================
	// レイの当り判定::::::::::::::::::::ここから::::::::::::::
	// ====================================================

	// レイ判定用に必要なParameter
	KdCollider::RayInfo rayInfo;

	// レイの何処から発射するか
	rayInfo.m_pos = m_position;

	// 段差の許容範囲を設定
	static const float enableStepHigh = 0.2f;
	rayInfo.m_pos.y += enableStepHigh;			// 0.2f までの段差は登れる

	// レイの方向を設定
	rayInfo.m_dir = { 0.0f,-1.0f,0.0f };

	// レイの長さを設定
	rayInfo.m_range = enableStepHigh + std::fabs(m_physics.gravity);

	// アタリ判定したいタイプを設定
	rayInfo.m_type = KdCollider::TypeGround;

	m_pDebugWire->AddDebugLine(rayInfo.m_pos, rayInfo.m_dir, rayInfo.m_range);

	// レイに当たったオブジェクト情報を格納するリスト
	std::list<KdCollider::CollisionResult> retRayList;

	if (m_refs.collision.expired()) return;

	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::Collision, m_refs.referencedObjects);

	for (auto& collision : m_refs.referencedObjects)
	{
		if (auto collisionObj = collision.lock(); collisionObj)
		{
			collisionObj->Intersects(rayInfo, &retRayList);
			m_refs.referencedObjects.clear();
		}
	}

	// レイに当たったリストから一番近いオブジェクトを検出
	bool hit = false;
	float maxOverLap = 0;
	Math::Vector3 groundPos = {};	// レイが遮断された(Hitした)座標

	for (auto& ret : retRayList)
	{
		// レイが当たったオブジェクトの中から
		// 「m_overlapDistance = 貫通した長さ」が一番長いものを探す
		// 「m_overlapDistance が一番長い = 一番近くで当たった」と判定できる
		if (maxOverLap < ret.m_overlapDistance)
		{
			maxOverLap = ret.m_overlapDistance;
			groundPos = ret.m_hitPos;
			hit = true;
		}
	}

	// 当たっていたら
	if (hit)
	{
		m_physics.gravity = 0.0f;
		m_position.y = groundPos.y;
	}

	//=====================================================
	// レイ当り判定::::::::::::::::::::ここまで::::::::::::::::
	//=====================================================

	// 球判定
	// 球判定用の変数
	KdCollider::SphereInfo sphereInfo;
	// 球の中心座標を設定
	sphereInfo.m_sphere.Center = m_position + Math::Vector3(0.0f, 0.3f, 0.0f);
	// 球の半径を設定
	sphereInfo.m_sphere.Radius = 0.2f;
	// アタリ判定をしたいタイプを設定  
	sphereInfo.m_type = KdCollider::TypeBump;

	m_pDebugWire->AddDebugSphere(sphereInfo.m_sphere.Center, sphereInfo.m_sphere.Radius);

	// 球に当たったオブジェクト情報を格納するリスト
	std::list<KdCollider::CollisionResult> retSpherelist;

	for (auto& collision : m_refs.referencedObjects)
	{
		if (auto collisionObj = collision.lock(); collisionObj)
		{
			collisionObj->Intersects(sphereInfo, &retSpherelist);
			m_refs.referencedObjects.clear();
		}
	}

	//  球にあたったリストから一番近いオブジェクトを探す
	// オーバーした長さが1番長いものを探す。
	// 使いまわしの変数を使う
	maxOverLap = 0.0f;
	hit = false;
	// 当たった方向を格納する変数
	Math::Vector3 hitDir;

	for (auto& ret : retSpherelist)
	{
		// 球からはみ出た長さが１番長いものを探す。
		if (maxOverLap < ret.m_overlapDistance)
		{
			maxOverLap = ret.m_overlapDistance;
			hitDir = ret.m_hitDir;
			hit = true;
		}
	}

	if (hit)
	{
		// Y方向の押し出しを無効化（XZ平面のみ）
		hitDir.y = 0.0f;
		hitDir.Normalize();

		//当たってたらその方向から押し出す
		m_position += hitDir * maxOverLap;
	}
}

bool CharacterBase::ModelLoad(std::string _path)
{
	if (m_modelWork)
	{
		m_modelWork->SetModelData(_path);
		return true;
	}
	return false;
}

void CharacterBase::ImGuiInspector()
{
	SelectDraw3dModel::ImGuiInspector();
}

void CharacterBase::JsonInput(const nlohmann::json& _json)
{
	SelectDraw3dModel::JsonInput(_json);
}

void CharacterBase::JsonSave(nlohmann::json& _json) const
{
	SelectDraw3dModel::JsonSave(_json);
}

void CharacterBase::SetIsMoving(Math::Vector3 _move)
{
	m_movement.movement = _move;
}


const Math::Vector3& CharacterBase::GetMovement() const
{
	return m_movement.movement;
}

Math::Matrix& CharacterBase::GetRotationMatrix()
{
	return m_transform.rotationM;
}

void CharacterBase::SetRotation(const Math::Quaternion& _rotation)
{
	m_rotation = _rotation;
}

Math::Quaternion& CharacterBase::GetRotationQuaternion()
{
	return m_rotation;
}

void CharacterBase::SetPosition(const Math::Vector3& _position)
{
	m_position = _position;
}
