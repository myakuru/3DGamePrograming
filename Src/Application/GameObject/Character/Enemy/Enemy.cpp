#include "Enemy.h"
#include"../Player/Player.h"
#include"../../../Scene/SceneManager.h"
#include"EnemyState/EnemyState_Idle/EnemyState_Idle.h"
#include"EnemyState/EnemyState_Hit/EnemyState_Hit.h"
#include"../../Camera/PlayerCamera/PlayerCamera.h"
#include"../../Weapon/EnemySword/EnemySword.h"
#include"../../Weapon/EnemyShield/EnemyShield.h"
#include"../../Collition/Collition.h"
#include"../../../main.h"
#include"../../../../MyFramework/Manager/JsonManager/JsonManager.h"
#include"EnemyState/EnemyState_Dath/EnemyState_Dath.h"

const uint32_t Enemy::TypeID = KdGameObject::GenerateTypeID();

void Enemy::Init()
{
	CharaBase::Init();

	m_rotateSpeed = 10.0f;

	m_animator->SetAnimation(m_modelWork->GetData()->GetAnimation("Idle"));

	m_pCollider = std::make_unique<KdCollider>();

	m_pCollider->RegisterCollisionShape("EnemySphere", sphere, KdCollider::TypeDamage);

	m_pCollider->RegisterCollisionShape("PlayerSphere", sphere, KdCollider::TypeEnemyHit);

	StateInit();

	m_isAtkPlayer = false;
	m_dissever = 0.0f;

	m_invincible = false;

	m_Expired = false;
}

void Enemy::Update()
{
	// 自分の武器が未割り当て/消滅なら一度だけ取得して所有者に設定する

	// reuse m_object as a temporary container: query swords first, process, then shields
	if (m_wpSword.expired())
	{
		SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemySword, m_object);
		for (auto& w : m_object)
		{
			if (auto weapon = w.lock())
			{
				if (weapon->GetTypeID() == EnemySword::TypeID)
				{
					auto castedEnemySword = std::static_pointer_cast<EnemySword>(weapon);
					if (castedEnemySword->GetOwnerEnemy().expired())
					{
						castedEnemySword->SetOwnerEnemy(std::static_pointer_cast<Enemy>(shared_from_this()));
						m_wpSword = castedEnemySword;
						break;
					}
				}
			}
		}
		m_object.clear();
	}

	if (m_wpShield.expired())
	{
		SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemyShield, m_object);
		for (auto& w : m_object)
		{
			if (auto weapon = w.lock())
			{
				if (weapon->GetTypeID() == EnemyShield::TypeID)
				{
					auto castedEnemyShield = std::static_pointer_cast<EnemyShield>(weapon);
					if (castedEnemyShield->GetOwnerEnemy().expired())
					{
						castedEnemyShield->SetOwnerEnemy(std::static_pointer_cast<Enemy>(shared_from_this()));
						m_wpShield = castedEnemyShield;
						break;
					}
				}
			}
		}
		m_object.clear();
	}

	// 球の中心座標と半径を設定
	sphere.Center = m_position + Math::Vector3(0.0f, 0.7f, 0.0f); // 敵の位置＋オフセット
	sphere.Radius = 0.2f; // 半径0.5

	m_pDebugWire->AddDebugSphere(sphere.Center, sphere.Radius, kBlueColor);

	SceneManager::Instance().GetObjectWeakPtr(m_wpPlayer);

	CharaBase::Update();

	// ヒット処理。
	if (m_isHit)
	{
		m_isHit = false;

		if (m_invincible) return;

		// ダメージステートに変更
		auto spDamageState = std::make_shared<EnemyState_Hit>();
		ChangeState(spDamageState);
		return;
	}

	// 自分が所有する剣のみ行列を更新
	if (auto sword = m_wpSword.lock())
	{
		if (auto rightHandNode = m_modelWork->FindWorkNode("weapon_r"); rightHandNode)
		{
			sword->SetEnemyRightHandMatrix(rightHandNode->m_worldTransform);
			sword->SetEnemyMatrix(m_mWorld);
		}
	}

	// 自分が所有する盾のみ行列を更新
	if (auto shield = m_wpShield.lock())
	{
		if (auto leftHandNode = m_modelWork->FindWorkNode("weapon_l"); leftHandNode)
		{
			shield->SetEnemyLeftHandMatrix(leftHandNode->m_worldTransform);
			shield->SetEnemyMatrix(m_mWorld);
		}
	}
}

void Enemy::DrawLit()
{
	//ディゾルブ処理
	KdShaderManager::Instance().m_StandardShader.SetDissolve(m_dissever,&m_dissolvePower, &m_dissolveColor);
	SelectDraw3dModel::DrawLit();
}

void Enemy::UpdateAttackCollision(float _radius, float _distance, int _attackCount, float _attackTimer, float _activeBeginSec, float _activeEndSec)
{
	Math::Vector3 forward = Math::Vector3::TransformNormal(Math::Vector3::Forward, Math::Matrix::CreateFromQuaternion(m_rotation));
	forward.Normalize();

	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	KdCollider::SphereInfo attackSphere;
	attackSphere.m_sphere.Center = m_position + Math::Vector3(0.0f, 0.5f, 0.0f) + forward * _distance;
	attackSphere.m_sphere.Radius = _radius;
	attackSphere.m_type = KdCollider::TypeDamage;

	m_pDebugWire->AddDebugSphere(attackSphere.m_sphere.Center, attackSphere.m_sphere.Radius);

	// 初回セットアップ
	if (!m_hitOnce)
	{
		m_isChargeAttackActive = true;
		m_chargeAttackCount = 0;
		m_chargeAttackTimer = 0.0f;
		m_hitOnce = true;

		// Just回避の一発ゲートをリセット
		m_justAvoidSuccess = false;

		// クランプしない。開始 > 終了なら入れ替えのみ
		float begin = _activeBeginSec;
		float end = _activeEndSec;
		if (begin > end) { float t = begin; begin = end; end = t; }

		m_attackActiveTime = 0.0f;
		m_attackActiveBegin = begin;
		m_attackActiveEnd = end;
	}

	if (!m_isChargeAttackActive) return;

	// 攻撃ウィンドウを進める
	m_attackActiveTime += deltaTime;

	// 開始前は何もしない
	if (m_attackActiveTime < m_attackActiveBegin) return;

	// 既にJust回避が成立していたら、この攻撃中は以後の再判定をしない
	if (m_justAvoidSuccess)
	{
		// 必要ならこの攻撃自体を終了させたい場合は以下を有効化
		m_isChargeAttackActive = false;
		m_justAvoidSuccess = false; // 次回の攻撃に備えてリセット
		return;
	}

	// 終了超過で攻撃終了
	if (m_attackActiveTime > m_attackActiveEnd)
	{
		m_isChargeAttackActive = false;
		return;
	}

	// プレイヤーが存在しない場合は処理しない
	if (m_wpPlayer.expired()) return;

	{
		constexpr float kBroadPhaseMargin = 0.5f;
		const float searchRadius = attackSphere.m_sphere.Radius + kBroadPhaseMargin;
		SceneManager::Instance().GetObjectWeakPtrListByTagInSphere(ObjTag::PlayerLike, attackSphere.m_sphere.Center, searchRadius, m_object);
	}

	// ジャスト回避成功チェック（有効時間内のみ）
	for (const auto& players : m_object)
	{
		if (auto playerPtr = players.lock())
		{
			if(playerPtr->GetTypeID() != Player::TypeID) continue;

			auto castedPlayer = std::static_pointer_cast<Player>(playerPtr);

			std::list<KdCollider::CollisionResult> results;
			if (castedPlayer->Intersects(attackSphere, &results) && !results.empty())
			{

				// プレイヤーが回避中か判定
				if (castedPlayer->GetAvoidFlg())
				{
					const float kJustAvoidWindowSec = 0.5f; // 30f/60fps
					const float avoidElapsed = castedPlayer->GetAvoidStartTime();
					if (avoidElapsed >= 0.0f && avoidElapsed <= kJustAvoidWindowSec)
					{
						m_justAvoidSuccess = true;

						// プレイヤーへも成立通知（プレイヤー側の状態遷移/効果に利用）
						castedPlayer->SetJustAvoidSuccess(true);

						// プレイヤー設定からスローモーション倍率・グレースケール適用を取得
						auto& justCfg = castedPlayer->GetPlayerConfig().GetJustAvoidParam();
						Application::Instance().SetFpsScale(justCfg.m_slowMoScale);
						SceneManager::Instance().SetDrawGrayScale(justCfg.m_useGrayScale);

						// 必要に応じてこの攻撃の当たり判定を終了
						// m_isChargeAttackActive = false;

						return; // ダメージ処理は行わない
					}
				}
			}
		}
	}

	// 多段ヒットのインターバル管理
	m_chargeAttackTimer += deltaTime;

	if (m_chargeAttackCount < _attackCount && m_chargeAttackTimer >= _attackTimer)
	{
		for (const auto& players : m_object)
		{
			if (auto playerPtr = players.lock())
			{
				if (playerPtr->GetTypeID() != Player::TypeID) continue;

				auto castedPlayer = std::static_pointer_cast<Player>(playerPtr);

				std::list<KdCollider::CollisionResult> results;

				if (castedPlayer->Intersects(attackSphere, &results) && !results.empty())
				{
					castedPlayer->TakeDamage(m_status.attack);
					castedPlayer->SetHitCheck(true);
				}
			}
		}

		m_chargeAttackCount++;
		m_chargeAttackTimer = 0.0f;

		if (m_chargeAttackCount >= _attackCount)
		{
			m_isChargeAttackActive = false;
		}
	}

	m_object.clear();

}

void Enemy::PostUpdate()
{
	// 球判定
	// 球判定用の変数
	KdCollider::SphereInfo sphereInfo;
	// 球の中心座標を設定
	sphereInfo.m_sphere.Center = m_position + Math::Vector3(0.0f, 0.5f, 0.0f);
	// 球の半径を設定
	sphereInfo.m_sphere.Radius = 0.2f;
	// アタリ判定をしたいタイプを設定
	sphereInfo.m_type = KdCollider::TypeBump; // 地面のアタリ判定

	m_pDebugWire->AddDebugSphere(sphereInfo.m_sphere.Center, sphereInfo.m_sphere.Radius);

	// 球に当たったオブジェクト情報を格納するリスト
	std::list<KdCollider::CollisionResult> retSpherelist;

	if (m_collision.expired()) return;

	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::Collision, m_object);

	for(auto & weakCol : m_object)
	{
		if (auto col = weakCol.lock(); col)
		{
			col->Intersects(sphereInfo, &retSpherelist);
		}
	}

	m_object.clear();

	// 球にあたったリストから一番近いオブジェクトを探す
	// オーバーした長さが1番長いものを探す。
	// 使いまわしの変数を使う
	float maxOverLap = 0.0f;
	bool hit = false;
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
        // 正規化して押し出す方向を求める
        hitDir.Normalize();

        // Y方向の押し出しを無効化（XZ平面のみ）
        hitDir.y = 0.0f;
        hitDir.Normalize();

        //当たってたらその方向から押し出す
        m_position += hitDir * maxOverLap;
    }
}

void Enemy::ImGuiInspector()
{
	CharaBase::ImGuiInspector();

	ImGui::DragFloat(U8("重力の大きさ"), &m_gravitySpeed, 0.01f);
	ImGui::DragFloat(U8("アニメーション速度"), &m_fixedFrameRate, 1.f);
	ImGui::Text(U8("プレイヤーの回転速度"));
	ImGui::DragFloat(U8("回転速度"), &m_rotateSpeed, 0.1f);

	ImGui::Text(U8("現在の状態"));
	ImGui::DragFloat(U8("移動速度"), &m_moveSpeed, 0.1f);

	// ディゾルブ関係
	ImGui::ColorEdit3(U8("ディゾルブカラー"), &m_dissolveColor.x);
	ImGui::DragFloat(U8("ディゾルブ進行度"), &m_dissolvePower, 0.01f, 0.0f, 1.0f);
}

void Enemy::JsonInput(const nlohmann::json& _json)
{
	CharaBase::JsonInput(_json);
	if (_json.contains("GravitySpeed")) m_gravitySpeed = _json["GravitySpeed"].get<float>();
	if (_json.contains("fixedFps")) m_fixedFrameRate = _json["fixedFps"].get<float>();
	if (_json.contains("moveSpeed")) m_moveSpeed = _json["moveSpeed"].get<float>();
	if (_json.contains("rotationspeed")) m_rotateSpeed = _json["rotationspeed"].get<float>();
	if (_json.contains("dissolveColor")) m_dissolveColor = JSON_MANAGER.JsonToVector(_json["dissolveColor"]);
	if (_json.contains("dissolvePower")) m_dissolvePower = _json["dissolvePower"].get<float>();
}

void Enemy::JsonSave(nlohmann::json& _json) const
{
	CharaBase::JsonSave(_json);
	_json["GravitySpeed"] = m_gravitySpeed;
	_json["fixedFps"] = m_fixedFrameRate;
	_json["moveSpeed"] = m_moveSpeed;
	_json["rotationspeed"] = m_rotateSpeed;
	_json["dissolveColor"] = JSON_MANAGER.VectorToJson(m_dissolveColor);
	_json["dissolvePower"] = m_dissolvePower;
}

void Enemy::UpdateQuaternion(Math::Vector3& _moveVector)
{
	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	if (_moveVector == Math::Vector3::Zero) return;

	_moveVector.Normalize();

	// 敵方向ベクトルからクォータニオンを作成
	Math::Quaternion targetRotation = Math::Quaternion::LookRotation(_moveVector, Math::Vector3::Up);

	// 滑らかに回転させる
	m_rotation = Math::Quaternion::Slerp(m_rotation, targetRotation, deltaTime * m_fixedFrameRate);
}

void Enemy::StateInit()
{
	m_isAtkPlayer = false;
	m_hitOnce = false;

	// 初期状態を設定
	auto state = std::make_shared<EnemyState_Idle>();
	ChangeState(state);
}

void Enemy::ChangeState(std::shared_ptr<EnemyStateBase> _state)
{
	_state->SetEnemy(this);
	m_stateManager.ChangeState(_state);
}

void Enemy::Damage(int _damage)
{
	if (m_Expired) return;

	m_getDamage = _damage;
	m_status.hp -= _damage;
	if (m_status.hp < 0)
	{
		m_status.hp = 0;
	}

	if (m_status.hp == 0)
	{
		//　死亡処理
		m_Expired = true;
		m_isHit = false;
		m_invincible = true; // 不要な追加ヒット抑止

		auto spDeathState = std::make_shared<EnemyState_Death>();
		ChangeState(spDeathState);
		return;
	}
	
}

void Enemy::SetDissolve(float v)
{
	// 0～1にクランプ
	if (v < 0.0f) v = 0.0f;
	else if (v > 1.0f) v = 1.0f;
	m_dissever = v;
}

float Enemy::GetDissolve() const
{
	return m_dissever;
}
