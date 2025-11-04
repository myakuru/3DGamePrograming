#include "BossEnemyState_Idle.h"
#include"../../../Player/Player.h"
#include"../BossEnemyState_Run/BossEnemyState_Run.h"
#include"../BossEnemyAI.h"

void BossEnemyState_Idle::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("Idle");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, 0.25f, true);
	BossEnemyStateBase::StateStart();
	m_bossEnemy->SetAnimeSpeed(60.0f);

	m_bossEnemy->SetLastAction(BossEnemy::ActionType::Idle);
}

void BossEnemyState_Idle::StateUpdate()
{
	const float deltaTime = Application::Instance().GetDeltaTime();
	m_time += deltaTime;

	// 停止
	m_bossEnemy->SetIsMoving(Math::Vector3::Zero);

	// 最低待機時間まではそのまま
	if (m_time < m_minWaitSec) return;

	// 待機後はAIに委譲
	auto next = BossEnemyAI::DecideNext(m_bossEnemy);
	m_bossEnemy->ChangeState(next);
}

void BossEnemyState_Idle::StateEnd()
{
}