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

#include "Application/Data/CharacterData/CharacterData.h"

#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/AetheriusEnemy.h"
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"
#include "Application/GameObject/Collition/Collition.h"
#include "Application/GameObject/Character/AfterImage/AfterImage.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_Attack/PlayerState_Attack.h"
#include "Application/GameObject/Character/Player/PlayerConfig.h"

const uint32_t Player::TypeID = KdGameObject::GenerateTypeID();

Player::Player()
{
	m_typeID = TypeID; AddTag(ObjTag::PlayerLike);
}

// デストラクタ(ユニークポインタを使用してるため、cppで定義)
Player::~Player() = default;

void Player::Init()
{
	CharacterBase::Init();

	// デフォルトはIdleステートにする
	GetAnimator()->SetAnimation(m_modelWork->GetData()->GetAnimation("Idle"));
	m_modelWork->CalcNodeMatrices(); // ノードの再計算

	StateInit();

	// 当たり判定の設定
	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape("PlayerSphere", GetBoundingSphere(), KdCollider::TypeDamage);
	m_pCollider->RegisterCollisionShape("PlayerSphere", GetBoundingSphere(), KdCollider::TypeGround);

	// フラグ初期化（新しい構造体へ移行）
	m_action.onceEffect = false;
	m_action.isAtkPlayer = false;

	// 無敵・被弾フラグ初期化
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
	GetCharacterData()->SetCharacterData().hp = 1000;
	GetCharacterData()->SetCharacterData().maxHp = 1000;
	GetCharacterData()->SetCharacterData().attack = 10;

	m_visual.rimLightOn = false;

	// ステートのデーターを作成
	m_playerConfig = std::make_shared<PlayerConfig>();

	if (m_playerConfig)
	{
		m_playerConfig->CreateStates();
	}

	// PlayerConfig.json を読み込み、その内容を PlayerConfig に反映
	if (m_playerConfig)
	{
		const nlohmann::json cfg = JSON_MANAGER.JsonDeserialize("Json/PlayerConfig/PlayerConfig");
		if (!cfg.is_null())
		{
			m_playerConfig->JsonInput(cfg);
		}
	}
}

void Player::PreUpdate()
{
	// 残像のUpdate
	m_visual.afterImage->CaptureAfterImage(m_modelWork.get(), m_mWorld);

	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::PlayerKatana, m_katana);
	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::PlayerScabbard, m_sheaths);

	// カタナの取得
	for (const auto& katanaWeak : m_katana)
	{
		if (auto katana = katanaWeak.lock())
		{
			katana->SetPlayerMatrix(m_mWorld);
			break;
		}
	}

	for (const auto& sheathWeak : m_sheaths)
	{
		if (auto sheath = sheathWeak.lock())
		{
			sheath->SetPlayerMatrix(m_mWorld);
			break;
		}
	}
}

void Player::PostUpdate()
{
	CollisionUpdate();

	if (m_action.isAtkPlayer) return;

	KdCollider::SphereInfo enemyHit = {};
	enemyHit.m_sphere.Center = m_position + Math::Vector3(0.0f, 0.5f, 0.0f);
	enemyHit.m_sphere.Radius = 0.5f;
	enemyHit.m_type = KdCollider::TypeEnemyHit; // 敵のアタリ判定

	m_pDebugWire->AddDebugSphere(enemyHit.m_sphere.Center, enemyHit.m_sphere.Radius);

	// 球に当たったオブジェクト情報を格納するリスト
	std::list<KdCollider::CollisionResult> retSpherelist;

	// 敵の取得（近傍のみ）
	{
		constexpr float kBroadPhaseMargin = 1.0f;
		const float     searchRadius = enemyHit.m_sphere.Radius + kBroadPhaseMargin;
		SceneManager::Instance().GetObjectWeakPtrListByTagInSphere(ObjTag::EnemyLike, enemyHit.m_sphere.Center, m_enemyLike);
	}

	// 球と敵の当たり判定をチェック
	for (const auto& enemyWeakPtr : m_enemyLike)
	{
		auto obj = enemyWeakPtr.lock();
		if (!obj) { continue; }

		obj->Intersects(enemyHit, &retSpherelist);
	}

	// 最も重なりの大きい衝突を採用
	float maxOverLap = 0.0f;
	bool  hit = false;
	Math::Vector3 hitDir;

	for (const auto& ret : retSpherelist)
	{
		if (maxOverLap > ret.m_overlapDistance) { continue; }
		
		maxOverLap = ret.m_overlapDistance;
		hitDir = ret.m_hitDir;
		hit = true;
		
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

void Player::CollisionUpdate()
{
	// 球判定
	// 球判定用の変数
	KdCollider::SphereInfo sphereInfo;
	// 球の中心座標を設定
	sphereInfo.m_sphere.Center = m_position + Math::Vector3(0.0f, 0.5f, 0.0f);
	// 球の半径を設定
	sphereInfo.m_sphere.Radius = 0.2f;
	// アタリ判定をしたいタイプを設定  
	sphereInfo.m_type = KdCollider::TypeBump;

	m_pDebugWire->AddDebugSphere(sphereInfo.m_sphere.Center, sphereInfo.m_sphere.Radius);

	// 球に当たったオブジェクト情報を格納するリスト
	std::list<KdCollider::CollisionResult> retSpherelist;

	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::Collision, Refs().collisionObjects);

	for (const auto& collision : Refs().collisionObjects)
	{
		auto collisionObj = collision.lock();
		if (!collisionObj) continue;

		collisionObj->Intersects(sphereInfo, &retSpherelist);
	}

	bool hit = false;
	float maxOverLap = 0;

	//  球にあたったリストから一番近いオブジェクトを探す
	// オーバーした長さが1番長いものを探す。
	// 使いまわしの変数を使う
	maxOverLap = 0.0f;
	hit = false;
	// 当たった方向を格納する変数
	Math::Vector3 hitDir;

	for (const auto& ret : retSpherelist)
	{
		// 球からはみ出た長さが１番長いものを探す。
		if (maxOverLap > ret.m_overlapDistance) { continue; }
		
		maxOverLap = ret.m_overlapDistance;
		hitDir = ret.m_hitDir;
		hit = true;
		
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

void Player::Update()
{
	if (SceneManager::Instance().GetCurrentScene()->GetSceneName() == "Title") return;

	auto& sceneManager = SceneManager::Instance();

	sceneManager.GetObjectWeakPtr(Refs().playerCamera);

	if (sceneManager.m_gameClear)
	{
		Movement().movement = Math::Vector3::Zero;
		return;
	}

	// 無敵状態ならヒットしない
	if (GetInvincible())
	{
		SetHitCheck(false);
	}

	// ヒット処理
	if (GetHitCheck())
	{
		// 無敵状態ならヒットしない
		if (GetInvincible()) return;

		auto spDamageState = std::make_shared<PlayerState_Hit>();
		ChangeState(spDamageState);

		SetHitCheck(false);
		return;
	}

	m_stateManager.Update();

	// スキル・スペシャル使用可能判定
	{
		m_action.useSkill = (GetCharacterData()->GetPlayerStatus().skillPoint >= 30.0);
		m_action.useSpecial = (GetCharacterData()->GetPlayerStatus().specialPoint == GetCharacterData()->GetPlayerStatus().specialPointMax);

		m_visual.rimLightOn = (GetCharacterData()->GetPlayerStatus().chargeCount >= 3);
	}


	// 時間スケール（ジャスト回避中はアンスケール）
	if (m_avoid.justAvoidAttack)
	{
		const float unScaleDeltaTime = Application::Instance().GetUnscaledDeltaTime();
		GetAnimator()->AdvanceTime(m_modelWork->WorkNodes(), Physics().fixedFrameRate * unScaleDeltaTime);
		m_modelWork->CalcNodeMatrices();


		Raycast().prevPosition = m_position;                         // 移動前位置を保存
		Physics().gravity += Physics().gravitySpeed * unScaleDeltaTime;     // 重力更新
		ApplyHorizontalMove(Movement().movement, unScaleDeltaTime);
	}
	else
	{
		const float deltaTime = Application::Instance().GetDeltaTime();
		const float unScaleDeltaTime = Application::Instance().GetUnscaledDeltaTime();

		GetAnimator()->AdvanceTime(m_modelWork->WorkNodes(), Physics().fixedFrameRate * deltaTime);
		m_modelWork->CalcNodeMatrices();

		Raycast().prevPosition = m_position;
		Physics().gravity += Physics().gravitySpeed * deltaTime;
		ApplyHorizontalMove(Movement().movement, unScaleDeltaTime);
	}

	// 垂直移動
	ApplyVerticalMove(Physics().gravity);

	// ワールド行列
	const Math::Matrix scale = Math::Matrix::CreateScale(m_scale);
	const Math::Matrix quaternion = Math::Matrix::CreateFromQuaternion(m_rotation);
	const Math::Matrix translation = Math::Matrix::CreateTranslation(m_position);

	m_mWorld = scale * quaternion * translation;
}

void Player::UpdateAttackCollision(float _radius         , float         _distance      , int   _attackCount ,
								   float _attackTimer    , Math::Vector2 _cameraShakePow, float _cameraTime  ,
								   float _activeBeginSec , float         _activeEndSec)
{
	Math::Vector3 forward = Math::Vector3::TransformNormal(Math::Vector3::Forward, Math::Matrix::CreateFromQuaternion(m_rotation));
	forward.Normalize();

	const float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	KdCollider::SphereInfo attackSphere = {};
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

		if (begin > end)
		{
			float t = begin; begin = end; end = t;
		}

		Combat().attackWindow.elapsed = 0.0f;
		Combat().attackWindow.begin = begin;
		Combat().attackWindow.end = end;

		m_action.onceEffect = true;
	}

	if (!m_charge.active) return;

	// 攻撃ウィンドウ経過
	Combat().attackWindow.elapsed += deltaTime;

	// 開始前はまだ当てない
	if (Combat().attackWindow.elapsed < Combat().attackWindow.begin) return;

	// 終了超過で攻撃終了
	if (Combat().attackWindow.elapsed > Combat().attackWindow.end)
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
			SceneManager::Instance().GetObjectWeakPtrListByTagInSphere(ObjTag::EnemyLike, attackSphere.m_sphere.Center, nearEnemies);
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
					e->Damage(GetCharacterData()->GetCharacterData().attack);
					e->SetHitCheck(true);
				}
				else if (obj->GetTypeID() == BossEnemy::TypeID)
				{
					auto b = std::static_pointer_cast<BossEnemy>(obj);
					b->Damage(GetCharacterData()->GetCharacterData().attack);
					b->SetHitCheck(true);
				}
				hitAny = true;
			}
		}

		if (hitAny)
		{
			if (auto camera = Refs().playerCamera.lock())
			{
				camera->StartShake(_cameraShakePow, _cameraTime);
			}

			// SkillPoint
			if (GetCharacterData()->GetPlayerStatus().skillPoint <= GetCharacterData()->GetPlayerStatus().skillPointMax)
			{
				GetCharacterData()->SetPlayerStatus().skillPoint += _attackCount;
			}


			// SpecialPoint 飽和加算（絶対上限付き）
			GetCharacterData()->SetPlayerStatus().specialPoint =
				std::min(GetCharacterData()->GetPlayerStatus().specialPoint + (_attackCount * 20), GetCharacterData()->GetPlayerStatus().specialPointMax);
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

	ImGui::Text(U8("プレイヤー設定 (Prototype + Runtime 同一ビュー)"));
	ImGui::Separator();

	// 基本パラメータ
	ImGui::DragFloat(U8("重力の大きさ"), &Physics().gravitySpeed, 0.01f);
	ImGui::DragFloat(U8("フレームレート制限"), &Physics().fixedFrameRate, 1.f);
	ImGui::DragFloat(U8("移動速度"), &Movement().moveSpeed, 0.1f);
	ImGui::DragFloat(U8("回転速度"), &Movement().rotateSpeed, 0.1f);
	ImGui::DragFloat3(U8("回転(Yaw Pitch Roll)"), &m_degree.x, 1.0f);

	ImGui::DragFloat2(U8("Attack1揺れPow"), &m_cameraShake.power.x, 0.01f);
	ImGui::DragFloat(U8("Attack1揺れ時間"), &m_cameraShake.time, 0.01f);

	// Quaternion 再計算
	m_rotation = Math::Quaternion::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z));

	ImGui::Separator();
	ImGui::Text(U8("変更したいステート"));
	m_playerConfig->InGuiInspector();
	ImGui::Separator();
}

void Player::JsonInput(const nlohmann::json& _json)
{
	CharacterBase::JsonInput(_json);

	if (_json.contains("GravitySpeed"))    Physics().gravitySpeed   = _json["GravitySpeed"].get<float>();
	if (_json.contains("fixedFps"))        Physics().fixedFrameRate = _json["fixedFps"].get<float>();
	if (_json.contains("moveSpeed"))       Movement().moveSpeed     = _json["moveSpeed"].get<float>();
	if (_json.contains("cameraShake"))     m_cameraShake.power      = JSON_MANAGER.JsonToVector2(_json["cameraShake"]);
	if (_json.contains("cameraShakeTime")) m_cameraShake.time       = _json["cameraShakeTime"].get<float>();
	if (_json.contains("rotateSpeed"))     Movement().rotateSpeed   = _json["rotateSpeed"].get<float>();
	if (_json.contains("degree"))          m_degree                 = JSON_MANAGER.JsonToVector(_json["degree"]);
}

void Player::JsonSave(nlohmann::json& _json) const
{
	CharacterBase::JsonSave(_json);
	_json["GravitySpeed"] = Physics().gravitySpeed;
	_json["fixedFps"] = Physics().fixedFrameRate;
	_json["moveSpeed"] = Movement().moveSpeed;
	_json["cameraShake"] = JSON_MANAGER.Vector2ToJson(m_cameraShake.power);
	_json["cameraShakeTime"] = m_cameraShake.time;
	_json["rotateSpeed"] = Movement().rotateSpeed;
	_json["degree"] = JSON_MANAGER.VectorToJson(m_degree);


	// PlayerConfig.json を PlayerConfig から生成して保存
	if (m_playerConfig)
	{
		m_playerConfig->JsonSave();
	}
}

void Player::StateInit()
{
	auto state = std::make_shared<PlayerState_Idle>();
	ChangeState(state);
}

void Player::ChangeState(std::shared_ptr<PlayerStateBase> _state)
{
	_state->SetPlayer(this);
	// Configからパラメータ注入（StateStart前に行う）
	if (m_playerConfig)
	{
		m_playerConfig->ApplyPrototypeParametersTo(*_state);
	}
	m_stateManager.ChangeState(_state);
}

void Player::UpdateMoveDirectionFromInput()
{
	const auto& kb = KeyboardManager::GetInstance();

	const bool w = kb.IsKeyPressed('W');
	const bool s = kb.IsKeyPressed('S');
	const bool a = kb.IsKeyPressed('A');
	const bool d = kb.IsKeyPressed('D');

	Movement().movement = Math::Vector3::Zero;

	// 片方のみ押されている場合だけ加算（排他的）
	if (w || s) Movement().movement += w ? Math::Vector3::Backward : Math::Vector3::Forward;
	if (a || d) Movement().movement += a ? Math::Vector3::Left : Math::Vector3::Right;

	if (!(w || s || a || d))
	{
		Movement().isMoving = false;
	}
	else
	{
		Movement().isMoving = true;
	}

	if (Movement().movement.LengthSquared() > 0.0f)
	{
		Movement().movement.Normalize();
		Movement().lastDir = Movement().movement;
	}
}

void Player::TakeDamage(int _damage)const
{
	GetCharacterData()->SetCharacterData().hp -= _damage;
	if (GetCharacterData()->GetCharacterData().hp < 0) GetCharacterData()->SetCharacterData().hp = 0;
}

void Player::ApplyHorizontalMove(const Math::Vector3& _inputMove, float _deltaTime)
{
	if (_inputMove == Math::Vector3::Zero) return;

	const Math::Vector3 desired = _inputMove * Movement().moveSpeed * Physics().fixedFrameRate * _deltaTime;
	const float         desiredLen = desired.Length();
	if (desiredLen <= FLT_EPSILON) return;

	const Math::Vector3 dir = desired / desiredLen;

	KdCollider::RayInfo ray;
	ray.m_pos = Raycast().prevPosition + Math::Vector3(0.0f, Raycast().bumpSphereYOffset, 0.0f);
	ray.m_dir = dir;
	ray.m_range = desiredLen + Raycast().bumpSphereRadius;
	ray.m_type = KdCollider::TypeBump;

	m_pDebugWire->AddDebugLine(ray.m_pos, ray.m_dir, ray.m_range, kRedColor);

	std::list<KdCollider::CollisionResult> rayHits;

	if (!Refs().collision.expired()) return;

	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::Collision, Refs().collisionObjects);

	for (const auto& weakCol : Refs().collisionObjects)
	{
		auto collisionObj = weakCol.lock();

		if (!collisionObj) continue;

		collisionObj->Intersects(ray, &rayHits);
	}

	Math::Vector3 hitNormal = Math::Vector3::Zero;

	bool  blocked = false;
	float bestOverlap = 0.0f;
	Math::Vector3 hitPos = Math::Vector3::Zero;
	for (const auto& h : rayHits)
	{
		if (bestOverlap > h.m_overlapDistance) { continue; }
		
		bestOverlap = h.m_overlapDistance;
		hitPos = h.m_hitPos;
		hitNormal = h.m_hitDir; // 面の法線
		blocked = true;
		
	}

	if (blocked)
	{
		const float hitDist = (hitPos - ray.m_pos).Length();
		const float allow = std::max(0.0f, hitDist - Raycast().bumpSphereRadius - Raycast().collisionMargin);

		m_position = Raycast().prevPosition + dir * allow;
	}
	else
	{
		m_position = Raycast().prevPosition + desired;
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

	KdCollider::RayInfo ray = {};
	ray.m_pos               = m_position + Math::Vector3(0.0f, Raycast().bumpSphereYOffset, 0.0f);
	ray.m_dir               = dir;
	ray.m_range             = len + Raycast().bumpSphereRadius;
	ray.m_type              = KdCollider::TypeBump;

	std::list<KdCollider::CollisionResult> rayHits;

	if (!Refs().collision.expired()) return;

	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::Collision, Refs().collisionObjects);

	for (const auto& wk : Refs().collisionObjects)
	{
		auto collisionObj = wk.lock();

		if (!collisionObj) continue;

		collisionObj->Intersects(ray, &rayHits);

	}

	bool blocked = false;
	float bestOverlap = 0.0f;
	Math::Vector3 hitPos = Math::Vector3::Zero;
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
		const float allow = std::max(0.0f, hitDist - Raycast().bumpSphereRadius - Raycast().collisionMargin);
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
	start.y = Raycast().prevPosition.y;

	// 両タイプ（地形/壁）を判定したいからラムダ式で共通化
	auto sweep = [&](KdCollider::Type type, std::list<KdCollider::CollisionResult>& out)
		{
			KdCollider::RayInfo ray;

			ray.m_pos = start + Math::Vector3(0.0f, Raycast().bumpSphereYOffset, 0.0f);
			ray.m_dir = (_deltaY < 0.0f) ? Math::Vector3(0.0f, -1.0f, 0.0f) : Math::Vector3(0.0f, 1.0f, 0.0f);
			ray.m_range = std::abs(_deltaY) + Raycast().bumpSphereRadius;
			ray.m_type = type;

			m_pDebugWire->AddDebugLine(ray.m_pos, ray.m_dir, ray.m_range, kRedColor);

			if (!Refs().collision.expired()) return;

			SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::Collision, Refs().collisionObjects);

			for (const auto& weakCol : Refs().collisionObjects)
			{
				auto col = weakCol.lock();

				if (!col) continue;

				col->Intersects(ray, &out);

			}
		};

	std::list<KdCollider::CollisionResult> rayHits;

	sweep(KdCollider::TypeGround , rayHits);
	sweep(KdCollider::TypeBump   , rayHits);

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
		const float hitDist = (hitPos - (start + Math::Vector3(0.0f, Raycast().bumpSphereYOffset, 0.0f))).Length();
		const float allow = std::max(0.0f, hitDist - Raycast().bumpSphereRadius - Raycast().collisionMargin);

		const float dirSign = (_deltaY < 0.0f) ? -1.0f : 1.0f;
		m_position.y = Raycast().prevPosition.y + dirSign * allow;

		Physics().gravity = 0.0f; // 衝突したので重力速度をリセット
	}
	else
	{
		m_position.y = Raycast().prevPosition.y + _deltaY;
	}
}