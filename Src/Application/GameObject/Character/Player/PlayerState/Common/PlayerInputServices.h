#pragma once
#include "MyFramework/Manager/KeyboardManager/KeyboardManager.h"
#include "MyFramework/State/StateBase/StateBase.h"
#include "Application/GameObject/Character/Player/Player.h"
#include "Application/Data/CharacterData/CharacterData.h"

namespace PlayerInputServices {

	// 必殺技（Q）
	template<class StateCutIn>
	inline void TrySpecialAttack(Player* owner, StateBase<Player>* current)
	{
		if (KeyboardManager::GetInstance().IsKeyJustPressed('Q'))
		{
			const auto& st = owner->GetStatus().GetPlayerStatus();
			if (st.specialPoint == st.specialPointMax)
			{
				owner->SetStatus().SetPlayerStatus().specialPoint = 0;
				current->GetStateMachine()->template ChangeState<StateCutIn>();
			}
		}
	}

	// Eスキル（E）
	template<class StateChargeStart>
	inline void TryESkill(Player* owner, StateBase<Player>* current)
	{
		if (KeyboardManager::GetInstance().IsKeyJustPressed('E'))
		{
			const auto& st = owner->GetStatus().GetPlayerStatus();
			if (st.skillPoint >= st.skillPointMax)
			{
				owner->SetStatus().SetPlayerStatus().skillPoint = 0;
				current->GetStateMachine()->template ChangeState<StateChargeStart>();
			}
		}
	}

	// 回避（RBUTTON 短押し/長押し）
	template<class StateForwardAvoidFast, class StateBackAvoid, class StateForwardAvoid>
	inline void TryAvoid(Player* owner, StateBase<Player>* current, bool& rButtonKeyInput)
	{
		constexpr float kShortPressMin = 0.1f;
		constexpr float kLongPressThreshold = 0.2f;

		const auto& KM = KeyboardManager::GetInstance();
		const float rDuration = KM.GetKeyPressDuration(VK_RBUTTON);

		if (KM.IsKeyJustPressed(VK_RBUTTON))
			rButtonKeyInput = true;

		if (rButtonKeyInput && rDuration >= kLongPressThreshold && !KM.IsKeyJustReleased(VK_RBUTTON))
		{
			rButtonKeyInput = false;
			current->GetStateMachine()->template ChangeState<StateForwardAvoidFast>();
		}

		if (rButtonKeyInput && KM.IsKeyJustReleased(VK_RBUTTON) && !owner->GetIsMoving())
		{
			rButtonKeyInput = false;
			current->GetStateMachine()->template ChangeState<StateBackAvoid>();
		}

		if (rButtonKeyInput && KM.IsKeyJustReleased(VK_RBUTTON) && owner->GetIsMoving())
		{
			if (rDuration >= kShortPressMin && rDuration < kLongPressThreshold)
			{
				rButtonKeyInput = false;
				current->GetStateMachine()->template ChangeState<StateForwardAvoid>();
			}
			rButtonKeyInput = false;
		}
	}

	// Just回避Attack
	template<class StateJustAvoidAttack>
	inline void TryJustAvoidAttack(Player* _owner, StateBase<Player>* _current)
	{
		if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
		{
			_owner->SetJustAvoidAttackSuccess(true);
			_current->GetStateMachine()->template ChangeState<StateJustAvoidAttack>();
		}
	}

	// 攻撃（LButton フラグ）
	template<class StateAttack>
	inline void TryAttack(StateBase<Player>* current, bool& lButtonKeyInput)
	{
		if (lButtonKeyInput)
		{
			lButtonKeyInput = false;
			current->GetStateMachine()->template ChangeState<StateAttack>();
		}
	}

	// 回避アタック
	template<class StateAvoidAttack>
	inline void TryAvoidAttack(StateBase<Player>* current)
	{
		if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
		{
			current->GetStateMachine()->template ChangeState<StateAvoidAttack>();
		}
	}

	// チェンジするだけ
	template<class StateChange>
	inline void TryChange(StateBase<Player>* current)
	{
		current->GetStateMachine()->template ChangeState<StateChange>();
	}

	// アニメーション終了判定
	template<class StateNext>
	inline void TryAnimationEnd(Player* owner, StateBase<Player>* current)
	{
		if (owner->GetAnimator()->IsAnimationEnd())
		{
			current->GetStateMachine()->template ChangeState<StateNext>();
		}
	}

	// 納刀（アニメ終了）
	template<class StateSheath>
	inline void TrySheath(Player* owner, StateBase<Player>* current)
	{
		if (owner->GetAnimator()->IsAnimationEnd())
		{
			current->GetStateMachine()->template ChangeState<StateSheath>();
		}
	}

}