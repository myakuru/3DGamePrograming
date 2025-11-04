#include "KdCollider.h"

// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// KdCollider
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####

///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 当たり判定形状の登録関数群
///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdCollider::RegisterCollisionShape(std::string_view name, std::unique_ptr<KdCollisionShape> spShape)
{
	if (!spShape) { return; }

	m_collisionShapes.emplace(name.data(), std::move(spShape));
}

///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdCollider::RegisterCollisionShape(std::string_view name, const DirectX::BoundingSphere& sphere, UINT type)
{
	RegisterCollisionShape(name, std::make_unique<KdSphereCollision>(sphere, type));
}

///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdCollider::RegisterCollisionShape(std::string_view name, const DirectX::BoundingBox& box, UINT type)
{
	RegisterCollisionShape(name, std::make_unique<KdBoxCollision>(box, type));
}

///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdCollider::RegisterCollisionShape(std::string_view name, const DirectX::BoundingOrientedBox& box, UINT type)
{
	RegisterCollisionShape(name, std::make_unique<KdBoxCollision>(box, type));
}

///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdCollider::RegisterCollisionShape(std::string_view name, const Math::Vector3& localPos, float radius, UINT type)
{
	RegisterCollisionShape(name, std::make_unique<KdSphereCollision>(localPos, radius, type));
}

///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdCollider::RegisterCollisionShape(std::string_view name, const std::shared_ptr<KdModelData>& model, UINT type)
{
	RegisterCollisionShape(name, std::make_unique<KdModelCollision>(model, type));
}

///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdCollider::RegisterCollisionShape(std::string_view name, KdModelData* model, UINT type)
{
	RegisterCollisionShape(name, std::make_unique<KdModelCollision>(std::shared_ptr<KdModelData>(model), type));
}

///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdCollider::RegisterCollisionShape(std::string_view name, const std::shared_ptr<KdModelWork>& model, UINT type)
{
	RegisterCollisionShape(name, std::make_unique<KdModelCollision>(model, type));
}

///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdCollider::RegisterCollisionShape(std::string_view name, KdModelWork* model, UINT type)
{
	RegisterCollisionShape(name, std::make_unique<KdModelCollision>(std::shared_ptr<KdModelWork>(model), type));
}

///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdCollider::RegisterCollisionShape(std::string_view name, const std::shared_ptr<KdPolygon> polygon, UINT type)
{
	RegisterCollisionShape(name, std::make_unique<KdPolygonCollision>(polygon, type));
}

///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdCollider::RegisterCollisionShape(std::string_view name, KdPolygon* polygon, UINT type)
{
	RegisterCollisionShape(name, std::make_unique<KdPolygonCollision>(std::shared_ptr<KdPolygon>(polygon), type));
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// コライダーvs球に登録された任意の形状の当たり判定
// 球に合わせて何のために当たり判定をするのか type を渡す必要がある
// 第3引数に詳細結果の受け取る機能が付いている
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdCollider::Intersects(const SphereInfo& targetShape, const Math::Matrix& ownerMatrix, std::list<KdCollider::CollisionResult>* pResults) const
{
	// 当たり判定無効のタイプの場合は返る
	if (targetShape.m_type & m_disableType) { return false; }

	bool isHit = false;

	for (auto& collisionShape : m_collisionShapes)
	{
		// 用途が一致していない当たり判定形状はスキップ
		if (!(targetShape.m_type & collisionShape.second->GetType())) { continue; }

		KdCollider::CollisionResult tmpRes;
		KdCollider::CollisionResult* pTmpRes = pResults ? &tmpRes : nullptr;

		if (collisionShape.second->Intersects(targetShape.m_sphere, ownerMatrix, pTmpRes))
		{
			isHit = true;

			// 詳細な衝突結果を必要としない場合は1つでも接触して返す
			if (!pResults) { break; }

			pResults->push_back(tmpRes);
		}
	}

	return isHit;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// コライダーvsBOXに登録された任意の形状の当たり判定
// BOXに合わせて何のために当たり判定をするのか type を渡す必要がある
// 第3引数に詳細結果の受け取る機能が付いている
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdCollider::Intersects(const BoxInfo& targetShape, const Math::Matrix& ownerMatrix, std::list<KdCollider::CollisionResult>* pResults) const
{
	// 当たり判定無効のタイプの場合は返る
	if (targetShape.m_type & m_disableType) { return false; }

	bool isHit = false;

	for (auto& collisionShape : m_collisionShapes)
	{
		// 用途が一致していない当たり判定形状はスキップ
		if (!(targetShape.m_type & collisionShape.second->GetType())) { continue; }

		KdCollider::CollisionResult tmpRes;
		KdCollider::CollisionResult* pTmpRes = pResults ? &tmpRes : nullptr;

		bool isIntersects = (targetShape.CheckBoxType(BoxInfo::BoxType::BoxAABB)) ? collisionShape.second->Intersects(targetShape.m_Abox, ownerMatrix, pTmpRes) :
			collisionShape.second->Intersects(targetShape.m_Obox, ownerMatrix, pTmpRes);
		if (isIntersects)
		{
			isHit = true;

			// 詳細な衝突結果を必要としない場合は1つでも接触して返す
			if (!pResults) { break; }

			pResults->push_back(tmpRes);
		}
	}

	return isHit;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// コライダーvsレイに登録された任意の形状の当たり判定
// レイに合わせて何のために当たり判定をするのか type を渡す必要がある
// 第3引数に詳細結果の受け取る機能が付いている
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdCollider::Intersects(const RayInfo& targetShape, const Math::Matrix& ownerMatrix, std::list<KdCollider::CollisionResult>* pResults) const
{
	// 当たり判定無効のタイプの場合は返る
	if (targetShape.m_type & m_disableType) { return false; }

	// レイの方向ベクトルが存在しない場合は判定不能なのでそのまま返る
	if (!targetShape.m_dir.LengthSquared())
	{
		assert(0 && "KdCollider::Intersects：レイの方向ベクトルが存在していないため、正しく判定できません");

		return false;
	}

	bool isHit = false;

	for (auto& collisionShape : m_collisionShapes)
	{
		// 用途が一致していない当たり判定形状はスキップ
		if (!(targetShape.m_type & collisionShape.second->GetType())) { continue; }

		KdCollider::CollisionResult tmpRes;
		KdCollider::CollisionResult* pTmpRes = pResults ? &tmpRes : nullptr;

		if (collisionShape.second->Intersects(targetShape, ownerMatrix, pTmpRes))
		{
			isHit = true;

			// 詳細な衝突結果を必要としない場合は1つでも接触して返す
			if (!pResults) { break; }

			pResults->push_back(tmpRes);
		}
	}

	return isHit;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 任意のCollisionShapeを検索して有効/無効を切り替える
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdCollider::SetEnable(std::string_view name, bool flag)
{
	auto targetCol = m_collisionShapes.find(name.data());

	if (targetCol != m_collisionShapes.end())
	{
		targetCol->second->SetEnable(flag);
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 特定のタイプの有効/無効を切り替える
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdCollider::SetEnable(int type, bool flag)
{
	// 有効にしたい
	if (flag)
	{
		m_disableType &= ~type;
	}
	// 無効にしたい
	else
	{
		m_disableType |= type;
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 全てのCollisionShapeの有効/無効を一気に切り替える
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdCollider::SetEnableAll(bool flag)
{
	for (auto& col : m_collisionShapes)
	{
		col.second->SetEnable(flag);
	}
}


// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// SphereCollision
// 球形の形状
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 球vs球の当たり判定
// 判定回数は 1 回　計算自体も軽く最も軽量な当たり判定　計算回数も固定なので処理効率は安定
// 片方の球の判定を0にすれば単純な距離判定も作れる
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdSphereCollision::Intersects(const DirectX::BoundingSphere& target, const Math::Matrix& world, KdCollider::CollisionResult* pRes)
{
	if (!m_enable) { return false; }

	DirectX::BoundingSphere myShape;

	m_shape.Transform(myShape, world);

	// 球同士の当たり判定
	bool isHit = myShape.Intersects(target);

	// 詳細リザルトが必要無ければ即結果を返す
	if (!pRes) { return isHit; }

	// 当たった時のみ計算
	if (isHit)
	{
		// お互いに当たらない最小距離
		float needDistance = target.Radius + myShape.Radius;

		// 自身から相手に向かう方向ベクトル
		pRes->m_hitDir = (Math::Vector3(target.Center) - Math::Vector3(myShape.Center));
		float betweenDistance = pRes->m_hitDir.Length();

		// 重なり量 = お互い当たらない最小距離 - お互いの実際距離
		pRes->m_overlapDistance = needDistance - betweenDistance;

		pRes->m_hitDir.Normalize();

		// 当たった座標はお互いの球の衝突の中心点
		pRes->m_hitPos = myShape.Center + pRes->m_hitDir * (myShape.Radius + pRes->m_overlapDistance * 0.5f);
	}

	return isHit;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 球vsBOX(AABB)の当たり判定
// 判定回数は 1 回　計算自体も軽く最も軽量な当たり判定　計算回数も固定なので処理効率は安定
// 片方の球の判定を0にすれば単純な距離判定も作れる
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdSphereCollision::Intersects(const DirectX::BoundingBox& target, const Math::Matrix& world, KdCollider::CollisionResult* pRes)
{
	if (!m_enable) { return false; }

	DirectX::BoundingSphere myShape;

	m_shape.Transform(myShape, world);

	// 球vsBOXの当たり判定
	bool isHit = myShape.Intersects(target);

	// 詳細リザルトが必要無ければ即結果を返す
	if (!pRes) { return isHit; }

	// 当たった時のみ計算
	if (isHit)
	{
		// 点をOBBのローカル座標系へ変換(これでOBBからAABBの判定にできる)
		Math::Vector3 _pointCenter = myShape.Center - Math::Vector3(target.Center);

		// BOXの最近接点を求める
		Math::Vector3 _outPos = { 0,0,0 };
		for (int i = 0; i < 3; i++)
		{
			float dist = (&_pointCenter.x)[i];
			if ((&_pointCenter.x)[i] > (&target.Extents.x)[i])
			{
				dist = (&target.Extents.x)[i];
			}
			else if (dist < -(&target.Extents.x)[i])
			{
				dist = -(&target.Extents.x)[i];
			}
			(&_outPos.x)[i] += dist;
		}
		_outPos += target.Center;

		// 自身から相手に向かう方向ベクトル
		pRes->m_hitDir = (_outPos - Math::Vector3(myShape.Center));
		float betweenDistance = pRes->m_hitDir.Length();

		// 重なり量 = お互い当たらない最小距離 - お互いの実際距離
		pRes->m_overlapDistance = myShape.Radius - betweenDistance;

		pRes->m_hitDir.Normalize();
		pRes->m_hitPos = myShape.Center + pRes->m_hitDir * (myShape.Radius + pRes->m_overlapDistance * 0.5f);
	}

	return isHit;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 球vsBOX(OBB)の当たり判定
// 判定回数は 1 回　計算自体も軽く最も軽量な当たり判定　計算回数も固定なので処理効率は安定
// 片方の球の判定を0にすれば単純な距離判定も作れる
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdSphereCollision::Intersects(const DirectX::BoundingOrientedBox& target, const Math::Matrix& world, KdCollider::CollisionResult* pRes)
{
	if (!m_enable) { return false; }

	DirectX::BoundingSphere myShape;

	m_shape.Transform(myShape, world);

	// 球vsBOXの当たり判定
	bool isHit = myShape.Intersects(target);

	// 詳細リザルトが必要無ければ即結果を返す
	if (!pRes) { return isHit; }

	// 当たった時のみ計算
	if (isHit)
	{
		// OBBの回転(クォータニオン)
		DirectX::XMFLOAT4 obbQuat = target.Orientation;

		// 点をOBBのローカル座標系へ変換(これでOBBからAABBの判定にできる)
		Math::Vector3 _pointCenter = XMVector3InverseRotate(myShape.Center - Math::Vector3(target.Center), Math::Vector4(obbQuat));

		// BOXの最近接点を求める
		Math::Vector3 _outPos = { 0,0,0 };
		for (int i = 0; i < 3; i++)
		{
			float dist = (&_pointCenter.x)[i];
			if ((&_pointCenter.x)[i] > (&target.Extents.x)[i])
			{
				dist = (&target.Extents.x)[i];
			}
			else if (dist < -(&target.Extents.x)[i])
			{
				dist = -(&target.Extents.x)[i];
			}
			(&_outPos.x)[i] += dist;
		}
		// OBBのローカル座標系からワールドへ戻す
		_outPos = XMVector3Rotate(_outPos, Math::Vector4(obbQuat));
		_outPos += target.Center;

		// 自身から相手に向かう方向ベクトル
		pRes->m_hitDir = (_outPos - Math::Vector3(myShape.Center));
		float betweenDistance = pRes->m_hitDir.Length();

		// 重なり量 = お互い当たらない最小距離 - お互いの実際距離
		pRes->m_overlapDistance = myShape.Radius - betweenDistance;

		pRes->m_hitDir.Normalize();
		pRes->m_hitPos = myShape.Center + pRes->m_hitDir * (myShape.Radius + pRes->m_overlapDistance * 0.5f);
	}

	return isHit;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 球vsレイの当たり判定
// 判定回数は 1 回　計算回数が固定なので処理効率は安定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdSphereCollision::Intersects(const KdCollider::RayInfo& target, const Math::Matrix& world, KdCollider::CollisionResult* pRes)
{
	if (!m_enable) { return false; }

	DirectX::BoundingSphere myShape;

	m_shape.Transform(myShape, world);

	float hitDistance = 0.0f;

	bool isHit = myShape.Intersects(target.m_pos, target.m_dir, hitDistance);

	// 判定限界距離を加味
	isHit &= (target.m_range >= hitDistance);

	// 詳細リザルトが必要無ければ即結果を返す
	if (!pRes) { return isHit; }

	// 当たった時のみ計算
	if (isHit)
	{
		// レイ発射位置 + レイの当たった位置までのベクトル 
		pRes->m_hitPos = target.m_pos + target.m_dir * hitDistance;

		pRes->m_hitDir = target.m_dir * (-1);

		pRes->m_overlapDistance = target.m_range - hitDistance;
	}

	return isHit;
}

// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// BOXCollision
// BOXの形状
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// BOXvs球の当たり判定
// 判定回数は 1 回　計算自体も軽く最も軽量な当たり判定　計算回数も固定なので処理効率は安定
// 片方の球の判定を0にすれば単純な距離判定も作れる
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdBoxCollision::Intersects(const DirectX::BoundingSphere& target, const Math::Matrix& /*world*/, KdCollider::CollisionResult* pRes)
{
	if (!m_enable) { return false; }

	DirectX::XMFLOAT4				obbQuat = Math::Vector4::Zero;
	Math::Vector3					myShapeCenter = (!m_IsOriented) ? m_Abox.Center : m_Obox.Center;

	// 球vsBOXの当たり判定
	bool isHit = (!m_IsOriented) ? m_Abox.Intersects(target) : m_Obox.Intersects(target);

	// 詳細リザルトが必要無ければ即結果を返す
	if (!pRes) { return isHit; }

	// 当たった時のみ計算
	if (isHit)
	{
		// 点をOBBのローカル座標系へ変換(これでOBBからAABBの判定にできる)
		Math::Vector3 _pointCenter = Math::Vector3::Zero;
		if (!m_IsOriented)
		{
			_pointCenter = myShapeCenter - Math::Vector3(target.Center);
		}
		else
		{
			// OBBの回転(クォータニオン)
			obbQuat = m_Obox.Orientation;
			_pointCenter = XMVector3InverseRotate(myShapeCenter - Math::Vector3(target.Center), Math::Vector4(obbQuat));
		}

		// BOXの最近接点を求める
		Math::Vector3 _outPos = { 0,0,0 };
		Math::Vector3 _myExtents = (!m_IsOriented) ? m_Abox.Extents : m_Obox.Extents;
		for (int i = 0; i < 3; i++)
		{
			float dist = (&_pointCenter.x)[i];
			if ((&_pointCenter.x)[i] > (&_myExtents.x)[i])
			{
				dist = (&_myExtents.x)[i];
			}
			else if (dist < -(&_myExtents.x)[i])
			{
				dist = -(&_myExtents.x)[i];
			}
			(&_outPos.x)[i] += dist;
		}
		// OBBのローカル座標系からワールドへ戻す
		if (m_IsOriented)_outPos = XMVector3Rotate(_outPos, Math::Vector4(obbQuat));
		_outPos += target.Center;

		// 自身から相手に向かう方向ベクトル
		pRes->m_hitDir = (_outPos - Math::Vector3(myShapeCenter));
		float betweenDistance = pRes->m_hitDir.Length();

		// 重なり量 = お互い当たらない最小距離 - お互いの実際距離
		pRes->m_overlapDistance = target.Radius - betweenDistance;

		pRes->m_hitDir.Normalize();
		pRes->m_hitPos = myShapeCenter + pRes->m_hitDir * (target.Radius + pRes->m_overlapDistance * 0.5f);
	}

	return isHit;
}
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// BOXvsBOX(AABB)の当たり判定
// 判定回数は 1 回　計算自体も軽く最も軽量な当たり判定　計算回数も固定なので処理効率は安定
// 片方の球の判定を0にすれば単純な距離判定も作れる
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdBoxCollision::Intersects(const DirectX::BoundingBox& target, const Math::Matrix& /*world*/, KdCollider::CollisionResult* /*pRes*/)
{
	if (!m_enable) { return false; }

	// BOXvsBOX(AABB)の当たり判定
	bool isHit = (!m_IsOriented) ? m_Abox.Intersects(target) : m_Obox.Intersects(target);

	// 即結果を返す(HITしたかどうかだけが知れる)
	return isHit;
}
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// BOXvsBOX(OBB)の当たり判定
// 判定回数は 1 回　計算自体も軽く最も軽量な当たり判定　計算回数も固定なので処理効率は安定
// 片方の球の判定を0にすれば単純な距離判定も作れる
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdBoxCollision::Intersects(const DirectX::BoundingOrientedBox& target, const Math::Matrix& /*world*/, KdCollider::CollisionResult* /*pRes*/)
{
	if (!m_enable) { return false; }

	// BOXvsBOX(OBB)の当たり判定
	bool isHit = (!m_IsOriented) ? m_Abox.Intersects(target) : m_Obox.Intersects(target);

	// 即結果を返す(HITしたかどうかだけが知れる)
	return isHit;
}
bool KdBoxCollision::Intersects(const KdCollider::RayInfo& target, const Math::Matrix& /*world*/, KdCollider::CollisionResult* /*pRes*/)
{
	if (!m_enable) { return false; }

	// AABB vs レイ
	float AABBdist = FLT_MAX;

	// BOXvsBOX(OBB)の当たり判定
	bool isHit = (!m_IsOriented) ? m_Abox.Intersects(target.m_pos, target.m_dir, AABBdist) : m_Obox.Intersects(target.m_pos, target.m_dir, AABBdist);

	// 即結果を返す(HITしたかどうかだけが知れる)
	return isHit;
}

// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// ModelCollision
// 3Dメッシュの形状
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// モデルvs球の当たり判定
// 判定回数は メッシュの個数 x 各メッシュのポリゴン数 計算回数がモデルのデータ依存のため処理効率は不安定
// 単純に計算回数が多くなる可能性があるため重くなりがち
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdModelCollision::Intersects(const DirectX::BoundingSphere& target, const Math::Matrix& world, KdCollider::CollisionResult* pRes)
{
	// 当たり判定が無効 or 形状が解放済みなら判定せず返る
	if (!m_enable || !m_shape) { return false; }

	std::shared_ptr<KdModelData> spModelData = m_shape->GetData();

	// データが無ければ判定不能なので返る
	if (!spModelData) { return false; }

	const std::vector<KdModelData::Node>& dataNodes = spModelData->GetOriginalNodes();
	const std::vector<KdModelWork::Node>& workNodes = m_shape->GetNodes();

	// 各メッシュに押される用の球・押される毎に座標を更新する必要がある
	DirectX::BoundingSphere pushedSphere = target;
	// 計算用にFloat3 → Vectorへ変換
	Math::Vector3 pushedSphereCenter = DirectX::XMLoadFloat3(&pushedSphere.Center);

	bool isHit = false;

	Math::Vector3 hitPos;
	Math::Vector3 hitNDir;

	// 当たり判定ノードとのみ当たり判定
	for (int index : spModelData->GetCollisionMeshNodeIndices())
	{
		const KdModelData::Node& dataNode = dataNodes[index];
		const KdModelWork::Node& workNode = workNodes[index];

		// あり得ないはずだが一応チェック
		if (!dataNode.m_spMesh) { continue; }

		CollisionMeshResult tmpResult;
		CollisionMeshResult* pTmpResult = pRes ? &tmpResult : nullptr;

		// メッシュと球形の当たり判定実行
		if (!MeshIntersect(*dataNode.m_spMesh, pushedSphere, workNode.m_worldTransform * world, pTmpResult))
		{
			continue;
		}

		// 詳細リザルトが必要無ければ即結果を返す
		if (!pRes) { return true; }

		isHit = true;

		// 重なった分押し戻す
		pushedSphereCenter = DirectX::XMVectorAdd(pushedSphereCenter, DirectX::XMVectorScale(tmpResult.m_hitDir, tmpResult.m_overlapDistance));

		DirectX::XMStoreFloat3(&pushedSphere.Center, pushedSphereCenter);

		// とりあえず当たった座標で更新
		hitPos = tmpResult.m_hitPos;

		// 最後に当たった面の法線情報を記憶しておく
		hitNDir = tmpResult.m_hitDir;
	}

	if (pRes && isHit)
	{
		// 最後に当たった座標が使用される
		pRes->m_hitPos = hitPos;

		// 複数のメッシュに押された最終的な位置 - 移動前の位置 = 押し出しベクトル
		pRes->m_hitDir = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&pushedSphere.Center), DirectX::XMLoadFloat3(&target.Center));

		pRes->m_overlapDistance = DirectX::XMVector3Length(pRes->m_hitDir).m128_f32[0];

		pRes->m_hitDir = DirectX::XMVector3Normalize(pRes->m_hitDir);

		// 最後に当たった面の法線が使用される
		pRes->m_hitNDir = hitNDir;
	}

	return isHit;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// モデルvsBOX(AABB)の当たり判定
// 判定回数は メッシュの個数 x 各メッシュのポリゴン数 計算回数がモデルのデータ依存のため処理効率は不安定
// 単純に計算回数が多くなる可能性があるため重くなりがち
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdModelCollision::Intersects(const DirectX::BoundingBox& /*target*/, const Math::Matrix& /*world*/, KdCollider::CollisionResult* /*pRes*/)
{
	// TODO: 当たり計算は各自必要に応じて拡張して下さい
	return false;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// モデルvsBOX(OBB)の当たり判定
// 判定回数は メッシュの個数 x 各メッシュのポリゴン数 計算回数がモデルのデータ依存のため処理効率は不安定
// 単純に計算回数が多くなる可能性があるため重くなりがち
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdModelCollision::Intersects(const DirectX::BoundingOrientedBox& /*target*/, const Math::Matrix& /*world*/, KdCollider::CollisionResult* /*pRes*/)
{
	// TODO: 当たり計算は各自必要に応じて拡張して下さい
	return false;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// モデルvsレイの当たり判定
// 判定回数は メッシュの個数 x 各メッシュのポリゴン数 計算回数がモデルのデータ依存のため処理効率は不安定
// 単純に計算回数が多くなる可能性があるため重くなりがち
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdModelCollision::Intersects(const KdCollider::RayInfo& target, const Math::Matrix& world, KdCollider::CollisionResult* pRes)
{
	// 当たり判定が無効 or 形状が解放済みなら判定せず返る
	if (!m_enable || !m_shape) { return false; }

	std::shared_ptr<KdModelData> spModelData = m_shape->GetData();

	// データが無ければ判定不能なので返る
	if (!spModelData) { return false; }

	CollisionMeshResult nearestResult;

	bool isHit = false;

	const std::vector<KdModelData::Node>& dataNodes = spModelData->GetOriginalNodes();
	const std::vector<KdModelWork::Node>& workNodes = m_shape->GetNodes();

	for (int index : spModelData->GetCollisionMeshNodeIndices())
	{
		const KdModelData::Node& dataNode = dataNodes[index];
		const KdModelWork::Node& workNode = workNodes[index];

		if (!dataNode.m_spMesh) { continue; }

		CollisionMeshResult tmpResult;
		CollisionMeshResult* pTmpResult = pRes ? &tmpResult : nullptr;

		if (!MeshIntersect(*dataNode.m_spMesh, target.m_pos, target.m_dir, target.m_range,
			workNode.m_worldTransform * world, pTmpResult))
		{
			continue;
		}

		// 詳細リザルトが必要無ければ即結果を返す
		if (!pRes) { return true; }

		isHit = true;

		if (tmpResult.m_overlapDistance > nearestResult.m_overlapDistance)
		{
			nearestResult = tmpResult;
		}
	}

	if (pRes && isHit)
	{
		// 最も近くで当たったヒット情報をコピーする
		pRes->m_hitPos = nearestResult.m_hitPos;

		pRes->m_hitDir = nearestResult.m_hitDir;

		pRes->m_overlapDistance = nearestResult.m_overlapDistance;

		// 最も近くで当たった面の法線が使用される
		pRes->m_hitNDir = nearestResult.m_hitNDir;
	}

	return isHit;
}


// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// PolygonCollision
// 多角形ポリゴン(頂点の集合体)の形状
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 多角形ポリゴン(頂点の集合体)vs球の当たり判定
// 判定回数は ポリゴンの個数 計算回数がポリゴンデータ依存のため処理効率は不安定
// 単純に計算回数が多くなる可能性があるため重くなりがち
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdPolygonCollision::Intersects(const DirectX::BoundingSphere& target, const Math::Matrix& world, KdCollider::CollisionResult* pRes)
{
	// 当たり判定が無効 or 形状が解放済みなら判定せず返る
	if (!m_enable || !m_shape) { return false; }

	CollisionMeshResult result;
	CollisionMeshResult* pTmpResult = pRes ? &result : nullptr;

	// メッシュと球形の当たり判定実行
	if (!PolygonsIntersect(*m_shape, target, world, pTmpResult))
	{
		// 当たっていなければ無条件に返る
		return false;
	}

	if (pRes)
	{
		pRes->m_hitPos = result.m_hitPos;

		pRes->m_hitDir = result.m_hitDir;

		pRes->m_overlapDistance = result.m_overlapDistance;

		pRes->m_hitNDir = result.m_hitNDir;
	}

	return true;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 多角形ポリゴン(頂点の集合体)vsBOX(AABB)の当たり判定
// 判定回数は ポリゴンの個数 計算回数がポリゴンデータ依存のため処理効率は不安定
// 単純に計算回数が多くなる可能性があるため重くなりがち
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdPolygonCollision::Intersects(const DirectX::BoundingBox& /*target*/, const Math::Matrix& /*world*/, KdCollider::CollisionResult* /*pRes*/)
{
	// TODO: 当たり計算は各自必要に応じて拡張して下さい
	return false;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 多角形ポリゴン(頂点の集合体)vsBOX(OBB)の当たり判定
// 判定回数は ポリゴンの個数 計算回数がポリゴンデータ依存のため処理効率は不安定
// 単純に計算回数が多くなる可能性があるため重くなりがち
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdPolygonCollision::Intersects(const DirectX::BoundingOrientedBox& /*target*/, const Math::Matrix& /*world*/, KdCollider::CollisionResult* /*pRes*/)
{
	// TODO: 当たり計算は各自必要に応じて拡張して下さい
	return false;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 多角形ポリゴン(頂点の集合体)vsレイの当たり判定
// 判定回数は ポリゴンの個数 計算回数がポリゴンデータ依存のため処理効率は不安定
// 単純に計算回数が多くなる可能性があるため重くなりがち
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdPolygonCollision::Intersects(const KdCollider::RayInfo& target, const Math::Matrix& world, KdCollider::CollisionResult* pRes)
{
	// 当たり判定が無効 or 形状が解放済みなら判定せず返る
	if (!m_enable || !m_shape) { return false; }

	CollisionMeshResult result;
	CollisionMeshResult* pTmpResult = pRes ? &result : nullptr;

	if (!PolygonsIntersect(*m_shape, target.m_pos, target.m_dir, target.m_range, world, pTmpResult))
	{
		// 当たっていなければ無条件に返る
		return false;
	}

	if (pRes)
	{
		pRes->m_hitPos = result.m_hitPos;

		pRes->m_hitDir = result.m_hitDir;

		pRes->m_overlapDistance = result.m_overlapDistance;

		pRes->m_hitNDir = result.m_hitNDir;
	}

	return true;
}
