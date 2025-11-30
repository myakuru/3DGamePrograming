#pragma once
#include"../../../MyFramework/State/StateManager/StateManager.h"
class Player;
class CameraBase : public KdGameObject
{
public:
	CameraBase() = default;
	virtual ~CameraBase()	override = default;

	void Init()				override;
	void PreDraw()			override;
	void ImGuiInspector()	override;
	void JsonSave(nlohmann::json& _json) const override;
	void JsonInput(const nlohmann::json& _json) override;

	//void SetTarget(const std::shared_ptr<KdGameObject>& target);

	// 「絶対変更しません！見るだけ！」な書き方
	const std::shared_ptr<KdCamera>& GetCamera() const
	{
		return m_spCamera;
	}

	// 「中身弄るかもね」な書き方
	std::shared_ptr<KdCamera> WorkCamera() const
	{
		return m_spCamera;
	}

	const Math::Matrix GetRotationMatrix()const
	{
		return Math::Matrix::CreateFromYawPitchRoll(
		       DirectX::XMConvertToRadians(m_degree.y),
		       DirectX::XMConvertToRadians(m_degree.x),
		       DirectX::XMConvertToRadians(m_degree.z));
	}

	const Math::Quaternion GetRotationQuaternion() const
	{
		return Math::Quaternion::CreateFromYawPitchRoll(
			   DirectX::XMConvertToRadians(m_degree.y),
			   DirectX::XMConvertToRadians(m_degree.x),
			   DirectX::XMConvertToRadians(m_degree.z));
	}

	const Math::Matrix GetRotationYMatrix() const
	{
		return Math::Matrix::CreateRotationY(
			   DirectX::XMConvertToRadians(m_degree.y));
	}

	void RegistHitObject(const std::shared_ptr<KdGameObject>& object)
	{
		m_wpHitObjectList.push_back(object);
	}

	void SwitchShowCursor(bool show)
	{

		// カーソルの表示非表示切り替え (念の為確実に切り替えができるようにしておく)
		int cnt = 0;
		if (show)
		{
			// マウスが表示されるまで実行
			do {
				cnt = ShowCursor(true);
			} while (cnt < 0);
		}
		else
		{
			// マウスが非表示になるまで実行
			do {
				cnt = ShowCursor(false);
			} while (cnt >= 0);
		}
	}

	bool m_enabled = false;

	void UpdateRotateByMouse();
	void UpdateMoveKey();

	Math::Quaternion GetPrevRotation() const { return m_prevRotation; }
	void SetPrevRotation(const Math::Quaternion& rot) { m_prevRotation = rot; }

	Math::Matrix GetRotaionMatrix() const { return m_mRotation; }
	void SetRotaionMatrix(const Math::Matrix& mat) { m_mRotation = mat; }

	Math::Quaternion GetTargetRotation() const { return m_targetRotation; }
	void SetTargetRotation(const Math::Quaternion& rot) { m_targetRotation = rot; }

	std::shared_ptr<KdCamera> GetSpCamera() const { return m_spCamera; }
	void SetSpCamera(const std::shared_ptr<KdCamera>& camera) { m_spCamera = camera; }

protected:

	float moveSpeed = 50.0f; // 移動速度
	bool m_freeCameraFlg = false;

	Math::Quaternion m_rotation = Math::Quaternion::Identity; // 回転用クォータニオン

	std::shared_ptr<KdCamera>					m_spCamera		= nullptr;
	std::weak_ptr<Player>						m_Player;
	std::vector<std::weak_ptr<KdGameObject>>	m_wpHitObjectList{};

	Math::Matrix								m_mLocalPos		= Math::Matrix::Identity;
	Math::Matrix								m_mRotation		= Math::Matrix::Identity;
	Math::Quaternion							m_prevRotation = Math::Quaternion::Identity;
	Math::Quaternion							m_targetRotation = Math::Quaternion::Identity;

	// カメラ回転用マウス座標の差分
	POINT										m_FixMousePos{};

	StateManager m_stateManager;
};