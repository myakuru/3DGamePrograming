#pragma once

enum class PlayerStateId
{
	Attack,
	Attack1,
	Attack2,
	JustAvoid
};

enum class EnemyStateId
{
	Idle
};

struct IPlayerStateParameter
{
	virtual ~IPlayerStateParameter() = default;
	virtual PlayerStateId Id() const = 0;
	virtual void LoadJson(const nlohmann::json& js) = 0;
	virtual void SaveJson(nlohmann::json& js) const = 0;
	virtual void OnImGui() = 0;
};

// ------- 各パラメータ -------

struct PlayerState_AttackParameter : IPlayerStateParameter
{
	float m_dashTimer = 0.0f;
	PlayerStateId Id() const override { return PlayerStateId::Attack; }
	void LoadJson(const nlohmann::json& js) override
	{
		if (js.contains("DashTime")) m_dashTimer = js["DashTime"].get<float>();
	}
	void SaveJson(nlohmann::json& js) const override
	{
		js["DashTime"] = m_dashTimer;
	}
	void OnImGui() override
	{
		ImGui::Text(U8("アタックステート"));
		ImGui::DragFloat(U8("DashTime"), &m_dashTimer, 0.01f);
	}
};

struct PlayerState_Attack1Parameter : IPlayerStateParameter
{
	float m_dashTimer = 0.0f;
	PlayerStateId Id() const override { return PlayerStateId::Attack1; }
	void LoadJson(const nlohmann::json& js) override
	{
		if (js.contains("DashTime1")) m_dashTimer = js["DashTime1"].get<float>();
	}
	void SaveJson(nlohmann::json& js) const override
	{
		js["DashTime1"] = m_dashTimer;
	}
	void OnImGui() override
	{
		ImGui::Text(U8("アタック1ステート"));
		ImGui::DragFloat(U8("DashTime1"), &m_dashTimer, 0.01f);
	}
};

struct PlayerState_Attack2Parameter : IPlayerStateParameter
{
	float m_dashTimer = 0.0f;
	PlayerStateId Id() const override { return PlayerStateId::Attack2; }
	void LoadJson(const nlohmann::json& js) override
	{
		if (js.contains("DashTime2")) m_dashTimer = js["DashTime2"].get<float>();
	}
	void SaveJson(nlohmann::json& js) const override
	{
		js["DashTime2"] = m_dashTimer;
	}
	void OnImGui() override
	{
		ImGui::Text(U8("アタック2ステート"));
		ImGui::DragFloat(U8("DashTime2"), &m_dashTimer, 0.01f);
	}
};

struct JustAvoidParameter : IPlayerStateParameter
{
	float m_slowMoScale = 0.1f;
	bool  m_useGrayScale = true;
	PlayerStateId Id() const override { return PlayerStateId::JustAvoid; }
	void LoadJson(const nlohmann::json& js) override
	{
		if (js.contains("JustSlowMoScale")) m_slowMoScale = js["JustSlowMoScale"].get<float>();
		if (js.contains("JustGrayScale"))   m_useGrayScale = js["JustGrayScale"].get<bool>();
	}
	void SaveJson(nlohmann::json& js) const override
	{
		js["JustSlowMoScale"] = m_slowMoScale;
		js["JustGrayScale"] = m_useGrayScale;
	}
	void OnImGui() override
	{
		ImGui::Text(U8("ジャスト回避"));
		ImGui::DragFloat(U8("JustSlowMoScale"), &m_slowMoScale, 0.01f, 0.05f, 1.0f);
		ImGui::Checkbox(U8("JustGrayScale"), &m_useGrayScale);
	}
};

// ------- ファクトリー -------
class PlayerStateParameterFactory
{
public:

	static PlayerStateParameterFactory& Instance()
	{
		static PlayerStateParameterFactory inst;
		return inst;
	}

	template <class T>
	void PlayerStateRegister(PlayerStateId id)
	{
		m_creators[id] = [] { return std::make_unique<T>(); };
	}

	std::unique_ptr<IPlayerStateParameter> Create(PlayerStateId id) const
	{
		auto it = m_creators.find(id);
		if (it == m_creators.end()) return nullptr;
		return it->second();
	}

private:
	std::unordered_map<PlayerStateId, std::function<std::unique_ptr<IPlayerStateParameter>()>> m_creators;


	PlayerStateParameterFactory()
	{
		PlayerStateRegister<PlayerState_AttackParameter>(PlayerStateId::Attack);
		PlayerStateRegister<PlayerState_Attack1Parameter>(PlayerStateId::Attack1);
		PlayerStateRegister<PlayerState_Attack2Parameter>(PlayerStateId::Attack2);
		PlayerStateRegister<JustAvoidParameter>(PlayerStateId::JustAvoid);
	}
};