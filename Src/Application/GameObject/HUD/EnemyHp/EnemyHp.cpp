#include "EnemyHp.h"
#include"../../../Scene/SceneManager.h"
#include"../../../main.h"
#include"Application/GameObject/Character/EnemyBase/AetheriusEnemy/AetheriusEnemy.h"
#include"../../Camera/PlayerCamera/PlayerCamera.h"
#include"../../../Data/CharacterData/CharacterData.h"

const uint32_t EnemyHp::TypeID = KdGameObject::GenerateTypeID();

void EnemyHp::Init()
{
	m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/EnemyUI/BlackBar.png");
	m_hpBarTexture = KdAssets::Instance().m_textures.GetData("Asset/Textures/EnemyUI/HpBar.png");
}

void EnemyHp::Update()
{
	// デフォルトは非表示
	m_bDrawTexture = false;
	m_screenPosList.clear();
	m_displayRectangleList.clear();

	SceneManager::Instance().GetObjectWeakPtr(m_camera);

	if (auto camera = m_camera.lock(); camera)
	{
		// このフレームで参照可能な敵を列挙
		SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemyLike, m_enemies);

		// ビューポート取得（中心原点の±半分で画面内判定）
		Math::Viewport vp;
		KdDirect3D::Instance().CopyViewportInfo(vp);
		const float halfW = vp.width * 0.5f;
		const float halfH = vp.height * 0.5f;

		if (auto cam = camera->GetCamera())
		{
			for (const auto& enemy : m_enemies)
			{
				if (auto enemyPtr = enemy.lock())
				{
					// 敵以外はスキップ
					if(enemyPtr->GetTypeID() != AetheriusEnemy::TypeID) continue;

					// 敵を一時的にキャスト
					auto castedEnemy = std::static_pointer_cast<AetheriusEnemy>(enemyPtr);

					Math::Vector3 screenPos;
					cam->ConvertWorldToScreenDetail(castedEnemy->GetPos() + m_offsetPos, screenPos);

					// Hpバーの表示割合を計算
					float hpRate = 0.0f;

					const auto& enemyStatus = castedEnemy->GetEnemyStatus();

					if (enemyStatus.GetCharacterData().hp > 0)
					{
						hpRate = static_cast<float>(enemyStatus.GetCharacterData().hp) / static_cast<float>(enemyStatus.GetCharacterData().maxHp);
					}

					// Hpバーの表示割合に応じてレクトを作成
					Math::Rectangle hpRect = m_hpBarRect;
					hpRect.width = static_cast<long>(1500.0f * hpRate);


					// 1) カメラ前方（w > 0）
					if (screenPos.z <= 0.0f) { continue; }

					// 2) 画面内（中心原点の±半分）
					if (screenPos.x < -halfW || screenPos.x > halfW) { continue; }
					if (screenPos.y < -halfH || screenPos.y > halfH) { continue; }

					// 可視：描画リストに追加
					m_screenPosList.push_back(screenPos);

					m_displayRectangleList.push_back(hpRect);

					if (m_screenPosList.size() >= m_maxDrawCount) { break; }
				}
			}
		}

		m_bDrawTexture = !m_screenPosList.empty();
	}
}

void EnemyHp::DrawSprite()
{
	if (SceneManager::Instance().IsIntroCamera()) return;

	// 非表示なら描画しない
	if (!m_bDrawTexture || m_screenPosList.empty()) { return; }


	for (const auto& pos : m_screenPosList)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex
		(
			m_texture,
			static_cast<int>(pos.x - 5.0f),
			static_cast<int>(pos.y),
			m_srcRect.width / 8,
			m_srcRect.height / 8,
			&m_srcRect,
			&m_color,
			{ 0.0f, 0.5f }
		);
	}

	// HPバー（各敵ごとに対応するレクトで描画）
	const size_t drawCount = std::min(m_screenPosList.size(), m_displayRectangleList.size());
	for (size_t i = 0; i < drawCount; ++i)
	{
		const auto& pos = m_screenPosList[i];
		auto& rect = m_displayRectangleList[i];

		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_hpBarTexture,
			static_cast<int>(pos.x),
			static_cast<int>(pos.y),
			(rect.width) / 8,
			(rect.height) / 8,
			&rect,
			&m_color,
			{ 0.0f, 0.5f }
		);
	}

}

void EnemyHp::ImGuiInspector()
{
	SelectDraw2DTexture::ImGuiInspector();
}
