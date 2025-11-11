#include "EffekseerEffectBase.h"
#include"../../Character/Player/Player.h"
#include"../../../Scene/SceneManager.h"
#include"../../../main.h"
#include"../../../../MyFramework/Manager/ImGuiManager/ImGuiManager.h"
#include"../../../../MyFramework/Manager/JsonManager/JsonManager.h"

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
	KdGameObject::ImGuiInspector();

	ImGui::DragFloat(U8("エフェクトの出現位置"), &m_distance, 0.1f);
	ImGui::DragFloat(U8("エフェクトの再生速度"), &m_effectSpeed, 0.1f);
	
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
}

void EffekseerEffectBase::JsonInput(const nlohmann::json& _json)
{
	KdGameObject::JsonInput(_json);
	if (_json.contains("distance")) m_distance = _json["distance"].get<float>();
	if (_json.contains("EffectSpeed")) m_effectSpeed = _json["EffectSpeed"].get<float>();
	if (_json.contains("effectColor")) m_effectColor = JSON_MANAGER.JsonToVector4(_json["effectColor"]);
}

bool EffekseerEffectBase::ModelLoad(std::string _path)
{
	// .~ 以降の拡張子を識別するために部分文字列を取得
	std::string ext = _path.substr(_path.find_last_of('.') + 1);

	if (ext == "efkefc")
	{
		m_path = _path;
		return true;
	}

	// それ以外は読み込み失敗
	return false;
}
