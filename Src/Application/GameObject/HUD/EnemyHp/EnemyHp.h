#pragma once
#include "../../Utility/SelectDraw2DTexture.h"
#include <vector>

class Enemy;
class PlayerCamera;

class EnemyHp : public SelectDraw2DTexture
{
public:
	static const uint32_t TypeID;

	EnemyHp() = default;
	~EnemyHp() override = default;

private:

	void Init() override;
	void Update() override;
	void DrawSprite() override;
	void ImGuiInspector() override;

	std::weak_ptr<PlayerCamera> m_camera;

	// 複数敵のスクリーン座標を保持
	std::vector<Math::Vector3> m_screenPosList;

	// 複数的のHpバーの表示割合を保持
	std::vector<Math::Rectangle> m_displayRectangleList;

	// ワールド座標での頭上オフセット
	Math::Vector3 m_offsetPos = Math::Vector3(0, 2.5f, 0);

	std::list<std::weak_ptr<KdGameObject>> m_enemies;

	Math::Rectangle m_srcRect = { 0,0,1600,300 };

	int   m_displayTime = 0;
	float m_timer = 0.0f;

	// 1フレームで描く最大数
	size_t m_maxDrawCount = 8;

	// 敵のHpバーのテクスチャ
	std::shared_ptr<KdTexture> m_hpBarTexture;

	// 敵のHpバーのレクト
	Math::Rectangle m_hpBarRect = { 0,0,1500,300 };

	// 追加: 敵ごとの「表示用HP割合」を保持
	std::unordered_map<const Enemy*, float> m_dispHpRate;

	// 追加: 補間係数（減少時/回復時）
	float m_dropLerp = 0.20f;  // 小さめ=ゆっくり減る
	float m_healLerp = 0.50f;  // 大きめ=やや早く戻る

	Math::Vector3 m_blackBarPos = Math::Vector3(0, 0, 0);


};