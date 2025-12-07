#include "EnemyBase.h"
#include "../Player/Player.h"
#include "Application/main.h"
#include "Application/Scene/SceneManager.h"
#include "Application/Data/CharacterData/CharacterData.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"
#include "Application/GameObject/Collition/Collition.h"
#include "Application/GameObject/Effect/EffekseerEffect/EnemyHitEffect/EnemyHitEffect.h"

void EnemyBase::Init()
{
	CharacterBase::Init();

	Movement().rotateSpeed = 10.0f;

	if (GetAnimator())
	{
		GetAnimator()->SetAnimation(m_modelWork->GetData()->GetAnimation("Idle"));
	}

	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape("EnemySphere", GetBoundingSphere(), KdCollider::TypeDamage);
	m_pCollider->RegisterCollisionShape("PlayerSphere", GetBoundingSphere(), KdCollider::TypeEnemyHit);
	m_pCollider->RegisterCollisionShape("EnemyToEnemySphere", GetBoundingSphere(), KdCollider::TypeEnemyToEnemyHit);


	m_action.isAttack = false;
	m_visual.enableRadialBlur = false;

	SetInvincible(false);

	ResetAttackCollision();
}

void EnemyBase::DrawLit()
{
	// ディゾルブ適用
	KdShaderManager::Instance().m_StandardShader.SetDissolve(
		Rendering().dissolvePower,
		&Rendering().dissolvePower,
		&Rendering().dissolveColor);
	SelectDraw3dModel::DrawLit();
}

void EnemyBase::ResetAttackCollision()
{
	// 連続攻撃状態初期化
	m_charge.count = 0;
	m_charge.timer = 0.0f;
	m_charge.active = false;
	m_charge.targetTotal = 0;
	m_charge.interval = 0.0f;

	m_action.attackSetupDone = false;
	m_avoid.justSuccess = false;

	// CharacterBase の攻撃ウィンドウを初期化
	auto wnd = AttackWindow();
	wnd.elapsed = 0.0f;
	wnd.begin = 0.0f;
	wnd.end = 3.0f;
}

void EnemyBase::UpdateAttackCollision(float _radius, float _distance,
	int _attackCount, float _attackInterval,
	float _activeBeginSec, float _activeEndSec)
{
	Math::Vector3 forward = Math::Vector3::TransformNormal(
		Math::Vector3::Forward,
		Math::Matrix::CreateFromQuaternion(m_rotation));
	forward.Normalize();

	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	KdCollider::SphereInfo attackSphere;
	attackSphere.m_sphere.Center = m_position + Math::Vector3(0.0f, 0.5f, 0.0f) + forward * _distance;
	attackSphere.m_sphere.Radius = _radius;
	attackSphere.m_type = KdCollider::TypeDamage;
	m_pDebugWire->AddDebugSphere(attackSphere.m_sphere.Center, attackSphere.m_sphere.Radius);

	// 初回セットアップ
	if (!m_action.attackSetupDone)
	{
		m_action.attackSetupDone = true;

		m_charge.active = true;
		m_charge.count = 0;
		m_charge.timer = 0.0f;
		m_charge.targetTotal = _attackCount;
		m_charge.interval = _attackInterval;

		m_avoid.justSuccess = false;

		auto wnd = AttackWindow();
		if (_activeBeginSec > _activeEndSec) std::swap(_activeBeginSec, _activeEndSec);
		wnd.elapsed = 0.0f;
		wnd.begin = _activeBeginSec;
		wnd.end = _activeEndSec;
	}

	if (!m_charge.active) return;

	// 攻撃ウィンドウ経過
	{
		auto wnd = AttackWindow();
		wnd.elapsed += deltaTime;

		// 有効開始前
		if (wnd.elapsed < wnd.begin) return;

		// ウィンドウ終了超過
		if (wnd.elapsed > wnd.end)
		{
			m_charge.active = false;
			return;
		}
	}

	// 対象探索
	SceneManager::Instance().GetObjectWeakPtrByTag(ObjTag::PlayerLike, Refs().playerObjects);

	// Just回避チェック
	for (const auto& wPlayer : Refs().playerObjects)
	{
		if (auto player = wPlayer.lock())
		{
			std::list<KdCollider::CollisionResult> results;
			if (player->Intersects(attackSphere, &results) && !results.empty())
			{
				if (player->GetAvoidFlg())
				{
					const float kJustAvoidWindowSec = 0.5f;
					const float avoidElapsed = player->GetAvoidStartTime();
					if (avoidElapsed >= 0.0f && avoidElapsed <= kJustAvoidWindowSec)
					{
						m_avoid.justSuccess = true;
						player->SetJustAvoidSuccess(true);

						Application::Instance().SetFpsScale(0.1f);
						SceneManager::Instance().SetDrawGrayScale(true);

						m_charge.active = false;
						return;
					}
				}
			}
		}
	}

	// 多段ヒットインターバル
	m_charge.timer += deltaTime;
	if (m_charge.count < m_charge.targetTotal && m_charge.timer >= m_charge.interval)
	{
		for (const auto& wPlayer : Refs().playerObjects)
		{
			if (auto player = wPlayer.lock())
			{
				std::list<KdCollider::CollisionResult> results;
				if (player->Intersects(attackSphere, &results) && !results.empty())
				{
					player->TakeDamage(GetCharacterData()->GetCharacterData().attack);
					player->SetHitCheck(true);
				}
			}
		}

		m_charge.count++;
		m_charge.timer = 0.0f;

		if (m_charge.count >= m_charge.targetTotal)
		{
			m_charge.active = false;
		}
	}
}

void EnemyBase::PostUpdate()
{
	// 既存の押し出し処理（球判定）は CharacterBase::PostUpdate に類似実装あり。
	// Enemy特有の高さ/Yオフセットが違うため、ここはそのまま維持。
	KdCollider::SphereInfo sphereInfo;
	sphereInfo.m_sphere.Center = m_position + Math::Vector3(0.0f, 0.5f, 0.0f);
	sphereInfo.m_sphere.Radius = 0.2f;
	sphereInfo.m_type = KdCollider::TypeBump;

	m_pDebugWire->AddDebugSphere(sphereInfo.m_sphere.Center, sphereInfo.m_sphere.Radius);

	std::list<KdCollider::CollisionResult> retSpherelist;

	SceneManager::Instance().GetObjectWeakPtrByTag(ObjTag::Collision, Refs().collisionObjects);

	for (const auto& wCol : Refs().collisionObjects)
	{
		if (auto col = wCol.lock())
		{
			col->Intersects(sphereInfo, &retSpherelist);
		}
	}

	float maxOverLap = 0.0f;
	bool hit = false;
	Math::Vector3 hitDir;

	for (const auto& ret : retSpherelist)
	{
		if (maxOverLap < ret.m_overlapDistance)
		{
			maxOverLap = ret.m_overlapDistance;
			hitDir = ret.m_hitDir;
			hit = true;
		}
	}

	if (hit)
	{
		hitDir.y = 0.0f;
		if (hitDir.LengthSquared() > 0) hitDir.Normalize();

		const Math::Vector3 push = hitDir * (maxOverLap * 0.5f);
		ApplyPushWithCollision(push); // 壁を貫通しないようスイープして移動
	}


	EnemytoEnemyCollision();
}

void EnemyBase::ImGuiInspector()
{
	CharacterBase::ImGuiInspector();

	ImGui::Text(U8("移動 / 物理"));
	ImGui::DragFloat(U8("回転速度"), &Movement().rotateSpeed, 0.1f);
	ImGui::DragFloat(U8("移動速度"), &Movement().moveSpeed, 0.1f);
	ImGui::DragFloat(U8("重力速度"), &Physics().gravitySpeed, 0.01f);
	ImGui::DragFloat(U8("再生速度 (fixedFrameRate)"), &Physics().fixedFrameRate, 0.01f);

	ImGui::Separator();
	ImGui::Text(U8("攻撃ウィンドウ"));
	{
		auto wnd = AttackWindow();
		ImGui::DragFloat(U8("経過時間"), &wnd.elapsed, 0.01f);
		ImGui::DragFloat(U8("開始秒"), &wnd.begin, 0.01f);
		ImGui::DragFloat(U8("終了秒"), &wnd.end, 0.01f);
	}

	ImGui::Separator();
	ImGui::Text(U8("ChargeState"));
	ImGui::DragInt(U8("ヒット数"), &m_charge.count);
	ImGui::DragInt(U8("目標ヒット数"), &m_charge.targetTotal);
	ImGui::DragFloat(U8("インターバル"), &m_charge.interval, 0.01f);
	ImGui::Checkbox(U8("アクティブ"), &m_charge.active);

	ImGui::Separator();
	ImGui::Text(U8("Avoid / Flags"));
	ImGui::Checkbox(U8("Just回避成功"), &m_avoid.justSuccess);
	ImGui::Checkbox(U8("攻撃初期化済"), &m_action.attackSetupDone);
	ImGui::Checkbox(U8("プレイヤーへ攻撃中"), &m_action.isAttack);

	ImGui::Separator();
	ImGui::Text(U8("ビジュアル"));
	ImGui::Checkbox(U8("ラジアルブラー有効"), &m_visual.enableRadialBlur);
	ImGui::DragFloat(U8("ブラー時間"), &m_visual.blurTime, 0.01f);

	ImGui::Separator();
	ImGui::ColorEdit3(U8("ディゾルブカラー"), &Rendering().dissolveColor.x);
	ImGui::DragFloat(U8("ディゾルブ進行度"), &Rendering().dissolvePower, 0.01f, 0.0f, 1.0f);
}

void EnemyBase::JsonInput(const nlohmann::json& _json)
{
	CharacterBase::JsonInput(_json);
	if (_json.contains("GravitySpeed"))     Physics().gravitySpeed = _json["GravitySpeed"].get<float>();
	if (_json.contains("fixedFps"))         Physics().fixedFrameRate = _json["fixedFps"].get<float>();
	if (_json.contains("moveSpeed"))        Movement().moveSpeed = _json["moveSpeed"].get<float>();
	if (_json.contains("rotationspeed"))    Movement().rotateSpeed = _json["rotationspeed"].get<float>();
	if (_json.contains("dissolveColor"))    Rendering().dissolveColor = JSON_MANAGER.JsonToVector(_json["dissolveColor"]);
	if (_json.contains("dissolvePower"))    Rendering().dissolvePower = _json["dissolvePower"].get<float>();
}

void EnemyBase::JsonSave(nlohmann::json& _json) const
{
	CharacterBase::JsonSave(_json);
	_json["GravitySpeed"] = Physics().gravitySpeed;
	_json["fixedFps"] = Physics().fixedFrameRate;
	_json["moveSpeed"] = Movement().moveSpeed;
	_json["rotationspeed"] = Movement().rotateSpeed;
	_json["dissolveColor"] = JSON_MANAGER.VectorToJson(Rendering().dissolveColor);
	_json["dissolvePower"] = Rendering().dissolvePower;
}

void EnemyBase::UpdateQuaternion(Math::Vector3& _moveVector)
{
	float deltaTime = Application::Instance().GetUnscaledDeltaTime();
	if (_moveVector == Math::Vector3::Zero) return;

	_moveVector.Normalize();
	Math::Quaternion targetRotation = Math::Quaternion::LookRotation(_moveVector, Math::Vector3::Up);
	m_rotation = Math::Quaternion::Slerp(m_rotation, targetRotation, deltaTime * Physics().fixedFrameRate);
}

void EnemyBase::SearchHitEffect()
{
	SceneManager::Instance().GetObjectWeakPtr(m_hitEffect);
}

void EnemyBase::EnemytoEnemyCollision()
{
	KdCollider::SphereInfo sphereInfo;
	sphereInfo.m_sphere.Center = m_position + Math::Vector3(0.0f, 0.5f, 0.0f);
	sphereInfo.m_sphere.Radius = 0.2f;
	sphereInfo.m_type = KdCollider::TypeEnemyToEnemyHit;

	m_pDebugWire->AddDebugSphere(sphereInfo.m_sphere.Center, sphereInfo.m_sphere.Radius);

	std::list<KdCollider::CollisionResult> retSpherelist;

	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemyLike, Refs().enemyList);

	for (const auto& eCol : Refs().enemyList)
	{
		if (auto col = eCol.lock())
		{
			// 自分自身を除外
			if (col.get() == this) { continue; }

			col->Intersects(sphereInfo, &retSpherelist);
		}
	}

	float maxOverLap = 0.0f;
	bool hit = false;
	Math::Vector3 hitDir;

	for (const auto& ret : retSpherelist)
	{
		if (maxOverLap < ret.m_overlapDistance)
		{
			maxOverLap = ret.m_overlapDistance;
			hitDir = ret.m_hitDir;
			hit = true;
		}
	}

	if (hit)
	{
		hitDir.y = 0.0f;
		if (hitDir.LengthSquared() > 0) hitDir.Normalize();

		const Math::Vector3 push = hitDir * (maxOverLap * 0.5f);
		m_position += push;
	}
}
