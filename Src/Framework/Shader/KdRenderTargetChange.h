#pragma once

//===========================================
//
// レンダーターゲット変更
//
//===========================================
struct KdRenderTargetPack
{
	KdRenderTargetPack() {}

	void CreateRenderTarget(int width, int height, bool needDSV = false, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_VIEWPORT* pVP = nullptr);
	void SetRenderTarget(std::shared_ptr<KdTexture> RTT, std::shared_ptr<KdTexture> DST = nullptr, D3D11_VIEWPORT* pVP = nullptr);

	void SetViewPort(D3D11_VIEWPORT* pVP);

	void ClearTexture(const Math::Color& fillColor = kBlueColor);

	std::shared_ptr<KdTexture> m_RTTexture = nullptr;
	std::shared_ptr<KdTexture> m_ZBuffer = nullptr;
	D3D11_VIEWPORT m_viewPort = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
};

struct KdRenderTargetChanger
{
	~KdRenderTargetChanger() { Release(); }

	ID3D11RenderTargetView* m_saveRTV = nullptr;
	ID3D11DepthStencilView* m_saveDSV = nullptr;
	D3D11_VIEWPORT			m_saveVP = {};
	UINT					m_numVP = 1;
	bool					m_changeVP = false;

	bool Validate(ID3D11RenderTargetView* pRTV);

	bool ChangeRenderTarget(ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV = nullptr, D3D11_VIEWPORT* pVP = nullptr);
	bool ChangeRenderTarget(std::shared_ptr<KdTexture> RTT, std::shared_ptr<KdTexture> DST = nullptr, D3D11_VIEWPORT* pVP = nullptr);
	bool ChangeRenderTarget(KdRenderTargetPack& RTPack);

	void UndoRenderTarget();

	void Release();
};
