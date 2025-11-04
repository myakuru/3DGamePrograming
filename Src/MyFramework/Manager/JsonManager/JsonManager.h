#pragma once
class JsonManager
{
public:
	JsonManager() = default;
	~JsonManager() = default;

	// Jsonからオブジェクトを生成
	void JsonToObj() const;

	// この関数ですべての登録されたオブジェクトをJsonに渡す処理
	void AllSave() const;

	// オブジェクトに追加
	std::shared_ptr<KdGameObject> AddJsonObject
	(
		const std::string& _className,
		const nlohmann::json& _json = nlohmann::json()
	)	const;
	// Jsonのファイルを生成する。
	void JsonSerialize(const nlohmann::json& _json, const std::string& _path) const;

	// Jsonから値を取得する(デシリアライズ)
	nlohmann::json JsonDeserialize(const std::string& _path) const;

	// Jsonからのベクターの値に変換する
	Math::Vector3 JsonToVector(const nlohmann::json& _json) const;

	// ベクターの値をJsonに変換する
	nlohmann::json VectorToJson(const Math::Vector3& _vec) const;

	// Jsonの値をベクター２に変換する
	Math::Vector2 JsonToVector2(const nlohmann::json& _json) const;

	// ベクター２の値をJsonに変換する
	nlohmann::json Vector2ToJson(const Math::Vector2& _vec) const;

	// Jsonの値をベクター４に変換する
	Math::Vector4 JsonToVector4(const nlohmann::json& _json) const;

	// ベクター４の値をJsonに変換する
	nlohmann::json Vector4ToJson(const Math::Vector4& _vec) const;
};