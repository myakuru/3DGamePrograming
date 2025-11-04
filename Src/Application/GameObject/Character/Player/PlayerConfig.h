#pragma once

struct PlayerState_AttackParameter
{
	float m_dashTimer = 0.0f; // ダッシュのタイマー
};
struct PlayerState_Attack1Parameter
{
	float m_dashTimer = 0.0f; // ダッシュのタイマー
};

struct PlayerState_Attack2Parameter
{
	float m_dashTimer = 0.0f; // ダッシュのタイマー
};

struct JustAvoidParameter
{
	float m_slowMoScale = 0.3f;   // ジャスト回避時のスローモーション倍率
	bool  m_useGrayScale = true;  // グレースケール適用フラグ
};

class PlayerConfig
{
public:
	PlayerConfig() = default;
	~PlayerConfig() = default;

	void InGuiInspector()
	{
		{
			ImGui::Text(U8("アタックステート"));
			ImGui::DragFloat(U8("DashTime"), &m_playerState_AttackParam.m_dashTimer, 0.01f);
		}

		ImGui::Separator();
		{
			ImGui::Text(U8("アタック1ステート"));
			ImGui::DragFloat(U8("DashTime1"), &m_playerState_Attack1Param.m_dashTimer, 0.01f);
		}

		ImGui::Separator();
		{
			ImGui::Text(U8("アタック2ステート"));
			ImGui::DragFloat(U8("DashTime2"), &m_playerState_Attack2Param.m_dashTimer, 0.01f);
		}

		ImGui::Separator();
		{
			ImGui::Text(U8("ジャスト回避"));
			ImGui::DragFloat(U8("JustSlowMoScale"), &m_justAvoidParam.m_slowMoScale, 0.01f, 0.05f, 1.0f);
			ImGui::Checkbox(U8("JustGrayScale"), &m_justAvoidParam.m_useGrayScale);
		}
	}

	void JsonInput(const nlohmann::json& _json)
	{
		if (_json.contains("DashTime"))
		{
			m_playerState_AttackParam.m_dashTimer = _json["DashTime"].get<float>();
			m_playerState_Attack1Param.m_dashTimer = _json["DashTime1"].get<float>();
			m_playerState_Attack2Param.m_dashTimer = _json["DashTime2"].get<float>();
		}

		if (_json.contains("JustSlowMoScale")) m_justAvoidParam.m_slowMoScale = _json["JustSlowMoScale"].get<float>();
		if (_json.contains("JustGrayScale"))   m_justAvoidParam.m_useGrayScale = _json["JustGrayScale"].get<bool>();
	}

	void JsonSave(nlohmann::json& _json) const
	{
		_json["DashTime"] = m_playerState_AttackParam.m_dashTimer;
		_json["DashTime1"] = m_playerState_Attack1Param.m_dashTimer;
		_json["DashTime2"] = m_playerState_Attack2Param.m_dashTimer;

		_json["JustSlowMoScale"] = m_justAvoidParam.m_slowMoScale;
		_json["JustGrayScale"] = m_justAvoidParam.m_useGrayScale;
	}

	PlayerState_AttackParameter& GetAttackParam() { return m_playerState_AttackParam; }
	PlayerState_Attack1Parameter& GetAttack1Param() { return m_playerState_Attack1Param; }
	PlayerState_Attack2Parameter& GetAttack2Param() { return m_playerState_Attack2Param; }
	JustAvoidParameter& GetJustAvoidParam() { return m_justAvoidParam; }

private:
	PlayerState_AttackParameter  m_playerState_AttackParam;   // 攻撃ステートのパラメータ
	PlayerState_Attack1Parameter m_playerState_Attack1Param;  // 攻撃1ステートのパラメータ
	PlayerState_Attack2Parameter m_playerState_Attack2Param;  // 攻撃2ステートのパラメータ
	JustAvoidParameter           m_justAvoidParam;            // ジャスト回避のパラメータ
};