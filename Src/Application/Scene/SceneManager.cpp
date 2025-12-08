#include "SceneManager.h"

// シーンのインクルード
#include "BaseScene/BaseScene.h"
#include "GameScene/GameScene.h"
#include "TitleScene/TitleScene.h"
#include "ResultScene/ResultScene.h"
#include "TestStage/TestStage.h"
#include "ConstructionSiteStage/ConstructionSiteStage.h"

// フレームワークのインクルード
#include"../../MyFramework/RegisterObject/RegisterObject.h"
#include"../main.h"
#include"../../MyFramework/Manager/JsonManager/JsonManager.h"
#include"../GameObject/Utility/Time.h"

// ゲームオブジェクトのインクルード
#include"../GameObject/Camera/TPSCamera/TPSCamera.h"
#include"../GameObject/Camera/FPSCamera/FPSCamera.h"
#include"../GameObject/Camera/TitleCamera/TitleCamera.h"

#include"../GameObject/SkySphere/SkySphere.h"
#include"../GameObject/HUD/NormalUI/NormalUI.h"
#include"../GameObject/HUD/AlpfaMoving/AlpfaMoving.h"
#include"../GameObject/Map/Map.h"
#include"../GameObject/Character/Player/Player.h"
#include"../GameObject/Weapon/Katana/Katana.h"
#include"../GameObject/Camera/PlayerCamera/PlayerCamera.h"
#include"../GameObject/HUD/ScaleMoving/ScaleMoving.h"
#include"../GameObject/HUD/Scoring/Scoring.h"
#include"../GameObject/HUD/Timer/Timer.h"
#include"../GameObject/HUD/HpBar/HpBar.h"
#include"../GameObject/HUD/Number/NumBer.h"
#include"../GameObject/HUD/NowHp/NowHp.h"
#include"../GameObject/Winner/Winner.h"
#include"../GameObject/HUD/InvestigationComplete/InvestigationComplete.h"
#include"../GameObject/HUD/BlackBanner/BlackBanner.h"
#include"../GameObject/ResultScore/ResultScore.h"
#include"../GameObject/Collition/Collition.h"
#include"../GameObject/Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include"../GameObject/Weapon/EnemySword/EnemySword.h"
#include"../GameObject/Weapon/EnemyShield/EnemyShield.h"
#include"../GameObject/HUD/EnemyHp/EnemyHp.h"
#include"../GameObject/Effect/BillBoardEffect/MapGard/MapGard.h"
#include"../GameObject/HUD/SkillBar/SkillBar.h"
#include"../GameObject/HUD/ChallengeResults/ChallengeResults.h"
#include"../GameObject/HUD/ScoreBackBar/ScoreBackBar.h"
#include"../GameObject/HUD/TitleRemoveUI/TitleRemoveUI.h"
#include"../GameObject/HUD/SkillUI/SkillUI.h"
#include"../GameObject/HUD/SpecialAttackUI/SpecialAttackUI.h"
#include"../GameObject/HUD/BossEnemyUI/BossEnemyUI.h"
#include"../GameObject/HUD/BossEnemyBlackBarUI/BossEnemyBlackBarUI.h"
#include"Application/GameObject/Character/EnemyBase/AetheriusEnemy/AetheriusEnemy.h"
#include"Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"
#include"Application/GameObject/Effect/EffekseerEffect/EnemyHitEffect/EnemyHitEffect.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectManager.h"


#include "Application/GameObject/Effect/EffekseerEffect/CharacterEffect/CharacterEffect.h"


void SceneManager::Init()
{
	Register();
	// 開始シーンに切り替え
	ChangeScene(m_currentSceneType);
}

void SceneManager::Register() const
{
	// カメラ
	RegisterObject::GetInstance().Register<TPSCamera>();
	RegisterObject::GetInstance().Register<FPSCamera>();
	RegisterObject::GetInstance().Register<PlayerCamera>();
	RegisterObject::GetInstance().Register<TitleCamera>();

	// スカイスフィア
	RegisterObject::GetInstance().Register<SkySphere>();

	// UI系
	RegisterObject::GetInstance().Register<NormalUI>();
	RegisterObject::GetInstance().Register<AlpfaMoving>();
	RegisterObject::GetInstance().Register<ScaleMoving>();
	RegisterObject::GetInstance().Register<Scoring>();
	RegisterObject::GetInstance().Register<Timer>();
	RegisterObject::GetInstance().Register<HpBar>();
	RegisterObject::GetInstance().Register<NumBer>();
	RegisterObject::GetInstance().Register<NowHp>();
	RegisterObject::GetInstance().Register<InvestigationComplete>();
	RegisterObject::GetInstance().Register<BlackBanner>();
	RegisterObject::GetInstance().Register<ResultScore>();
	RegisterObject::GetInstance().Register<Winner>();
	RegisterObject::GetInstance().Register<EnemyHp>();
	RegisterObject::GetInstance().Register<SkillBar>();
	RegisterObject::GetInstance().Register<ChallengeResults>();
	RegisterObject::GetInstance().Register<ScoreBackBar>();
	RegisterObject::GetInstance().Register<TitleRemoveUI>();
	RegisterObject::GetInstance().Register<SkillUI>();
	RegisterObject::GetInstance().Register<SpecialAttackUI>();
	RegisterObject::GetInstance().Register<BossEnemyUI>();
	RegisterObject::GetInstance().Register<BossEnemyBlackBarUI>();

	// Map系
	RegisterObject::GetInstance().Register<Map>();

	// キャラクター系
	RegisterObject::GetInstance().Register<Player>();
	RegisterObject::GetInstance().Register<BossEnemy>();
	RegisterObject::GetInstance().Register<AetheriusEnemy>();

	// 武器系
	RegisterObject::GetInstance().Register<Katana>();
	RegisterObject::GetInstance().Register<WeaponKatanaScabbard>();
	RegisterObject::GetInstance().Register<EnemySword>();
	RegisterObject::GetInstance().Register<EnemyShield>();

	// 当たり判定系
	RegisterObject::GetInstance().Register<Collision>();

	// Effekseer系
	RegisterObject::GetInstance().Register<EnemyHitEffect>();
	RegisterObject::GetInstance().Register<EffekseerEffectManager>();
	RegisterObject::GetInstance().Register<CharacterEffect>();	// Playerのエフェクト系

	// Fieldのエフェクト系
	RegisterObject::GetInstance().Register<MapGard>();
}

void SceneManager::PreUpdate()
{
	// シーン切替
	if (m_currentSceneType != m_nextSceneType)
	{
		ChangeScene(m_nextSceneType);
	}

	m_currentScene->PreUpdate();
}

void SceneManager::Update()
{
	m_currentScene->Update();
}

void SceneManager::PostUpdate()
{
	m_currentScene->PostUpdate();
}

void SceneManager::PreDraw()
{
	m_currentScene->PreDraw();
}

void SceneManager::Draw()
{
	m_currentScene->Draw();
}

void SceneManager::DrawSprite()
{
	m_currentScene->DrawSprite();
}

void SceneManager::DrawDebug()
{
	m_currentScene->DrawDebug();
}

void SceneManager::DrawImGui()
{
	m_currentScene->DrawImGui();
}

std::list<std::shared_ptr<KdGameObject>>& SceneManager::GetObjList()
{
	return m_currentScene->GetObjList();
}

std::list<std::shared_ptr<KdGameObject>>& SceneManager::GetCameraList()
{
	return m_currentScene->GetCameraObjList();
}

void SceneManager::AddObject(const std::shared_ptr<KdGameObject>& _obj)
{
	m_currentScene->AddObject(_obj);
}

void SceneManager::ChangeScene(SceneType _sceneType)
{

	// 次のシーンを作成し、現在のシーンにする
	switch (_sceneType)
	{
	case SceneType::Title:
		m_currentScene = std::make_shared<TitleScene>();
		break;
	case SceneType::Game:
		m_currentScene = std::make_shared<GameScene>();
		break;
	case SceneType::Result:
		m_currentScene = std::make_shared<ResultScene>();
		break;
	case SceneType::Test:
		m_currentScene = std::make_shared<TestScene>();
		break;
	case SceneType::ConstructionSiteStage:
		m_currentScene = std::make_shared<ConstructionSiteStage>();
		break;
	}

	Time::Instance().Reset(); // 時間リセット

	// 現在のシーン情報を更新
	m_currentSceneType = _sceneType;

	if (m_currentScene)
	{
		// ここでJsonからオブジェクトを追加
		JSON_MANAGER.JsonToObj();
	}
}
