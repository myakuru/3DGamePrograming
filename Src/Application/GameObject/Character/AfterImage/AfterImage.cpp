#include"AfterImage.h"
#include"Application/main.h"

void AfterImage::CaptureAfterImage(KdModelWork* _work, Math::Matrix  _wordMat)
{
	if (!m_afterImageEnable)
	{
		// 残像無効ならクリアして終了
		m_afterImages.clear();
		m_afterImageCounter = 0;
		return;
	}
	// モデルワークが無効なら処理しない
	if (!_work || !_work->IsEnable()) return;
	// m_afterImageIntervalを超えるまでカウンタを進める
	m_afterImageCounter += 100.0f * Application::Instance().GetDeltaTime();
	if (m_afterImageCounter < m_afterImageInterval) return;
	m_afterImageCounter = 0.0f;
	// ノード worldTransform をスナップショット
	const auto& nodes = _work->GetNodes();
	AfterImageFrame frame;
	// ノード worldTransform を保存
	frame.nodeWorlds.resize(nodes.size());
	// 各ノードの worldTransform を保存
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		frame.nodeWorlds[i] = nodes[i].m_worldTransform;
	}
	// 現在の m_mWorld を保存
	frame.ownerWorld = _wordMat;
	Math::Matrix scale = Math::Matrix::CreateScale(1, 1, 1);
	frame.ownerWorld = scale * frame.ownerWorld;
	//先頭に追加し、上限を超えたら末尾を捨てる
	m_afterImages.push_front(std::move(frame));
	while ((int)m_afterImages.size() > m_afterImageMax) m_afterImages.pop_back();
}

void AfterImage::DrawAfterImages()
{
	if (!m_afterImageEnable) return;

	auto& stdShader = KdShaderManager::Instance().m_StandardShader;

	// 古いもの→新しいものの順
	for (int i = (int)m_afterImages.size() - 1; i >= 0; --i)
	{
		// フレームデータ取得
		const auto& frameData = m_afterImages[i];

		// ノード数が異なる場合はスキップ
		auto& workNodes = m_afterImageFrame.m_afterImageWork->WorkNodes();
		if (workNodes.size() != frameData.nodeWorlds.size()) continue;

		// ノード worldTransform を上書き
		for (size_t nodeIndex = 0; nodeIndex < workNodes.size(); ++nodeIndex)
		{
			workNodes[nodeIndex].m_worldTransform = frameData.nodeWorlds[nodeIndex];
		}

		m_afterImageFrame.m_afterImageWork->SetNeedCalcNodeMatrices(false);

		// 残像の描画

		KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

		KdShaderManager::Instance().m_StandardShader.SetRimLightEnable(true);

		KdShaderManager::Instance().m_StandardShader.SetRimLight(5.0f, { 0.2f, 1.0f, 1.0f });

		Math::Matrix scale = Math::Matrix::CreateScale({ 1.009f,1.009f,1.009f });

		static float t = 0.0f;
		t += 0.1f;

		KdShaderManager::Instance().m_StandardShader.SetUVOffset({ t, -t * 0.1f });

		Math::Matrix ownerWorld = scale * frameData.ownerWorld;

		stdShader.DrawModel(*m_afterImageFrame.m_afterImageWork, ownerWorld, m_afterImageColor);

		KdShaderManager::Instance().UndoBlendState();
	}
}