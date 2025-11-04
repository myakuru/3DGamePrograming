#include "KdDebugWireFrame.h"

void KdDebugWireFrame::AddDebugLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Color& col)
{
	// デバッグラインの始点
	KdPolygon::Vertex v1;
	v1.color = col.RGBA().v;
	v1.UV = Math::Vector2::Zero;
	v1.pos = start;

	// デバッグラインの終点
	KdPolygon::Vertex v2;
	v2.color = col.RGBA().v;
	v2.UV = Math::Vector2::Zero;
	v2.pos = end;

	m_debugVertices.push_back(v1);
	m_debugVertices.push_back(v2);
}

void KdDebugWireFrame::AddDebugLine(const Math::Vector3& start, const Math::Vector3& dir, float length, const Math::Color& col)
{
	// デバッグラインの始点
	KdPolygon::Vertex v1;
	v1.color = col.RGBA().v;
	v1.UV = Math::Vector2::Zero;
	v1.pos = start;

	// デバッグラインの終点
	KdPolygon::Vertex v2;
	v2.color = col.RGBA().v;
	v2.UV = Math::Vector2::Zero;
	v2.pos = v1.pos + (dir * length);

	m_debugVertices.push_back(v1);
	m_debugVertices.push_back(v2);
}

void KdDebugWireFrame::AddDebugLineFromMatrix(const Math::Matrix& mat, float scale)
{
	Math::Vector3 start = mat.Translation();
	Math::Vector3 vAxis;

	// X軸描画
	vAxis = mat.Right();
	vAxis.Normalize();
	AddDebugLine(start, start + vAxis * scale, kRedColor);

	// Y軸描画
	vAxis = mat.Up();
	vAxis.Normalize();
	AddDebugLine(start, start + vAxis * scale, kGreenColor);

	// Z軸描画
	vAxis = mat.Backward();
	vAxis.Normalize();
	AddDebugLine(start, start + vAxis * scale, kBlueColor);
}

// デバッグスフィアの描画
void KdDebugWireFrame::AddDebugSphere(const Math::Vector3& pos, float radius, const Math::Color& col)
{
	KdPolygon::Vertex v;
	v.UV = Math::Vector2::Zero;
	v.color = col.RGBA().v;

	int detail = 16;
	for (int i = 0; i < detail + 1; ++i)
	{
		// XZ面
		v.pos = pos;
		v.pos.x += cos((float)i * (360 / detail) * KdToRadians) * radius;
		v.pos.z += sin((float)i * (360 / detail) * KdToRadians) * radius;
		m_debugVertices.push_back(v);

		v.pos = pos;
		v.pos.x += cos((float)(i + 1) * (360 / detail) * KdToRadians) * radius;
		v.pos.z += sin((float)(i + 1) * (360 / detail) * KdToRadians) * radius;
		m_debugVertices.push_back(v);

		// XY面
		v.pos = pos;
		v.pos.x += cos((float)i * (360 / detail) * KdToRadians) * radius;
		v.pos.y += sin((float)i * (360 / detail) * KdToRadians) * radius;
		m_debugVertices.push_back(v);

		v.pos = pos;
		v.pos.x += cos((float)(i + 1) * (360 / detail) * KdToRadians) * radius;
		v.pos.y += sin((float)(i + 1) * (360 / detail) * KdToRadians) * radius;
		m_debugVertices.push_back(v);

		// YZ面
		v.pos = pos;
		v.pos.y += cos((float)i * (360 / detail) * KdToRadians) * radius;
		v.pos.z += sin((float)i * (360 / detail) * KdToRadians) * radius;
		m_debugVertices.push_back(v);

		v.pos = pos;
		v.pos.y += cos((float)(i + 1) * (360 / detail) * KdToRadians) * radius;
		v.pos.z += sin((float)(i + 1) * (360 / detail) * KdToRadians) * radius;
		m_debugVertices.push_back(v);
	}
}

// デバッグボックスの描画
void KdDebugWireFrame::AddDebugBox(const Math::Matrix& matrix, const Math::Vector3& size, const Math::Vector3& offset, const bool isOriented, const Math::Color& col)
{
	KdPolygon::Vertex v;
	v.UV = Math::Vector2::Zero;
	v.color = col.RGBA().v;

	Math::Vector3					corners[8];
	DirectX::BoundingBox			aabb;
	DirectX::BoundingOrientedBox	obb;
	// ボックス(AABB)
	if (!isOriented)
	{
		aabb.Center = matrix.Translation() + offset;
		aabb.Extents = size;

		aabb.GetCorners(corners);
	}
	// ボックス(OBB)
	else
	{
		Math::Matrix createMat = matrix;
		createMat.Translation(createMat.Translation() + offset);
		obb.Transform(obb, createMat);
		obb.Extents = size;

		obb.GetCorners(corners);
	}	

	// 底辺定義
	{
		//底辺1
		v.pos = corners[0];
		m_debugVertices.push_back(v);

		v.pos = corners[1];
		m_debugVertices.push_back(v);

		//底辺2
		v.pos = corners[0];
		m_debugVertices.push_back(v);

		v.pos = corners[4];
		m_debugVertices.push_back(v);

		//底辺3
		v.pos = corners[5];
		m_debugVertices.push_back(v);

		v.pos = corners[1];
		m_debugVertices.push_back(v);

		//底辺4
		v.pos = corners[5];
		m_debugVertices.push_back(v);

		v.pos = corners[4];
		m_debugVertices.push_back(v);
	}

	// 中辺定義
	{
		//中辺1
		v.pos = corners[0];
		m_debugVertices.push_back(v);

		v.pos = corners[3];
		m_debugVertices.push_back(v);

		//中辺2
		v.pos = corners[1];
		m_debugVertices.push_back(v);

		v.pos = corners[2];
		m_debugVertices.push_back(v);

		//中辺3
		v.pos = corners[4];
		m_debugVertices.push_back(v);

		v.pos = corners[7];
		m_debugVertices.push_back(v);

		//中辺4
		v.pos = corners[5];
		m_debugVertices.push_back(v);

		v.pos = corners[6];
		m_debugVertices.push_back(v);
	}

	// 上辺定義
	{
		//上辺1
		v.pos = corners[2];
		m_debugVertices.push_back(v);

		v.pos = corners[3];
		m_debugVertices.push_back(v);

		//上辺2
		v.pos = corners[2];
		m_debugVertices.push_back(v);

		v.pos = corners[6];
		m_debugVertices.push_back(v);

		//上辺3
		v.pos = corners[7];
		m_debugVertices.push_back(v);

		v.pos = corners[3];
		m_debugVertices.push_back(v);

		//上辺4
		v.pos = corners[7];
		m_debugVertices.push_back(v);

		v.pos = corners[6];
		m_debugVertices.push_back(v);
	}
}

void KdDebugWireFrame::Draw()
{
	if (m_debugVertices.size() < 2) { return; }

	KdShaderManager::Instance().m_StandardShader.DrawVertices(m_debugVertices, Math::Matrix::Identity);

	m_debugVertices.clear();
}

void KdDebugWireFrame::Release()
{
	m_debugVertices.clear();
}
