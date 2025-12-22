#include "ConstructionSiteStage.h"
#include"Application/Scene/SceneManager.h"
#include"Application/GameObject/Utility/Time.h"
#include"Application/GameObject/Character/Player/Player.h"
#include"Application/GameObject/Character/EnemyBase/AetheriusEnemy/RedEnemy.h"
#include"Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"
#include"Application/Data/CharacterData/CharacterData.h"

#include"MyFramework/Manager/JsonManager/JsonManager.h"
#include"Application/main.h"

void ConstructionSiteStage::Event()
{
	// 敵を探して、いなかったらゲームクリアさせる
	SearchEnemy();

	// イントロBGMが再生終了したらループBGMへ切り替え
	{
		auto bgm = SceneManager::Instance().GetGameSound(); // 値取得
		const bool needSwitch = (!bgm) || !bgm->IsPlaying();
		if (needSwitch)
		{
			auto loopBgm = KdAudioManager::Instance().Play(
				"Asset/Sound/FieldBGM/ToDo_game_bgm_loop.wav",
				true
			);
			SceneManager::Instance().SetGameSound(loopBgm);

			if (loopBgm)
			{
				loopBgm->SetVolume(1.0f);
			}
		}
	}

	if (SceneManager::Instance().m_gameClear)
	{
		KdShaderManager::Instance().WorkAmbientController().SetheightFog({ m_fogColor }, 250.0f, -200.0f, 5.0f);
		m_brightThreshold = 0.40f;
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

	if (!m_isCountDown && !SceneManager::Instance().IsIntroCamera())
	{
		m_isCountDown = true; // カウントダウン開始

		if (m_isCountDown)
		{
			Time::Instance().StartCountdown(m_countDownTimer);
		}
	}

	Time::Instance().Update();

	if (Time::Instance().IsCountdownFinished())
	{
		SceneManager::Instance().SetResultFlag(true);	// 結果フラグを立てる
		Time::Instance().Reset();						// タイマーリセット
		SceneManager::Instance().SetNextScene(SceneManager::SceneType::Result);
	}

}

void ConstructionSiteStage::Init()
{
	//KdShaderManager::Instance().m_postProcessShader.SetEnableGray(false);

	Time::Instance().SetCountdownFinished(false);	// カウントダウン終了フラグを初期化

	m_isCountDown = false;	// カウントダウンフラグを初期化

	m_countDownTimer = m_countDownTimeMax; // カウントダウンタイマーを200秒に設定

	SceneManager::Instance().SetIntroCamera(true); // カメラのイントロを開始

	// イントロBGM（非ループ）: SetGameSound を使う
	{
		auto intro = KdAudioManager::Instance().Play(
			"Asset/Sound/FieldBGM/ToDo_game_bgm.wav",
			false
		);
		SceneManager::Instance().SetGameSound(intro);

		if (intro)
		{
			intro->SetVolume(1.0f);
		}
	}

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
	SceneManager::Instance().SetBossAppear(false);
	m_bossDefeated = false; // ボス撃破フラグを初期化
}

void ConstructionSiteStage::SearchEnemy()
{
	// 現在の存在状況を集計
	bool enemyExists = false;
	bool bossExists = false;

	SceneManager::Instance().GetObjectWeakPtrByTag(ObjTag::EnemyLike, m_aetheriusEnemies);
	SceneManager::Instance().GetObjectWeakPtrByTag(ObjTag::EnemyLike, m_bossEnemies);

	for (const auto& we : m_aetheriusEnemies)
	{
		if (auto enemy = we.lock())
		{
			enemyExists = true;
			break;
		}
	}

	for (const auto& wb : m_bossEnemies)
	{
		if (auto boss = wb.lock())
		{
			bossExists = true;
			if (boss->GetStatus().GetCharacterData().hp <= 0) // ボスの体力を参照
			{
				m_bossDefeated = true;
			}
			break;
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
		SceneManager::Instance().GetObjectWeakPtrByTag(ObjTag::EnemyLike, m_bossEnemies);

		for(auto wb : m_bossEnemies)
		{
			if (auto boss = wb.lock())
			{
				boss->StateInit();
				m_bossDefeated = false;
				SceneManager::Instance().SetBossAppear(true);
			}
		}

		// このフレームではbossExistsはまだfalseのため、即クリアへ入らないよう終了
		return;
	}

	// ボスフェーズ中で、シーン上にボスが存在しなければクリア
	if (m_bossAppear && m_bossDefeated && !enemyExists)
	{
		SceneManager::Instance().m_gameClear = true;
	}
}

void ConstructionSiteStage::DrawImGui()
{
	BaseScene::DrawImGui();

	ImGui::Begin("Time");
	{
		ImGui::Text(U8("ゲームの制限時間"));
		ImGui::Text("Time: %.2f", m_countDownTimeMax);
		ImGui::DragFloat("CountdownTime", &m_countDownTimeMax, 1.0f, 0.0f, 1000.0f);

		if (ImGui::Button("SetTimer"))
		{
			m_countDownTimer = m_countDownTimeMax;
		}

	}
	ImGui::End();

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

void ConstructionSiteStage::JsonInput(const nlohmann::json& _json)
{
	BaseScene::JsonInput(_json);
	if (_json.contains("m_countDownTimer")) m_countDownTimeMax = _json["m_countDownTimer"];
}

void ConstructionSiteStage::JsonSave(nlohmann::json& _json) const
{
	BaseScene::JsonSave(_json);
	_json["m_countDownTimer"] = m_countDownTimeMax;
}
