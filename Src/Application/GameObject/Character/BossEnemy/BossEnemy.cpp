#include "BossEnemy.h"
#include"../Player/Player.h"
#include"../../../Scene/SceneManager.h"
#include"../../Camera/PlayerCamera/PlayerCamera.h"
#include"../../Weapon/EnemySword/EnemySword.h"
#include"../../Weapon/EnemyShield/EnemyShield.h"
#include"../../Collition/Collition.h"
#include"../../../main.h"
#include"../../../../MyFramework/Manager/JsonManager/JsonManager.h"
#include"BossEnemyState/BossEnemyState_Enter/BossEnemyState_Enter.h"
#include"BossEnemyState/BossEnemyState_Hit/BossEnemyState_Hit.h"
#include"../BossEnemy/BossEnemyState/BossEnemyState_Dodge/BossEnemyState_Dodge.h"

const uint32_t BossEnemy::TypeID = KdGameObject::GenerateTypeID();

void BossEnemy::Init()
{
	CharaBase::Init();

	m_modelWork->SetModelData("Asset/Models/Enemy/BossEnemy/BossEnemy.gltf");

	SetDrawFlag("DrawLit", true);

	m_rotateSpeed = 10.0f;

	m_animator->SetAnimation(m_modelWork->GetData()->GetAnimation("Idle"));

	m_pCollider = std::make_unique<KdCollider>();

	m_pCollider->RegisterCollisionShape("EnemySphere", sphere, KdCollider::TypeDamage);

	m_pCollider->RegisterCollisionShape("PlayerSphere", sphere, KdCollider::TypeEnemyHit);

	StateInit();

	m_position = { 15.0f,9.8f,-58.0f };

	m_isAtkPlayer = false;
	m_dissever = 0.0f;

	m_moveSpeed = 0.1f;

	m_invincible = false;
	m_stateChange = false;

	m_lastAction = ActionType::None;
	m_meleeCooldown = 0.0f;
	m_waterCooldown = 0.0f;
}

void BossEnemy::Update()
{
	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemySword, m_enemySwords);
	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemyShield, m_enemyShields);

	// 球の中心座標と半径を設定
	sphere.Center = m_position + Math::Vector3(0.0f, 0.7f, 0.0f); // 敵の位置＋オフセット
	sphere.Radius = 0.2f; // 半径0.5

	m_pDebugWire->AddDebugSphere(sphere.Center, sphere.Radius, kBlueColor);

	SceneManager::Instance().GetObjectWeakPtr(m_wpPlayer);

	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	if (SceneManager::Instance().m_gameClear == true)
	{
		m_isExpired = true;
	}

	if (m_Expired)
	{

		if (m_dissever < 1.0f)
		{
			m_dissever += 2.0f * deltaTime;
		}
		else
		{
			m_dissever = 1.0f;
			m_isExpired = true;
		}
	}

	// クールダウン処理
	TickCooldowns(Application::Instance().GetDeltaTime());

	CharaBase::Update();

	// ヒット処理。
	if (m_isHit)
	{
		m_isHit = false;

		if (m_invincible) return;

		// ダメージステートに変更
		auto spDamageState = std::make_shared<BossEnemyState_Hit>();
		ChangeState(spDamageState);
		return;
	}

	// ダメージが半分以下になったら攻撃状態を遷移
	if (m_status.hp <= m_status.maxHp / 2 && !m_stateChange)
	{
		auto state = std::make_shared<BossEnemyState_Dodge>();
		ChangeState(state);
		return;
	}
}

void BossEnemy::DrawLit()
{
	//ディゾルブ処理
	KdShaderManager::Instance().m_StandardShader.SetDissolve(m_dissever, &m_dissolvePower, &m_dissolveColor);
	CharaBase::DrawLit();
}

void BossEnemy::UpdateAttackCollision(float _radius, float _distance, int _attackCount, float _attackTimer, float _activeBeginSec, float _activeEndSec)
{
	Math::Vector3 forward = Math::Vector3::TransformNormal(Math::Vector3::Forward, Math::Matrix::CreateFromQuaternion(m_rotation));
	forward.Normalize();

	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	KdCollider::SphereInfo attackSphere;
	attackSphere.m_sphere.Center = m_position + Math::Vector3(0.0f, 0.5f, 0.0f) + forward * _distance;
	attackSphere.m_sphere.Radius = _radius;
	attackSphere.m_type = KdCollider::TypeDamage;

	m_pDebugWire->AddDebugSphere(attackSphere.m_sphere.Center, attackSphere.m_sphere.Radius);

	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::PlayerLike, m_player);

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

	// ジャスト回避成功チェック（有効時間内のみ）
	for (const auto& players : m_player)
	{
		if (auto playerPtr = players.lock())
		{
			std::list<KdCollider::CollisionResult> results;
			if (playerPtr->Intersects(attackSphere, &results) && !results.empty())
			{
				if (playerPtr->GetTypeID() != Player::TypeID) continue;

				auto castedPlayer = std::static_pointer_cast<Player>(playerPtr);

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
		for (const auto& players : m_player)
		{
			if (auto playerPtr = players.lock())
			{
				std::list<KdCollider::CollisionResult> results;
				if (playerPtr->Intersects(attackSphere, &results) && !results.empty())
				{
					if (playerPtr->GetTypeID() != Player::TypeID) continue;

					auto castedPlayer = std::static_pointer_cast<Player>(playerPtr);

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
}

void BossEnemy::PostUpdate()
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

	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::Collision, m_object); // 衝突対象リストを取得

	for (auto& weakCol : m_object)
	{
		if (auto col = weakCol.lock(); col)
		{
			col->Intersects(sphereInfo, &retSpherelist);
			m_object.clear();
		}
	}

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

void BossEnemy::ImGuiInspector()
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

	// ボスを出現させる
	if (ImGui::Button(U8("ボスを出現させる")))
	{
		SceneManager::Instance().SetBossAppear(true);
	}

	// ボスのHpを変更
	ImGui::DragInt(U8("ボスのHp"), &m_status.hp);
}

void BossEnemy::JsonInput(const nlohmann::json& _json)
{
	CharaBase::JsonInput(_json);
	if (_json.contains("GravitySpeed")) m_gravitySpeed = _json["GravitySpeed"].get<float>();
	if (_json.contains("fixedFps")) m_fixedFrameRate = _json["fixedFps"].get<float>();
	if (_json.contains("moveSpeed")) m_moveSpeed = _json["moveSpeed"].get<float>();
	if (_json.contains("rotationspeed")) m_rotateSpeed = _json["rotationspeed"].get<float>();
	if (_json.contains("dissolveColor")) m_dissolveColor = JSON_MANAGER.JsonToVector(_json["dissolveColor"]);
	if (_json.contains("dissolvePower")) m_dissolvePower = _json["dissolvePower"].get<float>();
}

void BossEnemy::JsonSave(nlohmann::json& _json) const
{
	CharaBase::JsonSave(_json);
	_json["GravitySpeed"] = m_gravitySpeed;
	_json["fixedFps"] = m_fixedFrameRate;
	_json["moveSpeed"] = m_moveSpeed;
	_json["rotationspeed"] = m_rotateSpeed;
	_json["dissolveColor"] = JSON_MANAGER.VectorToJson(m_dissolveColor);
	_json["dissolvePower"] = m_dissolvePower;
}

void BossEnemy::UpdateQuaternion(Math::Vector3& _moveVector)
{
	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	if (_moveVector == Math::Vector3::Zero) return;

	_moveVector.Normalize();

	// 敵方向ベクトルからクォータニオンを作成
	Math::Quaternion targetRotation = Math::Quaternion::LookRotation(_moveVector, Math::Vector3::Up);

	// 滑らかに回転させる
	m_rotation = Math::Quaternion::Slerp(m_rotation, targetRotation, deltaTime * m_fixedFrameRate);
}

void BossEnemy::StateInit()
{
	auto spIdleState = std::make_shared<BossEnemyState_Enter>();
	ChangeState(spIdleState);
}

void BossEnemy::ChangeState(std::shared_ptr<BossEnemyStateBase> _state)
{
	_state->SetBossEnemy(this);
	m_stateManager.ChangeState(_state);
}

void BossEnemy::Damage(int _damage)
{
	m_getDamage = _damage;
	m_status.hp -= _damage;
	if (m_status.hp <= 0)
	{
		m_Expired = true;
	}
}
