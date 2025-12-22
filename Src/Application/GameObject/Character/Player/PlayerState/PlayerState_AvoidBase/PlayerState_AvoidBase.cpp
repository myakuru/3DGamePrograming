#include "PlayerState_AvoidBase.h"
#include "Application/main.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"
#include "Application/GameObject/Character/AfterImage/AfterImage.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"

#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/PlayerState_AvoidAttack/PlayerState_AvoidAttack.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/Charge/JustAvoidAttack/PlayerState_JustAvoidAttack/PlayerState_JustAvoidAttack.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevelBase/PlayerState_ChargeLevel0/PlayerState_ChargeLevel0.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include "Application/GameObject/Character/Player/PlayerConfig.h"

void PlayerState_AvoidBase::StateStart(Player* _owner)
{
	if (_owner && _owner->GetPlayerConfig())
	{
		_owner->GetPlayerConfig()->ApplyPrototypeParametersTo(*this);
	}

	auto anime = _owner->GetAnimeModel()->GetAnimation(GetAnimationName());
	_owner->GetAnimator()->SetAnimation(anime, m_params.blendTime, false);
	_owner->ResetAttackCollision();
	_owner->SetAnimeSpeed(m_params.animationSpeed);
	OnStartExtra(_owner);

	m_afterImagePlayed = false;

	_owner->SetAvoidStartTime(0.0f);

	_owner->SetAvoidFlg(true);

	// 攻撃SE再生
	if (const char* se = GetAttackSE()) { KdAudioManager::Instance().Play(se, false)->SetVolume(1.0f); }
}

void PlayerState_AvoidBase::StateUpdate(Player* _owner)
{
	float deltaTime = Application::Instance().GetUnscaledDeltaTime();
	m_time += deltaTime;

	// 経過時間を Player クラスに伝える
	_owner->SetAvoidStartTime(m_time);

	// ジャスト回避成立時の演出を一度だけ
	if (!m_afterImagePlayed && _owner->GetJustAvoidSuccess())
	{
		KdAudioManager::Instance().Play("Asset/Sound/Player/SlowMotion.WAV", false)->SetVolume(1.0f);

		if (auto bgm = SceneManager::Instance().GetGameSound())
		{
			bgm->SetPitch(-1.0f);
		}

		m_afterImagePlayed = true;

		if (auto camera = _owner->GetPlayerCamera().lock())
		{
			camera->SetTargetLookAt(m_justAvoidCameraOffset);
		}

		// 残像
		_owner->GetAfterImage()->AddAfterImage
		(
			true,
			m_params.afterImageMax,
			m_params.afterImageInterval,
			m_params.afterImageColor
		);

		// 無敵化(アニメーションが終了するまで)
		_owner->SetInvincible(true);
		// 被ヒット判定解除
		_owner->SetHitCheck(false);

		Application::Instance().SetFpsScale(0.1f);
		SceneManager::Instance().SetDrawGrayScale(true);
	}

	// 前方ベクトルを取得
	Math::Vector3 forward = Math::Vector3::TransformNormal(Math::Vector3::Forward, Math::Matrix::CreateFromQuaternion(_owner->GetRotationQuaternion()));
	forward.Normalize();

	if (_owner->GetJustAvoidSuccess())
	{
		PlayerInputServices::TryJustAvoidAttack<PlayerState_JustAvoidAttack>(_owner, this);
	}
	else
	{
		// スローモーション解除
		Application::Instance().SetFpsScale(1.0f);
		SceneManager::Instance().SetDrawGrayScale(false);

		// 回避中に攻撃ボタンが押されたら回避攻撃へ移行
		PlayerInputServices::TryAvoidAttack<PlayerState_AvoidAttack>(this);

		// 必殺技入力処理
		PlayerInputServices::TrySpecialAttack<PlayerState_SpecialAttackCutIn>(_owner, this);

		// Eスキル入力処理
		PlayerInputServices::TryESkill<PlayerState_ChargeLevel0>(_owner, this);
	}

	// アニメーションが終了したらIdleへ移行
	if (_owner->GetAnimator()->IsAnimationEnd())
	{
		// ジャスト回避フラグを戻す
		_owner->SetJustAvoidSuccess(false);

		_owner->SetJustAvoidAttackSuccess(false);

		if (auto bgm = SceneManager::Instance().GetGameSound())
		{
			bgm->SetPitch(0.0f);
		}

		// スローモーション解除
		Application::Instance().SetFpsScale(1.f);
		SceneManager::Instance().SetDrawGrayScale(false);

		PlayerInputServices::TrySheath<PlayerState_SheathKatana>(_owner,this);
	}

	// 刀は鞘の中にある状態
	PlayerStateServices::UpdateUnsheathed(_owner);

	// 回避中の移動処理
	if (m_time < m_params.dashSpeedTime)
	{
		_owner->SetIsMoving(forward * m_params.dashSpeed);
	}
	else
	{
		_owner->SetIsMoving(Math::Vector3::Zero);
	}
}

void PlayerState_AvoidBase::StateEnd(Player* _owner)
{
	if (auto camera = _owner->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt(m_endCameraOffset);
	}

	_owner->SetAvoidFlg(false);

	// Just回避時間のリセット
	_owner->SetAvoidStartTime(0.0f);

	// 被ヒット判定解除
	_owner->SetHitCheck(false);

	// 残像解除
	_owner->GetAfterImage()->AddAfterImage();

	// 敵との当たり判定を無効化解除（押し出し処理を元に戻す）
	_owner->SetAtkPlayer(false);

	// 無敵解除
	_owner->SetInvincible(false);
}

void PlayerState_AvoidBase::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_params.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_params.animationSpeed);
	ImGui::Separator();
	ImGui::DragFloat(U8("ダッシュ速度"), &m_params.dashSpeed);
	ImGui::DragFloat(U8("ダッシュ速度時間"), &m_params.dashSpeedTime);
	ImGui::Separator();
	ImGui::DragFloat3(U8("カメラ注視点オフセット"), &m_startCameraOffset.x);
	ImGui::DragFloat3(U8("ボス戦時カメラ注視点オフセット"), &m_startBossCameraOffset.x);
	ImGui::DragFloat3(U8("ジャスト回避成功時カメラ注視点オフセット"), &m_justAvoidCameraOffset.x);
	ImGui::DragFloat3(U8("回避終了時カメラ注視点オフセット"), &m_endCameraOffset.x);
	ImGui::Separator();
	// 残像設定
	ImGui::DragInt(U8("残像最大数"), &m_params.afterImageMax, 1.0f, 1, 20);
	ImGui::DragFloat(U8("残像生成間隔"), &m_params.afterImageInterval, 0.01f, 0.01f, 1.0f);
	ImGui::ColorEdit4(U8("残像色"), &m_params.afterImageColor.x);
}
