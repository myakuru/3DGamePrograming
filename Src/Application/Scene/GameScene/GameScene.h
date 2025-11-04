#pragma once

#include"../BaseScene/BaseScene.h"

class GameScene : public BaseScene
{
public :

	GameScene()  { Init(); }
	~GameScene() override = default;

	std::string GetSceneName() const override { return "Game"; }

private:

	void Event() override;
	void Init()  override;
};
