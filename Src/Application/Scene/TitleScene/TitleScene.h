#pragma once

#include"../BaseScene/BaseScene.h"

class TitleScene : public BaseScene
{
public :

	TitleScene()  { Init(); }
	~TitleScene() override = default;

	std::string GetSceneName() const override { return "Title"; }

private :

	void Event() override;
	void Init()  override;

};
