#pragma once
#include"MyFramework/State/StateBase/StateBase.h"
#include"Application/GameObject/Character/EnemyBase/AetheriusEnemy/AetheriusEnemy.h"
#include"Application/main.h"
class Player;
class EnemyStateBase : public StateBase
{
	struct StateParameter
	{
		// 当たり判定系
		float attackRadius = 3.0f;
		float attackDistance = 1.0f;
		int   attackCount = 1;
		float attackInterval = 0.1f;    // 旧 m_attackTime
		float attackStartTime = 0.0f;
		float attackEndTime = 0.4f;


		float dashSpeed = 0.7f;				// ダッシュ移動速度
		float blendTime = 0.25f;			// ブレンドエフェクト表示時間
		float animationSpeed = 60.0f;		// アニメーション速度
		float dashSpeedTime = 0.2f;			// ダッシュ移動速度時間
		float changeStateTime = 0.7f;		// 状態遷移までの時間
	};

public:
	EnemyStateBase() = default;
	~EnemyStateBase() override = default;

	void SetEnemy(AetheriusEnemy* enemy) { m_enemy = enemy; }

	void ExposeParametersImGui() override {}
	// JSON 読み込み
	void LoadParametersJson(const nlohmann::json& _json) override {}
	// 保存
	void SaveParametersJson(nlohmann::json& _json) const override {}
	// ImGuiで編集した変数を実行時反映させるための関数
	virtual void ApplyFromConfig(const EnemyStateBase& other) {}

protected:

	void StateStart() override = 0;
	void StateUpdate() override = 0;
	void StateEnd() override = 0;


	Math::Vector3 m_attackDirection = Math::Vector3::Zero;

	Math::Vector3 m_playerPos = Math::Vector3::Zero;
	Math::Vector3 m_enemyPos = Math::Vector3::Zero;

	AetheriusEnemy* m_enemy = nullptr;
	float m_time = 0.0f;

	bool m_hasHitPlayer = false;

	// ヒット回数カウント
	int m_hitCount = 0;

	// プレイヤーとの距離
	float m_distance = 0.0f;

	// アニメーション再生時間
	float m_animeTime = 0.0f;

	std::vector<std::weak_ptr<Player>> m_player;

	StateParameter m_stateParameter;

};