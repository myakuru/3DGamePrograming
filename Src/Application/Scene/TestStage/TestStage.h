#pragma once
#include"../BaseScene/BaseScene.h"
class Player;
class BossEnemy;
class TestScene : public BaseScene
{
public:

	TestScene() { Init(); }
	~TestScene() override = default;
		
	std::string GetSceneName() const override { return "Test"; }

private:
	void Event() override;
	void Init()  override;

	void DrawImGui() override;

	// 敵を探して、いなかったらゲームクリアさせる
	void SearchEnemy();

	bool m_isCountDown = false; // カウントダウン中かどうか

	float m_countDownTimer = 0.0f; // カウントダウンタイマー

	std::weak_ptr<Player> m_player;

	// ボスが出現したか
	bool m_bossAppear = false;

};