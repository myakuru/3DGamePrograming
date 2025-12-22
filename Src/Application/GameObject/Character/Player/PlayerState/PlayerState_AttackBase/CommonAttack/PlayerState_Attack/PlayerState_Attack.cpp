#include "PlayerState_Attack.h"
#include "Application/main.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"
#include "Application/Scene/SceneManager.h"
#include "Application/GameObject/Character/Player/PlayerState/Common/PlayerStateServices.h"
#include "Application/GameObject/Utility/EffectReference.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/CommonAttack/PlayerState_Attack1/PlayerState_Attack1.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
	
void PlayerState_Attack::ExposeParametersImGui()
{
	// ベースのパラメータUI
	PlayerState_AttackBase::ExposeParametersImGui();

	// ステート固有（カメラターゲットオフセット）
	if (ImGui::CollapsingHeader("PlayerState_Attack Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::DragFloat3("Target Offset", &m_cameraTargetOffset.x, 0.01f);
		ImGui::DragFloat3("Boss Target Offset", &m_cameraBossTargetOffset.x, 0.01f);
	}
}

void PlayerState_Attack::JsonInput(const nlohmann::json& _js)
{
	if(_js.contains("PlayerState_Attack")) 
	{
		PlayerState_AttackBase::JsonInput(_js["PlayerState_Attack"]);
	}

	// ステート固有（カメラオフセット）
	if (_js.contains("PlayerState_Attack") && _js["PlayerState_Attack"].contains("Camera"))
	{
		const auto& cam = _js["PlayerState_Attack"]["Camera"];
		if (cam.contains("targetOffset"))     m_cameraTargetOffset = JSON_MANAGER.JsonToVector(cam["targetOffset"]);
		if (cam.contains("bossTargetOffset")) m_cameraBossTargetOffset = JSON_MANAGER.JsonToVector(cam["bossTargetOffset"]);
	}
}

void PlayerState_Attack::JsonSave(nlohmann::json& _js) const
{
	PlayerState_AttackBase::JsonSave(_js["PlayerState_Attack"]);

	// ステート固有（カメラオフセット）
	_js["PlayerState_Attack"]["Camera"]["targetOffset"] = JSON_MANAGER.VectorToJson(m_cameraTargetOffset);
	_js["PlayerState_Attack"]["Camera"]["bossTargetOffset"] = JSON_MANAGER.VectorToJson(m_cameraBossTargetOffset);
}

void PlayerState_Attack::OnUpdateAfterDash(Player* _owner)
{
	// 攻撃入力受付
	PlayerInputServices::TryAttack<PlayerState_Attack1>(this, m_lButtonKeyInput);

	// アニメーション終了後、納刀モーションに自動遷移
	PlayerInputServices::TrySheath<PlayerState_SheathKatana>(_owner, this);
}
