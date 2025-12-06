#pragma once
class Player;
class Enemy;
class EffekseerEffectManager;
class EffekseerEffectBase : public KdGameObject
{
public:

	EffekseerEffectBase() = default;
	~EffekseerEffectBase() override = default;

	void SetPlayEffect(bool once) 
	{
		m_load = once;
	}

	// エフェクトが再生中か
	bool IsEffectPlaying() const { return m_isEffectPlaying; }

	// エフェクトを終了させる
	void StopEffect()
	{
		// 追加: 複数同時再生の個体も停止
		for (auto it = m_effects.begin(); it != m_effects.end(); ++it)
		{
			if (auto e = it->lock()) { e->StopEffect(); }
		}
		m_effects.clear();

		if (auto effect = m_wpEffect.lock(); effect)
		{
			effect->StopEffect();
			m_isEffectPlaying = false;
		}
		m_load = false;
		m_spawnOnce = false; // ワンショットフラグも解除
	}

	// 任意ターゲットに対してエフェクトを即時再生（GetPos/GetRotationQuaternion必須）
	template<typename T>
	void PlayForTarget(const std::shared_ptr<T>& target)
	{
		if (!target) return;

		// ワンショット: 既に一度再生していたら何もしない
		if (m_spawnOnce) return;

		// 前方・右方向ベクトル
		Math::Vector3 forward = Math::Vector3::TransformNormal(
			Math::Vector3::Forward,
			Math::Matrix::CreateFromQuaternion(target->GetRotationQuaternion()));
		Math::Vector3 right = Math::Vector3::TransformNormal(
			Math::Vector3::Right,
			Math::Matrix::CreateFromQuaternion(target->GetRotationQuaternion()));
		forward.Normalize();
		right.Normalize();

		// ワールド行列
		Math::Matrix mWorld = Math::Matrix::CreateScale(m_scale);
		mWorld *= Math::Matrix::CreateFromYawPitchRoll(
			DirectX::XMConvertToRadians(m_degree.y),
			DirectX::XMConvertToRadians(m_degree.x),
			DirectX::XMConvertToRadians(m_degree.z))
			* Math::Matrix::CreateFromQuaternion(target->GetRotationQuaternion());
		mWorld.Translation(m_position + target->GetPos() + (forward * m_distance) + (right * m_sideDistance));

		// 毎回スポーン（複数同時対応）
		if (auto sp = KdEffekseerManager::GetInstance().Play(m_path, mWorld, m_effectSpeed, false, m_effectColor).lock())
		{
			m_effects.emplace_back(sp);
			m_isEffectPlaying = true;
			m_spawnOnce = true; // 一度だけ
		}
	}

	// エフェクト再生

	void Init() override;
	void Update() override;

	const std::string& GetName() const override
	{
		return m_name;
	}

protected:

	// 表示はベースシーンで行ってるので、ここでは何もしない

	virtual void EffectUpdate();

	std::weak_ptr<Player> m_player;
	std::weak_ptr<Enemy> m_enemy;

	void ImGuiInspector() override;
	void JsonSave(nlohmann::json& _json) const override;
	void JsonInput(const nlohmann::json& _json) override;

	bool ModelLoad(const std::string& _path) override;

	bool m_once = false; // エフェクトを１回だけ再生する
	bool m_load = false; // エフェクトがロードされたか
	bool m_spawnOnce = false; // PlayForTargetのワンショットフラグ

	// 現在再生中のエフェクト参照（単一）
	std::weak_ptr<KdEffekseerObject> m_wpEffect;
	// 複数同時再生を管理
	std::vector<std::weak_ptr<KdEffekseerObject>> m_effects;

	// 再生状態
	bool m_isEffectPlaying = false;

	float m_distance = 0.0f;
	float m_sideDistance = 0.0f;
	float m_effectSpeed = 0.0f;

	Math::Color m_effectColor = Math::Color{ 1,1,1,1 };

	std::weak_ptr<EffekseerEffectManager> m_effectManager;

	std::string m_name;

};