#include "Framework/KdFramework.h"

#include "KdUtility.h"

// viewから画像情報を取得する
void KdGetTextureInfo(ID3D11View* view, D3D11_TEXTURE2D_DESC& outDesc)
{
	outDesc = {};

	ID3D11Resource* res;
	view->GetResource(&res);

	ID3D11Texture2D* tex2D;
	if (SUCCEEDED(res->QueryInterface<ID3D11Texture2D>(&tex2D)))
	{
		tex2D->GetDesc(&outDesc);
		tex2D->Release();
	}
	res->Release();
}

bool ConvertRectToUV(const KdTexture* srcTex, const Math::Rectangle& src, Math::Vector2& uvMin, Math::Vector2& uvMax)
{
	if (!srcTex) { return false; }

	uvMin.x = src.x / (float)srcTex->GetInfo().Width;
	uvMin.y = src.y / (float)srcTex->GetInfo().Height;

	uvMax.x = ( src.width  / (float)srcTex->GetInfo().Width) + uvMin.x;
	uvMax.y = ( src.height / (float)srcTex->GetInfo().Height) + uvMin.y;

	return true;
}

float EaseInOutSine(float progress)
{
	return (float)(-(std::cos(M_PI * progress) - 1.0f) / 2.0f);
}
