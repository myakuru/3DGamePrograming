#pragma once
class StateBase
{
public:
	StateBase() = default;
	virtual ~StateBase() = default;

	// ステートが開始されたときに呼び出される
	virtual void StateStart() = 0;

	// ステートが更新されるたびに呼び出される
	virtual void StateUpdate() = 0;

	// ステートが終了されたときに呼び出される
	virtual void StateEnd() = 0;

	// パラメータ編集
	virtual void ExposeParametersImGui() {}
	// JSON 読み込み　/ 書き出し
	virtual void JsonInput(const nlohmann::json& _json) { (void)_json; }
	virtual void JsonSave(nlohmann::json& _json) const { (void)_json; }
};