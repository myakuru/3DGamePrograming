#include "EnemyBase.h"
#include "../Player/Player.h"
#include "Application/main.h"
#include "Application/Scene/SceneManager.h"
#include "Application/Data/CharacterData/CharacterData.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"
#include "Application/GameObject/Collition/Collition.h"

void EnemyBase::Init()
{
	CharacterBase::Init();

	m_movement.rotateSpeed = 10.0f;

	m_animator->SetAnimation(m_modelWork->GetData()->GetAnimation("Idle"));

	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape("EnemySphere", m_sphere, KdCollider::TypeDamage);
	m_pCollider->RegisterCollisionShape("PlayerSphere", m_sphere, KdCollider::TypeEnemyHit);

	m_action.isAttack = false;
	m_visual.enableRadialBlur = false;

	SetInvincible(false);

	ResetAttackCollision();
}

void EnemyBase::DrawLit()
{
	// ディゾルブ適用
	KdShaderManager::Instance().m_StandardShader.SetDissolve(
		/* 外部進行度(旧 m_dissever が別所にあるなら適宜差し替え) */ m_rendering.dissolvePower,
		&m_rendering.dissolvePower,
		&m_rendering.dissolveColor);
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
	auto wnd = AttackWindow();
	wnd.elapsed += deltaTime;

	// 有効開始前
	if (wnd.elapsed < wnd.begin) return;

	// Just回避成立後はこの攻撃終了
	if (m_avoid.justSuccess)
	{
		m_charge.active = false;
		m_avoid.justSuccess = false; // 次回へ備える
		return;
	}

	// ウィンドウ終了超過
	if (wnd.elapsed > wnd.end)
	{
		m_charge.active = false;
		return;
	}

	// 対象探索（ブロードフェーズ余白）
	constexpr float kBroadPhaseMargin = 0.5f;
	const float searchRadius = attackSphere.m_sphere.Radius + kBroadPhaseMargin;
	(void)searchRadius; // 必要なら距離利用で絞り込み
	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::PlayerLike, m_refs.playerObjects);

	// Just回避チェック
	for (const auto& wPlayer : m_refs.playerObjects)
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
		for (const auto& wPlayer : m_refs.playerObjects)
		{
			if (auto player = wPlayer.lock())
			{
				std::list<KdCollider::CollisionResult> results;
				if (player->Intersects(attackSphere, &results) && !results.empty())
				{
					player->TakeDamage(m_characterData->GetCharacterData().attack);
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

	if (m_refs.collision.expired()) return;
	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::Collision, m_refs.collisionObjects);

	for (auto& wCol : m_refs.collisionObjects)
	{
		if (auto col = wCol.lock())
		{
			col->Intersects(sphereInfo, &retSpherelist);
		}
	}

	float maxOverLap = 0.0f;
	bool hit = false;
	Math::Vector3 hitDir;

	for (auto& ret : retSpherelist)
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
		hitDir.Normalize();
		hitDir.y = 0.0f;
		hitDir.Normalize();
		m_position += hitDir * maxOverLap;
	}
}

void EnemyBase::ImGuiInspector()
{
	CharacterBase::ImGuiInspector();

	ImGui::Text(U8("移動 / 物理"));
	ImGui::DragFloat(U8("回転速度"), &m_movement.rotateSpeed, 0.1f);
	ImGui::DragFloat(U8("移動速度"), &m_movement.moveSpeed, 0.1f);
	ImGui::DragFloat(U8("重力速度"), &m_physics.gravitySpeed, 0.01f);
	ImGui::DragFloat(U8("再生速度 (fixedFrameRate)"), &m_physics.fixedFrameRate, 0.01f);

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
	ImGui::ColorEdit3(U8("ディゾルブカラー"), &m_rendering.dissolveColor.x);
	ImGui::DragFloat(U8("ディゾルブ進行度"), &m_rendering.dissolvePower, 0.01f, 0.0f, 1.0f);
}

void EnemyBase::JsonInput(const nlohmann::json& _json)
{
	CharacterBase::JsonInput(_json);
	if (_json.contains("GravitySpeed"))     m_physics.gravitySpeed = _json["GravitySpeed"].get<float>();
	if (_json.contains("fixedFps"))         m_physics.fixedFrameRate = _json["fixedFps"].get<float>();
	if (_json.contains("moveSpeed"))        m_movement.moveSpeed = _json["moveSpeed"].get<float>();
	if (_json.contains("rotationspeed"))    m_movement.rotateSpeed = _json["rotationspeed"].get<float>();
	if (_json.contains("dissolveColor"))    m_rendering.dissolveColor = JSON_MANAGER.JsonToVector(_json["dissolveColor"]);
	if (_json.contains("dissolvePower"))    m_rendering.dissolvePower = _json["dissolvePower"].get<float>();
}

void EnemyBase::JsonSave(nlohmann::json& _json) const
{
	CharacterBase::JsonSave(_json);
	_json["GravitySpeed"] = m_physics.gravitySpeed;
	_json["fixedFps"] = m_physics.fixedFrameRate;
	_json["moveSpeed"] = m_movement.moveSpeed;
	_json["rotationspeed"] = m_movement.rotateSpeed;
	_json["dissolveColor"] = JSON_MANAGER.VectorToJson(m_rendering.dissolveColor);
	_json["dissolvePower"] = m_rendering.dissolvePower;
}

void EnemyBase::UpdateQuaternion(Math::Vector3& _moveVector)
{
	float deltaTime = Application::Instance().GetUnscaledDeltaTime();
	if (_moveVector == Math::Vector3::Zero) return;

	_moveVector.Normalize();
	Math::Quaternion targetRotation = Math::Quaternion::LookRotation(_moveVector, Math::Vector3::Up);
	m_rotation = Math::Quaternion::Slerp(m_rotation, targetRotation, deltaTime * m_physics.fixedFrameRate);
}
