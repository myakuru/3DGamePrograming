#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class Timer :public SelectDraw2DTexture
{
public:

	static const uint32_t TypeID;

	Timer();
	~Timer() override = default;

private:
	void Init() override;
	void Update() override;

	void ResultTimerUpdate();

	void DrawSprite() override;

	void ImGuiInspector() override;
	void JsonSave(nlohmann::json& _json) const override;
	void JsonInput(const nlohmann::json& _json) override;

	float m_lastTime = 0.0;	// 最後に更新した時間

	Math::Rectangle m_srcRect;
	int m_displayTime = 0;
	std::shared_ptr<KdTexture> m_resultTexture = std::make_shared<KdTexture>();	// 結果画面用のテクスチャ

	bool m_notDraw = false;	// 描画しないフラグ

	float m_dawnTimer = 0.0f;
	Math::Vector3 m_dawnPos = Math::Vector3::Zero;

	// リザルト開始時に一度だけ初期化するためのフラグと開始時間
	bool  m_resultScaleInited = false;

};