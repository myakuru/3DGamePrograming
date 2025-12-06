#include "EffekseerEffectBase.h"
#include"Application/GameObject/Character/Player/Player.h"
#include"Application/Scene/SceneManager.h"
#include"Application/main.h"
#include"MyFramework/Manager/ImGuiManager/ImGuiManager.h"
#include"MyFramework/Manager/JsonManager/JsonManager.h"
#include"Application/GameObject/Effect/EffekseerEffect/EffekseerEffectManager.h"

void EffekseerEffectBase::Init()
{
	KdGameObject::Init();

	// 画面の大きさが変わっても問題ないようにする
	Math::Viewport vp;
	KdDirect3D::Instance().CopyViewportInfo(vp);
	KdEffekseerManager::GetInstance().Create(static_cast<int>(vp.width), static_cast<int>(vp.height));

	m_once = false;
	m_load = false;

	m_isEffectPlaying = false;

	SceneManager::Instance().GetObjectWeakPtr(m_effectManager);

	if (auto manager = m_effectManager.lock())
	{
		manager->m_effectObjList.push_back(std::static_pointer_cast<EffekseerEffectBase>(shared_from_this()));
	}

	if (m_name.empty())
	{
		m_name = m_className;
	}

}

void EffekseerEffectBase::Update()
{

	KdEffekseerManager::GetInstance().Update();

	if (KeyboardManager::GetInstance().IsKeyJustPressed('F'))
	{
		m_load = true;
	}
	else if(KeyboardManager::GetInstance().IsKeyJustReleased('F'))
	{
		m_load = false;
	}

	SceneManager::Instance().GetObjectWeakPtr(m_player);
	auto player = m_player.lock();
	if (!player) return;

	if (SceneManager::Instance().m_gameClear)
	{
		// プレイヤーがゲームクリアしていたらエフェクトを停止
		StopEffect();
	}

	// プレイヤーの前方ベクトル
	Math::Vector3 forward = Math::Vector3::TransformNormal(Math::Vector3::Forward, Math::Matrix::CreateFromQuaternion(player->GetRotationQuaternion()));
	forward.Normalize();

	m_mWorld = Math::Matrix::CreateScale(m_scale);
	m_mWorld *= Math::Matrix::CreateFromYawPitchRoll
	(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z)
	) * Math::Matrix::CreateFromQuaternion(player->GetRotationQuaternion());

	m_mWorld.Translation(m_position + player->GetPos() + forward * m_distance);

	EffectUpdate();
}

void EffekseerEffectBase::EffectUpdate()
{
	// 再生要求が来た瞬間だけ再生開始
	if (!m_once && m_load)
	{
		m_wpEffect = KdEffekseerManager::GetInstance().Play(m_path, m_mWorld, m_effectSpeed, false, m_effectColor).lock();
		m_once = m_load;
	}

	if (!m_load) m_once = false;

	// 再生状態更新
	if (auto effect = m_wpEffect.lock(); effect)
	{
		m_isEffectPlaying = effect->IsPlaying();
		if (!m_isEffectPlaying)
		{
			// 終了したので参照破棄
			m_wpEffect.reset();
			m_isEffectPlaying = false;
		}
	}
	else
	{
		m_isEffectPlaying = false;
	}
}

void EffekseerEffectBase::ImGuiInspector()
{
	ImGui::Text(U8("トランスフォーム"));

	ImGui::InputText("name", m_name.data(), ImGuiInputTextFlags_EnterReturnsTrue);

	ImGui::DragFloat3(U8("位置"), &m_position.x, 0.1f);
	ImGui::DragFloat3(U8("拡大、縮小"), &m_scale.x, 0.01f);
	ImGui::DragFloat3(U8("回転"), &m_degree.x, 0.001f);


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

	ImGui::DragFloat(U8("エフェクトの前方方向距離"), &m_distance, 0.1f);
	ImGui::DragFloat(U8("エフェクトの再生速度"), &m_effectSpeed, 0.1f);
	ImGui::DragFloat(U8("エフェクトの横方向距離"), &m_sideDistance, 0.1f);
	
	// エフェクト再生ボタン
	if (ImGui::Button(U8("エフェクト再生")))
	{
		m_load = true;
	}
	if (ImGui::Button(U8("エフェクト停止")))
	{
		StopEffect();
	}

	// エフェクトの色設定
	ImGui::ColorEdit4(U8("エフェクトの色"), &m_effectColor.x);
}

void EffekseerEffectBase::JsonSave(nlohmann::json& _json) const
{
	KdGameObject::JsonSave(_json);
	_json["distance"] = m_distance;
	_json["EffectSpeed"] = m_effectSpeed;
	_json["effectColor"] = JSON_MANAGER.Vector4ToJson(m_effectColor);
	_json["sideDistance"] = m_sideDistance;
	_json["name"] = m_name;
}

void EffekseerEffectBase::JsonInput(const nlohmann::json& _json)
{
	KdGameObject::JsonInput(_json);

	if (_json.contains("distance")) m_distance = _json["distance"].get<float>();
	if (_json.contains("EffectSpeed")) m_effectSpeed = _json["EffectSpeed"].get<float>();
	if (_json.contains("effectColor")) m_effectColor = JSON_MANAGER.JsonToVector4(_json["effectColor"]);
	if (_json.contains("sideDistance")) m_sideDistance = _json["sideDistance"].get<float>();
	if (_json.contains("name")) m_name = _json["name"].get<std::string>();

}

bool EffekseerEffectBase::ModelLoad(const std::string& _path)
{
	// 拡張子を取得
	if (std::string name = _path.substr(_path.find_last_of('.') + 1); name == "efkefc")
	{
		m_path = _path;
		return true;
	}

	// それ以外は読み込み失敗
	return false;
}
