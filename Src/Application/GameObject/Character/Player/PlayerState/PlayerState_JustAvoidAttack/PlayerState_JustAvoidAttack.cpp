#include "PlayerState_JustAvoidAttack.h"
#include"../../../../../main.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../PlayerState_Run/PlayerState_Run.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/SwordFlash/SwordFlash.h"

#include"../PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include"../PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"

#include"../PlayerState_JustAvoidAttack_end/PlayerState_JustAvoidAttack_end.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"

#include"../../../../Effect/EffekseerEffect/JustAvoidAttackEffect1/JustAvoidAttackEffect1.h"
#include"Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"
#include"Application/GameObject/Character/AfterImage/AfterImage.h"

void PlayerState_JustAvoidAttack::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("Eskill");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);

	PlayerStateBase::StateStart();

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	// アニメーション速度設定
	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	// 敵との当たり判定を無効化
	m_player->SetAtkPlayer(true);

	// 無敵状態にする
	m_player->SetInvincible(true);

	// ガードブレイク状態にする
	m_player->SetGuardBreak(true);

	SceneManager::Instance().GetObjectWeakPtr(m_justAvoidAttackEffect);

	// 残像
	m_player->GetAfterImage()->AddAfterImage
	(
		true,
		m_stateParameter.afterImageMax,
		m_stateParameter.afterImageInterval,
		m_stateParameter.afterImageColor
	);


	KdAudioManager::Instance().Play("Asset/Sound/Player/JustAttack.WAV", false)->SetVolume(1.0f);

	SceneManager::Instance().GetObjectWeakPtr(m_bossEnemy);

	m_dashDirection = Math::Vector3::Zero;

}

void PlayerState_JustAvoidAttack::StateUpdate()
{
	// 被ヒット判定無効化
	m_player->SetHitCheck(false);

	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<PlayerState_JustAvoidAttack_end>();
		m_player->ChangeState(state);
		return;
	}

	// 当たり判定有効時間: 1.2秒
	m_player->UpdateAttackCollision(
		m_stateParameter.attackRadius,
		m_stateParameter.attackDistance,
		m_stateParameter.attackCount,
		m_stateParameter.attackInterval,
		m_stateParameter.cameraShake,
		m_stateParameter.cameraTime,
		m_stateParameter.attackStartTime,
		m_stateParameter.attackEndTime
	);

	UpdateKatanaPos();

	// 攻撃中の移動方向で回転を更新
	if (m_player->GetMovement() != Math::Vector3::Zero)
	{
		Math::Vector3 moveDir = m_player->GetMovement();
		moveDir.y = 0.0f;
		moveDir.Normalize();
		m_player->UpdateQuaternionDirect(moveDir);
	}

	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	// 先行ダッシュ処理
	{

		// 攻撃方向が指定されていない場合は敵の方向に向かう
		Math::Vector3 toEnemyDir = m_nearestEnemyPos - m_player->GetPos();


		if (m_attackDirection != Math::Vector3::Zero)
		{
			m_dashDirection = m_attackDirection;
		}
		else
		{
			m_dashDirection = toEnemyDir;
		}

		// 敵の奥に行くようにする距離
		const Math::Vector3 desiredPoint = m_nearestEnemyPos + m_dashDirection * m_overshootDist;

		Math::Vector3 toDesired = desiredPoint - m_player->GetPos();
		toDesired.y = 0.0f;
		float distance = toDesired.Length();

		const float arriveEps = 1.0f;

		if (distance <= arriveEps)
		{
			m_player->SetIsMoving(Math::Vector3::Zero);

			// キャラを敵の方を向ける
			if (toEnemyDir != Math::Vector3::Zero)
			{
				toEnemyDir.y = 0.0f;
				toEnemyDir.Normalize();
				m_player->UpdateQuaternionDirect(toEnemyDir);
			}

			// カメラをキャラの後ろに回す（セッター使用）
			if (auto camera = m_player->GetPlayerCamera().lock())
			{
				camera->SetTargetLookAt(m_cameraBossTargetOffset);

				// キャラ前方からヨー角(deg)を計算してカメラ回転に反映
				if (toEnemyDir != Math::Vector3::Zero)
				{
					const float yawRad = std::atan2(toEnemyDir.x, toEnemyDir.z);
					const float yawDeg = DirectX::XMConvertToDegrees(yawRad);
					camera->SetTargetRotation({ 0.0f, yawDeg, 0.0f });
				}
			}
		}
		else
		{

			if (auto effect = m_justAvoidAttackEffect.lock())
			{
				effect->SetPlayEffect(true);
			}

			toDesired.Normalize();

			// 移動速度の上限
			float maxStep = 10.0f * deltaTime;

			// 今フレームの移動速度
			float speedThisFrame = (distance < maxStep) ? (distance / deltaTime) : 10.0f;

			// 移動ベクトルをセット
			m_player->SetIsMoving(toDesired * speedThisFrame);
			m_time += deltaTime;
		}

	}
}

void PlayerState_JustAvoidAttack::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto effect = m_justAvoidAttackEffect.lock())
	{
		effect->SetPlayEffect(false);
	}

	m_player->GetAfterImage()->AddAfterImage();
}

void PlayerState_JustAvoidAttack::ApplyFromConfig(const PlayerStateBase& other)
{
	assert(typeid(other) == typeid(PlayerState_JustAvoidAttack));
	const auto& p = static_cast<const PlayerState_JustAvoidAttack&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_searchEnemyRadius = p.m_searchEnemyRadius;

	// 当たり判定設定
	m_stateParameter.attackRadius = p.m_stateParameter.attackRadius;
	m_stateParameter.attackDistance = p.m_stateParameter.attackDistance;
	m_stateParameter.attackCount = p.m_stateParameter.attackCount;
	m_stateParameter.attackInterval = p.m_stateParameter.attackInterval;
	m_stateParameter.attackStartTime = p.m_stateParameter.attackStartTime;
	m_stateParameter.attackEndTime = p.m_stateParameter.attackEndTime;

	// 残像設定
	m_stateParameter.afterImageMax = p.m_stateParameter.afterImageMax;
	m_stateParameter.afterImageInterval = p.m_stateParameter.afterImageInterval;
	m_stateParameter.afterImageColor = p.m_stateParameter.afterImageColor;

}

void PlayerState_JustAvoidAttack::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);

	// 索敵範囲の設定
	ImGui::DragFloat(U8("索敵範囲の設定"), &m_searchEnemyRadius);

	// 残像設定
	ImGui::DragInt(U8("残像最大数"), &m_stateParameter.afterImageMax, 1.0f, 1, 20);
	ImGui::DragFloat(U8("残像生成間隔"), &m_stateParameter.afterImageInterval, 0.01f, 0.01f, 1.0f);
	ImGui::ColorEdit4(U8("残像色"), &m_stateParameter.afterImageColor.x);

	const float kLabelWidth = 160.0f;
	const float kItemWidth = 180.0f;

	// 当たり判定
	if (ImGui::CollapsingHeader(U8("当たり判定"), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::BeginTable("tbl_hit", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, kLabelWidth);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

			// 半径
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("当たり判定の半径"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderFloat("##attackRadius", &m_stateParameter.attackRadius, 0.0f, 10.0f, "%.2f");

			// 距離
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("当たり判定の距離"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderFloat("##attackDistance", &m_stateParameter.attackDistance, 0.0f, 10.0f, "%.2f");

			// 回数
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("当たり判定の回数"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderInt("##attackCount", &m_stateParameter.attackCount, 0, 10);

			// 間隔
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("当たり判定の間隔(秒)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderFloat("##attackInterval", &m_stateParameter.attackInterval, 0.0f, 1.0f, "%.03f");

			// 開始/終了時間（同時編集）
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("当たり判定の時間範囲(秒)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth * 1.3f);
			ImGui::DragFloatRange2("##attackTimeRange", &m_stateParameter.attackStartTime, &m_stateParameter.attackEndTime, 0.01f, 0.0f, 10.0f,
				U8("開始: %.02f"), U8("終了: %.02f"));
			if (m_stateParameter.attackEndTime < m_stateParameter.attackStartTime) m_stateParameter.attackEndTime = m_stateParameter.attackStartTime;

			ImGui::EndTable();
		}
	}
}

void PlayerState_JustAvoidAttack::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("PlayerState_JustAvoidAttack")) return;
	const auto& stateNode = js["PlayerState_JustAvoidAttack"];
	if (stateNode.contains("Player"))
	{
		const auto& playerNode = stateNode["Player"];
		if (playerNode.contains("blendTime")) m_stateParameter.blendTime = playerNode["blendTime"].get<float>();
		if (playerNode.contains("animationSpeed")) m_stateParameter.animationSpeed = playerNode["animationSpeed"].get<float>();
		if (playerNode.contains("searchEnemyRadius")) m_searchEnemyRadius = playerNode["searchEnemyRadius"].get<float>();

		// 当たり判定設定
		if (playerNode.contains("attackRadius")) m_stateParameter.attackRadius = playerNode["attackRadius"].get<float>();
		if (playerNode.contains("attackDistance")) m_stateParameter.attackDistance = playerNode["attackDistance"].get<float>();
		if (playerNode.contains("attackCount")) m_stateParameter.attackCount = playerNode["attackCount"].get<int>();
		if (playerNode.contains("attackInterval")) m_stateParameter.attackInterval = playerNode["attackInterval"].get<float>();
		if (playerNode.contains("attackStartTime")) m_stateParameter.attackStartTime = playerNode["attackStartTime"].get<float>();
		if (playerNode.contains("attackEndTime")) m_stateParameter.attackEndTime = playerNode["attackEndTime"].get<float>();

		// 残像設定
		if (playerNode.contains("afterImageMax")) m_stateParameter.afterImageMax = playerNode["afterImageMax"].get<int>();
		if (playerNode.contains("afterImageInterval")) m_stateParameter.afterImageInterval = playerNode["afterImageInterval"].get<float>();
		if (playerNode.contains("afterImageColor")) m_stateParameter.afterImageColor = JSON_MANAGER.JsonToVector4(playerNode["afterImageColor"]);
		
	}
}

void PlayerState_JustAvoidAttack::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("PlayerState_JustAvoidAttack")) js["PlayerState_JustAvoidAttack"] = nlohmann::json::object();
	auto& stateNode = js["PlayerState_JustAvoidAttack"];

	stateNode["Player"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["Player"]["animationSpeed"] = m_stateParameter.animationSpeed;
	stateNode["Player"]["searchEnemyRadius"] = m_searchEnemyRadius;

	// 当たり判定設定
	stateNode["Player"]["attackRadius"] = m_stateParameter.attackRadius;
	stateNode["Player"]["attackDistance"] = m_stateParameter.attackDistance;
	stateNode["Player"]["attackCount"] = m_stateParameter.attackCount;
	stateNode["Player"]["attackInterval"] = m_stateParameter.attackInterval;
	stateNode["Player"]["attackStartTime"] = m_stateParameter.attackStartTime;
	stateNode["Player"]["attackEndTime"] = m_stateParameter.attackEndTime;

	// 残像設定
	stateNode["Player"]["afterImageMax"] = m_stateParameter.afterImageMax;
	stateNode["Player"]["afterImageInterval"] = m_stateParameter.afterImageInterval;
	stateNode["Player"]["afterImageColor"] = JSON_MANAGER.Vector4ToJson(m_stateParameter.afterImageColor);
}
