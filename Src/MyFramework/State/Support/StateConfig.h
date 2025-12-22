#pragma once

// デフォルトの ImGui 編集関数（ADL による発見用、必要な型だけオーバーロード/特殊化する）
template <class Params>
inline void ExposeImGuiParams(Params&) {} // 既定は何もしない

// ステート用コンフィグ
template <class Params>
class StateConfig
{
public:
	Params& Get()       noexcept { return m_data; }
	const Params& Get() const noexcept { return m_data; }

	void ApplyFrom(const Params& other) { m_data = other; }

	void ExposeImGui() { ExposeImGuiParams(m_data); }

	void LoadJson(const nlohmann::json& js) const
	{
		(void)js;
	}

	void SaveJson(const nlohmann::json& js) const
	{
		(void)js;
	}

private:
	Params m_data{};
};