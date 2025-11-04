#pragma once

#include"../BaseScene/BaseScene.h"

class ResultScene : public BaseScene
{
public:

	ResultScene() { Init(); }
	~ResultScene() override = default;

	std::string GetSceneName() const override { return "Result"; }

private:

	void Event() override;
	void Init()  override;
};
