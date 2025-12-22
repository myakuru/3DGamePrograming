#include "PlayerState_JustAvoidAttack_end.h"
#include "Application/GameObject/Character/AfterImage/AfterImage.h"
#include "Application/main.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_SheathKatana/PlayerState_SheathKatana.h"

void PlayerState_JustAvoidAttack_end::OnUpdateAfterDash(Player* _owner)
{
	_owner->SetIsMoving(Math::Vector3::Zero);

	PlayerInputServices::TryAnimationEnd<PlayerState_SheathKatana>(_owner,this);
}

void PlayerState_JustAvoidAttack_end::OnStartExtra(Player* _owner)
{
	// 残像設定
	if (auto afterImage = _owner->GetAfterImage())
	{
		afterImage->AddAfterImage(true, m_params.afterImageMax, m_params.afterImageInterval, m_params.afterImageColor);
	}
}

void PlayerState_JustAvoidAttack_end::OnEndExtra(Player* _owner)
{
	// 敵との当たり判定を有効化（押し出し処理を元に戻す）
	_owner->SetAtkPlayer(false);

	// 無敵解除
	_owner->SetInvincible(false);

	// スローモーション解除（ここを終点にする）
	Application::Instance().SetFpsScale(1.f);
	SceneManager::Instance().SetDrawGrayScale(false);

	// ジャスト回避フラグを戻す
	_owner->SetJustAvoidSuccess(false);
	_owner->SetJustAvoidAttackSuccess(false);

	if (auto afterImage = _owner->GetAfterImage())
	{
		afterImage->AddAfterImage();
	}

	// ガードブレイク状態解除
	_owner->SetGuardBreak(false);

	// ゲームのメインサウンドのピッチを元に戻す
	if (auto bgm = SceneManager::Instance().GetGameSound())
	{
		bgm->SetPitch(0.0f);
	}
}
