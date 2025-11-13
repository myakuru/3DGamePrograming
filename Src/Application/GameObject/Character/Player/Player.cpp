#include "Player.h"
#include "Application/Scene/SceneManager.h"
#include "Application/Scene/BaseScene/BaseScene.h"
#include "Application/GameObject/Weapon/Katana/Katana.h"
#include "Application/GameObject/Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include "Application/main.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"
#include "PlayerState/PlayerState_Idle/PlayerState_Idle.h"
#include "PlayerState/PlayerState_Hit/PlayerState_Hit.h"
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/AetheriusEnemy.h"
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"
#include "Application/GameObject/Collition/Collition.h"
#include "Application/Data/CharacterData/CharacterData.h"
#include "Application\\GameObject\\Character\\AfterImage\\AfterImage.h"

const uint32_t Player::TypeID = KdGameObject::GenerateTypeID();

void Player::Init()
{
	CharacterBase::Init();

	m_animator->SetAnimation(m_modelWork->GetData()->GetAnimation("Idle"));
	m_modelWork->CalcNodeMatrices(); // ノードの再計算

	StateInit();

	// 当たり判定の設定
	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape("PlayerSphere", sphere, KdCollider::TypeDamage);
	m_pCollider->RegisterCollisionShape("PlayerSphere", sphere, KdCollider::TypeGround);

	// フラグ初期化（新しい構造体へ移行）
	m_action.onceEffect = false;
	m_action.isAtkPlayer = false;
	SetInvincible(false);
	SetHitCheck(false);

	// 残像初期化
	m_visual.afterImage = std::make_shared<AfterImage>();
	if (auto* src = GetModelWork())
	{
		m_visual.afterImage->SetAfterImageFrame().m_afterImageWork = std::make_unique<KdModelWork>(src->GetData());
	}

	// 角度からクォータニオン生成
	m_rotation = Math::Quaternion::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z));

	m_dissever = 0.0f;

	// 初期ステータス（暫定）
	m_characterData->SetCharacterData().hp = 1000;
	m_characterData->SetCharacterData().maxHp = 1000;
	m_characterData->SetCharacterData().attack = 0;

	m_visual.rimLightOn = false;
}

void Player::PreUpdate()
{
	// 残像のUpdate
	m_visual.afterImage->CaptureAfterImage(m_modelWork.get(), m_mWorld);

	// カタナの取得
	if (auto katana = m_katana.lock(); katana)
	{
		katana->SetPlayerMatrix(m_mWorld);
		katana->SetPlayerHandMatrix(m_mWorld);
	}

	if (auto scabbard = m_scabbard.lock(); scabbard)
	{
		scabbard->SetPlayerMatrix(m_mWorld);
		scabbard->SetPlayerHandMatrix(m_mWorld);
	}
}

void Player::PostUpdate()
{
	// ライトの影の中心位置をプレイヤーに合わせる
	auto& amb = KdShaderManager::Instance().WorkAmbientController();
	amb.SetShadowCenter(m_position);

	if (m_action.isAtkPlayer) return;

	KdCollider::SphereInfo enemyHit;
	enemyHit.m_sphere.Center = m_position + Math::Vector3(0.0f, 0.5f, 0.0f);
	enemyHit.m_sphere.Radius = 0.2f;
	enemyHit.m_type = KdCollider::TypeEnemyHit; // 敵のアタリ判定
	m_pDebugWire->AddDebugSphere(enemyHit.m_sphere.Center, enemyHit.m_sphere.Radius);

	// 球に当たったオブジェクト情報を格納するリスト
	std::list<KdCollider::CollisionResult> retSpherelist;

	// 敵の取得（近傍のみ）
	{
		constexpr float kBroadPhaseMargin = 1.0f;
		const float searchRadius = enemyHit.m_sphere.Radius + kBroadPhaseMargin;
		SceneManager::Instance().GetObjectWeakPtrListByTagInSphere(ObjTag::EnemyLike, enemyHit.m_sphere.Center, searchRadius, m_enemyLike);
	}

	// 球と敵の当たり判定をチェック
	for (const auto& enemyWeakPtr : m_enemyLike)
	{
		if (auto obj = enemyWeakPtr.lock())
		{
			obj->Intersects(enemyHit, &retSpherelist);
		}
	}

	// 最も重なりの大きい衝突を採用
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
		hitDir.y = 0.0f;
		if (hitDir.LengthSquared() > 0) hitDir.Normalize();

		// 両者が等しく離れる想定 → プレイヤー側は半分だけ動く
		const Math::Vector3 push = hitDir * (maxOverLap * 0.5f);
		ApplyPushWithCollision(push); // 壁を貫通しないようスイープして移動
	}
}

void Player::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.SetDitherEnable(false);
	KdShaderManager::Instance().m_StandardShader.SetLitRimLight({ 0.1f, 1.0f, 1.0f }, m_visual.rimLightOn, 1.5f);
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_modelWork, m_mWorld, m_color);
	KdShaderManager::Instance().m_StandardShader.SetLitRimLight();
}

void Player::DrawRimLight()
{
	// 残像の描画
	m_visual.afterImage->DrawAfterImages();
}

void Player::Update()
{
	KdGameObject::Update();

	SceneManager::Instance().GetObjectWeakPtr(m_playerCamera);
	SceneManager::Instance().GetObjectWeakPtr(m_katana);
	SceneManager::Instance().GetObjectWeakPtr(m_scabbard);

	if (SceneManager::Instance().m_gameClear)
	{
		m_movement.movement = Math::Vector3::Zero;
	}

	// 近傍の敵をチェック
	std::list<std::weak_ptr<KdGameObject>> nearEnemies;
	{
		constexpr float kNearbyCheckRadius = 12.0f;
		SceneManager::Instance().GetObjectWeakPtrListByTagInSphere(ObjTag::EnemyLike, m_position, kNearbyCheckRadius, nearEnemies);
	}

	for (const auto& wk : nearEnemies)
	{
		auto obj = wk.lock();
		if (!obj) continue;

		if (obj->GetTypeID() == AetheriusEnemy::TypeID)
		{
			auto aetheriusEnemy = std::static_pointer_cast<AetheriusEnemy>(obj);
			if (aetheriusEnemy->GetJustAvoidSuccess()) SetHitCheck(false);
		}
		else if (obj->GetTypeID() == BossEnemy::TypeID)
		{
			auto bossEnemy = std::static_pointer_cast<BossEnemy>(obj);
			if (bossEnemy->GetJustAvoidSuccess()) SetHitCheck(false);
		}
	}

	if (GetInvincible())
	{
		SetHitCheck(false);
	}

	// ヒット処理
	if (GetHitCheck())
	{
		if (GetInvincible()) return; // 無敵状態ならヒットしない

		auto spDamageState = std::make_shared<PlayerState_Hit>();
		ChangeState(spDamageState);

		SetHitCheck(false);
		return;
	}

	m_stateManager.Update();

	// スキル・スペシャル使用可能判定
	{
		m_action.useSkill = (m_characterData->GetPlayerStatus().skillPoint >= 30.0);
		m_action.useSpecial = (m_characterData->SetPlayerStatus().specialPoint == m_characterData->GetPlayerStatus().specialPointMax);

		m_visual.rimLightOn = (m_characterData->GetPlayerStatus().chargeCount >= 3);
	}

	// 時間スケール（ジャスト回避中はアンスケール）
	if (m_avoid.justSuccess)
	{
		const float deltaTime = Application::Instance().GetUnscaledDeltaTime();
		m_animator->AdvanceTime(m_modelWork->WorkNodes(), m_fixedFrameRate * deltaTime);
		m_modelWork->CalcNodeMatrices();
		m_prevPosition = m_position;                 // 移動前位置を保存
		m_gravity += m_gravitySpeed * deltaTime;     // 重力更新
		ApplyHorizontalMove(m_movement.movement, deltaTime);
	}
	else
	{
		const float deltaTime = Application::Instance().GetDeltaTime();
		m_animator->AdvanceTime(m_modelWork->WorkNodes(), m_fixedFrameRate * deltaTime);
		m_modelWork->CalcNodeMatrices();
		m_prevPosition = m_position;
		m_gravity += m_gravitySpeed * deltaTime;
		ApplyHorizontalMove(m_movement.movement, deltaTime);
	}

	// 垂直移動
	ApplyVerticalMove(m_gravity);

	// ワールド行列
	const Math::Matrix scale = Math::Matrix::CreateScale(m_scale);
	const Math::Matrix quaternion = Math::Matrix::CreateFromQuaternion(m_rotation);
	const Math::Matrix translation = Math::Matrix::CreateTranslation(m_position);
	m_mWorld = scale * quaternion * translation;
}

void Player::UpdateAttackCollision(float _radius, float _distance, int _attackCount, float _attackTimer, Math::Vector2 _cameraShakePow, float _cameraTime, float _activeBeginSec, float _activeEndSec)
{
	Math::Vector3 forward = Math::Vector3::TransformNormal(Math::Vector3::Forward, Math::Matrix::CreateFromQuaternion(m_rotation));
	forward.Normalize();

	const float deltaTime = Application::Instance().GetDeltaTime();

	KdCollider::SphereInfo attackSphere;
	attackSphere.m_sphere.Center = m_position + Math::Vector3(0.0f, 0.5f, 0.0f) + forward * _distance;
	attackSphere.m_sphere.Radius = _radius;
	attackSphere.m_type = KdCollider::TypeDamage;

	m_pDebugWire->AddDebugSphere(attackSphere.m_sphere.Center, attackSphere.m_sphere.Radius);

	// 初回セットアップ
	if (!m_action.onceEffect)
	{
		m_charge.active = true;
		m_charge.count = 0;
		m_charge.timer = 0.0f;

		// クランプしない。開始 > 終了なら入れ替えのみ行う
		float begin = _activeBeginSec;
		float end = _activeEndSec;
		if (begin > end) { float t = begin; begin = end; end = t; }

		m_combat.attackWindow.elapsed = 0.0f;
		m_combat.attackWindow.begin = begin;
		m_combat.attackWindow.end = end;

		m_action.onceEffect = true;
	}

	if (!m_charge.active) return;

	// 攻撃ウィンドウ経過
	m_combat.attackWindow.elapsed += deltaTime;

	// 開始前はまだ当てない
	if (m_combat.attackWindow.elapsed < m_combat.attackWindow.begin) return;

	// 終了超過で攻撃終了
	if (m_combat.attackWindow.elapsed > m_combat.attackWindow.end)
	{
		m_charge.active = false;
		return;
	}

	// 多段ヒットのインターバル管理
	m_charge.timer += deltaTime;

	if (m_charge.count < _attackCount && m_charge.timer >= _attackTimer)
	{
		bool hitAny = false;

		// 近傍のみ（ブロードフェーズ）
		std::list<std::weak_ptr<KdGameObject>> nearEnemies;
		{
			constexpr float kBroadPhaseMargin = 0.5f;
			const float searchRadius = attackSphere.m_sphere.Radius + kBroadPhaseMargin;
			SceneManager::Instance().GetObjectWeakPtrListByTagInSphere(ObjTag::EnemyLike, attackSphere.m_sphere.Center, searchRadius, nearEnemies);
		}

		for (const auto& wk : nearEnemies)
		{
			auto obj = wk.lock();
			if (!obj) continue;

			std::list<KdCollider::CollisionResult> results;
			if (obj->Intersects(attackSphere, &results) && !results.empty())
			{
				if (obj->GetTypeID() == AetheriusEnemy::TypeID)
				{
					auto e = std::static_pointer_cast<AetheriusEnemy>(obj);
					e->Damage(m_characterData->GetCharacterData().attack);
					e->SetEnemyHit(true);
				}
				else if (obj->GetTypeID() == BossEnemy::TypeID)
				{
					auto b = std::static_pointer_cast<BossEnemy>(obj);
					b->Damage(m_characterData->GetCharacterData().attack);
					b->SetEnemyHit(true);
				}
				hitAny = true;
			}
		}

		if (hitAny)
		{
			if (auto camera = m_playerCamera.lock(); camera)
			{
				camera->StartShake(_cameraShakePow, _cameraTime);
			}

			// SkillPoint
			if (m_characterData->GetPlayerStatus().skillPoint <= 100)
			{
				m_characterData->SetPlayerStatus().skillPoint += _attackCount / 4;
			}

			// ChargeCount
			if (m_characterData->GetPlayerStatus().chargeCount < 3)
			{
				m_characterData->SetPlayerStatus().chargeCount++;
			}

			// SpecialPoint 飽和加算（絶対上限付き）
			constexpr int kAbsoluteSpecialMax = 3000;
			const int upper = std::min(m_characterData->GetPlayerStatus().specialPointMax, kAbsoluteSpecialMax);
			const int add = _attackCount * 20;
			m_characterData->SetPlayerStatus().specialPoint =
				std::min(m_characterData->GetPlayerStatus().specialPoint + add, upper);
		}

		m_charge.count++;
		m_charge.timer = 0.0f;

		if (m_charge.count >= _attackCount)
		{
			m_charge.active = false;
		}
	}
}

void Player::ImGuiInspector()
{
	CharacterBase::ImGuiInspector();

	ImGui::Text(U8("プレイヤーの設定"));
	ImGui::DragFloat(U8("重力の大きさ"), &m_gravitySpeed, 0.01f);
	ImGui::DragFloat(U8("フレームレート制限"), &m_fixedFrameRate, 1.f);

	ImGui::Text(U8("プレイヤーの状態"));
	ImGui::DragFloat(U8("移動速度"), &m_moveSpeed, 0.1f);

	ImGui::Text(U8("Attack１のカメラの揺れ"));
	ImGui::DragFloat2(U8("揺れの大きさ"), &m_cameraShake.power.x, 0.01f);

	ImGui::Text(U8("Attack1のカメラの揺れ時間"));
	ImGui::DragFloat(U8("揺れの時間"), &m_cameraShake.time, 0.01f);

	ImGui::Text(U8("プレイヤーの回転速度"));
	ImGui::DragFloat(U8("回転速度"), &m_rotateSpeed, 0.1f);

	ImGui::DragFloat3(U8("回転(Yaw Pitch Roll)"), &m_degree.x, 1.0f);

	ImGui::Separator();

	// クォータニオン表示
	m_rotation = Math::Quaternion::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z));

	ImGui::Separator();
	m_playerConfig.InGuiInspector();
}

void Player::JsonInput(const nlohmann::json& _json)
{
	CharacterBase::JsonInput(_json);
	if (_json.contains("GravitySpeed"))   m_gravitySpeed = _json["GravitySpeed"].get<float>();
	if (_json.contains("fixedFps"))       m_fixedFrameRate = _json["fixedFps"].get<float>();
	if (_json.contains("moveSpeed"))      m_moveSpeed = _json["moveSpeed"].get<float>();
	if (_json.contains("cameraShake"))    m_cameraShake.power = JSON_MANAGER.JsonToVector2(_json["cameraShake"]);
	if (_json.contains("cameraShakeTime")) m_cameraShake.time = _json["cameraShakeTime"].get<float>();
	if (_json.contains("rotateSpeed"))    m_rotateSpeed = _json["rotateSpeed"].get<float>();
	if (_json.contains("degree"))         m_degree = JSON_MANAGER.JsonToVector(_json["degree"]);

	m_playerConfig.JsonInput(_json);
}

void Player::JsonSave(nlohmann::json& _json) const
{
	CharacterBase::JsonSave(_json);
	_json["GravitySpeed"] = m_gravitySpeed;
	_json["fixedFps"] = m_fixedFrameRate;
	_json["moveSpeed"] = m_moveSpeed;
	_json["cameraShake"] = JSON_MANAGER.Vector2ToJson(m_cameraShake.power);
	_json["cameraShakeTime"] = m_cameraShake.time;
	_json["rotateSpeed"] = m_rotateSpeed;
	_json["degree"] = JSON_MANAGER.VectorToJson(m_degree);

	m_playerConfig.JsonSave(_json);
}

void Player::StateInit()
{
	auto state = std::make_shared<PlayerState_Idle>();
	ChangeState(state);
}

void Player::ChangeState(std::shared_ptr<PlayerStateBase> _state)
{
	_state->SetPlayer(this);
	m_stateManager.ChangeState(_state);
}

void Player::UpdateMoveDirectionFromInput()
{
	const auto& kb = KeyboardManager::GetInstance();
	const bool w = kb.IsKeyPressed('W');
	const bool s = kb.IsKeyPressed('S');
	const bool a = kb.IsKeyPressed('A');
	const bool d = kb.IsKeyPressed('D');

	m_movement.movement = Math::Vector3::Zero;

	// 片方のみ押されている場合だけ加算（排他的）
	if (w ^ s) m_movement.movement += w ? Math::Vector3::Backward : Math::Vector3::Forward;
	if (a ^ d) m_movement.movement += a ? Math::Vector3::Left : Math::Vector3::Right;

	if (m_movement.movement.LengthSquared() > 0.0f)
	{
		m_movement.movement.Normalize();
		m_movement.lastDir = m_movement.movement;
	}
}

void Player::TakeDamage(int _damage)
{
	m_characterData->SetCharacterData().hp -= _damage;
	if (m_characterData->GetCharacterData().hp < 0) m_characterData->SetCharacterData().hp = 0;
}

void Player::ApplyHorizontalMove(const Math::Vector3& _inputMove, float _deltaTime)
{
	if (_inputMove == Math::Vector3::Zero) return;

	const Math::Vector3 desired = _inputMove * m_moveSpeed * m_fixedFrameRate * _deltaTime;
	const float desiredLen = desired.Length();
	if (desiredLen <= FLT_EPSILON) return;

	const Math::Vector3 dir = desired / desiredLen;

	KdCollider::RayInfo ray;
	ray.m_pos = m_prevPosition + Math::Vector3(0.0f, kBumpSphereYOffset, 0.0f);
	ray.m_dir = dir;
	ray.m_range = desiredLen + kBumpSphereRadius;
	ray.m_type = KdCollider::TypeBump;

	m_pDebugWire->AddDebugLine(ray.m_pos, ray.m_dir, ray.m_range, kRedColor);

	std::list<KdCollider::CollisionResult> rayHits;

	if (!m_collision.expired()) return;

	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::Collision, m_object);

	for (auto& weakCol : m_object)
	{
		if (auto col = weakCol.lock()) { col->Intersects(ray, &rayHits); }
	}
	m_object.clear();

	bool blocked = false;
	float bestOverlap = 0.0f;
	Math::Vector3 hitPos{};
	for (auto& h : rayHits)
	{
		if (bestOverlap < h.m_overlapDistance)
		{
			bestOverlap = h.m_overlapDistance;
			hitPos = h.m_hitPos;
			blocked = true;
		}
	}

	if (blocked)
	{
		const float hitDist = (hitPos - ray.m_pos).Length();
		const float allow = std::max(0.0f, hitDist - kBumpSphereRadius - kCollisionMargin);
		m_position = m_prevPosition + dir * allow;
	}
	else
	{
		m_position = m_prevPosition + desired;
	}
}

void Player::ApplyPushWithCollision(const Math::Vector3& _rawPush)
{
	if (_rawPush.LengthSquared() <= 1e-8f) return;

	Math::Vector3 push = _rawPush;
	push.y = 0.0f;
	const float len = push.Length();
	if (len <= 1e-6f) return;
	const Math::Vector3 dir = push / len;

	KdCollider::RayInfo ray;
	ray.m_pos = m_position + Math::Vector3(0.0f, kBumpSphereYOffset, 0.0f);
	ray.m_dir = dir;
	ray.m_range = len + kBumpSphereRadius;
	ray.m_type = KdCollider::TypeBump;

	std::list<KdCollider::CollisionResult> rayHits;

	if (!m_collision.expired()) return;

	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::Collision, m_object);

	for (auto& wk : m_object)
	{
		if (auto col = wk.lock())
		{
			col->Intersects(ray, &rayHits);
		}
	}
	m_object.clear();

	bool blocked = false;
	float bestOverlap = 0.0f;
	Math::Vector3 hitPos{};
	for (const auto& h : rayHits)
	{
		if (bestOverlap < h.m_overlapDistance)
		{
			bestOverlap = h.m_overlapDistance;
			hitPos = h.m_hitPos;
			blocked = true;
		}
	}

	if (blocked)
	{
		const float hitDist = (hitPos - ray.m_pos).Length();
		const float allow = std::max(0.0f, hitDist - kBumpSphereRadius - kCollisionMargin);
		if (allow > 0.0f)
		{
			m_position += dir * allow;
		}
		// 衝突点を超える押し出しは捨てる
	}
	else
	{
		m_position += push;
	}
}

void Player::ApplyVerticalMove(float _deltaY)
{
	if (std::abs(_deltaY) <= FLT_EPSILON) return;

	Math::Vector3 start = m_position;
	start.y = m_prevPosition.y;

	// 両タイプ（地形/壁）を検出するスイープ関数
	auto sweep = [&](KdCollider::Type type, std::list<KdCollider::CollisionResult>& out)
		{
			KdCollider::RayInfo ray;
			ray.m_pos = start + Math::Vector3(0.0f, kBumpSphereYOffset, 0.0f);
			ray.m_dir = (_deltaY < 0.0f) ? Math::Vector3(0.0f, -1.0f, 0.0f) : Math::Vector3(0.0f, 1.0f, 0.0f);
			ray.m_range = std::abs(_deltaY) + kBumpSphereRadius;
			ray.m_type = type;
			m_pDebugWire->AddDebugLine(ray.m_pos, ray.m_dir, ray.m_range, kRedColor);

			if (!m_collision.expired()) return;

			SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::Collision, m_object);

			for (auto& weakCol : m_object)
			{
				if (auto col = weakCol.lock())
				{
					col->Intersects(ray, &out);
				}
			}
			m_object.clear();
		};

	std::list<KdCollider::CollisionResult> rayHits;
	sweep(KdCollider::TypeGround, rayHits);
	sweep(KdCollider::TypeBump, rayHits);

	bool blocked = false;
	float bestOverlap = 0.0f;
	Math::Vector3 hitPos{};
	for (auto& h : rayHits)
	{
		if (bestOverlap < h.m_overlapDistance)
		{
			bestOverlap = h.m_overlapDistance;
			hitPos = h.m_hitPos;
			blocked = true;
		}
	}

	if (blocked)
	{
		const float hitDist = (hitPos - (start + Math::Vector3(0.0f, kBumpSphereYOffset, 0.0f))).Length();
		const float allow = std::max(0.0f, hitDist - kBumpSphereRadius - kCollisionMargin);

		const float dirSign = (_deltaY < 0.0f) ? -1.0f : 1.0f;
		m_position.y = m_prevPosition.y + dirSign * allow;

		m_gravity = 0.0f; // 衝突したので重力速度をリセット
	}
	else
	{
		m_position.y = m_prevPosition.y + _deltaY;
	}
}