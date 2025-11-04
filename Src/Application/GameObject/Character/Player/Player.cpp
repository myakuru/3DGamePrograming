#include "Player.h"
#include"../../../Scene/SceneManager.h"
#include"../../../Scene/BaseScene/BaseScene.h"
#include"../../Weapon/Katana/Katana.h"
#include"../../Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include"../../../main.h"
#include"../../../../MyFramework/Manager/JsonManager/JsonManager.h"
#include"../../Camera/PlayerCamera/PlayerCamera.h"
#include"PlayerState/PlayerState_Idle/PlayerState_Idle.h"

#include"PlayerState/PlayerState_Hit/PlayerState_Hit.h"

#include"../Enemy/Enemy.h"
#include"../BossEnemy/BossEnemy.h"
#include"../../Collition/Collition.h"

#include"../../../Data/CharacterData/CharacterData.h"

const uint32_t Player::TypeID = KdGameObject::GenerateTypeID();

void Player::Init()
{
	CharaBase::Init();

	m_animator->SetAnimation(m_modelWork->GetData()->GetAnimation("Idle"));

	// ノードの再計算
	m_modelWork->CalcNodeMatrices();

	StateInit();

	m_position = Math::Vector3(31.0f,10.0f, -75.0f);

	m_pCollider = std::make_unique<KdCollider>();

	m_pCollider->RegisterCollisionShape("PlayerSphere", sphere, KdCollider::TypeDamage);
	m_pCollider->RegisterCollisionShape("PlayerSphere", sphere, KdCollider::TypeGround);

	m_onceEffect = false; // エフェクトの一回だけフラグ
	m_isAtkPlayer = false; // プレイヤーが攻撃しているかどうか
	m_invincible = false; // 無敵フラグ

	// 残像描画用 Work を元データで生成
	if (auto* src = GetModelWork())
	{
		m_afterImageWork = std::make_unique<KdModelWork>(src->GetData());
	}

	// クォータニオン表示
	m_rotation = Math::Quaternion::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z));

	m_dissever =0.0f;

	m_isHit = false;
}

void Player::PreUpdate()
{
	sphere.Center = m_position + Math::Vector3(0.0f,0.5f,0.0f); // 敵の位置＋オフセット
	sphere.Radius =0.2f;
	m_pDebugWire->AddDebugSphere(sphere.Center, sphere.Radius, kRedColor);


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
	// 残像処理
	CaptureAfterImage();

	// ライトの影の中心位置をプレイヤーに合わせる
	auto& amb = KdShaderManager::Instance().WorkAmbientController();
	amb.SetShadowCenter(m_position);

	if (m_isAtkPlayer) return;

	KdCollider::SphereInfo enemyHit;
	// 球の中心座標を設定
	enemyHit.m_sphere.Center = m_position + Math::Vector3(0.0f,0.5f,0.0f);
	// 球の半径を設定
	enemyHit.m_sphere.Radius =0.2f;
	// アタリ判定をしたいタイプを設定
	enemyHit.m_type = KdCollider::TypeEnemyHit; // 敵のアタリ判定
	m_pDebugWire->AddDebugSphere(enemyHit.m_sphere.Center, enemyHit.m_sphere.Radius);

	// 球に当たったオブジェクト情報を格納するリスト
	std::list<KdCollider::CollisionResult> retSpherelist;

	// 敵の取得（近傍のみ）: ブロードフェーズの半径を少し広めに
	{
		constexpr float kBroadPhaseMargin =1.0f; // プレイヤー球(0.2)にマージン
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


	// 球にあたったリストから一番近いオブジェクトを探す
	// オーバーした長さが1番長いものを探す。
	float maxOverLap =0.0f;
	float hit = false;

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
		hitDir.y =0.0f;
		if (hitDir.LengthSquared() >0) hitDir.Normalize();

		// 両者が等しく離れる想定 → プレイヤー側は半分だけ動く
		Math::Vector3 push = hitDir * (maxOverLap *0.5f);

		// 壁を貫通しないようスイープして移動
		ApplyPushWithCollision(push);
	}
}

void Player::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.SetDitherEnable(false);
	KdShaderManager::Instance().m_StandardShader.SetLitRimLight({ 0.5f,1.0f,1.0f }, false, 1.2f);

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_modelWork, m_mWorld, m_color);
	KdShaderManager::Instance().m_StandardShader.SetLitRimLight();
}

void Player::DrawRimLight()
{
	if (m_afterImageEnable)
	{
		DrawAfterImages();
	}
}

void Player::Update()
{
	KdGameObject::Update();

	KdShaderManager::Instance().WorkAmbientController().AddPointLight(m_pointLightColor, m_pointLightRadius, m_position + m_pointLightOffset, m_pointLightOn);

	SceneManager::Instance().GetObjectWeakPtr(m_playerCamera);
	SceneManager::Instance().GetObjectWeakPtr(m_katana);
	SceneManager::Instance().GetObjectWeakPtr(m_scabbard);

	if (SceneManager::Instance().m_gameClear)
	{
		m_movement = Math::Vector3::Zero;
	}

	// 雑魚・ボスをまとめてチェック（近傍だけ取得）
	std::list<std::weak_ptr<KdGameObject>> nearEnemies;
	{
		constexpr float kNearbyCheckRadius =12.0f; // 状態チェック用の緩い範囲
		SceneManager::Instance().GetObjectWeakPtrListByTagInSphere(ObjTag::EnemyLike, m_position, kNearbyCheckRadius, nearEnemies);
	}

	for (const auto& wk : nearEnemies)
	{
		if (auto obj = wk.lock())
		{
			if (obj->GetTypeID() == Enemy::TypeID)
			{
				auto e = std::static_pointer_cast<Enemy>(obj);
				if (e->GetJustAvoidSuccess()) m_isHit = false;
			}
			else if (obj->GetTypeID() == BossEnemy::TypeID)
			{
				auto b = std::static_pointer_cast<BossEnemy>(obj);
				if (b->GetJustAvoidSuccess()) m_isHit = false;
			}
		}
	}

	if (m_invincible)
	{
		m_isHit = false;
	}

	// ヒット処理。
	if (m_isHit)
	{
		if (m_invincible) return; // 無敵状態ならヒットしない

		// ダメージステートに変更
		auto spDamageState = std::make_shared<PlayerState_Hit>();
		ChangeState(spDamageState);

		m_isHit = false; // ダメージフラグをリセット
		return;
	}
	
	m_stateManager.Update();

	// スキル・スペシャル使用可能判定
	{
		m_useSkill = (CharacterData::Instance().GetPlayerStatus().skillPoint >= 30.0);
		m_useSpecial = (CharacterData::Instance().SetPlayerStatus().specialPoint == CharacterData::Instance().GetPlayerStatus().specialPointMax);
	}

	if (m_justAvoid)
	{

		float deltaTime = Application::Instance().GetUnscaledDeltaTime();
		m_animator->AdvanceTime(m_modelWork->WorkNodes(), m_fixedFrameRate * deltaTime);
		m_modelWork->CalcNodeMatrices();
		m_isMoving = m_movement.LengthSquared() > 0;
		// 移動前位置を保存
		m_prevPosition = m_position;
		// 重力更新
		m_gravity += m_gravitySpeed * deltaTime;
		// 水平移動
		ApplyHorizontalMove(m_movement, deltaTime);
	}
	else
	{
		float deltaTime = Application::Instance().GetDeltaTime();
		m_animator->AdvanceTime(m_modelWork->WorkNodes(), m_fixedFrameRate * deltaTime);
		m_modelWork->CalcNodeMatrices();
		m_isMoving = m_movement.LengthSquared() >0;
		// 移動前位置を保存
		m_prevPosition = m_position;
		// 重力更新
		m_gravity += m_gravitySpeed * deltaTime;
		// 水平移動
		ApplyHorizontalMove(m_movement, deltaTime);
	}


	// 垂直
	ApplyVerticalMove(m_gravity);

	Math::Matrix scale = Math::Matrix::CreateScale(m_scale);
	Math::Matrix quaternion = Math::Matrix::CreateFromQuaternion(m_rotation);
	Math::Matrix translation = Math::Matrix::CreateTranslation(m_position);
	m_mWorld = scale * quaternion * translation;

}

void Player::UpdateAttackCollision(float _radius, float _distance, int _attackCount, float _attackTimer, Math::Vector2 _cameraShakePow, float _cameraTime, float _activeBeginSec, float _activeEndSec)
{
	Math::Vector3 forward = Math::Vector3::TransformNormal(Math::Vector3::Forward, Math::Matrix::CreateFromQuaternion(m_rotation));
	forward.Normalize();

	float deltaTime = Application::Instance().GetDeltaTime();

	KdCollider::SphereInfo attackSphere;
	attackSphere.m_sphere.Center = m_position + Math::Vector3(0.0f,0.5f,0.0f) + forward * _distance;
	attackSphere.m_sphere.Radius = _radius;
	attackSphere.m_type = KdCollider::TypeDamage;

	m_pDebugWire->AddDebugSphere(attackSphere.m_sphere.Center, attackSphere.m_sphere.Radius);

	// 初回セットアップ
	if (!m_onceEffect)
	{
		m_isChargeAttackActive = true;
		m_chargeAttackCount =0;
		m_chargeAttackTimer =0.0f;

		// クランプしない。開始 > 終了なら入れ替えのみ行う
		float begin = _activeBeginSec;
		float end = _activeEndSec;
		if (begin > end) { float t = begin; begin = end; end = t; }

		m_attackActiveTime =0.0f;
		m_attackActiveBegin = begin;
		m_attackActiveEnd = end;

		m_onceEffect = true;
	}

	if (!m_isChargeAttackActive) return;

	// 攻撃ウィンドウ時間を進める
	m_attackActiveTime += deltaTime;

	// 開始前はまだ当てない
	if (m_attackActiveTime < m_attackActiveBegin) return;

	// 終了超過で攻撃終了
	if (m_attackActiveTime > m_attackActiveEnd)
	{
		m_isChargeAttackActive = false;
		return;
	}

	// 多段ヒットのインターバル管理
	m_chargeAttackTimer += deltaTime;

	if (m_chargeAttackCount < _attackCount && m_chargeAttackTimer >= _attackTimer)
	{
		bool hitAny = false;

		std::list<std::weak_ptr<KdGameObject>> nearEnemies;
		{
			// ブロードフェーズを攻撃球の近傍だけに限定
			constexpr float kBroadPhaseMargin =0.5f;
			const float searchRadius = attackSphere.m_sphere.Radius + kBroadPhaseMargin;
			SceneManager::Instance().GetObjectWeakPtrListByTagInSphere(ObjTag::EnemyLike, attackSphere.m_sphere.Center, searchRadius, nearEnemies);
		}

		for (const auto& wk : nearEnemies)
		{
			if (auto obj = wk.lock())
			{
				std::list<KdCollider::CollisionResult> results;
				if (obj->Intersects(attackSphere, &results) && !results.empty())
				{
					if (obj->GetTypeID() == Enemy::TypeID)
					{
						auto e = std::static_pointer_cast<Enemy>(obj);
						e->Damage(CharacterData::Instance().GetCharacterData().attack);
						e->SetEnemyHit(true);
					}
					else if (obj->GetTypeID() == BossEnemy::TypeID)
					{
						auto b = std::static_pointer_cast<BossEnemy>(obj);
						b->Damage(CharacterData::Instance().GetCharacterData().attack);
						b->SetEnemyHit(true);
					}
					hitAny = true;
				}
			}
		}

		if (hitAny)
		{
			if (auto camera = m_playerCamera.lock(); camera)
			{
				camera->StartShake(_cameraShakePow, _cameraTime);
			}

			if (CharacterData::Instance().GetPlayerStatus().skillPoint <= 100)
			{
				CharacterData::Instance().SetPlayerStatus().skillPoint += _attackCount / 4;
			}

			// specialPoint を絶対に3000 を超えないように飽和加算
			constexpr int kAbsoluteSpecialMax = 3000;
			const int upper = std::min(CharacterData::Instance().GetPlayerStatus().specialPointMax, kAbsoluteSpecialMax);
			const int add = _attackCount * 20;

			CharacterData::Instance().SetPlayerStatus().specialPoint = std::min(CharacterData::Instance().GetPlayerStatus().specialPoint + add, upper);
		}

		m_chargeAttackCount++;
		m_chargeAttackTimer =0.0f;

		if (m_chargeAttackCount >= _attackCount)
		{
			m_isChargeAttackActive = false;
		}
	}
}

void Player::ImGuiInspector()
{
	CharaBase::ImGuiInspector();

	ImGui::Text(U8("プレイヤーの設定"));

	ImGui::DragFloat(U8("重力の大きさ"), &m_gravitySpeed,0.01f);
	ImGui::DragFloat(U8("フレームレート制限"), &m_fixedFrameRate,1.f);

	ImGui::Text(U8("プレイヤーの状態"));
	ImGui::DragFloat(U8("移動速度"), &m_moveSpeed,0.1f);

	ImGui::Text(U8("Attack１のカメラの揺れ"));
	ImGui::DragFloat2(U8("揺れの大きさ"), &m_cameraShakePower.x,0.01f);

	ImGui::Text(U8("Attack1のカメラの揺れ時間"));
	ImGui::DragFloat(U8("揺れの時間"), &m_cameraShakeTime,0.01f);

	ImGui::Text(U8("プレイヤーの回転速度"));
	ImGui::DragFloat(U8("回転速度"), &m_rotateSpeed,0.1f);

	ImGui::DragFloat3(U8("回転(Yaw Pitch Roll)"), &m_degree.x,1.0f);

	ImGui::Separator();

	ImGui::Text(U8("プレイヤーのポイントライトの設定"));
	ImGui::ColorEdit3(U8("ポイントライトの色"), &m_pointLightColor.x);
	ImGui::DragFloat(U8("ポイントライトの半径"), &m_pointLightRadius, 1.0f);
	ImGui::DragFloat3(U8("ポイントライトのオフセット"), &m_pointLightOffset.x, 0.1f);
	ImGui::Checkbox(U8("ポイントライトのON/OFF"), &m_pointLightOn);



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
	CharaBase::JsonInput(_json);
	if (_json.contains("GravitySpeed")) m_gravitySpeed = _json["GravitySpeed"].get<float>();
	if (_json.contains("fixedFps")) m_fixedFrameRate = _json["fixedFps"].get<float>();
	if (_json.contains("moveSpeed")) m_moveSpeed = _json["moveSpeed"].get<float>();

	if (_json.contains("cameraShake")) m_cameraShakePower = JSON_MANAGER.JsonToVector2(_json["cameraShake"]);
	if (_json.contains("cameraShakeTime")) m_cameraShakeTime = _json["cameraShakeTime"].get<float>();
	if (_json.contains("rotateSpeed")) m_rotateSpeed = _json["rotateSpeed"].get<float>();

	if (_json.contains("degree")) m_degree = JSON_MANAGER.JsonToVector(_json["degree"]);

	// ポイントライト
	if (_json.contains("pointLightColor")) m_pointLightColor = JSON_MANAGER.JsonToVector(_json["pointLightColor"]);
	if (_json.contains("pointLightRadius")) m_pointLightRadius = _json["pointLightRadius"].get<float>();
	if (_json.contains("pointLightOffset")) m_pointLightOffset = JSON_MANAGER.JsonToVector(_json["pointLightOffset"]);
	if (_json.contains("pointLightOn")) m_pointLightOn = _json["pointLightOn"].get<bool>();

	m_playerConfig.JsonInput(_json);
}

void Player::JsonSave(nlohmann::json& _json) const
{
	CharaBase::JsonSave(_json);
	_json["GravitySpeed"] = m_gravitySpeed;
	_json["fixedFps"] = m_fixedFrameRate;
	_json["moveSpeed"] = m_moveSpeed;
	_json["cameraShake"] = JSON_MANAGER.Vector2ToJson(m_cameraShakePower);
	_json["cameraShakeTime"] = m_cameraShakeTime;
	_json["rotateSpeed"] = m_rotateSpeed;
	_json["degree"] = JSON_MANAGER.VectorToJson(m_degree);
	_json["pointLightColor"] = JSON_MANAGER.VectorToJson(m_pointLightColor);
	_json["pointLightRadius"] = m_pointLightRadius;
	_json["pointLightOffset"] = JSON_MANAGER.VectorToJson(m_pointLightOffset);
	_json["pointLightOn"] = m_pointLightOn;


	m_playerConfig.JsonSave(_json);
}

void Player::StateInit()
{
	// 初期状態を設定
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
	m_moveDirection = Math::Vector3::Zero;
	if (KeyboardManager::GetInstance().IsKeyPressed('W')) m_moveDirection += Math::Vector3::Backward;
	if (KeyboardManager::GetInstance().IsKeyPressed('S')) m_moveDirection += Math::Vector3::Forward;
	if (KeyboardManager::GetInstance().IsKeyPressed('A')) m_moveDirection += Math::Vector3::Left;
	if (KeyboardManager::GetInstance().IsKeyPressed('D')) m_moveDirection += Math::Vector3::Right;
	if (m_moveDirection.LengthSquared() >0.0f)
	{
		m_moveDirection.Normalize();
		m_lastMoveDirection = m_moveDirection; // 入力があった時だけ更新
	}
}

void Player::TakeDamage(int damage)
{
	CharacterData::Instance().SetCharacterData().hp -= damage;
	if (CharacterData::Instance().GetCharacterData().hp < 0) CharacterData::Instance().SetCharacterData().hp = 0;
}

void Player::CaptureAfterImage()
{
	if (!m_afterImageEnable)
	{
		// 残像無効ならクリアして終了
		m_afterImages.clear();
		m_afterImageCounter =0;
		return;
	}

	// モデルワークが無効なら処理しない
	KdModelWork* work = GetModelWork();
	if (!work || !work->IsEnable()) return;

	// m_afterImageIntervalを超えるまでカウンタを進める
	m_afterImageCounter +=100.0f * Application::Instance().GetDeltaTime();
	if (m_afterImageCounter < m_afterImageInterval) return;
	m_afterImageCounter =0.0f;

	// ノード worldTransform をスナップショット
	const auto& nodes = work->GetNodes();

	AfterImageFrame frame;

	// ノード worldTransform を保存
	frame.nodeWorlds.resize(nodes.size());

	// 各ノードの worldTransform を保存
	for (size_t i =0; i < nodes.size(); ++i)
	{
		frame.nodeWorlds[i] = nodes[i].m_worldTransform;
	}
	// 現在の m_mWorld を保存
	frame.ownerWorld = m_mWorld;

	Math::Matrix scale = Math::Matrix::CreateScale(1,1,1);

	frame.ownerWorld = scale * frame.ownerWorld;

	//先頭に追加し、上限を超えたら末尾を捨てる
	m_afterImages.push_front(std::move(frame));
	while ((int)m_afterImages.size() > m_afterImageMax) m_afterImages.pop_back();
}

void Player::DrawAfterImages()
{
	if (!m_afterImageEnable) return;

	auto& stdShader = KdShaderManager::Instance().m_StandardShader;

	// 古いもの→新しいものの順
	for (int i = (int)m_afterImages.size() - 1; i >= 0; --i)
	{
		// フレームデータ取得
		const auto& frameData = m_afterImages[i];

		// ノード数が異なる場合はスキップ
		auto& workNodes = m_afterImageWork->WorkNodes();
		if (workNodes.size() != frameData.nodeWorlds.size()) continue;

		// ノード worldTransform を上書き
		for (size_t nodeIndex = 0; nodeIndex < workNodes.size(); ++nodeIndex)
		{
			workNodes[nodeIndex].m_worldTransform = frameData.nodeWorlds[nodeIndex];
		}

		m_afterImageWork->SetNeedCalcNodeMatrices(false);

		// 残像の描画
		float a = 1.0f;
		Math::Vector3 color = { 0,1,1 };
		KdShaderManager::Instance().m_StandardShader.SetDissolve(m_dissever, &a, &color);

		KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

		KdShaderManager::Instance().m_StandardShader.SetRimLightEnable(true);

		KdShaderManager::Instance().m_StandardShader.SetRimLight(5.0f, { 0.2f, 1.0f, 1.0f });

		Math::Matrix scale = Math::Matrix::CreateScale({ 1.009f,1.009f,1.009f });

		static float t = 0.0f;
		t += 0.1f;

		KdShaderManager::Instance().m_StandardShader.SetUVOffset({ t, -t * 0.1f });

		Math::Matrix ownerWorld = scale * frameData.ownerWorld;

		stdShader.DrawModel(*m_afterImageWork, ownerWorld, m_afterImageColor);

		KdShaderManager::Instance().UndoBlendState();
	}
}

void Player::ApplyHorizontalMove(const Math::Vector3& inputMove, float deltaTime) // 水平移動をスイープ判定で安全に適用する
{
	if (inputMove == Math::Vector3::Zero) return; // 入力がゼロなら処理不要

	Math::Vector3 desired = inputMove * m_moveSpeed * m_fixedFrameRate * deltaTime; // 入力×速度×固定FPS×Δtで希望移動量を算出
	float desiredLen = desired.Length(); // 希望移動量の長さ（移動距離）
	if (desiredLen <= FLT_EPSILON) return; //ほぼゼロ距離なら終了

	Math::Vector3 dir = desired / desiredLen; //進行方向の正規化ベクトル

	KdCollider::RayInfo ray; // スイープ用のレイ情報（実質カプセル/球の移動をレイで近似）
	ray.m_pos = m_prevPosition + Math::Vector3(0.0f, kBumpSphereYOffset,0.0f); // 前フレーム位置＋バンプ球のYオフセット
	ray.m_dir = dir; // レイの方向＝移動方向
	ray.m_range = desiredLen + kBumpSphereRadius; // 到達距離＋球半径分までスイープ
	ray.m_type = KdCollider::TypeBump; // 壁・障害物との衝突タイプ

	m_pDebugWire->AddDebugLine(ray.m_pos, ray.m_dir, ray.m_range, kRedColor); // デバッグ表示：スイープ可視化

	std::list<KdCollider::CollisionResult> rayHits; // スイープで当たった結果の蓄積先

	if (!m_collision.expired()) return;

	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::Collision, m_object); // 衝突対象リストを取得

	for (auto& weakCol : m_object) // 各コライダへ問い合わせ
	{
		if (auto col = weakCol.lock()) { col->Intersects(ray, &rayHits); } // レイと交差判定し、当たりを収集
	}

	// コンテナは使用後に一度だけクリア
	m_object.clear();

	bool blocked = false; //進行が阻害されるかどうか
	float bestOverlap =0.0f; // 最も大きいオーバーラップ量（＝最も手前の衝突基準に利用）
	Math::Vector3 hitPos{}; // 採用した衝突点
	for (auto& h : rayHits) // 全ヒットから最大オーバーラップのものを選ぶ
	{
		if (bestOverlap < h.m_overlapDistance) // より手前（重なりが大きい）を優先
		{
			bestOverlap = h.m_overlapDistance; // 採用更新
			hitPos = h.m_hitPos; // 衝突点を記録
			blocked = true; //何かにぶつかった
		}
	}

	if (blocked) //ぶつかった場合は到達可能距離までに制限
	{
		float hitDist = (hitPos - ray.m_pos).Length(); // レイ開始から衝突点までの距離
		float allow = std::max(0.0f, hitDist - kBumpSphereRadius - kCollisionMargin); // 球半径とマージンを引いた許容移動距離
		// 入力は殺さず、適用する変位だけクランプ
		m_position = m_prevPosition + dir * allow; // 前位置＋許容分だけ進める
	}
	else //ぶつからないなら希望通り進める
	{
		m_position = m_prevPosition + desired; // 前位置＋希望移動量
	}
}

void Player::ApplyPushWithCollision(const Math::Vector3& rawPush)
{
	if (rawPush.LengthSquared() <=1e-8f) return;

	// 水平みのみ扱う（既存処理と整合）
	Math::Vector3 push = rawPush;
	push.y =0.0f;
	float len = push.Length();
	if (len <=1e-6f) return;
	Math::Vector3 dir = push / len;

	// スイープレイ生成（ApplyHorizontalMove と同等思想）
	KdCollider::RayInfo ray;
	ray.m_pos = m_position + Math::Vector3(0.0f, kBumpSphereYOffset,0.0f);
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

	// コンテナは使用後に一度だけクリア
	m_object.clear();

	bool blocked = false;
	float bestOverlap =0.0f;
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
		float hitDist = (hitPos - ray.m_pos).Length();
		float allow = std::max(0.0f, hitDist - kBumpSphereRadius - kCollisionMargin);
		if (allow >0.0f)
		{
			m_position += dir * allow;
		}
		// 衝突点を超える押し出しは捨てる（滑らせたいなら法線投影をここで実施）
	}
	else
	{
		m_position += push;
	}
}

void Player::ApplyVerticalMove(float deltaY) // 垂直移動をスイープ判定で安全に適用する
{
	if (std::abs(deltaY) <= FLT_EPSILON) return; // 垂直移動量がほぼゼロなら処理不要

	Math::Vector3 start = m_position; // スイープ開始位置の基準を作成
	start.y = m_prevPosition.y; // 垂直は前フレームのYから開始（水平移動分の変化を除外）

	// 両タイプ（地形/壁）を検出するスイープ関数
	auto sweep = [&](KdCollider::Type type, std::list<KdCollider::CollisionResult>& out)
		{
			KdCollider::RayInfo ray; // 垂直方向のスイープレイ
			ray.m_pos = start + Math::Vector3(0.0f, kBumpSphereYOffset,0.0f); // バンプ球の中心高に合わせて開始位置を補正
			ray.m_dir = (deltaY <0.0f) ? Math::Vector3(0.0f, -1.0f,0.0f) : Math::Vector3(0.0f,1.0f,0.0f); // 下向き/上向き
			ray.m_range = std::abs(deltaY) + kBumpSphereRadius; // 到達距離＋球半径までスイープ
			ray.m_type = type; // 検出したいコリジョンタイプ（地形/壁）
			m_pDebugWire->AddDebugLine(ray.m_pos, ray.m_dir, ray.m_range, kRedColor); // デバッグ可視化

			if (!m_collision.expired()) return;

			SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::Collision , m_object); // 衝突対象リストを取得

			for (auto& weakCol : m_object) // 各コライダに問い合わせ
			{
				if (auto col = weakCol.lock()) // 実体化に成功したら
				{
					col->Intersects(ray, &out); // レイと交差判定し、結果を蓄積
				}
			}

			m_object.clear(); // コンテナは使用後に一度だけクリア	

		};

	std::list<KdCollider::CollisionResult> rayHits; // 全スイープ結果の一時バッファ
	sweep(KdCollider::TypeGround, rayHits); // 床/天井（TypeGround）を検出
	sweep(KdCollider::TypeBump, rayHits); // 天井や壁（TypeBump）も検出

	bool blocked = false; //進行が阻害されたか
	float bestOverlap =0.0f; // 最大オーバーラップ量
	Math::Vector3 hitPos{}; // 衝突点
	for (auto& h : rayHits) //すべてのヒットから最も重なりの大きいものを採用
	{
		if (bestOverlap < h.m_overlapDistance) // より手前の衝突（重なり大）を優先
		{
			bestOverlap = h.m_overlapDistance; // 更新
			hitPos = h.m_hitPos; // 衝突点を保存
			blocked = true; //何かに当たった
		}
	}

	if (blocked) // 当たった場合は許容移動量に制限
	{
		float hitDist = (hitPos - (start + Math::Vector3(0.0f, kBumpSphereYOffset,0.0f))).Length(); // スイープ開始から衝突点までの距離
		float allow = std::max(0.0f, hitDist - kBumpSphereRadius - kCollisionMargin); // 球半径と安全マージンを差し引いた許容距離

		float dirSign = (deltaY <0.0f) ? -1.0f :1.0f; // 下移動は負、上移動は正
		m_position.y = m_prevPosition.y + dirSign * allow; // 前フレームYから許容分だけ動かす

		m_gravity =0.0f; // 衝突したので重力速度をリセット（床/天井で停止）
	}
	else // 当たらないなら希望通り移動
	{
		m_position.y = m_prevPosition.y + deltaY; // 前フレームYに移動量を加算
	}
}
