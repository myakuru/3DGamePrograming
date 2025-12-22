#include "PlayerState_Hit.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"
#include "Application/main.h"

void PlayerState_Hit::OnStartExtra(Player* owner)
{
	// 受け身中は刀の表示や向きが崩れないように
	PlayerStateServices::UpdateKatanaPos(owner);
}

void PlayerState_Hit::OnUpdateExtra(Player* owner, float /*dt*/)
{
	// 被弾中は移動しない
	owner->SetIsMoving(Math::Vector3::Zero);

	// 必要なら向き固定、または攻撃方向へ向ける等
	// ここではそのまま
}

void PlayerState_Hit::OnEndExtra(Player* owner)
{
}