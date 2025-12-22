#pragma once

// ベースのステートパラメータ
struct StateParameter
{
	float attackRadius = 1.0f;
	float attackDistance = 1.0f;
	int   attackCount = 1;
	float attackInterval = 0.1f; // 旧 m_attackTime
	float attackStartTime = 0.0f;
	float attackEndTime = 0.4f;
	float animeTime = 0.0f;

	Math::Vector3 moveSpeed = Math::Vector3::Zero;
	Math::Vector2 cameraShake = { 0.2f, 0.0f };
	float         cameraTime = 0.3f;

	float dashSpeed = 0.7f;
	float blendTime = 0.25f;
	float animationSpeed = 60.0f;
	float dashSpeedTime = 0.2f;
	float changeStateTime = 0.7f;

	// 敵探索範囲
	float searchEnemyRadius = 5.0f;

	// デフォルトの敵探索範囲
	static constexpr float kDefaultSearchEnemyRadius = 5.0f;

	// 残像
	int            afterImageMax = 5;
	float          afterImageInterval = 0.1f;
	Math::Vector4  afterImageColor = { 0.0f, 2.0f, 2.0f, 1.0f };

	bool lButtonKeyInput = false;
	bool rButtonKeyInput = false;

	float startSlowMotionTime = 0.0f;
	float endSlowMotionTime = 0.1f;

	// カメラターゲットオフセットデフォルト値
	static constexpr Math::Vector3 kCameraTargetOffset = { 0.0f,1.0f,-2.5f };
	static constexpr Math::Vector3 kCameraBossTargetOffset = { 0.0f,1.0f,-5.5f };


	void ExposeImGui();
	void LoadJson(const nlohmann::json& pj);
	void SaveJson(nlohmann::json& js) const;
};

inline void StateParameter::ExposeImGui()
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
			ImGui::SliderInt("##attackCount", &attackCount, 0, 20);
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

	// アニメ / 時間
	if (ImGui::CollapsingHeader(U8("アニメ / 時間")))
	{
		if (ImGui::BeginTable("tbl_time", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, kLabelWidth);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

			// アニメ時間
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("アニメ時間(秒)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::DragFloat("##animeTime", &animeTime, 0.01f, 0.0f, 60.0f);

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
			ImGui::DragFloat("##cameraTime", &cameraTime, 0.01f, 0.0f, 10.0f);

			ImGui::EndTable();
		}
	}

	// 移動 / ダッシュ
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
			ImGui::DragFloat("##dashSpeed", &dashSpeed, 0.01f, 0.0f, 20.0f);

			// ダッシュ速度時間
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("ダッシュ速度時間(秒)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::DragFloat("##dashSpeedTime", &dashSpeedTime, 0.01f, 0.0f, 5.0f);

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
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("ステートの切り替え(秒)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderFloat("##changeStateTime", &changeStateTime, 0.0f, 5.0f, "%.02f");

			// 敵探索範囲
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("敵探索範囲"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderFloat("##searchEnemyRadius", &searchEnemyRadius, 0.0f, 20.0f, "%.02f");

			ImGui::EndTable();
		}
	}

	// 残像
	if (ImGui::CollapsingHeader(U8("残像")))
	{
		if (ImGui::BeginTable("tbl_afterimage", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, kLabelWidth);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

			// 最大数
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("最大数"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderInt("##afterImageMax", &afterImageMax, 0, 50);

			// 生成間隔
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("生成間隔(秒)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::SliderFloat("##afterImageInterval", &afterImageInterval, 0.0f, 1.0f, "%.03f");

			// 色
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("色(RGBA)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth * 1.2f);
			ImGui::ColorEdit4("##afterImageColor", &afterImageColor.x);

			ImGui::EndTable();
		}
	}

	// 入力フラグ
	if (ImGui::CollapsingHeader(U8("入力フラグ")))
	{
		if (ImGui::BeginTable("tbl_input", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, kLabelWidth);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

			// Lボタン
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("Lボタン入力許可"));
			ImGui::TableSetColumnIndex(1);
			ImGui::Checkbox("##lButtonKeyInput", &lButtonKeyInput);

			// Rボタン
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("Rボタン入力許可"));
			ImGui::TableSetColumnIndex(1);
			ImGui::Checkbox("##rButtonKeyInput", &rButtonKeyInput);

			ImGui::EndTable();
		}
	}

	// スローモーション
	if (ImGui::CollapsingHeader(U8("スローモーション")))
	{
		if (ImGui::BeginTable("tbl_slow", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, kLabelWidth);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

			// 開始
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("開始(秒)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::DragFloat("##startSlowMotionTime", &startSlowMotionTime, 0.01f, 0.0f, 5.0f);

			// 終了
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text(U8("終了(秒)"));
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(kItemWidth);
			ImGui::DragFloat("##endSlowMotionTime", &endSlowMotionTime, 0.01f, 0.0f, 5.0f);

			ImGui::EndTable();
		}
	}

	ImGui::PopID();
}

inline void StateParameter::LoadJson(const nlohmann::json& pj)
{
	// "Player" ノードがある場合はその中を読む。無ければ直下を読む。
	const nlohmann::json& src = pj.contains("Player") ? pj["Player"] : pj;

	// 既存
	if (src.contains("AttackRadius"))    attackRadius = src["AttackRadius"].get<float>();
	if (src.contains("AttackDistance"))  attackDistance = src["AttackDistance"].get<float>();
	if (src.contains("AttackCount"))     attackCount = src["AttackCount"].get<int>();

	// attackInterval: 旧名 AttackTime も許容
	if (src.contains("AttackInterval"))  attackInterval = src["AttackInterval"].get<float>();
	else if (src.contains("AttackTime")) attackInterval = src["AttackTime"].get<float>();

	if (src.contains("AttackStartTime")) attackStartTime = src["AttackStartTime"].get<float>();
	if (src.contains("AttackEndTime"))   attackEndTime = src["AttackEndTime"].get<float>();

	// 追加: アニメ時間
	if (src.contains("AnimeTime"))       animeTime = src["AnimeTime"].get<float>();

	if (src.contains("MoveSpeedX"))      moveSpeed.x = src["MoveSpeedX"].get<float>();
	if (src.contains("MoveSpeedY"))      moveSpeed.y = src["MoveSpeedY"].get<float>();
	if (src.contains("MoveSpeedZ"))      moveSpeed.z = src["MoveSpeedZ"].get<float>();

	if (src.contains("CameraShakeX"))    cameraShake.x = src["CameraShakeX"].get<float>();
	if (src.contains("CameraShakeY"))    cameraShake.y = src["CameraShakeY"].get<float>();
	if (src.contains("CameraShakeTime")) cameraTime = src["CameraShakeTime"].get<float>();

	if (src.contains("DashSpeed"))       dashSpeed = src["DashSpeed"].get<float>();

	// blendTime: 旧名 BleedTime も許容
	if (src.contains("BlendTime"))       blendTime = src["BlendTime"].get<float>();
	else if (src.contains("BleedTime"))  blendTime = src["BleedTime"].get<float>();

	if (src.contains("AnimationSpeed"))  animationSpeed = src["AnimationSpeed"].get<float>();
	if (src.contains("DashSpeedTime"))   dashSpeedTime = src["DashSpeedTime"].get<float>();
	if (src.contains("ChangeStateTime")) changeStateTime = src["ChangeStateTime"].get<float>();

	// searchEnemyRadius: 大文字小文字両対応
	if (src.contains("searchEnemyRadius"))          searchEnemyRadius = src["searchEnemyRadius"].get<float>();
	else if (src.contains("SearchEnemyRadius"))     searchEnemyRadius = src["SearchEnemyRadius"].get<float>();

	// 追加: 残像
	if (src.contains("AfterImageMax"))       afterImageMax = src["AfterImageMax"].get<int>();
	if (src.contains("AfterImageInterval"))  afterImageInterval = src["AfterImageInterval"].get<float>();
	if (src.contains("AfterImageColor") && src["AfterImageColor"].is_array() && src["AfterImageColor"].size() >= 4)
	{
		const auto& c = src["AfterImageColor"];
		afterImageColor = { c[0].get<float>(), c[1].get<float>(), c[2].get<float>(), c[3].get<float>() };
	}
	else
	{
		if (src.contains("AfterImageColorR")) afterImageColor.x = src["AfterImageColorR"].get<float>();
		if (src.contains("AfterImageColorG")) afterImageColor.y = src["AfterImageColorG"].get<float>();
		if (src.contains("AfterImageColorB")) afterImageColor.z = src["AfterImageColorB"].get<float>();
		if (src.contains("AfterImageColorA")) afterImageColor.w = src["AfterImageColorA"].get<float>();
	}

	// 追加: 入力フラグ
	if (src.contains("LButtonKeyInput")) lButtonKeyInput = src["LButtonKeyInput"].get<bool>();
	if (src.contains("RButtonKeyInput")) rButtonKeyInput = src["RButtonKeyInput"].get<bool>();

	// 追加: スローモーション
	if (src.contains("StartSlowMotionTime")) startSlowMotionTime = src["StartSlowMotionTime"].get<float>();
	if (src.contains("EndSlowMotionTime"))   endSlowMotionTime = src["EndSlowMotionTime"].get<float>();
}

inline void StateParameter::SaveJson(nlohmann::json& js) const
{
	// 呼び出し側が js を適切なノード(例: stateNode["Player"])にしてから渡す前提で、フラットに書く
	js["AttackRadius"] = attackRadius;
	js["AttackDistance"] = attackDistance;
	js["AttackCount"] = attackCount;

	// 互換のため両方書き出す
	js["AttackInterval"] = attackInterval;
	js["AttackTime"] = attackInterval;

	js["AttackStartTime"] = attackStartTime;
	js["AttackEndTime"] = attackEndTime;

	js["AnimeTime"] = animeTime;

	js["MoveSpeedX"] = moveSpeed.x;
	js["MoveSpeedY"] = moveSpeed.y;
	js["MoveSpeedZ"] = moveSpeed.z;

	js["CameraShakeX"] = cameraShake.x;
	js["CameraShakeY"] = cameraShake.y;
	js["CameraShakeTime"] = cameraTime;

	js["DashSpeed"] = dashSpeed;

	// 互換のため両方書き出す
	js["BlendTime"] = blendTime;
	js["BleedTime"] = blendTime;

	js["AnimationSpeed"] = animationSpeed;
	js["DashSpeedTime"] = dashSpeedTime;
	js["ChangeStateTime"] = changeStateTime;

	// 既存の小文字キー維持（必要なら PascalCase も併記）
	js["searchEnemyRadius"] = searchEnemyRadius;

	// 残像
	js["AfterImageMax"] = afterImageMax;
	js["AfterImageInterval"] = afterImageInterval;
	js["AfterImageColorR"] = afterImageColor.x;
	js["AfterImageColorG"] = afterImageColor.y;
	js["AfterImageColorB"] = afterImageColor.z;
	js["AfterImageColorA"] = afterImageColor.w;

	// 入力フラグ
	js["LButtonKeyInput"] = lButtonKeyInput;
	js["RButtonKeyInput"] = rButtonKeyInput;

	// スローモーション
	js["StartSlowMotionTime"] = startSlowMotionTime;
	js["EndSlowMotionTime"] = endSlowMotionTime;
}

