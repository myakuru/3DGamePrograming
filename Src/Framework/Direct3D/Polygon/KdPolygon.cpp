#include "KdPolygon.h"

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 判定用ポリゴン頂点座標のコピー
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdPolygon::GetPositions(std::vector<Math::Vector3>& res) const
{
	res.resize(m_vertices.size());

	for (size_t i = 0; i < m_vertices.size(); ++i)
	{
		res[i] = m_vertices[i].pos;
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// テクスチャデータからマテリアルの作成
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// テクスチャデータからファイルパスを参照してファイルパスからマテリアルを作成する関数を呼び出す
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdPolygon::SetMaterial(const std::shared_ptr<KdTexture>& spBaseColTex)
{
	if (!spBaseColTex) { return; }

	if (!m_spMaterial) { m_spMaterial = std::make_shared<KdMaterial>(); }

	if (spBaseColTex->GetFilepath() == "")
	{
		m_spMaterial->SetTextures(spBaseColTex, nullptr, nullptr, nullptr);
	}
	else
	{
		SetMaterial(spBaseColTex->GetFilepath());
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ファイルパスからマテリアルを作成
// ベースカラーテクスチャと同階層にある特定の名前のファイルを自動的に読み込む
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// ファイル名_mtrf.png：金属性・粗さが書き込まれたテクスチャ
// ファイル名_emi.png ：光源が書き込まれたテクスチャ
// ファイル名_nml.png ：法線情報が書き込まれたテクスチャ
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdPolygon::SetMaterial(const std::string& filePath)
{
	if (!m_spMaterial) { m_spMaterial = std::make_shared<KdMaterial>(); }

	m_spMaterial->m_name = filePath;

	// ファイルパス生成用に引数のファイルパスを分解
	std::string dirPath = KdGetDirFromPath(filePath);
	std::string fileName = KdGetNameFromPath(filePath, true);

	m_spMaterial->SetTextures(dirPath, fileName + ".png", fileName + "_mtrf.png",
		fileName + "_emi.png", fileName + "_nml.png");
}
