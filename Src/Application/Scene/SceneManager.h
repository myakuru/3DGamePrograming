#pragma once
#include "BaseScene/BaseScene.h"

class BaseScene;
class SceneManager
{
public:

	// シーン情報
	enum class SceneType
	{
		Title,
		Game,
		Result,
		Test, // テストシーン
		ConstructionSiteStage, // 建設現場ステージ
	};

	// シーンの初期化
	void Init();

	void PreUpdate();
	void Update();
	void PostUpdate();

	void PreDraw();
	void Draw();
	void DrawSprite();
	void DrawDebug();
	void DrawImGui();

	// 次のシーンをセット (次のフレームから切り替わる)
	void SetNextScene(SceneType _nextScene)
	{
		m_nextSceneType = _nextScene;
	}

	// 現在のシーンのオブジェクトリストを取得
	std::list<std::shared_ptr<KdGameObject>>& GetObjList();
	std::list<std::shared_ptr<KdGameObject>>& GetCameraList();
	std::list<std::shared_ptr<KdGameObject>>& GetMapList();

	// 現在のシーンにオブジェクトを追加
	void AddObject(const std::shared_ptr<KdGameObject>& _obj);

	// シーンの種類をゲットするゲッター
	std::shared_ptr<BaseScene> GetCurrentScene() { return m_currentScene; }

	// 選択したオブジェクトをセット
	std::shared_ptr<KdGameObject> m_selectObject = nullptr;


	// 指定した型のオブジェクトを取得（型バケットから）
	template<class T>
	std::shared_ptr<T> FindObjectOfType()
	{
		if (!m_currentScene) return nullptr;
		return m_currentScene->template FindFirstObjectOfTypeFromBuckets<T>();
	}

	// 単体のオブジェクトのウィークポインタを取得
	template<class T>
	void GetObjectWeakPtr(std::weak_ptr<T>& outPtr)
	{
		if (outPtr.expired())
		{
			if (auto findObj = FindObjectOfType<T>())
			{
				outPtr = findObj;
			}
		}
	}

	// 追加: タグで近傍取得
	void GetObjectWeakPtrListByTagInSphere(ObjTag tag, const Math::Vector3& center, float radius,
		std::list<std::weak_ptr<KdGameObject>>& outPtrList)
	{
		if (!m_currentScene) { outPtrList.clear(); return; }
		m_currentScene->GetObjectWeakPtrListByTagInSphereFromBuckets(tag, center, radius, outPtrList);
	}

	// 追加: タグで全取得
	void GetObjectWeakPtrListByTag(ObjTag tag, std::list<std::weak_ptr<KdGameObject>>& outPtrList)
	{
		if (!m_currentScene) { outPtrList.clear(); return; }
		m_currentScene->GetObjectWeakPtrListByTagFromBuckets(tag, outPtrList);
	}

	// カメラのオブジェクトを取得
	template<typename T>
	std::shared_ptr<T> FindCameraObj()
	{
		uint32_t typeId = T::TypeID;
		for (auto& cameraObj : GetCameraList())
		{
			if (cameraObj->GetTypeID() == typeId)
			{
				// キャストして返す
				return std::static_pointer_cast<T>(cameraObj);
			}
		}
		return nullptr;
	}

	// カメラのウィークポインタを取得
	template<typename T>
	void GetCameraWeakPtr(std::weak_ptr<T>& _ptr)
	{
		if (_ptr.expired())
		{
			if (auto findCamera = FindCameraObj<T>())
			{
				_ptr = findCamera;
			}
		}
	}

	// シーンカメラかどうか
	bool m_sceneCamera = false;

	bool m_gameClear = false; // ゲームクリアフラグ

	void SetResultFlag(bool _result)
	{
		m_nowResult = _result;
	}

	bool GetResultFlag() const
	{
		return m_nowResult;
	}

	void SetScore(int _score)
	{
		m_score = _score;
	}

	int GetScore() const
	{
		return m_score;
	}

	void SetDrawGrayScale(bool _draw)
	{
		m_drawGrayScale = _draw;
	}
	bool GetDrawGrayScale() const
	{
		return m_drawGrayScale;
	}

	float GetTime() const
	{
		return m_time;
	}

	void SetTime(float _time)
	{
		m_time = _time;
	}

	void SetIntroCamera(bool _isIntro)
	{
		m_isIntro = _isIntro;
	}

	bool IsIntroCamera() const
	{
		return m_isIntro;
	}

	bool IsEffectActive() const
	{
		return m_effectActive;
	}

	void SetEffectActive(bool _active)
	{
		m_effectActive = _active;
	}

	void SetCutInScene(bool _cutIn)
	{
		m_CutInScene = _cutIn;
	}

	bool IsCutInScene() const
	{
		return m_CutInScene;
	}

	// ボス登場シーンかどうか
	void SetBossAppear(bool _appear)
	{
		m_bossAppear = _appear;
	}

	bool IsBossAppear() const
	{
		return m_bossAppear;
	}

	// ゲームサウンドの取得
	std::shared_ptr<KdSoundInstance> GetGameSound() const
	{
		return m_gameSound;
	}

	// ゲームサウンドのセット
	// ゲームサウンドのセット（以前のBGMを停止してから差し替え）
	void SetGameSound(const std::shared_ptr<KdSoundInstance>& _sound)
	{
		if (m_gameSound && m_gameSound != _sound)
		{
			m_gameSound->Stop();
		}
		m_gameSound = _sound;
	}

private:

	// RegisterObjectからオブジェクトを登録する関数
	void Register() const;

	// シーン切り替え関数
	void ChangeScene(SceneType _sceneType);

	// 現在のシーンのインスタンスを保持しているポインタ
	std::shared_ptr<BaseScene> m_currentScene = nullptr;

	// 現在のシーンの種類を保持している変数
	SceneType m_currentSceneType = SceneType::ConstructionSiteStage;

	// 次のシーンの種類を保持している変数
	SceneType m_nextSceneType = m_currentSceneType;

	SceneManager() = default;
	~SceneManager() = default;

	bool m_nowResult = false; // 現在のシーンがResultかどうか

	int m_score = 0; // スコア

	bool m_drawGrayScale = false; // グレースケール描画フラグ

	float m_time = 0.0f; // カウントダウンタイマー

	bool m_isIntro = false;		// Introカメラかどうか

	bool m_effectActive = true; // 最初のUpdateかどうか

	bool m_CutInScene = false; // カットインシーンかどうか

	bool m_bossAppear = false; // ボス登場シーンかどうか

	std::shared_ptr<KdSoundInstance> m_gameSound = nullptr;


public:

	// シングルトンパターン
	// 常に存在する && 必ず1つしか存在しない(1つしか存在出来ない)
	// どこからでもアクセスが可能で便利だが
	// 何でもかんでもシングルトンという思考はNG
	static SceneManager& Instance()
	{
		static SceneManager instance;
		return instance;
	}
};