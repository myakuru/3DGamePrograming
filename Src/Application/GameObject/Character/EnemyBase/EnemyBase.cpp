#include "EnemyBase.h"
#include "../Player/Player.h"
#include"../../../main.h"
#include"../../../Scene/SceneManager.h"
#include"../../../Data/CharacterData/CharacterData.h"
#include"../../../../MyFramework/Manager/JsonManager/JsonManager.h"

void EnemyBase::Init()
{
	CharacterBase::Init();

	m_movement.rotateSpeed = 10.0f;

	m_animator->SetAnimation(m_modelWork->GetData()->GetAnimation("Idle"));

	m_pCollider = std::make_unique<KdCollider>();

	m_pCollider->RegisterCollisionShape("EnemySphere", m_sphere, KdCollider::TypeDamage);

	m_pCollider->RegisterCollisionShape("PlayerSphere", m_sphere, KdCollider::TypeEnemyHit);

	m_isAtkPlayer = false;

	m_dissever = 0.0f;

	m_invincible = false;

	m_Expired = false;
}

void EnemyBase::Update()
{
}

void EnemyBase::DrawLit()
{
	//ディゾルブ処理
	KdShaderManager::Instance().m_StandardShader.SetDissolve(m_dissever, &m_rendering.dissolvePower, &m_rendering.dissolveColor);
	SelectDraw3dModel::DrawLit();
}

void EnemyBase::UpdateAttackCollision(float _radius, float _distance,
	int _attackCount, float _attackTimer,
	float _activeBeginSec, float _activeEndSec)
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
		SceneManager::Instance().GetObjectWeakPtrListByTagInSphere(ObjTag::PlayerLike, attackSphere.m_sphere.Center, searchRadius, m_refs.referencedObjects);
	}

	// ジャスト回避成功チェック（有効時間内のみ）
	for (const auto& players : m_refs.referencedObjects)
	{
		if (auto playerPtr = players.lock())
		{
			if (playerPtr->GetTypeID() != Player::TypeID) continue;

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
						m_isChargeAttackActive = false;

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
		for (const auto& players : m_refs.referencedObjects)
		{
			if (auto playerPtr = players.lock())
			{
				if (playerPtr->GetTypeID() != Player::TypeID) continue;

				auto castedPlayer = std::static_pointer_cast<Player>(playerPtr);

				std::list<KdCollider::CollisionResult> results;

				if (castedPlayer->Intersects(attackSphere, &results) && !results.empty())
				{
					castedPlayer->TakeDamage(m_characterData->GetCharacterData().attack);
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

	m_refs.referencedObjects.clear();
}

void EnemyBase::PostUpdate()
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

	if (m_refs.collision.expired()) return;

	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::Collision, m_refs.referencedObjects);

	for (auto& weakCol : m_refs.referencedObjects)
	{
		if (auto col = weakCol.lock(); col)
		{
			col->Intersects(sphereInfo, &retSpherelist);
		}
	}

	m_refs.referencedObjects.clear();

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

void EnemyBase::ImGuiInspector()
{
	CharacterBase::ImGuiInspector();

	ImGui::DragFloat(U8("重力の大きさ"), &m_physics.gravitySpeed, 0.01f);
	ImGui::DragFloat(U8("アニメーション速度"), &m_physics.fixedFrameRate, 1.f);
	ImGui::Text(U8("プレイヤーの回転速度"));
	ImGui::DragFloat(U8("回転速度"), &m_movement.rotateSpeed, 0.1f);

	ImGui::Text(U8("現在の状態"));
	ImGui::DragFloat(U8("移動速度"), &m_movement.moveSpeed, 0.1f);

	// ディゾルブ関係
	ImGui::ColorEdit3(U8("ディゾルブカラー"), &m_rendering.dissolveColor.x);
	ImGui::DragFloat(U8("ディゾルブ進行度"), &m_rendering.dissolvePower, 0.01f, 0.0f, 1.0f);
}

void EnemyBase::JsonInput(const nlohmann::json& _json)
{
	CharacterBase::JsonInput(_json);
	if (_json.contains("GravitySpeed")) m_physics.gravitySpeed = _json["GravitySpeed"].get<float>();
	if (_json.contains("fixedFps")) m_physics.fixedFrameRate = _json["fixedFps"].get<float>();
	if (_json.contains("moveSpeed")) m_movement.moveSpeed = _json["moveSpeed"].get<float>();
	if (_json.contains("rotationspeed")) m_movement.rotateSpeed = _json["rotationspeed"].get<float>();
	if (_json.contains("dissolveColor"))  m_rendering.dissolveColor = JSON_MANAGER.JsonToVector(_json["dissolveColor"]);
	if (_json.contains("dissolvePower")) m_rendering.dissolvePower = _json["dissolvePower"].get<float>();
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

	// 敵方向ベクトルからクォータニオンを作成
	Math::Quaternion targetRotation = Math::Quaternion::LookRotation(_moveVector, Math::Vector3::Up);

	// 滑らかに回転させる
	m_rotation = Math::Quaternion::Slerp(m_rotation, targetRotation, deltaTime * m_physics.fixedFrameRate);
}
