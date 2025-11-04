#pragma once

//==========================================================
// シェーダー描画用マテリアル
//==========================================================
struct KdMaterial
{
	void SetTextures(const std::shared_ptr<KdTexture>& spBaseColTex,
		const std::shared_ptr<KdTexture>& spMtRfColTex, const std::shared_ptr<KdTexture>& spEmiColTex,
		const std::shared_ptr<KdTexture>& spNmlColTex);

	void SetTextures(const std::string& fileDir, const std::string& baseColName,
		const std::string& mtRfColName, const std::string& emiColName, const std::string& nmlColName);

	//---------------------------------------
	// 材質データ
	//---------------------------------------
	// 名前
	std::string					m_name;

	// 基本色
	std::shared_ptr<KdTexture>	m_baseColorTex = nullptr;
	Math::Vector4				m_baseColorRate = kWhiteColor;

	// B:金属性 G:粗さ
	std::shared_ptr<KdTexture>	m_metallicRoughnessTex = nullptr;
	float						m_metallicRate = 0.0f;
	float						m_roughnessRate = 1.0f;

	// 自己発光
	std::shared_ptr<KdTexture>	m_emissiveTex = nullptr;
	Math::Vector3				m_emissiveRate = Math::Vector3::One;

	// 法線マップ
	std::shared_ptr<KdTexture>	m_normalTex = nullptr;
};
