#include "EnemState_Attack3.h"
#include"../EnemyState_Idle/EnemyState_Idle.h"
#include"../EnemyState_Run/EnemyState_Run.h"
#include"Application/GameObject/Character/Player/Player.h"

void EnemState_Attack3::StateStart()
{
	EnemyStateBase::StateStart();

	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Attack3");
	m_enemy->GetAnimator()->SetAnimation(anime, 0.25f, false);
	m_enemy->SetAnimeSpeed(60.0f);

	// 当たり判定リセット
	m_enemy->ResetAttackCollision();
}

void EnemState_Attack3::StateUpdate()
{

	// アニメーションの再生時間を取得
	m_animeTime = m_enemy->GetAnimator()->GetPlayProgress();

	// アニメーション時間の35％から100％の間、攻撃判定有効
	if (m_animeTime >= 0.0f && m_animeTime <= 1.0f)
	{
		m_enemy->UpdateAttackCollision(2.0f, 0.0f, 1, 0.2f);
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

		if (m_distance >= 6.0f)
		{
			auto state = std::make_shared<EnemyState_Run>();
			m_enemy->ChangeState(state);
			return;
		}
	}
	
	// 移動量リセット
	m_enemy->SetIsMoving(Math::Vector3::Zero);
	if (m_enemy->GetAnimator()->IsAnimationEnd())
	{
		auto attack = std::make_shared<EnemyState_Idle>();
		m_enemy->ChangeState(attack);
		return;
	}
}

void EnemState_Attack3::StateEnd()
{
	m_enemy->SetInvincible(false);
}

void EnemState_Attack3::ApplyFromConfig(const EnemyStateBase& other)
{
	assert(typeid(other) == typeid(EnemState_Attack3));
	const auto& p = static_cast<const EnemState_Attack3&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.dashSpeed = p.m_stateParameter.dashSpeed;
	m_stateParameter.dashSpeedTime = p.m_stateParameter.dashSpeedTime;

	// 当たり判定設定
	m_stateParameter.attackRadius = p.m_stateParameter.attackRadius;
	m_stateParameter.attackDistance = p.m_stateParameter.attackDistance;
	m_stateParameter.attackCount = p.m_stateParameter.attackCount;
	m_stateParameter.attackInterval = p.m_stateParameter.attackInterval;
	m_stateParameter.attackStartTime = p.m_stateParameter.attackStartTime;
	m_stateParameter.attackEndTime = p.m_stateParameter.attackEndTime;
}

void EnemState_Attack3::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("ダッシュ移動速度"), &m_stateParameter.dashSpeed, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat(U8("ダッシュ移動速度時間"), &m_stateParameter.dashSpeedTime, 0.01f, 0.0f, 5.0f);

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

void EnemState_Attack3::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("EnemState_Attack3")) return;
	const auto& stateNode = js["EnemState_Attack3"];
	if (stateNode.contains("AetheriusEnemy"))
	{
		const auto& enemyNode = stateNode["AetheriusEnemy"];
		if (enemyNode.contains("blendTime")) m_stateParameter.blendTime = enemyNode["blendTime"].get<float>();
		if (enemyNode.contains("dashSpeed")) m_stateParameter.dashSpeed = enemyNode["dashSpeed"].get<float>();
		if (enemyNode.contains("dashSpeedTime")) m_stateParameter.dashSpeedTime = enemyNode["dashSpeedTime"].get<float>();

		// 当たり判定設定
		if (enemyNode.contains("attackRadius")) m_stateParameter.attackRadius = enemyNode["attackRadius"].get<float>();
		if (enemyNode.contains("attackDistance")) m_stateParameter.attackDistance = enemyNode["attackDistance"].get<float>();
		if (enemyNode.contains("attackCount")) m_stateParameter.attackCount = enemyNode["attackCount"].get<int>();
		if (enemyNode.contains("attackInterval")) m_stateParameter.attackInterval = enemyNode["attackInterval"].get<float>();
		if (enemyNode.contains("attackStartTime")) m_stateParameter.attackStartTime = enemyNode["attackStartTime"].get<float>();
		if (enemyNode.contains("attackEndTime")) m_stateParameter.attackEndTime = enemyNode["attackEndTime"].get<float>();
	}
}

void EnemState_Attack3::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("EnemState_Attack3")) js["EnemState_Attack3"] = nlohmann::json::object();
	auto& stateNode = js["EnemState_Attack3"];

	stateNode["AetheriusEnemy"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["AetheriusEnemy"]["dashSpeed"] = m_stateParameter.dashSpeed;
	stateNode["AetheriusEnemy"]["dashSpeedTime"] = m_stateParameter.dashSpeedTime;

	// 当たり判定設定
	stateNode["AetheriusEnemy"]["attackRadius"] = m_stateParameter.attackRadius;
	stateNode["AetheriusEnemy"]["attackDistance"] = m_stateParameter.attackDistance;
	stateNode["AetheriusEnemy"]["attackCount"] = m_stateParameter.attackCount;
	stateNode["AetheriusEnemy"]["attackInterval"] = m_stateParameter.attackInterval;
	stateNode["AetheriusEnemy"]["attackStartTime"] = m_stateParameter.attackStartTime;
	stateNode["AetheriusEnemy"]["attackEndTime"] = m_stateParameter.attackEndTime;
}
