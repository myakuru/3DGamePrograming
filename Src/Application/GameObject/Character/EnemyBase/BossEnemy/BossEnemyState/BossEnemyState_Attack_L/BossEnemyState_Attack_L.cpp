#include "BossEnemyState_Attack_L.h"
#include"../BossEnemyState_Run/BossEnemyState_Run.h"
#include"../BossEnemyState_Attack_R/BossEnemyState_Attack_R.h"
#include"../BossEnemyState_WaterAttack/BossEnemyState_WaterAttack.h"
#include"Application/Scene/SceneManager.h"
#include"../BossEnemyAI.h"
#include"Application/GameObject/Character/Player/Player.h"
#include"Application/main.h"

#include "Application/GameObject/Utility/EffectReference.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"

void BossEnemyState_Attack_L::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("Attack_L");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	BossEnemyStateBase::StateStart();
	// アニメーション速度を変更
	m_bossEnemy->SetAnimeSpeed(m_stateParameter.animationSpeed);

	// 当たり判定リセット
	m_bossEnemy->ResetAttackCollision();

	// 近接CDと直前行動をセット
	m_bossEnemy->SetMeleeCooldown(1.0f);
	m_bossEnemy->SetLastAction(BossEnemy::ActionType::AttackL);

}

void BossEnemyState_Attack_L::StateUpdate()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	// アニメーション再生時間を取得
	m_animeTime = m_bossEnemy->GetAnimator()->GetPlayProgress();

	// アニメーション時間の35％から100％の間、攻撃判定有効
	if (m_animeTime >= m_stateParameter.attackActiveStartTime && m_animeTime <= m_stateParameter.attackActiveEndTime)
	{
		m_bossEnemy->UpdateAttackCollision
		(
			m_stateParameter.attackRadius,
			m_stateParameter.attackDistance,
			m_stateParameter.attackCount,
			m_stateParameter.attackInterval,
			m_stateParameter.attackStartTime,
			m_stateParameter.attackEndTime
		);
	}

	if (m_animeTime >= m_stateParameter.attackActiveStartTime)
	{
		// エフェクト再生・移動停止（複数）
		for (const auto& ref : m_enemyEffects)
		{
			if (auto effect = ref->GetEffectBase().lock())
			{
				effect->PlayForTarget<BossEnemy>(std::static_pointer_cast<BossEnemy>(m_bossEnemy->GetMyAdls()));
			}
		}
	}

	// Lが終わったら必ずRへ（距離やAI判定に依存しない）
	if (m_bossEnemy->GetAnimator()->IsAnimationEnd())
	{
		auto next = std::make_shared<BossEnemyState_Attack_R>();
		m_bossEnemy->ChangeState(next);
		return;
	}

	if (m_time < m_stateParameter.dashSpeedTime)
	{
		m_bossEnemy->SetIsMoving(m_attackDirection * m_stateParameter.dashSpeed);
		m_time += deltaTime;
	}
	else
	{
		m_bossEnemy->SetIsMoving(Math::Vector3::Zero);
	}
}

void BossEnemyState_Attack_L::StateEnd()
{
	for (const auto& ref : m_enemyEffects)
	{
		if (auto effect = ref->GetEffectBase().lock())
		{
			effect->StopEffect();
		}
	}
}

void BossEnemyState_Attack_L::ApplyFromConfig(const BossEnemyStateBase& other)
{
	assert(typeid(other) == typeid(BossEnemyState_Attack_L));
	const auto& p = static_cast<const BossEnemyState_Attack_L&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_stateParameter.dashSpeedTime = p.m_stateParameter.dashSpeedTime;
	m_stateParameter.dashSpeed = p.m_stateParameter.dashSpeed;
	m_stateParameter.attackActiveStartTime = p.m_stateParameter.attackActiveStartTime;
	m_stateParameter.attackActiveEndTime = p.m_stateParameter.attackActiveEndTime;

	// 当たり判定設定
	m_stateParameter.attackRadius = p.m_stateParameter.attackRadius;
	m_stateParameter.attackDistance = p.m_stateParameter.attackDistance;
	m_stateParameter.attackCount = p.m_stateParameter.attackCount;
	m_stateParameter.attackInterval = p.m_stateParameter.attackInterval;
	m_stateParameter.attackStartTime = p.m_stateParameter.attackStartTime;
	m_stateParameter.attackEndTime = p.m_stateParameter.attackEndTime;

	m_enemyEffects = p.m_enemyEffects;
}

void BossEnemyState_Attack_L::ExposeParametersImGui()
{

	ImGui::Text("Effects");
	for (size_t i = 0; i < m_enemyEffects.size(); ++i)
	{
		ImGui::PushID(static_cast<int>(i));
		m_enemyEffects[i]->ImGuiInspector("EffectSelection");
		ImGui::PopID();
	}
	if (ImGui::SmallButton("+")) { m_enemyEffects.emplace_back(std::make_shared<EffectReference>()); }
	if (ImGui::SmallButton("-")) { if (!m_enemyEffects.empty()) m_enemyEffects.pop_back(); }

	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed, 1.0f, 1.0f, 200.0f);
	ImGui::DragFloat(U8("ダッシュ移動速度"), &m_stateParameter.dashSpeed, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat(U8("ダッシュ移動速度時間"), &m_stateParameter.dashSpeedTime, 0.01f, 0.0f, 5.0f);
	ImGui::DragFloat(U8("当たり判定開始時間"), &m_stateParameter.attackActiveStartTime);
	ImGui::DragFloat(U8("当たり判定終了時間"), &m_stateParameter.attackActiveEndTime);

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

void BossEnemyState_Attack_L::LoadParametersJson(const nlohmann::json& js)
{

	// 複数のみ
	if (js.contains("BossEnemyState_Attack_L_Effects") && js["BossEnemyState_Attack_L_Effects"].is_array())
	{
		m_enemyEffects.clear();
		for (const auto& node : js["BossEnemyState_Attack_L_Effects"])
		{
			auto ref = std::make_shared<EffectReference>();
			ref->JsonInput("Effect", node);
			m_enemyEffects.emplace_back(std::move(ref));
		}
	}

	if (!js.contains("BossEnemyState_Attack_L")) return;
	const auto& stateNode = js["BossEnemyState_Attack_L"];
	if (stateNode.contains("BossEnemy"))
	{
		const auto& enemyNode = stateNode["BossEnemy"];
		if (enemyNode.contains("blendTime")) m_stateParameter.blendTime = enemyNode["blendTime"].get<float>();
		if (enemyNode.contains("dashSpeed")) m_stateParameter.dashSpeed = enemyNode["dashSpeed"].get<float>();
		if (enemyNode.contains("dashSpeedTime")) m_stateParameter.dashSpeedTime = enemyNode["dashSpeedTime"].get<float>();
		if (enemyNode.contains("attackActiveStartTime")) m_stateParameter.attackActiveStartTime = enemyNode["attackActiveStartTime"].get<float>();
		if (enemyNode.contains("attackActiveEndTime")) m_stateParameter.attackActiveEndTime = enemyNode["attackActiveEndTime"].get<float>();

		// 当たり判定設定
		if (enemyNode.contains("attackRadius")) m_stateParameter.attackRadius = enemyNode["attackRadius"].get<float>();
		if (enemyNode.contains("attackDistance")) m_stateParameter.attackDistance = enemyNode["attackDistance"].get<float>();
		if (enemyNode.contains("attackCount")) m_stateParameter.attackCount = enemyNode["attackCount"].get<int>();
		if (enemyNode.contains("attackInterval")) m_stateParameter.attackInterval = enemyNode["attackInterval"].get<float>();
		if (enemyNode.contains("attackStartTime")) m_stateParameter.attackStartTime = enemyNode["attackStartTime"].get<float>();
		if (enemyNode.contains("attackEndTime")) m_stateParameter.attackEndTime = enemyNode["attackEndTime"].get<float>();
	}
}

void BossEnemyState_Attack_L::SaveParametersJson(nlohmann::json& js) const
{
	// 複数のみ
	nlohmann::json arr = nlohmann::json::array();
	for (const auto& ref : m_enemyEffects)
	{
		nlohmann::json item = nlohmann::json::object();
		ref->JsonSave("Effect", item);
		arr.push_back(item);
	}
	js["BossEnemyState_Attack_L_Effects"] = std::move(arr);


	if (!js.contains("BossEnemyState_Attack_L")) js["BossEnemyState_Attack_L"] = nlohmann::json::object();
	auto& stateNode = js["BossEnemyState_Attack_L"];

	stateNode["BossEnemy"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["BossEnemy"]["dashSpeed"] = m_stateParameter.dashSpeed;
	stateNode["BossEnemy"]["dashSpeedTime"] = m_stateParameter.dashSpeedTime;
	stateNode["BossEnemy"]["attackActiveStartTime"] = m_stateParameter.attackActiveStartTime;
	stateNode["BossEnemy"]["attackActiveEndTime"] = m_stateParameter.attackActiveEndTime;

	// 当たり判定設定
	stateNode["BossEnemy"]["attackRadius"] = m_stateParameter.attackRadius;
	stateNode["BossEnemy"]["attackDistance"] = m_stateParameter.attackDistance;
	stateNode["BossEnemy"]["attackCount"] = m_stateParameter.attackCount;
	stateNode["BossEnemy"]["attackInterval"] = m_stateParameter.attackInterval;
	stateNode["BossEnemy"]["attackStartTime"] = m_stateParameter.attackStartTime;
	stateNode["BossEnemy"]["attackEndTime"] = m_stateParameter.attackEndTime;
}
