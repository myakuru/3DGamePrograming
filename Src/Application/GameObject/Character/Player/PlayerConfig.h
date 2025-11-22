#pragma once

class PlayerStateBase; // 前方宣言
class PlayerConfig
{
public:
	// 共通パラメータのみ(例)
	float commonMoveSpeed = 5.0f;

	void InGuiInspector(const std::vector<std::unique_ptr<PlayerStateBase>>& states);

	void JsonInput(const nlohmann::json& js,
		const std::vector<std::unique_ptr<PlayerStateBase>>& states);

	void JsonSave(nlohmann::json& js,
		const std::vector<std::unique_ptr<PlayerStateBase>>& states) const;

	struct AttackParams
	{
		float attackRadius = 1.0f;
		float attackDistance = 1.0f;
		int   attackCount = 1;
		float attackTime = 0.1f;
		float attackStartTime = 0.0f;
		float attackEndTime = 0.4f;
		Math::Vector3 moveSpeed = Math::Vector3::Zero;
	};
	AttackParams attack;
};