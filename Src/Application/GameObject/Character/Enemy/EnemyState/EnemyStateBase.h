#pragma once
#include"../../../../../MyFramework/State/StateBase/StateBase.h"
#include"../Enemy.h"
#include"../../../../main.h"
class EnemyStateBase : public StateBase
{
public:
	EnemyStateBase() = default;
	~EnemyStateBase() override = default;

	void SetEnemy(Enemy* player) { m_enemy = player; }


protected:

	void StateStart() override = 0;
	void StateUpdate() override = 0;
	void StateEnd() override = 0;


	Math::Vector3 m_attackDirection = Math::Vector3::Zero;

	Math::Vector3 m_playerPos = Math::Vector3::Zero;
	Math::Vector3 m_enemyPos = Math::Vector3::Zero;

	Enemy* m_enemy = nullptr;
	float m_time = 0.0f;

	bool m_hasHitPlayer = false;

	// ヒット回数カウント
	int m_hitCount = 0;

	// プレイヤーとの距離
	float m_distance = 0.0f;

	// アニメーション再生時間
	float m_animeTime = 0.0f;

};