#include "FieldEffect.h"
#include"../../../Camera/PlayerCamera/PlayerCamera.h"
#include"../../../../Scene/SceneManager.h"

const uint32_t FieldEffect::TypeID = KdGameObject::GenerateTypeID();

void FieldEffect::Init()
{
	SelectDraw3dPolygon::Init();

	const float fieldMin = -100.0f;
	const float fieldMax = 100.0f;
	const int polyCount = 100; // 生成するポリゴンの数

	for (int i = 0; i < polyCount; ++i)
	{
		PolyInfo poly;
		poly.pos = Math::Vector3(
			KdRandom::GetFloat(fieldMin, fieldMax),
			KdRandom::GetFloat(fieldMin, fieldMax),
			KdRandom::GetFloat(fieldMin, fieldMax)
		);
		poly.scale = Math::Vector3(5.0f, 5.0f, 5.0f);
		m_polys.push_back(poly);
	}

	SceneManager::Instance().GetObjectWeakPtr(m_camera);
}

void FieldEffect::Update()
{

	auto spCamera = m_camera.lock();

	if (!spCamera) return;

	// ビルボード行列
	Math::Matrix rot = Math::Matrix::CreateBillboard
	(
		m_position,													// オブジェクトの位置
		spCamera->GetCamera()->GetCameraMatrix().Translation(),		// カメラの位置
		Math::Vector3::UnitY										// カメラの上方向
	);

	// 最終的なワールド行列
	m_mWorld = Math::Matrix::CreateScale(m_scale) * rot;
}

void FieldEffect::DrawBright()
{
	for (const auto& poly : m_polys)
	{
		// 各ポリゴン用のワールド行列を作成
		Math::Matrix world = Math::Matrix::CreateScale(poly.scale)
			* Math::Matrix::CreateTranslation(poly.pos)
			* m_mWorld; // ビルボード行列を適用

		KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_polygon, world, m_color);
	}
}
