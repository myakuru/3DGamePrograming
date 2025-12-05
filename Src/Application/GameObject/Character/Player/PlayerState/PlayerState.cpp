#include "PlayerState.h"
#include"Application/GameObject/Weapon/Katana/Katana.h"
#include"Application/GameObject/Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include"Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"
#include"Application/Scene/SceneManager.h"
#include"Application/main.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SpecialAttack/PlayerState_SpecialAttack.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_FowardAvoidFast/PlayerState_FowardAvoidFast.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevel0/PlayerState_ChargeLevel0.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"

#include"Application/Data/CharacterData/CharacterData.h"

#include "Application/GameObject/Utility/EffectReference.h"

PlayerStateBase::PlayerStateBase()
{
	m_effect = std::make_shared<EffectReference>();
}

PlayerStateBase::~PlayerStateBase() = default;

void PlayerStateBase::StateStart()
{
	// まずフォーカスが生きていればそれを採用
	if (auto f = m_focusTarget.lock())
	{
		m_nearestEnemy = f;
		m_nearestEnemyPos = f->GetPos();
	}
	else
	{
		// 近い敵を検索（毎回リセットして選ぶ）
		std::list<std::weak_ptr<KdGameObject>> candidates;
		SceneManager::Instance().GetObjectWeakPtrListByTagInSphere(
			ObjTag::EnemyLike, m_player->GetPos(), candidates, m_searchEnemyRadius);

		float minSq = std::numeric_limits<float>::max();
		std::shared_ptr<KdGameObject> nearest;
		Math::Vector3 nearestPos = Math::Vector3::Zero;

		for (const auto& w : candidates)
		{
			auto enemy = w.lock();
			if (!enemy) continue;
			const float distance = (enemy->GetPos() - m_player->GetPos()).LengthSquared();
			if (distance < minSq)
			{
				minSq = distance;
				nearestPos = enemy->GetPos();
				nearest = enemy;
			}
		}

		m_minDistSq = minSq;
		m_nearestEnemy = nearest;
		m_nearestEnemyPos = nearestPos;

		// 新規フォーカス確定
		if (m_nearestEnemy)
		{
			m_focusTarget = m_nearestEnemy;
			m_focusRemainSec = m_focusDurationSec;
		}
	}

	// 攻撃方向決定（以降は StateUpdate で追従更新あり）
	if (m_nearestEnemy)
	{
		m_attackDirection = m_nearestEnemyPos - m_player->GetPos();
		m_attackDirection.y = 0.0f;
		if (m_attackDirection != Math::Vector3::Zero)
		{
			m_attackDirection.Normalize();
			m_player->UpdateQuaternionDirect(m_attackDirection);
		}
	}
	else
	{
		m_attackDirection = m_player->GetLastMoveDirection();
		if (m_attackDirection != Math::Vector3::Zero)
		{
			m_player->UpdateQuaternion(m_attackDirection);
		}
	}

	// 刀初期化など（既存）
	for (const auto& katanaWeak : m_player->GetKatanas())
	{
		if (auto katana = katanaWeak.lock()) katana->SetNowAttackState(false);
	}

	m_lButtonKeyInput = false;
	m_time = 0.0f;
	m_animeTime = 0.0f;
}

void PlayerStateBase::StateUpdate()
{
	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	// ヒットが発生していれば、その敵をフォーカスに設定してタイマーを全回復
	if (auto lastHit = m_player->GetLastHitEnemy().lock())
	{
		m_focusTarget = lastHit;
		m_focusRemainSec = m_focusDurationSec;

		// 現在のターゲット/攻撃方向も更新
		m_nearestEnemy = lastHit;
		m_nearestEnemyPos = lastHit->GetPos();

		// 使用後は一旦クリア（次回ヒットで再設定）
		m_player->ClearLastHitEnemy();
	}

	// フォーカス中は攻撃方向をターゲットへ追従
	if (auto f = m_focusTarget.lock())
	{
		Math::Vector3 dir = f->GetPos() - m_player->GetPos();
		dir.y = 0.0f;
		if (dir.LengthSquared() > 1e-6f)
		{
			dir.Normalize();
			m_attackDirection = dir;
		}
	}

	// フォーカスタイマー更新と自動解除
	if (m_focusRemainSec > 0.0f)
	{
		m_focusRemainSec -= deltaTime;
		if (m_focusRemainSec <= 0.0f)
		{
			m_focusRemainSec = 0.0f;
			m_focusTarget.reset();
		}
		else
		{
			if (auto f = m_focusTarget.lock())
			{
				if (f->IsExpired())
				{
					m_focusRemainSec = 0.0f;
					m_focusTarget.reset();
				}
			}
			else
			{
				m_focusRemainSec = 0.0f;
			}
		}
	}
}

void PlayerStateBase::StateEnd()
{
	// カタナの取得
	for (const auto& katanaWeak : m_player->GetKatanas())
	{
		if (auto katana = katanaWeak.lock())
		{
			katana->SetNowAttackState(false);
		}
	}
}

// 刀の位置を右手に追従するように更新
void PlayerStateBase::UpdateKatanaPos()
{
	// 右手持ち設定
	m_player->SetRightHanded(true);

	// 右手のワークノードを取得
	auto rightHandNode = m_player->GetModelWork()->FindWorkNode("Katana");
	// 左手のワークノードを取得
	auto leftHandNode = m_player->GetModelWork()->FindWorkNode("Sheath");

	if (!rightHandNode) return;
	if (!leftHandNode) return;

	// カタナの取得
	for (const auto& katanaWeak : m_player->GetKatanas())
	{
		if (auto katana = katanaWeak.lock())
		{
			katana->SetHandKatanaMatrix(rightHandNode->m_worldTransform);
		}
	}

	// 鞘の取得
	for (const auto& sheathWeak : m_player->GetKatanaSheaths())
	{
		if (auto sheath = sheathWeak.lock())
		{
			sheath->SetHandKatanaMatrix(leftHandNode->m_worldTransform);
		}
	}
}

// 刀と鞘の位置が左手に追従するように更新
void PlayerStateBase::UpdateUnsheathed()
{
	m_player->SetRightHanded(false);

	// 左手のワークノードを取得
	auto leftHandNode = m_player->GetModelWork()->FindWorkNode("Sheath");

	if (!leftHandNode) return;

	// カタナの取得
	for (const auto& katanaWeak : m_player->GetKatanas())
	{
		if (auto katana = katanaWeak.lock())
		{
			katana->SetHandKatanaMatrix(leftHandNode->m_worldTransform);
		}
	}

	// 鞘の取得
	for (const auto& sheathWeak : m_player->GetKatanaSheaths())
	{
		if (auto sheath = sheathWeak.lock())
		{
			sheath->SetHandKatanaMatrix(leftHandNode->m_worldTransform);
		}
	}
}

// 必殺技入力関連
bool PlayerStateBase::UpdateSpecialAttackInput()
{
	if (KeyboardManager::GetInstance().IsKeyJustPressed('Q'))
	{
		if (m_player->GetStatus().GetPlayerStatus().specialPoint == m_player->GetStatus().GetPlayerStatus().specialPointMax)
		{
			m_player->SetStatus().SetPlayerStatus().specialPoint = 0;
			auto specialAttackState = std::make_shared<PlayerState_SpecialAttackCutIn>();
			m_player->ChangeState(specialAttackState);
			return true;
		}
	}

	return false;
}

// 回避入力関連
bool PlayerStateBase::UpdateMoveAvoidInput()
{
	// 回避関係
	{
		const float kShortPressMin = 0.1f;       // 短押し有効開始
		const float kLongPressThreshold = 0.2f;  // 長押し閾値

		float rDuration = KeyboardManager::GetInstance().GetKeyPressDuration(VK_RBUTTON);

		// 押された瞬間
		if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_RBUTTON))
		{
			m_rButtonKeyInput = true; // 判定開始
		}

		// 長押し判定
		if (m_rButtonKeyInput &&
			rDuration >= kLongPressThreshold && 
			!KeyboardManager::GetInstance().IsKeyJustReleased(VK_RBUTTON))
		{
			m_rButtonKeyInput = false;
			auto avoidFast = std::make_shared<PlayerState_FowardAvoidFast>();
			m_player->ChangeState(avoidFast);
			return true;
		}

		// 短押し判定
		if (m_rButtonKeyInput && KeyboardManager::GetInstance().IsKeyJustReleased(VK_RBUTTON) && !m_player->GetIsMoving())
		{
			m_rButtonKeyInput = false;
			auto backAvoid = std::make_shared<PlayerState_BackWordAvoid>();
			m_player->ChangeState(backAvoid);
			return true;
		}

		if (m_rButtonKeyInput &&
			KeyboardManager::GetInstance().IsKeyJustReleased(VK_RBUTTON) &&
			m_player->GetIsMoving()
			)
		{
			if (rDuration >= kShortPressMin && rDuration < kLongPressThreshold)
			{
				m_rButtonKeyInput = false;
				auto backAvoid = std::make_shared<PlayerState_FowardAvoid>();
				m_player->ChangeState(backAvoid);
				return true;
			}

			// 0.1秒未満なら何もしない
			m_rButtonKeyInput = false;
		}

	}

	return false;
}

// 刀を鞘に納める入力関連
bool PlayerStateBase::UpdateSheathKatanaInput()
{
	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto sheath = std::make_shared<PlayerState_SheathKatana>();
		m_player->ChangeState(sheath);
		return true;
	}

	return false;
}

// Eスキル入力関連
bool PlayerStateBase::UpdateESkillInput()
{
	if (KeyboardManager::GetInstance().IsKeyJustPressed('E'))
	{

		if (m_player->GetStatus().GetPlayerStatus().skillPoint >= m_player->GetStatus().GetPlayerStatus().skillPointMax)
		{
			m_player->SetStatus().SetPlayerStatus().skillPoint = 0;
			auto specialAttackState = std::make_shared<PlayerState_ChargeLevel0>();
			m_player->ChangeState(specialAttackState);
			return true;
		}
	}

	return false;
}

// パラメータ編集
void PlayerStateBase::StateParameter::ExposeImGui()
{
	// 簡易ヘルプツールチップ
	auto Help = [](const char* desc)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(desc);
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
		};

	ImGui::PushID("PlayerStateParams");
	const float kLabelWidth = 160.0f;
	const float kItemWidth = 180.0f;

	// 当たり判定
	if (ImGui::CollapsingHeader(U8("当たり判定"), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::BeginTable("tbl_hit", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, kLabelWidth);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

			// 半径
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("当たり判定の半径"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderFloat("##attackRadius", &attackRadius, 0.0f, 10.0f, "%.2f");
			Help(U8("攻撃の判定半径。0で無効。"));

			// 距離
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("当たり判定の距離"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderFloat("##attackDistance", &attackDistance, 0.0f, 10.0f, "%.2f");
			Help(U8("攻撃中心からの到達距離。"));

			// 回数
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("当たり判定の回数"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderInt("##attackCount", &attackCount, 0, 10);
			Help(U8("攻撃を複数回ヒットさせる場合の回数。"));

			// 間隔
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("当たり判定の間隔(秒)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderFloat("##attackInterval", &attackInterval, 0.0f, 1.0f, "%.03f");
			Help(U8("複数回判定する際の間隔。"));

			// 開始/終了時間（同時編集）
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("当たり判定の時間範囲(秒)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth * 1.3f);
			ImGui::DragFloatRange2("##attackTimeRange", &attackStartTime, &attackEndTime, 0.01f, 0.0f, 10.0f,
				U8("開始: %.02f"), U8("終了: %.02f"));
			if (attackEndTime < attackStartTime) attackEndTime = attackStartTime;
			Help(U8("アニメ時間に対する有効区間。終了は開始以上に自動補正されます。"));

			ImGui::EndTable();
		}
	}

	// カメラ
	if (ImGui::CollapsingHeader(U8("カメラ")))
	{
		if (ImGui::BeginTable("tbl_camera", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, kLabelWidth);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

			// 揺れパラメータ
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("カメラ揺れ(幅, 周波数)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::DragFloat2("##cameraShake", &cameraShake.x, 0.01f, 0.0f, 5.0f, "%.02f");
			Help(U8("X: 振幅, Y: 周波数など用途に合わせて解釈。0で無効。"));

			// 揺れ時間
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("カメラ揺れ時間(秒)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::DragFloat("##cameraTime", &cameraTime);

			ImGui::EndTable();
		}
	}

	// 移動/ダッシュ
	if (ImGui::CollapsingHeader(U8("移動 / ダッシュ")))
	{
		if (ImGui::BeginTable("tbl_move", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, kLabelWidth);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

			// 移動速度
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("移動速度 (X,Y,Z)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth * 1.2f);
			ImGui::DragFloat3("##moveSpeed", &moveSpeed.x, 0.01f, -20.0f, 20.0f, "%.02f");

			// ダッシュ速度
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("ダッシュ速度"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::DragFloat("##dashSpeed", &dashSpeed);

			// ダッシュ速度時間
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("ダッシュ速度時間(秒)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::DragFloat("##dashSpeedTime", &dashSpeedTime);

			ImGui::EndTable();
		}
	}

	// アニメ / 状態遷移
	if (ImGui::CollapsingHeader(U8("アニメ / 状態遷移")))
	{
		if (ImGui::BeginTable("tbl_anim", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, kLabelWidth);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

			// ブレンド時間
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("ブレンド時間(秒)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderFloat("##blendTime", &blendTime, 0.0f, 2.0f, "%.02f");

			// アニメーション速度
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("アニメーション速度"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderFloat("##animationSpeed", &animationSpeed, 0.0f, 200.0f, "%.0f");

			// ステートの切り替え
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("ステートの切り替え"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderFloat("##changeStateTime", &changeStateTime, 0.0f, 5.0f, "%.02f");

			ImGui::EndTable();
		}
	}

	ImGui::PopID();
}

void PlayerStateBase::StateParameter::LoadJson(const nlohmann::json& pj)
{
	if (pj.contains("AttackRadius"))    attackRadius = pj["AttackRadius"].get<float>();
	if (pj.contains("AttackDistance"))  attackDistance = pj["AttackDistance"].get<float>();
	if (pj.contains("AttackCount"))     attackCount = pj["AttackCount"].get<int>();
	if (pj.contains("AttackTime"))      attackInterval = pj["AttackTime"].get<float>();
	if (pj.contains("AttackStartTime")) attackStartTime = pj["AttackStartTime"].get<float>();
	if (pj.contains("AttackEndTime"))   attackEndTime = pj["AttackEndTime"].get<float>();
	if (pj.contains("MoveSpeedX"))      moveSpeed.x = pj["MoveSpeedX"].get<float>();
	if (pj.contains("MoveSpeedY"))      moveSpeed.y = pj["MoveSpeedY"].get<float>();
	if (pj.contains("MoveSpeedZ"))      moveSpeed.z = pj["MoveSpeedZ"].get<float>();
	if (pj.contains("CameraShakeX"))    cameraShake.x = pj["CameraShakeX"].get<float>();
	if (pj.contains("CameraShakeY"))    cameraShake.y = pj["CameraShakeY"].get<float>();
	if (pj.contains("CameraShakeTime")) cameraTime = pj["CameraShakeTime"].get<float>();
	if (pj.contains("DashSpeed"))       dashSpeed = pj["DashSpeed"].get<float>();
	if (pj.contains("BleedTime"))      blendTime = pj["BleedTime"].get<float>();
	if (pj.contains("AnimationSpeed")) animationSpeed = pj["AnimationSpeed"].get<float>();
	if (pj.contains("DashSpeedTime"))  dashSpeedTime = pj["DashSpeedTime"].get<float>();
	if (pj.contains("ChangeStateTime")) changeStateTime = pj["ChangeStateTime"].get<float>();
}

void PlayerStateBase::StateParameter::SaveJson(nlohmann::json& js) const
{
	js["Player"]["AttackRadius"] = attackRadius;
	js["Player"]["AttackDistance"] = attackDistance;
	js["Player"]["AttackCount"] = attackCount;
	js["Player"]["AttackTime"] = attackInterval;
	js["Player"]["AttackStartTime"] = attackStartTime;
	js["Player"]["AttackEndTime"] = attackEndTime;
	js["Player"]["MoveSpeedX"] = moveSpeed.x;
	js["Player"]["MoveSpeedY"] = moveSpeed.y;
	js["Player"]["MoveSpeedZ"] = moveSpeed.z;
	js["Player"]["CameraShakeX"] = cameraShake.x;
	js["Player"]["CameraShakeY"] = cameraShake.y;
	js["Player"]["CameraShakeTime"] = cameraTime;
	js["Player"]["DashSpeed"] = dashSpeed;
	js["Player"]["BleedTime"] = blendTime;
	js["Player"]["AnimationSpeed"] = animationSpeed;
	js["Player"]["DashSpeedTime"] = dashSpeedTime;
	js["Player"]["ChangeStateTime"] = changeStateTime;
}
