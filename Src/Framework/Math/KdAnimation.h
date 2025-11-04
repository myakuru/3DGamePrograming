#pragma once

// アニメーションキー(クォータニオン
struct KdAnimKeyQuaternion
{
	float				m_time = 0;		// 時間
	Math::Quaternion	m_quat;			// クォータニオンデータ
};

// アニメーションキー(ベクトル
struct KdAnimKeyVector3
{
	float				m_time = 0;		// 時間
	Math::Vector3		m_vec;			// 3Dベクトルデータ
};

//============================
// アニメーションデータ
//============================
struct KdAnimationData
{
	// アニメーション名
	std::string		m_name;
	// アニメの長さ
	float			m_maxLength = 0;

	// １ノードのアニメーションデータ
	struct Node
	{
		int			m_nodeOffset = -1;	// 対象モデルノードのOffset値

		// 各チャンネル
		std::vector<KdAnimKeyVector3>		m_translations;	// 位置キーリスト
		std::vector<KdAnimKeyQuaternion>	m_rotations;	// 回転キーリスト
		std::vector<KdAnimKeyVector3>		m_scales;		// 拡縮キーリスト

		void Interpolate(Math::Matrix& rDst, float time);
		bool InterpolateTranslations(Math::Vector3& result, float time);
		bool InterpolateRotations(Math::Quaternion& result, float time);
		bool InterpolateScales(Math::Vector3& result, float time);
	};

	// 全ノード用アニメーションデータ
	std::vector<Node>	m_nodes;
};

class KdAnimator
{
public:

	// 新API: 第2引数はブレンド秒
	void SetAnimation(const std::shared_ptr<KdAnimationData>& rData, float blendTime = 0.25f, bool isLoop = true);


	// アニメーションが終了してる？
	bool IsAnimationEnd() const
	{
		// 再生中の実体（ブレンド中はNext、完了後はNow）を優先して判定
		const KdAnimator* pActive = m_pNextAnimator ? m_pNextAnimator.get() : m_pNowAnimator.get();
		if (pActive && pActive->m_spAnimation)
		{
			return (pActive->m_time >= pActive->m_spAnimation->m_maxLength);
		}


		// 再生中のアニメーションが無い場合は終了してるとみなす
		if (m_spAnimation == nullptr) { return true; }
		return (m_time >= m_spAnimation->m_maxLength);
	}

	float GetTime() { return m_blendTimer.GetNowFrame(); }

	// アニメーションの最大の長さを取得
	float GetMaxAnimationTime() const
	{
		if (m_spAnimation) { return m_spAnimation->m_maxLength; }
		return 0.0f;
	}
	
	// アニメーションの更新
	void AdvanceTime(std::vector<KdModelWork::Node>& rNodes, float speed = 1.0f);


	// 実アニメの再生時間(ブレンド中は Next 優先)
	float GetPlayTime() const;
	// 実アニメの総長
	float GetPlayLength() const;
	// 0～1 の進行率
	float GetPlayProgress() const;

private:

	inline void RegistAnimation(const std::shared_ptr<KdAnimationData>& rData, bool isLoop = true)
	{
		m_spAnimation = rData;
		m_isLoop = isLoop;
	}

	// アニメーションのブレンド更新
	void UpdateBlendAnimation(std::vector<KdModelWork::Node>& rNodes, float speed = 1.0f, float BlendWeight = 1.0f);

	std::shared_ptr<KdAnimationData>	m_spAnimation = nullptr;	// 再生するアニメーションデータ

	std::unique_ptr<KdAnimator>			m_pNowAnimator;				// 現状再生中のアニメーション
	std::unique_ptr<KdAnimator>			m_pNextAnimator;			// 次に遷移するアニメーション

	float m_time = 0.0f;

	bool m_isLoop = false;

	KdFrameTimer						m_blendTimer;				// ブレンド時間管理用タイマー
};
