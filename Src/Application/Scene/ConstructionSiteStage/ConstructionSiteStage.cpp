#include "ConstructionSiteStage.h"
#include"../SceneManager.h"
#include"../../GameObject/Utility/Time.h"
#include"../../GameObject/Character/Player/Player.h"
#include"Application/GameObject/Character/EnemyBase/AetheriusEnemy/AetheriusEnemy.h"
#include"Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"

#include"../../../MyFramework/Manager/JsonManager/JsonManager.h"
#include"../../main.h"

void ConstructionSiteStage::Event()
{
	// 敵を探して、いなかったらゲームクリアさせる
	SearchEnemy();

	if (SceneManager::Instance().m_gameClear)
	{
		KdShaderManager::Instance().WorkAmbientController().SetheightFog({ m_fogColor }, 110.0f, -200.0f, 5.0f);
		m_brightThreshold = 0.47f;
		m_fogEnable = true;
		m_fogUseRange = true;
		m_fogColor = { 0.93f, 0.86f, 0.0f };
		m_fogDensity = 0.001f;
	}
	else
	{
		KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(m_brightThreshold);
		KdShaderManager::Instance().WorkAmbientController().SetFogEnable(m_fogEnable, m_fogUseRange);
		KdShaderManager::Instance().WorkAmbientController().SetDistanceFog({ m_fogColor }, m_fogDensity);
		KdShaderManager::Instance().WorkAmbientController().SetheightFog({ m_highFogColor }, m_highFogHeight, m_lowFogHeight, m_highFogDistance);

		KdShaderManager::Instance().WorkAmbientController().SetDirLight(m_directionalLightDir, m_directionalLightColor);
		KdShaderManager::Instance().WorkAmbientController().SetDirLightShadowArea(m_lightingArea, m_dirLightHeight);

		KdShaderManager::Instance().WorkAmbientController().SetAmbientLight(m_anviLightColor);
	}

	if (SceneManager::Instance().IsCutInScene())
	{
		KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(m_brightThreshold);
		KdShaderManager::Instance().WorkAmbientController().SetFogEnable(true, true);
		KdShaderManager::Instance().WorkAmbientController().SetDistanceFog({ m_fogColor }, m_fogDensity);
		KdShaderManager::Instance().WorkAmbientController().SetheightFog({ m_highFogColor }, m_highFogHeight, m_lowFogHeight, m_highFogDistance);
	}

	if (KeyboardManager::GetInstance().IsKeyPressed('P'))
	{
		KdShaderManager::Instance().m_postProcessShader.SetEnableNoise(true);	// ノイズON
		KdShaderManager::Instance().m_postProcessShader.SetNoiseStrength(0.07f);
	}
	else
	{
		KdShaderManager::Instance().m_postProcessShader.SetEnableNoise(false);	// ノイズOFF
	}

	SceneManager::Instance().GetObjectWeakPtr(m_player);

	auto spPlayer = m_player.lock();

	if (!spPlayer) return;

	if (spPlayer->GetPos().x >= 4.0f && !m_isCountDown)
	{
		m_isCountDown = true; // カウントダウン開始

		if (m_isCountDown)
		{
			Time::Instance().StartCountdown(m_countDownTimer);
			SceneManager::Instance().SetTime(m_countDownTimer);
		}
	}

	Time::Instance().Update();

	if (KeyboardManager::GetInstance().IsKeyPressed('C'))
	{
		KdShaderManager::Instance().m_postProcessShader.SetEnableStrongBlur(true);
	}
	else
	{
		KdShaderManager::Instance().m_postProcessShader.SetEnableStrongBlur(false);
	}
}

void ConstructionSiteStage::Init()
{
	//KdShaderManager::Instance().m_postProcessShader.SetEnableGray(false);

	m_isCountDown = false;	// カウントダウンフラグを初期化

	m_countDownTimer = 200.0f; // カウントダウンタイマーを200秒に設定

	SceneManager::Instance().SetIntroCamera(true); // カメラのイントロを開始

	KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(m_brightThreshold);
	KdShaderManager::Instance().WorkAmbientController().SetFogEnable(m_fogEnable, m_fogUseRange);
	KdShaderManager::Instance().WorkAmbientController().SetDistanceFog({ m_fogColor }, m_fogDensity);
	KdShaderManager::Instance().WorkAmbientController().SetheightFog({ m_highFogColor }, m_highFogHeight, m_lowFogHeight, m_highFogDistance);

	KdShaderManager::Instance().WorkAmbientController().SetDirLight(m_directionalLightDir, m_directionalLightColor);
	KdShaderManager::Instance().WorkAmbientController().SetDirLightShadowArea(m_lightingArea, m_dirLightHeight);

	KdShaderManager::Instance().WorkAmbientController().SetAmbientLight(m_anviLightColor);

	SceneManager::Instance().SetDrawGrayScale(false);
	SceneManager::Instance().m_gameClear = false;	// ゲームクリアフラグを初期化
	SceneManager::Instance().SetResultFlag(false);	// 結果フラグを初期化

	KdShaderManager::Instance().m_postProcessShader.SetRadialBlur(m_radialBlurStrength, m_radialBlurSampleNum, m_radialBlurUvOffset); // 放射状ブラーの初期設定

	m_bossAppear = false; // ボス出現フラグを初期化
}

void ConstructionSiteStage::SearchEnemy()
{
	// 現在の存在状況を集計
	bool enemyExists = false;
	bool bossExists = false;

	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::EnemyLike, m_objects);

	for (const auto& we : m_objects)
	{
		if (auto enemy = we.lock())
		{
			if (enemy->GetTypeID() == AetheriusEnemy::TypeID)
			{
				enemyExists = true;
				break;
			}

			if (enemy->GetTypeID() == BossEnemy::TypeID)
			{
				bossExists = true;
				break;
			}

		}
	}

	// 雑魚全滅 → ボス出現要求
	if (!enemyExists && !SceneManager::Instance().IsBossAppear())
	{
		SceneManager::Instance().SetBossAppear(true);
	}

	// 出現要求が立っていて、まだスポーンしていなければスポーン
	if (SceneManager::Instance().IsBossAppear() && !m_bossAppear)
	{
		m_bossAppear = true;

		auto bossEnemy = std::make_shared<BossEnemy>();
		bossEnemy->Init();
		SceneManager::Instance().AddObject(bossEnemy);

		// このフレームではbossExistsはまだfalseのため、即クリアへ入らないよう終了
		return;
	}

	// ボスフェーズ中で、シーン上にボスが存在しなければクリア
	if (m_bossAppear && !bossExists && !enemyExists)
	{
		SceneManager::Instance().m_gameClear = true;
	}
}

void ConstructionSiteStage::DrawImGui()
{
	BaseScene::DrawImGui();
	ImGui::Begin("BossEnemySpawn");
	{
		ImGui::Text("BossEnemySpawn");
		if (ImGui::Button("SpawnBossEnemy"))
		{
			SceneManager::Instance().SetBossAppear(true);
		}

		// 放射状ブラー設定
		ImGui::DragFloat("RadialBlurStrength", &m_radialBlurStrength);
		ImGui::DragFloat("RadialBlurSampleNum", &m_radialBlurSampleNum);
		ImGui::DragFloat2("RadialBlurUvOffset", &m_radialBlurUvOffset.x);

		// 放射状ブラーの反映
		ImGui::Checkbox("RadialBlurEnable", &m_radialBlurEnable);

		KdShaderManager::Instance().m_postProcessShader.SetRadialBlur(m_radialBlurStrength, m_radialBlurSampleNum, m_radialBlurUvOffset);
		KdShaderManager::Instance().m_postProcessShader.SetEnableRadialBlur(m_radialBlurEnable);
	}
	ImGui::End();
}
