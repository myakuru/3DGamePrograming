#pragma once
#include "Application/Scene/SceneManager.h"
#include "Application/GameObject/Character/Player/Player.h"
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"
#include "Application/GameObject/Weapon/Katana/Katana.h"
#include "Application/GameObject/Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"
#include "Application/main.h"

// ステート横断ユーティリティ
namespace PlayerStateServices
{

	// フォーカス更新と最接近敵の探索
	inline void UpdateFocusAndNearest(Player* player,
		float searchRadius,
		std::weak_ptr<KdGameObject>& focusTarget,
		std::shared_ptr<KdGameObject>& nearestEnemy,
		Math::Vector3& nearestEnemyPos,
		float& minDistSq,
		float& focusRemainSec,
		const float focusDurationSec)
	{
		if (auto f = focusTarget.lock()) {
			nearestEnemy = f;
			nearestEnemyPos = f->GetPos();
		}
		else {
			std::list<std::weak_ptr<KdGameObject>> candidates;
			SceneManager::Instance().GetObjectWeakPtrListByTagInSphere(
				ObjTag::EnemyLike, player->GetPos(), candidates, searchRadius);

			float minSqLocal = std::numeric_limits<float>::max();
			std::shared_ptr<KdGameObject> nearest;
			Math::Vector3 nearestPos = Math::Vector3::Zero;

			for (const auto& w : candidates) {
				auto enemy = w.lock();
				if (!enemy) continue;
				// ボス未出現なら無視
				if (enemy->GetTypeID() == BossEnemy::TypeID && !SceneManager::Instance().IsBossAppear()) {
					continue;
				}
				const float d = (enemy->GetPos() - player->GetPos()).LengthSquared();
				if (d < minSqLocal) {
					minSqLocal = d;
					nearestPos = enemy->GetPos();
					nearest = enemy;
				}
			}

			minDistSq = minSqLocal;
			nearestEnemy = nearest;
			nearestEnemyPos = nearestPos;

			if (nearestEnemy) {
				focusTarget = nearestEnemy;
				focusRemainSec = focusDurationSec;
			}
		}
	}

	// 攻撃方向の決定と回転
	inline Math::Vector3 DecideAttackDirectionAndRotate(Player* player,
		const std::shared_ptr<KdGameObject>& nearestEnemy,
		const Math::Vector3& nearestEnemyPos)
	{
		Math::Vector3 dir = Math::Vector3::Zero;
		if (nearestEnemy)
		{
			dir = nearestEnemyPos - player->GetPos();
			dir.y = 0.0f;
			if (dir != Math::Vector3::Zero) {
				dir.Normalize();
				player->UpdateQuaternionDirect(dir);
			}
		}
		else {
			dir = player->GetLastMoveDirection();
			if (dir != Math::Vector3::Zero) {
				player->UpdateQuaternion(dir);
			}
		}
		return dir;
	}

	// ステート内の簡易初期化（タイマー・入力など）
	inline void ResetStateLocalVars(bool& lButtonKeyInput, float& time, float& animeTime) {
		lButtonKeyInput = false;
		time = 0.0f;
		animeTime = 0.0f;
	}

	// カタナ位置更新（抜刀）
	inline void UpdateKatanaPos(Player* owner)
	{
		if (!owner) return;
		owner->SetRightHanded(true);

		const auto* rightHandNode = owner->GetModelWork()->FindWorkNode("Katana");
		const auto* leftHandNode = owner->GetModelWork()->FindWorkNode("Sheath");
		if (!rightHandNode || !leftHandNode) return;

		for (const auto& wk : owner->GetKatanas())
			if (auto katana = wk.lock())
				katana->SetHandKatanaMatrix(rightHandNode->m_worldTransform);

		for (const auto& wk : owner->GetKatanaSheaths())
			if (auto sheath = wk.lock())
				sheath->SetHandKatanaMatrix(leftHandNode->m_worldTransform);
	}

	// カタナ位置更新（納刀）
	inline void UpdateUnsheathed(Player* owner)
	{
		if (!owner) return;
		owner->SetRightHanded(false);

		const auto* leftHandNode = owner->GetModelWork()->FindWorkNode("Sheath");
		if (!leftHandNode) return;

		for (const auto& wk : owner->GetKatanas())
			if (auto katana = wk.lock())
				katana->SetHandKatanaMatrix(leftHandNode->m_worldTransform);

		for (const auto& wk : owner->GetKatanaSheaths())
			if (auto sheath = wk.lock())
				sheath->SetHandKatanaMatrix(leftHandNode->m_worldTransform);
	}

	// 先行ダッシュ処理（攻撃開始前に敵へオーバーシュートしつつ向き/カメラ制御）
	inline void PreDashTowardEnemy(
		Player* player,
		const Math::Vector3& nearestEnemyPos,
		const Math::Vector3& attackDirection,     // 指定がゼロなら敵方向を使用
		float overshootDist,                      // 目標地点のオーバーシュート距離
		float arriveEps = 1.0f,                   // 到達判定の閾値
		float maxSpeed = 10.0f                    // ダッシュ時の速度上限（m/s）
	)
	{
		if (!player) return;

		const float deltaTime = Application::Instance().GetDeltaTime();

		// 攻撃方向が指定されていない場合は敵の方向に向かう
		Math::Vector3 toEnemyDir = nearestEnemyPos - player->GetPos();

		const Math::Vector3 dashDir = (attackDirection != Math::Vector3::Zero) ? attackDirection : toEnemyDir;

		// 目標地点を敵の少し先に設定
		const Math::Vector3 desiredPoint = nearestEnemyPos + dashDir * overshootDist;

		Math::Vector3 toDesired = desiredPoint - player->GetPos();
		toDesired.y = 0.0f;
		float distance = toDesired.Length();

		if (distance <= arriveEps)
		{
			// 到達：移動停止、向き合わせ、カメラ更新
			player->SetIsMoving(Math::Vector3::Zero);

			// キャラを敵の方に向ける
			if (toEnemyDir != Math::Vector3::Zero)
			{
				Math::Vector3 face = toEnemyDir;
				face.y = 0.0f;
				face.Normalize();
				player->UpdateQuaternionDirect(face);
			}

			// カメラをキャラの後ろに回す（セッター使用）
			if (auto camera = player->GetPlayerCamera().lock(); camera)
			{
				// Boss時に使うオフセットがあれば、ステート側から渡す方式にしたい場合は引数化しても良い
				// ここでは従来のボス用オフセットを使う前提とする
				camera->SetTargetLookAt({ 0.0f, 1.0f, -5.5f }); // 必要なら引数で受け取る

				// キャラ前方からヨー角(deg)を計算してカメラ回転に反映
				if (toEnemyDir != Math::Vector3::Zero)
				{
					const Math::Vector3 flat = { toEnemyDir.x, 0.0f, toEnemyDir.z };
					const float yawRad = std::atan2(flat.x, flat.z);
					const float yawDeg = DirectX::XMConvertToDegrees(yawRad);
					camera->SetPlayerRotation({ 0.0f, yawDeg, 0.0f });
				}
			}
		}
		else
		{
			// 未到達：目標に向けて移動
			toDesired.Normalize();

			// 今フレームで踏み出せる最大距離
			const float maxStep = maxSpeed * deltaTime;

			// 今フレームの移動速度（到達直前は距離に合わせて減速）
			const float speedThisFrame = (distance < maxStep) ? (distance / deltaTime) : maxSpeed;

			// 移動ベクトルをセット
			player->SetIsMoving(toDesired * speedThisFrame);
		}
	}

	// 攻撃方向に即座に向きを合わせる（ゼロベクトルは何もしない）
	inline void UpdateFacingDirect(Player* owner, const Math::Vector3& direction)
	{
		if (!owner) return;
		if (direction == Math::Vector3::Zero) return;

		// プレイヤーのクォータニオンを直接更新する既存APIを利用
		owner->UpdateQuaternionDirect(direction);
	}
}