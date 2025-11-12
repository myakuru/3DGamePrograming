#pragma once
class AfterImage
{

private:

	// 残像関連
	struct AfterImageFrame
	{
		// 各ノードの worldTransform（モデルローカル空間）
		std::vector<Math::Matrix> nodeWorlds;

		// その時点の m_mWorld
		Math::Matrix ownerWorld = Math::Matrix::Identity;

		// このフレームの不透明度
		float alpha = 1.0f;

		// 描画用テンポラリ
		std::unique_ptr<KdModelWork> m_afterImageWork;
	};

	AfterImageFrame m_afterImageFrame;

	// 残像設定・状態
	bool m_afterImageEnable = false;

	// 残像最大数
	int  m_afterImageMax = 1;

	// 何フレームごとに保存するか
	float  m_afterImageInterval = 0.1f;

	// カウンタ
	float m_afterImageCounter = 0.0f;

	// 基本色（半透明白）
	Math::Color m_afterImageColor = { 0,1,1,0.1f };

	// 残像フレーム群
	std::deque<AfterImageFrame> m_afterImages;


public:
	AfterImage() = default;
	~AfterImage() = default;

	void CaptureAfterImage(KdModelWork* _work,Math::Matrix  _wordMat);
	void DrawAfterImages();

	AfterImageFrame& SetAfterImageFrame() { return m_afterImageFrame; }

	AfterImageFrame const& GetAfterImageFrame() const { return m_afterImageFrame; }

	// 残像の状態設定(残像を使用するか？、残像の最台数、残像を何秒間表示するか、カラー)
	void AddAfterImage(bool _flg = false, int _max = 0, float _nterval = 0, const Math::Color& _color = { 0,0,0,0 })
	{
		m_afterImageEnable = _flg;
		m_afterImageMax = _max;
		m_afterImageInterval = _nterval;
		m_afterImageColor = _color;
	}

};