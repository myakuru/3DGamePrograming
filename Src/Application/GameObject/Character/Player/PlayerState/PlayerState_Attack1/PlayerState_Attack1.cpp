#include "PlayerState_Attack1.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../../../../../main.h"
#include"../PlayerState_Attack2/PlayerState_Attack2.h"

#include"../../../../../Scene/SceneManager.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"

#include"../PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include"../PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"

#include"../PlayerState_Skill/PlayerState_Skill.h"
#include"Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"

#include"../../../../Weapon/Katana/Katana.h"
#include "Application/GameObject/Utility/EffectReference.h"
#include "Application/GameObject/Character/AfterImage/AfterImage.h"

#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"

void PlayerState_Attack1::ApplyFromConfig(const PlayerStateBase& other)
{
	assert(typeid(other) == typeid(PlayerState_Attack1));
	const auto& p = static_cast<const PlayerState_Attack1&>(other);
	m_stateParameter = p.m_stateParameter;  // 構造体一括コピー
	m_playerEffects = p.m_playerEffects;
}

PlayerState_Attack1::PlayerState_Attack1()
{
}

void PlayerState_Attack1::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("Attack1");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);

	PlayerStateBase::StateStart();

	// 攻撃時はtrueにする
	for (const auto& katanaWeak : m_player->GetKatanas())
	{
		if (auto katana = katanaWeak.lock())
		{
			katana->SetNowAttackState(true);
		}
	}

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	// エフェクト再生・移動停止（複数）
	for (const auto& ref : m_playerEffects)
	{
		if (auto effect = ref->GetEffectBase().lock())
		{
			effect->PlayForTarget<Player>(std::static_pointer_cast<Player>(m_player->GetMyAdls()));
		}
	}

	// カメラの位置を変更
	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt(m_cameraTargetOffset);
	}

	// 残像の設定
	m_player->GetAfterImage()->AddAfterImage(true, 5, 1, Math::Color(1.0f, 1.0f, 0.2f, 1.0f));

	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	KdAudioManager::Instance().Play("Asset/Sound/Player/Attack1.WAV", false)->SetVolume(0.5f);
}

void PlayerState_Attack1::StateUpdate()
{
	UpdateKatanaPos();

	// アニメーション時間のデバッグ表示
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();
	}

	float deltaTime = Application::Instance().GetDeltaTime();

	if (m_attackDirection != Math::Vector3::Zero)
	{
		m_player->UpdateQuaternionDirect(m_attackDirection);
	}

	// 0.5秒間当たり判定有効
	m_player->UpdateAttackCollision(
		m_stateParameter.attackRadius,
		m_stateParameter.attackDistance,
		m_stateParameter.attackCount,
		m_stateParameter.attackInterval,
		m_stateParameter.cameraShake,
		m_stateParameter.cameraTime,
		m_stateParameter.attackStartTime,
		m_stateParameter.attackEndTime
	);

	m_player->UpdateMoveDirectionFromInput();

	// 回避入力処理
	if (UpdateMoveAvoidInput()) return;

	// 必殺技入力処理
	if (UpdateSpecialAttackInput()) return;

	// Eスキル入力処理
	if (UpdateESkillInput()) return;

	// 先行入力の予約
	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
	{
		m_lButtonKeyInput = true;
	}

	if (m_time < m_stateParameter.dashSpeedTime)
	{
		m_player->SetIsMoving(m_attackDirection * m_stateParameter.dashSpeed);
		m_time += deltaTime;
	}
	else
	{
		// 移動を止める
		m_player->SetIsMoving(Math::Vector3::Zero);

		// 攻撃入力受付
		if (m_animeTime >= m_stateParameter.changeStateTime)
		{
			// 攻撃入力処理
			if (UpdateAttackInput<PlayerState_Attack2>()) return;

			// アニメーション終了後の遷移処理
			if (UpdateSheathKatanaInput()) return;
		}
	}

	// 最後に Base 側の StateUpdate を呼び出すことで、フォーカス/方向の追従が反映されます。
	PlayerStateBase::StateUpdate();

}

void PlayerState_Attack1::StateEnd()
{
	PlayerStateBase::StateEnd();

	// エフェクト停止（複数）
	for (const auto& ref : m_playerEffects)
	{
		if (auto effect = ref->GetEffectBase().lock())
		{
			effect->StopEffect();
		}
	}

	m_player->GetAfterImage()->AddAfterImage();
}

void PlayerState_Attack1::ExposeParametersImGui()
{
	// 複数エフェクトの編集UI
	ImGui::Text("Effects");
	for (size_t i = 0; i < m_playerEffects.size(); ++i)
	{
		ImGui::PushID(static_cast<int>(i));
		m_playerEffects[i]->ImGuiInspector("PlayerState_Attack1_Effect");
		ImGui::PopID();
	}
	if (ImGui::SmallButton("+")) { m_playerEffects.emplace_back(std::make_shared<EffectReference>()); }
	if (ImGui::SmallButton("-")) { if (!m_playerEffects.empty()) m_playerEffects.pop_back(); }

	m_stateParameter.ExposeImGui();
}

void PlayerState_Attack1::LoadParametersJson(const nlohmann::json& js)
{
	if (js.contains("PlayerState_Attack1_Effects") && js["PlayerState_Attack1_Effects"].is_array())
	{
		m_playerEffects.clear();
		for (const auto& node : js["PlayerState_Attack1_Effects"])
		{
			auto ref = std::make_shared<EffectReference>();
			ref->JsonInput("Effect", node);
			m_playerEffects.emplace_back(std::move(ref));
		}
	}

	if (!js.contains("PlayerState_Attack1")) return;
	const auto& stateNode = js["PlayerState_Attack1"];
	if (stateNode.contains("Player"))
	{
		m_stateParameter.LoadJson(stateNode["Player"]);
	}
}

void PlayerState_Attack1::SaveParametersJson(nlohmann::json& js) const
{

	nlohmann::json arr = nlohmann::json::array();
	for (const auto& ref : m_playerEffects)
	{
		nlohmann::json item = nlohmann::json::object();
		ref->JsonSave("Effect", item);
		arr.push_back(item);
	}
	js["PlayerState_Attack1_Effects"] = std::move(arr);

	m_stateParameter.SaveJson(js["PlayerState_Attack1"]);
}
