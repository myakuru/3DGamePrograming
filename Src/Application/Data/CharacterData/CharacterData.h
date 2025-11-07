#pragma once
class CharacterData
{
public:
	CharacterData() = default;
	~CharacterData() = default;

private:

	void JsonInput(const nlohmann::json& _json);
	void JsonSave(nlohmann::json& _json) const;

	struct s_CharacterData
	{
		int hp = 0;				// 体力
		int attack = 0;			// 攻撃力
		int maxHp = 0;			// 最大体力
	};

public:

	struct s_PlayerStatus
	{
		int chargeCount = 0;		// チャージ攻撃の回数
		int skillPoint = 0;			// スキルポイント
		int skillPointMax = 100;	// スキルポイントの最大値
		int specialPoint = 0;		// スペシャルポイント
		int specialPointMax = 3000;	// スペシャルポイントの最大値
	};

	// キャラクターデータの取得
	const s_CharacterData& GetCharacterData() const { return m_characterData; }

	// プレイヤー固有ステータスの取得
	const s_PlayerStatus& GetPlayerStatus() const { return m_playerStatus; }

	// プレイヤー固有ステータスの設定
	s_PlayerStatus& SetPlayerStatus() { return m_playerStatus; }

	// キャラクターデータの設定
	s_CharacterData& SetCharacterData() { return m_characterData; }

private:

	// キャラクターデータ
	s_CharacterData m_characterData;

	// プレイヤー固有ステータス
	s_PlayerStatus m_playerStatus;

};