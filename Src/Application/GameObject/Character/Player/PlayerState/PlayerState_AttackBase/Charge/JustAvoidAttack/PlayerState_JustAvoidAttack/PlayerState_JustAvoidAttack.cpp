#include "PlayerState_JustAvoidAttack.h"
#include "Application/GameObject/Character/AfterImage/AfterImage.h"
#include "Application/main.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/Charge/JustAvoidAttack/PlayerState_JustAvoidAttack_end/PlayerState_JustAvoidAttack_end.h"

void PlayerState_JustAvoidAttack::OnUpdateAfterDash(Player* _owner)
{
	// アニメーション終了後の遷移
	PlayerInputServices::TryAnimationEnd<PlayerState_JustAvoidAttack_end>(_owner, this);
}

void PlayerState_JustAvoidAttack::OnStartExtra(Player* _owner)
{
	// 残像設定
	if (auto afterImage = _owner->GetAfterImage())
	{
		afterImage->AddAfterImage(true, m_params.afterImageMax, m_params.afterImageInterval, m_params.afterImageColor);
	}

	// ガードブレイク状態にする
	_owner->SetGuardBreak(true);

	// 被ヒット判定無効化
	_owner->SetHitCheck(false);

	// 敵との当たり判定を無効化
	_owner->SetAtkPlayer(true);

	// 無敵状態にする
	_owner->SetInvincible(true);
}