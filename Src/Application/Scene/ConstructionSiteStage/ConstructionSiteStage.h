#pragma once
#include"../BaseScene/BaseScene.h"
class Player;
class BossEnemy;
class RedEnemy;
class ConstructionSiteStage : public BaseScene
{
public:

	ConstructionSiteStage() { Init(); }
	~ConstructionSiteStage() override = default;

	std::string GetSceneName() const override { return "ConstructionSite"; }

	void JsonInput([[maybe_unused]] const nlohmann::json& _json) override;
	void JsonSave([[maybe_unused]] nlohmann::json& _json) const override;

private:
	void Event() override;
	void Init()  override;

	void DrawImGui() override;

	// 敵を探して、いなかったらゲームクリアさせる
	void SearchEnemy();

	bool m_isCountDown = false; // カウントダウン中かどうか

	float m_countDownTimer = 0.0f;		// カウントダウンタイマー
	float m_countDownTimeMax = 300.0f;	// カウントダウン時間最大値

	std::weak_ptr<Player> m_player;
	std::vector<std::weak_ptr<RedEnemy>> m_aetheriusEnemies;
	std::vector<std::weak_ptr<BossEnemy>> m_bossEnemies;


	// ボスが出現したか
	bool m_bossAppear = false;

	float m_radialBlurStrength = 0.2f;
	float m_radialBlurSampleNum = 10.0f;
	Math::Vector2 m_radialBlurUvOffset = { 0.5f,0.55f };
	bool m_radialBlurEnable = false;

	bool m_enemyExists = false;
	bool m_bossExists = false;

	bool m_bossDefeated = false;

};