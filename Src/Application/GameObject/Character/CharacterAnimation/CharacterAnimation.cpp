#include "CharacterAnimation.h"
#include "Application/GameObject/Character/CharacterBase.h"

std::shared_ptr<KdAnimator> CharacterAnimation::GetAnimator()
{
	if (!m_owner) return nullptr;
	return m_owner->GetAnimatorShared();
}

std::shared_ptr<KdModelWork> CharacterAnimation::GetAnimeModel()
{
	if (!m_owner) return nullptr;
	// KdGameObject 内部の m_modelWork を利用
	return m_owner->GetModelWork(); // 既存の取得関数がある場合に合わせて書き換えてください
}

void CharacterAnimation::SetAnimeSpeed(float _speed)
{
	auto animator = GetAnimator();
	if (!animator) return;
	// 速度適用ロジックは元の実装に応じて移植
	// 例：AdvanceTime 呼び出し時の speed 引数へ渡すなど
	// ここでは保持のみを想定し、CharacterBase 側で使用
	(void)_speed;
}