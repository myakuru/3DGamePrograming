#include "EnemyState_Attack1.h"
#include"../EnemyState_Attack2/EnemyState_Attack2.h"
#include"../EnemyState_Run/EnemyState_Run.h"

#include"Application/GameObject/Character/Player/Player.h"
#include"Application/main.h"
#include"Application/Scene/SceneManager.h"

#include "Application/GameObject/Effect/EffekseerEffect/AetheriusEnemyEffect/AetheriusEnemy_Attack_1Effect/AetheriusEnemy_Attack_1Effect.h"

void EnemyState_Attack1::StateStart()
{
	EnemyStateBase::StateStart();

	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Attack1");
	m_enemy->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);

	m_time = 0.0f;

	// 当たり判定リセット
	m_enemy->ResetAttackCollision();


	SceneManager::Instance().GetObjectWeakPtr(m_attackEffect);
}

void EnemyState_Attack1::StateUpdate()
{
	// 攻撃中の移動方向で回転を更新
	if (m_enemy->GetMovement() != Math::Vector3::Zero)
	{
		Math::Vector3 moveDir = m_enemy->GetMovement();
		moveDir.y = 0.0f;
		moveDir.Normalize();
		m_enemy->UpdateQuaternionDirect(moveDir);
	}


	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	m_animeTime = m_enemy->GetAnimator()->GetPlayProgress();

	// アニメーション時間の35％から100％の間、攻撃判定有効
	if (m_animeTime >= m_stateParameter.attackActiveStartTime && m_animeTime <= m_stateParameter.attackActiveEndTime)
	{
		// エフェクト再生
		if (auto effect = m_attackEffect.lock(); !m_effectPlayed)
		{
			effect->PlayForEnemy(std::static_pointer_cast<AetheriusEnemy>(m_enemy->GetMyAdls()));
			m_effectPlayed = true;
		}

		m_enemy->UpdateAttackCollision
		(
			m_stateParameter.attackRadius,
			m_stateParameter.attackDistance,
			m_stateParameter.attackCount,
			m_stateParameter.attackInterval,
			m_stateParameter.attackStartTime,
			m_stateParameter.attackEndTime
		);
	}

	// 距離が６以上離れたら追いかける
	{
		for (const auto& player : m_player)
		{
			if (auto p = player.lock())
			{
				m_playerPos = p->GetPos();
				m_enemyPos = m_enemy->GetPos();
				break;
			}
		}

		m_distance = (m_playerPos - m_enemyPos).Length();

		if (m_distance >= m_stateParameter.distanceThreshold)
		{
			auto state = std::make_shared<EnemyState_Run>();
			m_enemy->ChangeState(state);
			return;
		}
	}

	if (m_enemy->GetAnimator()->IsAnimationEnd())
	{
		auto attack = std::make_shared<EnemyState_Attack2>();
		m_enemy->ChangeState(attack);
		return;
	}

	if (m_time < m_stateParameter.dashSpeedTime)
	{
		m_enemy->SetIsMoving(m_attackDirection * m_stateParameter.dashSpeed);
	}
	else
	{
		m_enemy->SetIsMoving(Math::Vector3::Zero);
	}
}

void EnemyState_Attack1::StateEnd()
{
	m_enemy->SetInvincible(false);
}

void EnemyState_Attack1::ApplyFromConfig(const EnemyStateBase& other)
{
	assert(typeid(other) == typeid(EnemyState_Attack1));
	const auto& p = static_cast<const EnemyState_Attack1&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.dashSpeed = p.m_stateParameter.dashSpeed;
	m_stateParameter.dashSpeedTime = p.m_stateParameter.dashSpeedTime;
	m_stateParameter.attackActiveStartTime = p.m_stateParameter.attackActiveStartTime;
	m_stateParameter.attackActiveEndTime = p.m_stateParameter.attackActiveEndTime;
	m_stateParameter.distanceThreshold = p.m_stateParameter.distanceThreshold;

	// 当たり判定設定
	m_stateParameter.attackRadius = p.m_stateParameter.attackRadius;
	m_stateParameter.attackDistance = p.m_stateParameter.attackDistance;
	m_stateParameter.attackCount = p.m_stateParameter.attackCount;
	m_stateParameter.attackInterval = p.m_stateParameter.attackInterval;
	m_stateParameter.attackStartTime = p.m_stateParameter.attackStartTime;
	m_stateParameter.attackEndTime = p.m_stateParameter.attackEndTime;
}

void EnemyState_Attack1::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("ダッシュ移動速度"), &m_stateParameter.dashSpeed, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat(U8("ダッシュ移動速度時間"), &m_stateParameter.dashSpeedTime, 0.01f, 0.0f, 5.0f);
	ImGui::DragFloat(U8("当たり判定スタート時間"), &m_stateParameter.attackActiveStartTime, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat(U8("当たり判定エンド時間"), &m_stateParameter.attackActiveEndTime, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat(U8("距離閾値"), &m_stateParameter.distanceThreshold, 0.1f, 0.0f, 100.0f);

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

void EnemyState_Attack1::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("EnemyState_Attack1")) return;
	const auto& stateNode = js["EnemyState_Attack1"];
	if (stateNode.contains("AetheriusEnemy"))
	{
		const auto& enemyNode = stateNode["AetheriusEnemy"];
		if (enemyNode.contains("blendTime")) m_stateParameter.blendTime = enemyNode["blendTime"].get<float>();
		if (enemyNode.contains("dashSpeed")) m_stateParameter.dashSpeed = enemyNode["dashSpeed"].get<float>();
		if (enemyNode.contains("dashSpeedTime")) m_stateParameter.dashSpeedTime = enemyNode["dashSpeedTime"].get<float>();
		if (enemyNode.contains("attackActiveStartTime")) m_stateParameter.attackActiveStartTime = enemyNode["attackActiveStartTime"].get<float>();
		if (enemyNode.contains("attackActiveEndTime")) m_stateParameter.attackActiveEndTime = enemyNode["attackActiveEndTime"].get<float>();
		if (enemyNode.contains("distanceThreshold")) m_stateParameter.distanceThreshold = enemyNode["distanceThreshold"].get<float>();

		// 当たり判定設定
		if (enemyNode.contains("attackRadius")) m_stateParameter.attackRadius = enemyNode["attackRadius"].get<float>();
		if (enemyNode.contains("attackDistance")) m_stateParameter.attackDistance = enemyNode["attackDistance"].get<float>();
		if (enemyNode.contains("attackCount")) m_stateParameter.attackCount = enemyNode["attackCount"].get<int>();
		if (enemyNode.contains("attackInterval")) m_stateParameter.attackInterval = enemyNode["attackInterval"].get<float>();
		if (enemyNode.contains("attackStartTime")) m_stateParameter.attackStartTime = enemyNode["attackStartTime"].get<float>();
		if (enemyNode.contains("attackEndTime")) m_stateParameter.attackEndTime = enemyNode["attackEndTime"].get<float>();
	}
}

void EnemyState_Attack1::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("EnemyState_Attack1")) js["EnemyState_Attack1"] = nlohmann::json::object();
	auto& stateNode = js["EnemyState_Attack1"];

	stateNode["AetheriusEnemy"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["AetheriusEnemy"]["dashSpeed"] = m_stateParameter.dashSpeed;
	stateNode["AetheriusEnemy"]["dashSpeedTime"] = m_stateParameter.dashSpeedTime;
	stateNode["AetheriusEnemy"]["attackActiveStartTime"] = m_stateParameter.attackActiveStartTime;
	stateNode["AetheriusEnemy"]["attackActiveEndTime"] = m_stateParameter.attackActiveEndTime;
	stateNode["AetheriusEnemy"]["distanceThreshold"] = m_stateParameter.distanceThreshold;

	// 当たり判定設定
	stateNode["AetheriusEnemy"]["attackRadius"] = m_stateParameter.attackRadius;
	stateNode["AetheriusEnemy"]["attackDistance"] = m_stateParameter.attackDistance;
	stateNode["AetheriusEnemy"]["attackCount"] = m_stateParameter.attackCount;
	stateNode["AetheriusEnemy"]["attackInterval"] = m_stateParameter.attackInterval;
	stateNode["AetheriusEnemy"]["attackStartTime"] = m_stateParameter.attackStartTime;
	stateNode["AetheriusEnemy"]["attackEndTime"] = m_stateParameter.attackEndTime;
}
