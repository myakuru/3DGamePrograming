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
		float attackInterval = 0.1f;
		float attackStartTime = 0.0f;
		float attackEndTime = 0.4f;

		// 当たり判定スタート時間
		float attackActiveStartTime = 0.35f;
		// 当たり判定エンド時間
		float attackActiveEndTime = 1.0f;

		float distanceThreshold = 10.0f;		// プレイヤーからの距離で状態遷移する際の距離閾値

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
	virtual void LoadParametersJson(const nlohmann::json& _json) { (void)_json; }
	// 保存
	virtual void SaveParametersJson(nlohmann::json& _json) const { (void)_json; }
	// ImGuiで編集した変数を実行時反映させるための関数
	virtual void ApplyFromConfig(const EnemyStateBase& other) { (void)other; }

protected:

	void StateStart() override = 0;
	void StateUpdate() override = 0;
	void StateEnd() override = 0;


	Math::Vector3 m_attackDirection = Math::Vector3::Zero;

	Math::Vector3 m_playerPos = Math::Vector3::Zero;
	Math::Vector3 m_enemyPos = Math::Vector3::Zero;

	AetheriusEnemy* m_enemy = nullptr;		// 所有者敵キャラ

	float m_time = 0.0f;
	float m_distance = 0.0f;	// プレイヤーとの距離
	float m_animeTime = 0.0f;	// アニメーション再生時間

	bool m_hasHitPlayer = false;	// プレイヤーにヒットしたかどうか
	bool m_effectPlayed = false;	// ヒットエフェクト再生済みかどうか

	int m_hitCount = 0;			// ヒット回数カウント

	std::vector<std::weak_ptr<Player>> m_player;	// プレイヤー参照

	StateParameter m_stateParameter;	// ステートパラメータ

};