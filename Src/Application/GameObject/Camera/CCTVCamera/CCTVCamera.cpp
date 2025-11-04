#include "CCTVCamera.h"

void CCTVCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// キャラクターの原点から「どれだけ離れた位置」にカメラを置くか
	m_mLocalPos = Math::Matrix::CreateTranslation(0, 6, -10.f);
	// 一旦行列を確定させている
	m_mWorld	= m_mLocalPos;
}

void CCTVCamera::PostUpdate()
{
	// ターゲットの行列(有効な場合利用する)
	Math::Matrix targetMat = Math::Matrix::Identity;
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (spTarget)
	{
		targetMat = spTarget->GetMatrix();
	}

	// カメラの回転
	UpdateLookAtRotate(targetMat.Translation());
}

void CCTVCamera::UpdateLookAtRotate(const Math::Vector3& targetPos)
{
/*
	// ①
	// プレイヤーの方に徐々に向く
	// 自分の今向いている前方向
	Math::Vector3 nowDir = m_mWorld.Backward();

	// ターゲット(プレイヤー)への方向ベクトル
	Math::Vector3 targetDir = targetPos - GetPos();

	// 正規化：方向ベクトルの長さを1にする
	nowDir.Normalize();

	// 方向ベクトルをXZ平面上での方向ベクトルとする
	targetDir.y = 0;
	targetDir.Normalize();

	// ①今向いている方向のワールド角度を求める
	float nowAng = atan2(nowDir.x, nowDir.z);
	nowAng = DirectX::XMConvertToDegrees(nowAng);

	// ②ターゲットへ向くワールド角度を求める
	float targetAng = atan2(targetDir.x, targetDir.z);
	targetAng = DirectX::XMConvertToDegrees(targetAng);

	// ①と②の間の角度を求める　間の角度　＝　目的の角度　ー　現在の角度
	float betweenAng = targetAng - nowAng;

	// 180度の位置で角度の数値の切れ目
	if (betweenAng > 180)
	{
		betweenAng -= 360;
	}
	else if (betweenAng < -180)
	{
		betweenAng += 360;
	}

	// 1フレームで最大何度傾くのかの制御
	float rotateAng = std::clamp(betweenAng, -1.0f, 1.0f);

	// カメラの角度の更新
	Math::Matrix tmpRotation = Math::Matrix::Identity;
	m_DegAng.y += rotateAng;
	tmpRotation = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_DegAng.y));

	Math::Matrix tmpWorld = tmpRotation * Math::Matrix::CreateTranslation(GetPos());
	tmpWorld = tmpWorld.Invert();

	Math::Vector3 targetLocalPos;
	targetLocalPos = Math::Vector3::Transform(targetPos, tmpWorld);

	targetLocalPos.x = 0;
	float nowAngY = asin(targetLocalPos.y / targetLocalPos.Length());

	// 最終的な回転行列を生成する
	m_mRotation	= Math::Matrix::CreateRotationX(-nowAngY) * tmpRotation;
	m_mWorld	= m_mRotation * m_mLocalPos;
*/
	// ②
	Math::Vector3 _targetVec = Math::Vector3::Zero;
	_targetVec = targetPos - GetPos();

	// YAW角
	float _yaw = DirectX::XMConvertToDegrees(atan2(_targetVec.x, _targetVec.z));
	// PITCH角
	float _pitch = DirectX::XMConvertToDegrees(atan2(_targetVec.y, sqrt(pow(_targetVec.x, 2) + pow(_targetVec.z, 2))));

	Math::Matrix _RotationX = Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(-_pitch));
	Math::Matrix _RotationY = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(_yaw));
	
	m_DegAng = Math::Vector3::Zero;
	m_DegAng.x = -_pitch;
	m_DegAng.y = _yaw;
	m_mWorld = GetRotationMatrix() * Math::Matrix::CreateTranslation(0, 6, -10.f);
/*
	// ③
	Math::Vector3	_tmpPos = m_mLocalPos.Translation();
	Math::Matrix	_shadowVP = DirectX::XMMatrixLookAtLH(_tmpPos, targetPos, Math::Vector3::Up);

	m_mWorld = _shadowVP.Invert();
*/
}