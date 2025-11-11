#include "KdPostProcessShader.h"
#include"../../../Application/GameObject/Utility/Time.h"
#include"../../../Application/main.h"

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// シェーダー本体の生成、定数バッファの生成
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdPostProcessShader::Init()
{
	// VS と InputLayout作成
	{
#include "KdPostProcessShader_VS.shaderInc"

		if (FAILED(KdDirect3D::Instance().WorkDev()->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_VS))) {
			assert(0 && "頂点シェーダー作成失敗");
			Release();
			return false;
		}

		std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		if (FAILED(KdDirect3D::Instance().WorkDev()->CreateInputLayout(
			&layout[0], (UINT)layout.size(), compiledBuffer,
			sizeof(compiledBuffer), &m_inputLayout)))
		{
			assert(0 && "CreateInputLayout失敗");
			Release();
			return false;
		}
	}

	// PS 作成
	{
#include "KdPostProcessShader_PS_Blur.shaderInc"

		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(
			compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_Blur)))
		{
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();

			return false;
		}

	}

	{
#include "KdPostProcessShader_PS_DoF.shaderInc"

		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(
			compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_DoF)))
		{
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();

			return false;
		}
	}

	{
#include "KdPostProcessShader_PS_Bright.shaderInc"

		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(
			compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_Bright)))
		{
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();

			return false;
		}
	}
	{
#include "PS_Noise.shaderInc"
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(
			compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_Noise)))
		{
			assert(0 && "ノイズピクセルシェーダー作成失敗");
			Release();
			return false;
		}
	}
	{
#include "PS_RadialBlur.shaderInc"
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(
			compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_RadialBlur)))
		{
			assert(0 && "放射状ブラーシェーダー作成失敗");
			Release();
			return false;
		}
	}

	m_cb0_NoiseInfo.Create();

	m_cb0_BlurInfo.Create();

	m_cb0_DoFInfo.Create();

	m_cb0_BrightInfo.Create();

	m_cb0_RadialBlurInfo.Create();	// 放射ブラー用

	// 1440pでレンダーターゲット生成
	const int renderWidth = 2560;
	const int renderHeight = 1440;

	const int halfW = renderWidth / 2;
	const int halfH = renderHeight / 2;
	const int quarterW = renderWidth / 2;
	const int quarterH = renderHeight / 2;

	m_postEffectRTPack.CreateRenderTarget(renderWidth, renderHeight, true);
	m_noiseRTPack.CreateRenderTarget(renderWidth, renderHeight);
	m_radialBlurRTPack.CreateRenderTarget(renderWidth, renderHeight);

	m_blurRTPack.CreateRenderTarget(halfW, halfH);
	m_depthOfFieldRTPack.CreateRenderTarget(halfW, halfH);
	m_brightEffectRTPack.CreateRenderTarget(halfW, halfH);

	m_strongBlurRTPack.CreateRenderTarget(quarterW, quarterH);

	int lightBloomWidth = halfW;
	int lightBloomHeight = halfH;
	for (int i = 0; i < kLightBloomNum; ++i) {
		m_lightBloomRTPack[i].CreateRenderTarget(lightBloomWidth, lightBloomHeight);
		//lightBloomWidth = std::max(1, lightBloomWidth / 2);
		//lightBloomHeight = std::max(1, lightBloomHeight / 2);
	}

	// 画面全体に書き込む用の頂点情報
	m_screenVert[0] = { {-1,-1,0}, {0, 1} };
	m_screenVert[1] = { {-1, 1,0}, {0, 0} };
	m_screenVert[2] = { { 1,-1,0}, {1, 1} };
	m_screenVert[3] = { { 1, 1,0}, {1, 0} };

	SetBrightThreshold(1.2f);

	return true;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// シェーダー本体の解放、定数バッファの解放
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdPostProcessShader::Release()
{
	KdSafeRelease(m_VS);

	KdSafeRelease(m_inputLayout);

	KdSafeRelease(m_PS_Blur);
	KdSafeRelease(m_PS_DoF);
	KdSafeRelease(m_PS_Bright);
	KdSafeRelease(m_PS_Noise);
	KdSafeRelease(m_PS_RadialBlur);


	m_cb0_BlurInfo.Release();
	m_cb0_DoFInfo.Release();
	m_cb0_BrightInfo.Release();
	m_cb0_NoiseInfo.Release();
	m_cb0_RadialBlurInfo.Release();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdPostProcessShader::Draw()
{
	// ポストエフェクトテクスチャの描画クリア
	m_postEffectRTPack.ClearTexture();

	// 光源描画テクスチャの描画クリア
	m_brightEffectRTPack.ClearTexture(kBlackColor);

	// レンダーターゲット変更
	if (!m_postEffectRTChanger.ChangeRenderTarget(m_postEffectRTPack))
	{
		// 失敗したらUndo
		m_postEffectRTChanger.UndoRenderTarget();
	}

}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdPostProcessShader::BeginBright()
{
	// 深度を使わないため DSV は nullptr にする（RTV と DSV のサイズ不一致を回避）
	if (!m_brightRTChanger.ChangeRenderTarget(
		m_brightEffectRTPack.m_RTTexture, nullptr, &m_brightEffectRTPack.m_viewPort))
	{
		m_brightRTChanger.UndoRenderTarget();
	}

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);
	KdShaderManager::Instance().ChangeDepthStencilState(KdDepthStencilState::ZWriteDisable);
}

void KdPostProcessShader::EndBright()
{
	KdShaderManager::Instance().UndoDepthStencilState();

	KdShaderManager::Instance().UndoBlendState();

	m_brightRTChanger.UndoRenderTarget();
}

void KdPostProcessShader::PostEffectProcess()
{
	m_postEffectRTChanger.UndoRenderTarget();

	LightBloomProcess();
	BlurProcess();
	DepthOfFieldProcess();
	NoiseProcess();
	RadialBlurProcess();

	Math::Viewport vp;
	KdDirect3D::Instance().CopyViewportInfo(vp);

	// ノイズを条件付きで合成
	if (m_enableNoise)
	{
		KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);
		KdShaderManager::Instance().m_spriteShader.DrawTex(m_noiseRTPack.m_RTTexture.get(), 0, 0, vp.width, vp.height);
		KdShaderManager::Instance().UndoBlendState();
	}

	// グレースケールも条件付きで合成
	if (m_enableGray)
	{
		// グレースケール用の合成処理
		KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
		KdShaderManager::Instance().m_spriteShader.DrawTex(m_noiseRTPack.m_RTTexture.get(), 0, 0, vp.width, vp.height);
		KdShaderManager::Instance().UndoBlendState();
	}

	if (m_enableRadialBlur)
	{
		// 放射状ブラー合成
		KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
		KdShaderManager::Instance().m_spriteShader.DrawTex(m_radialBlurRTPack.m_RTTexture.get(), 0, 0, vp.width, vp.height);
		KdShaderManager::Instance().UndoBlendState();
		return;
	}

	// --- 最終合成結果を画面に出力 ---
	KdShaderManager::Instance().m_spriteShader.DrawTex(m_noiseRTPack.m_RTTexture.get(), 0, 0, vp.width, vp.height);
}

void KdPostProcessShader::LightBloomProcess()
{
	SetBrightToDevice();

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

	// 高輝度抽出
	DrawTexture(&m_postEffectRTPack.m_RTTexture, 1, m_brightEffectRTPack.m_RTTexture, &m_brightEffectRTPack.m_viewPort);

	KdShaderManager::Instance().UndoBlendState();

	// LightBloom画像の作成
	SetBlurToDevice();

	std::shared_ptr<KdTexture> srcRTTex = m_brightEffectRTPack.m_RTTexture;

	for (int i = 0; i < kLightBloomNum; ++i)
	{
		GenerateBlurTexture(srcRTTex, m_lightBloomRTPack[i].m_RTTexture, m_lightBloomRTPack[i].m_viewPort, kBlurSamplingRadius);

		srcRTTex = m_lightBloomRTPack[i].m_RTTexture;
	}

	KdRenderTargetChanger RTChanger;
	RTChanger.ChangeRenderTarget(m_postEffectRTPack);

	KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Linear_Clamp);

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

	// 光源ぼかし画像の合成
	for (int i = 0; i < kLightBloomNum; ++i)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(m_lightBloomRTPack[i].m_RTTexture.get(), 0, 0, m_postEffectRTPack.m_RTTexture->GetWidth(), m_postEffectRTPack.m_RTTexture->GetHeight());
	}

	RTChanger.UndoRenderTarget();

	KdShaderManager::Instance().UndoBlendState();

	KdShaderManager::Instance().UndoSamplerState();
}

void KdPostProcessShader::BlurProcess()
{
	SetBlurToDevice();

	GenerateBlurTexture(m_postEffectRTPack.m_RTTexture, m_blurRTPack.m_RTTexture, m_blurRTPack.m_viewPort, kBlurSamplingRadius);

	GenerateBlurTexture(m_blurRTPack.m_RTTexture, m_strongBlurRTPack.m_RTTexture, m_strongBlurRTPack.m_viewPort, kBlurSamplingRadius);
}

void KdPostProcessShader::DepthOfFieldProcess()
{
	SetDoFToDevice();

	std::shared_ptr<KdTexture> srcTexList[4] =
	{
		m_postEffectRTPack.m_RTTexture,
		m_blurRTPack.m_RTTexture,
		m_strongBlurRTPack.m_RTTexture,
		m_postEffectRTPack.m_ZBuffer
	};

	DrawTexture(srcTexList, 4, m_depthOfFieldRTPack.m_RTTexture, &m_depthOfFieldRTPack.m_viewPort);
}

void KdPostProcessShader::NoiseProcess()
{
	// 定数バッファセット
	m_cb0_NoiseInfo.Work().Time = Time::Instance().GetElapsedTime();
	m_cb0_NoiseInfo.Work().EnableGray = m_enableGray ? 1 : 0;
	m_cb0_NoiseInfo.Work().EnableNoise = m_enableNoise ? 1 : 0;
	m_cb0_NoiseInfo.Write();

	ID3D11DeviceContext* DevCon = KdDirect3D::Instance().WorkDevContext();
	DevCon->PSSetConstantBuffers(3, 1, m_cb0_NoiseInfo.GetAddress());

	KdShaderManager& shaderMgr = KdShaderManager::Instance();
	if (shaderMgr.SetVertexShader(m_VS))
	{
		DevCon->IASetInputLayout(m_inputLayout);
	}
	shaderMgr.SetPixelShader(m_PS_Noise);

	// ポストエフェクトテクスチャにノイズを描画
	m_enableStrongBlur ? DrawTexture(&m_blurRTPack.m_RTTexture, 1, m_noiseRTPack.m_RTTexture, &m_noiseRTPack.m_viewPort) :
							DrawTexture(&m_postEffectRTPack.m_RTTexture, 1, m_noiseRTPack.m_RTTexture, &m_noiseRTPack.m_viewPort);
	
}

void KdPostProcessShader::RadialBlurProcess()
{
	if (!m_enableRadialBlur) return;

	// 定数バッファ更新
	m_cb0_RadialBlurInfo.Write();

	ID3D11DeviceContext* DevCon = KdDirect3D::Instance().WorkDevContext();
	// b0 に合わせる
	DevCon->PSSetConstantBuffers(4, 1, m_cb0_RadialBlurInfo.GetAddress());

	KdShaderManager& shaderMgr = KdShaderManager::Instance();
	if (shaderMgr.SetVertexShader(m_VS))
	{
		DevCon->IASetInputLayout(m_inputLayout);
	}
	shaderMgr.SetPixelShader(m_PS_RadialBlur);

	DrawTexture(&m_postEffectRTPack.m_RTTexture, 1, m_radialBlurRTPack.m_RTTexture, &m_radialBlurRTPack.m_viewPort);
}

void KdPostProcessShader::CreateBlurOffsetList(std::vector<Math::Vector3>& dstInfo, const std::shared_ptr<KdTexture>& spSrcTex, int samplingRadius, const Math::Vector2& dir)
{
	Math::Vector2 blurDir = dir;
	blurDir.Normalize();

	// 両サイドのサンプリング回数 ＋ サンプル開始中央のピクセル
	int totalSamplingNum = samplingRadius * 2 + 1;

	// サンプリングするテクセルのオフセット値
	Math::Vector2 texelSize;
	texelSize.x = 1.0f / spSrcTex->GetWidth();
	texelSize.y = 1.0f / spSrcTex->GetHeight();

	dstInfo.resize(totalSamplingNum);

	float totalWeight = 0;
	for (int i = 0; i < totalSamplingNum; ++i)
	{
		int samplingOffset = i - samplingRadius;
		dstInfo[i].x = blurDir.x * (samplingOffset * texelSize.x);
		dstInfo[i].y = blurDir.y * (samplingOffset * texelSize.y);

		// 中心のピクセルのウェイトが大きくなる計算
		float weight = exp(-(samplingOffset * samplingOffset) / 18.0f);

		// サンプリングする各ピクセルに重みをつける
		dstInfo[i].z = weight;
		totalWeight += weight;
	}

	// ウェイトを全体のウェイトから割り算し、各ピクセルのウェイトの意味を割合に置き換える
	// 全部足して1になるように数値を調整する
	for (int i = 0; i < totalSamplingNum; ++i)
	{
		dstInfo[i].z /= totalWeight;
	}
}

void KdPostProcessShader::GenerateBlurTexture(std::shared_ptr<KdTexture>& spSrcTex, std::shared_ptr<KdTexture>& spDstTex, D3D11_VIEWPORT& VP, int blurRadius)
{
	KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Linear_Clamp);

	KdRenderTargetPack tmpBlurRTPack;
	tmpBlurRTPack.CreateRenderTarget(spDstTex->GetWidth(), spDstTex->GetHeight());

	// 横にぼかす
	std::vector<Math::Vector3> horizontalBlurInfo;
    // 入力ソースのテクセルサイズ基準でオフセットを計算
    CreateBlurOffsetList(horizontalBlurInfo, spSrcTex, blurRadius, { 1.0f, 0 });
	SetBlurInfo(horizontalBlurInfo);

	DrawTexture(&spSrcTex, 1, tmpBlurRTPack.m_RTTexture, &tmpBlurRTPack.m_viewPort);

	// 横にぼかした画像を更に縦にぼかす
	std::vector<Math::Vector3> verticalBlurInfo;
    // 縦ブラーは横ブラー結果(一時RT)のテクセルサイズ基準でオフセットを計算
    CreateBlurOffsetList(verticalBlurInfo, tmpBlurRTPack.m_RTTexture, blurRadius, { 0, 1.0f });
	SetBlurInfo(verticalBlurInfo);

	DrawTexture(&tmpBlurRTPack.m_RTTexture, 1, spDstTex, &VP);

	KdShaderManager::Instance().UndoSamplerState();
}

void KdPostProcessShader::DrawTexture(std::shared_ptr<KdTexture>* spSrcTex, int srcTexSize, std::shared_ptr<KdTexture> spDstTex, D3D11_VIEWPORT* pVP)
{
	if (!spSrcTex) { return; }

	KdRenderTargetChanger RTChanger;

	if (spDstTex)
	{
		RTChanger.ChangeRenderTarget(spDstTex, nullptr, pVP);
	}

	ID3D11DeviceContext* pDevCon = KdDirect3D::Instance().WorkDevContext();

	// SRVのセット
	for (int i = 0; i < srcTexSize; ++i)
	{
		pDevCon->PSSetShaderResources(i, 1, spSrcTex[i]->WorkSRViewAddress());
	}

	// テクスチャーの描画
	KdDirect3D::Instance().DrawVertices(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, &m_screenVert[0], sizeof(Vertex));

	// SRVの解放
	ID3D11ShaderResourceView* nullSRV = nullptr;

	for (int i = 0; i < srcTexSize; ++i)
	{
		pDevCon->PSSetShaderResources(i, 1, &nullSRV);
	}

	RTChanger.UndoRenderTarget();
}

void KdPostProcessShader::SetBlurInfo(const std::shared_ptr<KdTexture>& spSrcTex, int samplingRadius, const Math::Vector2& dir)
{
	std::vector<Math::Vector3> blurOffsetList;

	CreateBlurOffsetList(blurOffsetList, spSrcTex, samplingRadius, dir);

	SetBlurInfo(blurOffsetList);
}

void KdPostProcessShader::SetBlurInfo(const std::vector<Math::Vector3>& srcInfo)
{
	KdPostProcessShader::cbBlur& blurInfo = m_cb0_BlurInfo.Work();

	blurInfo.SamplingNum = (signed)srcInfo.size();

	if (blurInfo.SamplingNum > kMaxSampling)
	{
		assert(0 && "サンプリング指定回数が上限を超えています。");

		blurInfo.SamplingNum = 0;

		return;
	}

	for (int i = 0; i < blurInfo.SamplingNum; ++i)
	{
		blurInfo.Info[i].x = srcInfo[i].x;
		blurInfo.Info[i].y = srcInfo[i].y;
		blurInfo.Info[i].z = srcInfo[i].z;
	}

	m_cb0_BlurInfo.Write();
}

void KdPostProcessShader::SetBlurToDevice()
{
	ID3D11DeviceContext* DevCon = KdDirect3D::Instance().WorkDevContext();
	if (!DevCon) { return; }

	m_cb0_BlurInfo.Write();

	KdDirect3D::Instance().WorkDevContext()->PSSetConstantBuffers(0, 1, m_cb0_BlurInfo.GetAddress());

	KdShaderManager& shaderMgr = KdShaderManager::Instance();

	if (shaderMgr.SetVertexShader(m_VS))
	{
		DevCon->IASetInputLayout(m_inputLayout);
	}

	shaderMgr.SetPixelShader(m_PS_Blur);
}

void KdPostProcessShader::SetDoFToDevice()
{
	ID3D11DeviceContext* DevCon = KdDirect3D::Instance().WorkDevContext();
	if (!DevCon) { return; }

	m_cb0_DoFInfo.Write();

	KdDirect3D::Instance().WorkDevContext()->PSSetConstantBuffers(0, 1, m_cb0_DoFInfo.GetAddress());

	KdShaderManager& shaderMgr = KdShaderManager::Instance();

	if (shaderMgr.SetVertexShader(m_VS))
	{
		DevCon->IASetInputLayout(m_inputLayout);
	}

	shaderMgr.SetPixelShader(m_PS_DoF);
}

void KdPostProcessShader::SetBrightToDevice()
{
	ID3D11DeviceContext* DevCon = KdDirect3D::Instance().WorkDevContext();
	if (!DevCon) { return; }

	m_cb0_BrightInfo.Write();

	KdDirect3D::Instance().WorkDevContext()->PSSetConstantBuffers(0, 1, m_cb0_BrightInfo.GetAddress());

	KdShaderManager& shaderMgr = KdShaderManager::Instance();

	if (shaderMgr.SetVertexShader(m_VS))
	{
		DevCon->IASetInputLayout(m_inputLayout);
	}

	shaderMgr.SetPixelShader(m_PS_Bright);
}
