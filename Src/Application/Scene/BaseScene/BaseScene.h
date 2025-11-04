#pragma once
#include <unordered_map>
#include <vector>
#include <algorithm>

class PlayerCamera;
class KdGameObject;
class BaseScene
{
public:

	BaseScene() { Init(); }
	virtual ~BaseScene() {}

	void PreUpdate();
	void Update();
	void PostUpdate();

	void PreDraw();
	void Draw();
	void DrawSprite();
	void DrawDebug();

	virtual void JsonInput([[maybe_unused]] const nlohmann::json& _json);
	virtual void JsonSave([[maybe_unused]] nlohmann::json& _json) const;
	virtual void LoadSceneSettingsFromJson([[maybe_unused]] const std::string& filePath);
	virtual void SaveSceneSettingsToJson([[maybe_unused]] const std::string& filePath) const;

	virtual void DrawImGui();

	// オブジェクトリストを取得
	std::list<std::shared_ptr<KdGameObject>>& GetObjList()
	{
		return m_objList;
	}

	std::list<std::shared_ptr<KdGameObject>>& WorkObjList()
	{
		return m_objList;
	}

	// オブジェクトリストに追加（バケットにも登録）
	void AddObject(const std::shared_ptr<KdGameObject>& _obj)
	{
		m_objList.emplace_back(_obj);
		IndexObject(_obj);
	}

	// カメラオブジェクトリストを取得
	void AddCameraObject(const std::shared_ptr<KdGameObject>& _obj)
	{
		m_CameraObjList.emplace_back(_obj);
		// 必要に応じてインデックスするならここで IndexObject(_obj);
	}
	// カメラオブジェクトリストを取得
	std::list<std::shared_ptr<KdGameObject>>& GetCameraObjList()
	{
		return m_CameraObjList;
	}

	void AddMapObject(const std::shared_ptr<KdGameObject>& _obj)
	{
		m_MapObjectList.emplace_back(_obj);
		// 必要に応じてインデックスするならここで IndexObject(_obj);
	}

	std::list<std::shared_ptr<KdGameObject>>& GetMapObjectList()
	{
		return m_MapObjectList;
	}

	virtual std::string GetSceneName() const = 0;

	const KdRenderTargetPack& GetRenderTargetPack() const
	{
		return m_renderTargetPack;
	}

	const KdRenderTargetPack& GetRenderTargetUIPack() const
	{
		return m_renderTargetUIPack;
	}

	// ----- 型バケット経由の検索API（SceneManagerから呼び出される） -----

	 // タグ経由の取得（全件）
	void GetObjectWeakPtrListByTagFromBuckets(ObjTag tag, std::list<std::weak_ptr<KdGameObject>>& outPtrList)
	{
		outPtrList.clear();
		auto it = m_tagBuckets.find(ToMask(tag));
		if (it == m_tagBuckets.end()) return;
		for (auto& w : it->second) if (!w.expired()) outPtrList.emplace_back(w);
	}

	// タグ経由の取得（球範囲）
	void GetObjectWeakPtrListByTagInSphereFromBuckets(ObjTag tag, const Math::Vector3& center, float radius,
		std::list<std::weak_ptr<KdGameObject>>& outPtrList)
	{
		outPtrList.clear();
		const float r2 = radius * radius;
		auto it = m_tagBuckets.find(ToMask(tag));
		if (it == m_tagBuckets.end()) return;
		for (auto& w : it->second)
		{
			if (auto sp = w.lock())
			{
				const auto d = sp->GetPos() - center;
				if (d.LengthSquared() <= r2) outPtrList.emplace_back(sp);
			}
		}
	}

	// 単一型の先頭1件取得（従来の FindObjectOfType の高速版）
	template<class T>
	std::shared_ptr<T> FindFirstObjectOfTypeFromBuckets()
	{
		auto it = m_typeBuckets.find(T::TypeID);
		if (it == m_typeBuckets.end()) return nullptr;
		for (const auto& w : it->second)
		{
			if (auto sp = w.lock()) return std::static_pointer_cast<T>(sp);
		}
		return nullptr;
	}

protected:

	// 継承先シーンで必要ならオーバーライドする
	virtual void Event();
	virtual void Init();

	// PostProcess用パラメータ
	float m_brightThreshold = 0.9f;
	bool m_fogEnable = true;
	bool m_fogUseRange = false;
	Math::Vector3 m_fogColor = { 0.7f, 0.7f, 0.7f };
	Math::Vector3 m_highFogColor = { 0.7f, 0.7f, 0.7f };
	float m_highFogHeight = 0.0f;
	float m_lowFogHeight = 0.0f;
	float m_highFogDistance = 0.0f;
	Math::Vector3 m_directionalLightDir = { 1,1,1 };
	Math::Vector3 m_directionalLightColor = { 1.0f,1.0f,1.0f };
	Math::Vector4 m_anviLightColor = { 0.2f, 0.2f, 0.2f, 0.2f };

	Math::Vector2 m_lightingArea = { 1.0f, 1.0f };
	float m_dirLightHeight = 1.0f;

	float m_fogDensity = 0.005f;

	std::weak_ptr<PlayerCamera> m_playerCamera;

	KdRenderTargetChanger m_renderTargetChanger;
	KdRenderTargetPack m_renderTargetPack;

	// UI用のレンダーターゲット切り替えるやつ
	KdRenderTargetChanger m_renderTargetUIChanger;
	// UIのテクスチャいれるやつ
	KdRenderTargetPack m_renderTargetUIPack;

	int m_gameWindowSizeX = 0;
	int m_gameWindowSizeY = 0;


	// 全オブジェクトのアドレスをリストで管理
	std::list<std::shared_ptr<KdGameObject>> m_objList;
	std::list<std::shared_ptr<KdGameObject>> m_CameraObjList;
	std::list<std::shared_ptr<KdGameObject>> m_MapObjectList;
	std::list<std::shared_ptr<KdGameObject>> m_CollisionList;
	std::list<std::shared_ptr<KdGameObject>> m_drawObjectList;

	// ----- 型ごとのレジストリ（TypeID → バケット） -----
	std::unordered_map<uint32_t, std::vector<std::weak_ptr<KdGameObject>>> m_typeBuckets;

	std::unordered_map<uint32_t, std::vector<std::weak_ptr<KdGameObject>>> m_tagBuckets;


	// 既存: 追加時にバケットへ登録
	inline void IndexObject(const std::shared_ptr<KdGameObject>& obj)
	{
		if (!obj) return;
		// 型バケット
		m_typeBuckets[obj->GetTypeID()].emplace_back(obj);
		// タグバケット（セットされた全ビットを展開）
		uint32_t mask = obj->GetTagMask();
		while (mask)
		{
			uint32_t bit = mask & (~mask + 1); // 最下位セットビット
			mask ^= bit;
			m_tagBuckets[bit].emplace_back(obj);
		}
	}

	// 既存: PreUpdateでのコンパクション
	inline void CompactTypeBuckets()
	{
		auto comp = [](auto& vec)
			{
				vec.erase(std::remove_if(vec.begin(), vec.end(),
					[](const std::weak_ptr<KdGameObject>& w) { return w.expired(); }),
					vec.end());
			};
		for (auto& kv : m_typeBuckets) comp(kv.second);
		for (auto& kv : m_tagBuckets)  comp(kv.second); // 追加: タグ側も整理
	}
};
