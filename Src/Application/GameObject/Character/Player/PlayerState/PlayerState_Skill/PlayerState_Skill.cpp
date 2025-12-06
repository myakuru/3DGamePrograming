#include "PlayerState_Skill.h"
#include"../../../../../main.h"
#include"../PlayerState_Run/PlayerState_Run.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../../../../../Scene/SceneManager.h"
#include"Application\GameObject\Character\AfterImage\AfterImage.h"

#include "Application/GameObject/Utility/EffectReference.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"
	
void PlayerState_Skill::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("Eskill");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	

	PlayerStateBase::StateStart();

	KdAudioManager::Instance().Play("Asset/Sound/Player/Eskill.WAV", false)->SetVolume(1.0f);

	// 敵との当たり判定を無効化
	m_player->SetAtkPlayer(true);

	// 残像の設定
	m_player->GetAfterImage()->AddAfterImage(true, 10, 0.05f, { 0.0f,0.5f,1.0f,0.5f });

	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);
}

void PlayerState_Skill::StateUpdate()
{

}

void PlayerState_Skill::StateEnd()
{
	PlayerStateBase::StateEnd();

	// 敵との当たり判定をもとに戻す
	m_player->SetAtkPlayer(false);

	// 残像のリセット
	m_player->GetAfterImage()->AddAfterImage();
}
