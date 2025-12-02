#pragma once
#include "Application/GameObject/Character/CharacterBaseFwd.h"

class CharacterBase;

// アニメーション専用コンポーネント
class CharacterAnimation
{
public:
	CharacterAnimation() = default;

	void SetOwner(CharacterBase* owner) { m_owner = owner; }

	std::shared_ptr<KdAnimator> GetAnimator();
	std::shared_ptr<KdModelWork> GetAnimeModel();
	void SetAnimeSpeed(float _speed);

private:
	CharacterBase* m_owner = nullptr;
};