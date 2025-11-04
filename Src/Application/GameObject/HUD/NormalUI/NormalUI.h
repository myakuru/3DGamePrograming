#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
class NormalUI :public SelectDraw2DTexture
{
public:

	static const uint32_t TypeID;

	NormalUI();
	~NormalUI() override = default;

	void SetSvgPath(const std::string& path);
	void SetTargetSizePx(uint32_t w, uint32_t h); // UIレイアウトやスケールから更新
	void SetUiScale(float scale) { m_uiScale = scale; } // DPIや全体UIスケール用(任意)


private:

	void Init() override;
	void DrawSprite() override;
	void Update() override;

	void EnsureSvgTexture(); // 必要なら再ラスタライズ

	std::string m_svgPath;
	KdTexture   m_texture;

	// 論理上の描画サイズ(レイアウト基準)。実際に使うピクセルは UiScale を掛けたもの
	uint32_t    m_targetW = 0;
	uint32_t    m_targetH = 0;
	float       m_uiScale = 1.0f;

	// 直近でラスタライズしたピクセルサイズ
	uint32_t    m_rasterizedW = 0;
	uint32_t    m_rasterizedH = 0;

	// 再生成のしきい値（小さな変動で作り直さない）
	float       m_regenScaleUpRatio = 1.25f; // 25%以上拡大なら作り直し
	float       m_regenScaleDownRatio = 0.80f; // 20%以上縮小なら作り直し
};