#pragma once
#include"../BaseScene/BaseScene.h"
class Player;
class BossEnemy;
class AetheriusEnemy;
class ConstructionSiteStage : public BaseScene
{
public:

	ConstructionSiteStage() { Init(); }
	~ConstructionSiteStage() override = default;

	std::string GetSceneName() const override { return "ConstructionSite"; }

private:
	void Event() override;
	void Init()  override;

	void DrawImGui() override;

	// 敵を探して、いなかったらゲームクリアさせる
	void SearchEnemy();

	bool m_isCountDown = false; // カウントダウン中かどうか

	float m_countDownTimer = 0.0f; // カウントダウンタイマー

	std::weak_ptr<Player> m_player;
	std::vector<std::weak_ptr<AetheriusEnemy>> m_aetheriusEnemies;
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