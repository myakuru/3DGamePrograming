#include "KdGameObject.h"
#include"../../Application/main.h"
#include"../../MyFramework/Manager/JsonManager/JsonManager.h"
#include"../../MyFramework/Manager/ImGuiManager/ImGuiManager.h"
#include"../../Application/Scene/SceneManager.h"
#include"../../Application/Scene/BaseScene/BaseScene.h"

void KdGameObject::Init()
{
	ModelLoad(m_path);
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	m_mWorld = Math::Matrix::CreateScale(m_scale);
	m_mWorld *= Math::Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z)
	);
	m_mWorld.Translation(m_position);


	if (m_className.empty() || m_className == "Name")
	{
		m_className = typeid(*this).name();
	}

}

void KdGameObject::DrawDebug()
{
	//早期リターン
	if (!m_pDebugWire)return;
	// デバッグ表示が無効の場合は描画せずに蓄積をクリア
	if (!IMGUI_MANAGER.GetShowDebugWireFrame()) { m_pDebugWire->Clear(); return; }
	m_pDebugWire->Draw();

}

void KdGameObject::SetScale(float scalar)
{
	Math::Vector3 scale(scalar);

	SetScale(scale);
}

void KdGameObject::SetScale(const Math::Vector3& scale)
{
	Math::Vector3 vecX = m_mWorld.Right();
	Math::Vector3 vecY = m_mWorld.Up();
	Math::Vector3 vecZ = m_mWorld.Backward();
	
	vecX.Normalize();
	vecY.Normalize();
	vecZ.Normalize();

	m_mWorld.Right(vecX * scale.x);
	m_mWorld.Up(vecY * scale.y);
	m_mWorld.Backward(vecZ * scale.z);
}

Math::Vector3 KdGameObject::GetScale() const
{
	return Math::Vector3(m_mWorld.Right().Length(), m_mWorld.Up().Length(), m_mWorld.Backward().Length());
}

bool KdGameObject::CheckInScreen(const DirectX::BoundingFrustum& _BoundingFrustum) const
{
	if (!m_model) return false;

	DirectX::BoundingBox combinedBox;

	for (auto& nodeIdx : m_model->GetDrawMeshNodeIndices())
	{
		auto& node = m_model->GetOriginalNodes()[nodeIdx];

		if (!node.m_spMesh) continue;

		node.m_spMesh->GetBoundingBox().Transform(combinedBox, m_mWorld);

		if (_BoundingFrustum.Intersects(combinedBox)) return true;
	}

	m_pDebugWire->AddDebugSphere(combinedBox.Center,2, kRedColor);

	return false;
}

void KdGameObject::CalcDistSqrFromCamera(const Math::Vector3& camPos)
{
	m_distSqrFromCamera = (m_mWorld.Translation() - camPos).LengthSquared();
}

bool KdGameObject::Intersects(const KdCollider::SphereInfo& targetShape, std::list<KdCollider::CollisionResult>* pResults)
{
	if (!m_pCollider) { return false; }

	return m_pCollider->Intersects(targetShape, m_mWorld, pResults);
}

bool KdGameObject::Intersects(const KdCollider::BoxInfo& targetBox, std::list<KdCollider::CollisionResult>* pResults)
{
	if (!m_pCollider) { return false; }

	return m_pCollider->Intersects(targetBox, m_mWorld, pResults);
}

bool KdGameObject::Intersects(const KdCollider::RayInfo& targetShape, std::list<KdCollider::CollisionResult>* pResults)
{
	if (!m_pCollider) { return false; }

	return m_pCollider->Intersects(targetShape, m_mWorld, pResults);
}

bool KdGameObject::SelectObjectIntersects(const KdCollider::RayInfo& targetShape, std::list<KdCollider::CollisionResult>* pResults)
{
	if (!m_pCollider) { return false; }
	// 当たったかどうか判別
	if (m_pCollider->Intersects(targetShape, m_mWorld, pResults))
	{
		// リストの最後に自分自身のポインタを追加することで、何と衝突したかを識別できるようにする
		pResults->back().m_resultObject = shared_from_this();
		return true;
	}

	return false;
}

void KdGameObject::JsonInput(const nlohmann::json& _json)
{
	if (_json.contains("Name")) m_className = _json["Name"].get<std::string>();
	if (_json.contains("path")) m_path = _json["path"];
	if (_json.contains("pos")) m_position = JSON_MANAGER.JsonToVector(_json["pos"]);
	if (_json.contains("scale")) m_scale = JSON_MANAGER.JsonToVector(_json["scale"]);
	if (_json.contains("deg")) m_degree = JSON_MANAGER.JsonToVector(_json["deg"]);
	if (_json.contains("color")) m_color = JSON_MANAGER.JsonToVector4(_json["color"]);
	if (_json.contains("type")) m_type = static_cast<KdCollider::Type>(_json["type"].get<int>());
}

void KdGameObject::JsonSave(nlohmann::json& _json) const
{
	std::string className = typeid(*this).name(); // クラス名の所得(this)にすると基底クラスの名前が取得されるので自分自身のポインタを使用

	_json["Name"] = className;
	_json["path"] = m_path;
	_json["pos"] = JSON_MANAGER.VectorToJson(m_position);
	_json["scale"] = JSON_MANAGER.VectorToJson(m_scale);
	_json["deg"] = JSON_MANAGER.VectorToJson(m_degree);
	_json["color"] = JSON_MANAGER.Vector4ToJson(m_color);
	_json["type"] = static_cast<int>(m_type);
}

bool KdGameObject::ModelLoad(std::string _path)
{
	// モデル読み込み
	m_model = KdAssets::Instance().m_modeldatas.GetData(_path);
	

	if (m_model)
	{
		m_pCollider = std::make_unique<KdCollider>();
		m_pCollider->RegisterCollisionShape("ModelMesh", std::make_unique<KdModelCollision>(m_model, m_type));
		return true;
	}

 return false;
}

void KdGameObject::ImGuiInspector()
{
	ImGui::Text(U8("トランスフォーム"));

	static std::string currentName = "Transform";

	// ImGuiのコンボボックスを作成
	
	ImGui::DragFloat3(U8("位置"), &m_position.x,0.1f);
	ImGui::DragFloat3(U8("拡大、縮小"), &m_scale.x,0.01f);
	ImGui::DragFloat3(U8("回転"), &m_degree.x,0.001f);


	ImGui::ColorEdit4("color", &m_color.x);

	SetCollider();
	
	ImGuiSelectGltf();

	m_mWorld = Math::Matrix::CreateScale(m_scale);
	m_mWorld *= Math::Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z)
	);
	m_mWorld.Translation(m_position);

}

void KdGameObject::ImGuiSelectGltf()
{
	ImGui::Text(U8("GLTFの読み込み"));

	if (ImGui::Button(("LoadPath: %s", m_path.data())))
	{
		if (Application::Instance().GetWindow().OpenFileDialog(m_path))
		{
			ModelLoad(m_path);
		}
	}
}

void KdGameObject::SetCollider()
{
	ImGui::Text(U8("当たり判定フラグ"));

	static std::string currentName = "Collider Type";
	// ImGuiのコンボボックスを作成
	if (ImGui::BeginCombo("##Collider", currentName.data()))
	{

		CheckBoxBit("Ground", m_type,(UINT)KdCollider::TypeGround);
		CheckBoxBit("Bump", m_type,(UINT)KdCollider::TypeBump);
		CheckBoxBit("Damage", m_type,(UINT)KdCollider::TypeDamage);
		CheckBoxBit("TypeDamageLine", m_type,(UINT)KdCollider::TypeDamageLine);
		CheckBoxBit("TypeSight", m_type,(UINT)KdCollider::TypeSight);
		CheckBoxBit("TypeEvent", m_type,(UINT)KdCollider::TypeEvent);

		ImGui::EndCombo();
	}
}

bool KdGameObject::CheckBoxBit(std::string _name, UINT& _ID, UINT _checkID)
{
	bool flg = _ID & _checkID;
	bool change = ImGui::Checkbox(_name.c_str(), &flg);
	if (flg) _ID |= _checkID;
	else _ID &= (~_checkID);
	return change;
}

void KdGameObject::AddChild(std::weak_ptr<KdGameObject> a_child)
{
	std::shared_ptr<KdGameObject> _child = a_child.lock();
	if (!_child)return;

	// 自分の子には入れれないようにする(子の子も同様)
	if (SearchChild(_child->GetMyAdls()))return;

	// すでに親がいる場合は親の子リストから削除
	if (_child->GetParent().lock())EraceChild(a_child);

	_child->SetParent(GetMyAdls());                        // 子に親のアドレスを渡す

	m_childObjects.push_back(_child->GetMyAdls());        // 自分の子リストに追加
}

void KdGameObject::EraceChild(std::weak_ptr<KdGameObject>& _child)
{
	if (auto child = _child.lock(); child)
	{
		auto peaent = child->GetParent().lock();
		if (!peaent) return;

		auto& childList = peaent->GetChild();
		if (childList.empty()) return;

		for (auto it = childList.begin(); it != childList.end();)
		{
			if (it->lock() == child)
			{
				it = childList.erase(it);
				return;
			}
		}

	}
}

void KdGameObject::SortChild()
{
	if (m_childObjects.empty()) return;

	auto it = m_childObjects.begin();

	while (it != m_childObjects.end())
	{
		auto child = it->lock();
		if (child)
		{
			++it;
		}
		else
		{
			it = m_childObjects.erase(it);
		}
	}
}

bool KdGameObject::SearchChild(const std::shared_ptr<KdGameObject>& _temp)
{
	if (std::shared_ptr<KdGameObject> _parent = GetParent().lock(); _parent)
	{
		if (_parent == _temp) return true;
		else if (_parent->SearchChild(_temp)) return true;
	}

	return false;
}
