#pragma once

enum class ObjTag : uint32_t
{
	None = 0,
	EnemyLike = 1u << 0,
	PlayerLike = 1u << 1,
	Collision = 1u << 2,
	EnemySword = 1u << 3,
	EnemyShield = 1u << 4,
};

inline uint32_t ToMask(ObjTag t) { return static_cast<uint32_t>(t); }

// ゲーム上に存在するすべてのオブジェクトの基底となるクラス
class KdGameObject : public std::enable_shared_from_this<KdGameObject>
{
public:

	virtual uint32_t GetTypeID() const { return m_typeID; }

	static uint32_t GenerateTypeID()
	{
		static uint32_t s_nextTypeID = 1;
		return s_nextTypeID++;
	}

	uint32_t m_typeID = 0;

	// どのような描画を行うのかを設定するTypeID：Bitフラグで複数指定可能
	enum class DrawTypeID
	{
		eDrawTypeLit = 1 << 0,
		eDrawTypeUnLit = 1 << 1,
		eDrawTypeBright = 1 << 2,
		eDrawTypeUI = 1 << 3,
		eDrawTypeDepthOfShadow = 1 << 4,
	};

	KdGameObject() = default;
	virtual ~KdGameObject() { Release(); }


	// 生成される全てに共通するパラメータに対する初期化のみ
	virtual void Init();

	virtual void PreUpdate() 
	{
		m_mWorld = Math::Matrix::CreateScale(m_scale);
		m_mWorld *= Math::Matrix::CreateFromYawPitchRoll(
			DirectX::XMConvertToRadians(m_degree.y),
			DirectX::XMConvertToRadians(m_degree.x),
			DirectX::XMConvertToRadians(m_degree.z)
		);
		m_mWorld.Translation(m_position);
	}
	virtual void Update() {}
	virtual void PostUpdate() {}
	virtual void CameraUpdate() {}

	// それぞれの状況で描画する関数
	virtual void GenerateDepthMapFromLight() {}
	virtual void PreDraw() {}
	virtual void DrawLit() {}
	virtual void DrawRimLight() {}
	virtual void DrawUnLit() {}
	virtual void DrawEffect() {}
	virtual void DrawBright() {}
	virtual void DrawSprite() {}
	virtual void DrawToon() {}
	virtual void DrawGradation() {}
	virtual void DrawGrayScale() {}
	virtual void DrawDebug();

	virtual void SetAsset(const std::string&) {}

	virtual void SetPos(const Math::Vector3& pos) { m_mWorld.Translation(pos); }
	virtual Math::Vector3 GetPos() const { return m_mWorld.Translation(); }

	// 拡大率を変更する関数
	void SetScale(float scalar);
	virtual void SetScale(const Math::Vector3& scale);
	virtual Math::Vector3 GetScale() const;

	const Math::Matrix& GetMatrix() const { return m_mWorld; }

	virtual bool IsExpired() const { return m_isExpired; }
	virtual void SetExpired(bool isExpired) { m_isExpired = isExpired; }

	virtual bool IsVisible()	const { return false; }
	virtual bool IsRideable()	const { return false; }


	// タグ操作
	void AddTag(ObjTag t) { m_tagMask |= ToMask(t); }
	void RemoveTag(ObjTag t) { m_tagMask &= ~ToMask(t); }
	bool HasTag(ObjTag t) const { return (m_tagMask & ToMask(t)) != 0; }
	void SetTagMask(uint32_t mask) { m_tagMask = mask; }
	uint32_t GetTagMask() const { return m_tagMask; }

	// 視錐台範囲内に入っているかどうか
	virtual bool CheckInScreen(const DirectX::BoundingFrustum& a_BoundingFrustum)const;

	// カメラからの距離を計算
	virtual void CalcDistSqrFromCamera(const Math::Vector3& camPos);

	float GetDistSqrFromCamera() const { return m_distSqrFromCamera; }

	UINT GetDrawType() const { return m_drawType; }

	bool Intersects(const KdCollider::SphereInfo& targetShape, std::list<KdCollider::CollisionResult>* pResults);
	bool Intersects(const KdCollider::BoxInfo& targetBox, std::list<KdCollider::CollisionResult>* pResults);
	bool Intersects(const KdCollider::RayInfo& targetShape, std::list<KdCollider::CollisionResult>* pResults);

	// 自分で追加したもの
	bool SelectObjectIntersects(const KdCollider::RayInfo& targetShape, std::list<KdCollider::CollisionResult>* pResults);

	//===================================================================
	// 自分で追加したもの
	//===================================================================

	// 各オブジェクトのデーターをここでJsonから取ってくる関数
	virtual void JsonInput(const nlohmann::json& _json);

	// 各オブジェクトのデーターをここでJsonに保存する関数
	virtual void JsonSave(nlohmann::json&_json) const;

	virtual bool ModelLoad(std::string _path);

	// 各オブジェクトのImGuiインスペクターを実装する関数
	virtual void ImGuiInspector();

	virtual void ImGuiSelectGltf();

	// 当たり判定セットする関数(インスペクターで変更される)
	void SetCollider();

	// 当たり判定の仕組み
	bool CheckBoxBit(std::string _name, UINT& _ID, UINT _checkID);

	// クラス名を取得する関数
	const std::string &GetNameClass() const { return m_className; }

	// 親子関係の作成
	virtual void AddChild(std::weak_ptr<KdGameObject> a_child)
	{
		std::shared_ptr<KdGameObject> _child = a_child.lock();
		if (!_child)return;

		// 自分の子には入れれないようにする(子の子も同様)
		//if (!SearchChild(_child->GetNameClass(), a_child).expired())return;

		// すでに親がいる場合は親の子リストから削除
		//if (_child->GetParent().lock())EraceChild(a_child);

		_child->SetParent(GetMyAdls());                        // 子に親のアドレスを渡す

		m_childObjects.push_back(_child->GetMyAdls());        // 自分の子リストに追加
	}

	// 自分のアドレスを返す
	virtual std::shared_ptr<KdGameObject> GetMyAdls() { return shared_from_this(); }
	virtual void SetParent(const std::weak_ptr<KdGameObject>& a_obj) { m_parentObjects = a_obj; }
	virtual std::weak_ptr<KdGameObject>& GetParent() { return m_parentObjects; }
	virtual std::list<std::weak_ptr<KdGameObject>>& GetChild() { return m_childObjects; }

protected:

	void Release() {}

	// 描画タイプ・何の描画を行うのかを決める / 最適な描画リスト作成用
	UINT m_drawType = 0;

	// カメラからの距離
	float m_distSqrFromCamera = 0;

	// 存在消滅フラグ
	bool m_isExpired = false;

	// 3D空間に存在する機能
	Math::Matrix	m_mWorld;

	// 当たり判定クラス
	std::unique_ptr<KdCollider> m_pCollider = nullptr;

	// デバッグ情報クラス
	std::unique_ptr<KdDebugWireFrame> m_pDebugWire = nullptr;

	uint32_t m_tagMask = 0; // タグビット


	//自分で追加したもの

	// jsonでクラスの名前を保存する変数
	std::string m_className = "Name";

	// モデルのロードパス
	std::string m_path = "Asset/Models/Sky/Sky.gltf";

	UINT m_type = (UINT)KdCollider::TypeEvent;	// 当たり判定のタイプを格納する。

	// モデルの初期化
	std::shared_ptr<KdModelData> m_model = std::make_shared<KdModelData>();
	std::shared_ptr<KdModelWork> m_modelWork = std::make_shared<KdModelWork>();
	// ポリゴンの初期化
	std::shared_ptr<KdSquarePolygon> m_polygon = std::make_shared<KdSquarePolygon>();
	// 2Dテクスチャの初期化
	std::shared_ptr <KdTexture> m_texture = std::make_shared<KdTexture>();

	float m_dissever = 0.0f;			// ディゾルブ値

	std::list<std::weak_ptr<KdGameObject>> m_childObjects; // 子オブジェクトを格納するリスト
	std::weak_ptr<KdGameObject> m_parentObjects; // 親オブジェクトを格納するリスト

	// 位置
	Math::Vector3 m_position = Math::Vector3::Zero;
	// 拡大率
	Math::Vector3 m_scale = Math::Vector3::One;
	// 回転用クォータニオン
	Math::Quaternion m_rotation = Math::Quaternion::Identity;

	// 回転角度（デグリー）
	Math::Vector3 m_degree = Math::Vector3::Zero;
	// カラー
	Math::Color m_color = {1,1,1,1};

	// ゲームの音量を管理する変数
	float m_gameVolume = 0.0f;

};
