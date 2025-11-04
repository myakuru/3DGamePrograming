#include "KdAnimation.h"
#include "../Direct3D/KdModel.h"

// 二分探索で、指定時間から次の配列要素のKeyIndexを求める関数
// list		… キー配列
// time		… 時間
// 戻り値	… 次の配列要素のIndex
template<class T>
int BinarySearchNextAnimKey(const std::vector<T>& list, float time)
{
	int low = 0;
	int high = (int)list.size();
	while (low < high)
	{
		int mid = (low + high) / 2;
		float midTime = list[mid].m_time;

		if (midTime <= time) low = mid + 1;
		else high = mid;
	}
	return low;
}

bool KdAnimationData::Node::InterpolateTranslations(Math::Vector3& result, float time)
{
	if (m_translations.size() == 0)return false;

	// キー位置検索
	UINT keyIdx = BinarySearchNextAnimKey(m_translations, time);

	// 先頭のキーなら、先頭のデータを返す
	if (keyIdx == 0) {
		result = m_translations.front().m_vec;
		return true;
	}
	// 配列外のキーなら、最後のデータを返す
	else if (keyIdx >= m_translations.size()) {
		result = m_translations.back().m_vec;
		return true;
	}
	// それ以外(中間の時間)なら、その時間の値を補間計算で求める
	else {
		auto& prev = m_translations[keyIdx - 1];	// 前のキー
		auto& next = m_translations[keyIdx];		// 次のキー
		// 前のキーと次のキーの時間から、0～1間の時間を求める
		float f = (time - prev.m_time) / (next.m_time - prev.m_time);
		// 補間
		result = DirectX::XMVectorLerp(
			prev.m_vec,
			next.m_vec,
			f
		);
	}

	return true;
}

bool KdAnimationData::Node::InterpolateRotations(Math::Quaternion& result, float time)
{
	if (m_rotations.size() == 0)return false;

	// キー位置検索
	UINT keyIdx = BinarySearchNextAnimKey(m_rotations, time);
	// 先頭のキーなら、先頭のデータを返す
	if (keyIdx == 0) {
		result = m_rotations.front().m_quat;
	}
	// 配列外のキーなら、最後のデータを返す
	else if (keyIdx >= m_rotations.size()) {
		result = m_rotations.back().m_quat;
	}
	// それ以外(中間の時間)なら、その時間の値を補間計算で求める
	else {
		auto& prev = m_rotations[keyIdx - 1];	// 前のキー
		auto& next = m_rotations[keyIdx];		// 次のキー
		// 前のキーと次のキーの時間から、0～1間の時間を求める
		float f = (time - prev.m_time) / (next.m_time - prev.m_time);
		// 補間
		result = DirectX::XMQuaternionSlerp(
			prev.m_quat,
			next.m_quat,
			f
		);
	}

	return true;
}

bool KdAnimationData::Node::InterpolateScales(Math::Vector3& result, float time)
{
	if (m_scales.size() == 0)return false;

	// キー位置検索
	UINT keyIdx = BinarySearchNextAnimKey(m_scales, time);

	// 先頭のキーなら、先頭のデータを返す
	if (keyIdx == 0) {
		result = m_scales.front().m_vec;
		return true;
	}
	// 配列外のキーなら、最後のデータを返す
	else if (keyIdx >= m_scales.size()) {
		result = m_scales.back().m_vec;
		return true;
	}
	// それ以外(中間の時間)なら、その時間の値を補間計算で求める
	else {
		auto& prev = m_scales[keyIdx - 1];	// 前のキー
		auto& next = m_scales[keyIdx];		// 次のキー
		// 前のキーと次のキーの時間から、0～1間の時間を求める
		float f = (time - prev.m_time) / (next.m_time - prev.m_time);
		// 補間
		result = DirectX::XMVectorLerp(
			prev.m_vec,
			next.m_vec,
			f
		);
	}

	return true;
}

void KdAnimationData::Node::Interpolate(Math::Matrix& rDst, float time)
{
	// ベクターによる拡縮補間
	bool isChange = false;
	Math::Matrix scale;
	Math::Vector3 resultVec;
	if (InterpolateScales(resultVec, time))
	{
		scale = scale.CreateScale(resultVec);
		isChange = true;
	}

	// クォタニオンによる回転補間
	Math::Matrix rotate;
	Math::Quaternion resultQuat;
	if (InterpolateRotations(resultQuat, time))
	{
		rotate = rotate.CreateFromQuaternion(resultQuat);
		isChange = true;
	}

	// ベクターによる座標補間
	Math::Matrix trans;
	if (InterpolateTranslations(resultVec, time))
	{
		trans = trans.CreateTranslation(resultVec);
		isChange = true;
	}

	if (isChange)
	{
		rDst = scale * rotate * trans;
	}
}

void KdAnimator::UpdateBlendAnimation(std::vector<KdModelWork::Node>& rNodes, float speed, float BlendWeight)
{
	if (!m_spAnimation) { return; }

	// 全てのアニメーションノード（モデルの行列を補間する情報）の行列補間を実行する
	for (auto& rAnimNode : m_spAnimation->m_nodes)
	{
		// 対応するモデルノードのインデックス
		UINT idx = rAnimNode.m_nodeOffset;

		// ブレンドの比重が10以上ならそのまま補間
		if (BlendWeight >= 1.0f)
		{
			// アニメーションデータによる行列補間
			rAnimNode.Interpolate(rNodes[idx].m_localTransform, m_time);
		}
		// ブレンド比重に従ってブレンディング補間
		else
		{
			Math::Matrix NextAnim;

			rAnimNode.Interpolate(NextAnim, m_time);

			Math::Vector3 scale, nextScale;
			Math::Quaternion rotate, nextRotate;
			Math::Vector3 trans, nextTrans;

			rNodes[idx].m_localTransform.Decompose(scale, rotate, trans);
			NextAnim.Decompose(nextScale, nextRotate, nextTrans);

			// 補間拡大率生成
			Math::Vector3 blendScale = DirectX::XMVectorLerp(scale, nextScale, BlendWeight);

			// 補間回転クォタニオン生成
			Math::Quaternion blendRotate = DirectX::XMQuaternionSlerp(rotate, nextRotate, BlendWeight);

			// 補間座標生成
			Math::Vector3 blendTrans = DirectX::XMVectorLerp(trans, nextTrans, BlendWeight);

			// 拡大率の補間
			Math::Matrix mScale = Math::Matrix::CreateScale(blendScale);

			// 回転角度の補間
			Math::Matrix mRotate = Math::Matrix::CreateFromQuaternion(blendRotate);

			rNodes[idx].m_localTransform = mScale * mRotate;

			// 座標の補間
			rNodes[idx].m_localTransform.Translation(blendTrans);
		}
	}

	// アニメーションのフレームを進める
	m_time += speed;

	// アニメーションデータの最後のフレームを超えたら
	if (m_time >= m_spAnimation->m_maxLength)
	{
		if (m_isLoop)
		{
			// アニメーションの最初に戻る（ループさせる
			m_time = 0.0f;
		}
		else
		{
			m_time = m_spAnimation->m_maxLength;
		}
	}
}

void KdAnimator::SetAnimation(const std::shared_ptr<KdAnimationData>& rData, float blendTime, bool isLoop)
{
	if (!rData) { return; }

	// 遷移中のアニメーションがあったら、一気に遷移してしまう
	if (m_pNextAnimator)
	{
		m_pNowAnimator = std::move(m_pNextAnimator);
	}

	m_pNextAnimator = std::make_unique<KdAnimator>();
	m_pNextAnimator->RegistAnimation(rData, isLoop);

	m_blendTimer.Restart();
	m_blendTimer.SetSec(blendTime);
}

void KdAnimator::AdvanceTime(std::vector<KdModelWork::Node>& rNodes, float speed)
{
	if (m_pNowAnimator)
	{
		// アニメーション更新
		m_pNowAnimator->UpdateBlendAnimation(rNodes, speed);
	}

	// 遷移先のアニメーションあり？無ければブレンド無し
	if (m_pNextAnimator)
	{
		// ブレンドアニメーション
		m_pNextAnimator->UpdateBlendAnimation(rNodes, speed, m_blendTimer.GetProgress());

		// ブレンドタイマーの更新
		m_blendTimer.Update(speed);

		if (m_blendTimer.IsEnd())
		{
			// 遷移先のアニメーションユニットを再生中アニメーションに置き換え
			m_pNowAnimator = std::move(m_pNextAnimator);
			// 遷移先（ブレンド先）を空にする
			m_pNextAnimator = nullptr;

			// タイマーのリセット
			m_blendTimer.Restart();
		}
	}
}

float KdAnimator::GetPlayTime() const
{
	const KdAnimator* pActive = m_pNextAnimator ? m_pNextAnimator.get() : m_pNowAnimator.get();
	if (pActive) return pActive->m_time;
	return m_time;
}

float KdAnimator::GetPlayLength() const
{
	const KdAnimator* pActive = m_pNextAnimator ? m_pNextAnimator.get() : m_pNowAnimator.get();
	if (pActive && pActive->m_spAnimation) return pActive->m_spAnimation->m_maxLength;
	if (m_spAnimation) return m_spAnimation->m_maxLength;
	return 0.0f;
}

float KdAnimator::GetPlayProgress() const
{
	float len = GetPlayLength();
	if (len <= 0.0f) return 0.0f;
	return GetPlayTime() / len;
}
