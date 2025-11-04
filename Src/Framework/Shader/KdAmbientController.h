#pragma once

struct PointLight;

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ゲーム内の空間環境をコントロールするパラメータ群
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
struct KdAmbientParameter
{
	// 環境光
	Math::Vector4	m_ambientLightColor;

	// 平行光
	Math::Vector3	m_directionalLightDir;
	Math::Vector3	m_directionalLightColor;

	// 距離フォグ
	Math::Vector3	m_distanceFogColor;
	float			m_distanceFogDensity = 0.0f;		// フォグ減衰率

	// 高さフォグ
	Math::Vector3	m_heightFogColor;
	float			m_heightFogTopValue = 0.0f;			// フォグを開始する上限の高さ
	float			m_heightFogBottomValue = 0.0f;		// フォグ色に染まる下限の高さ
	float			m_heightFogBeginDistance = 0.0f;	// フォグの開始する距離
};

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 光やフォグなどの空間環境をコントロールするパラメータを制御
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// 光：環境光・平行光・点光源の情報を変化させる（シェーダーへの情報の転送を行う）
// フォグ：距離フォグ・高さフォグの情報を変化させる
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class KdAmbientController
{
public:
	KdAmbientController() {}
	~KdAmbientController() {}

	// シェーダーマネージャで設定したシェーダーの初期値を取得してくる
	void Init();

	void Update();

	void Draw();

	void AddPointLight(const Math::Vector3& Color, float Radius, const Math::Vector3& Pos, bool IsBright = true);
	void AddPointLight(const PointLight& pointLight);

	// 平行光の影生成用の射影行列設定：エリアを指定 x:幅 y:奥行
	void SetDirLightShadowArea(const Math::Vector2& lightingArea, float dirLightHeight);

	// 平行光の方向と色を設定
	void SetDirLight(const Math::Vector3& dir, const Math::Vector3& col);

	// 環境光の色を設定
	void SetAmbientLight(const Math::Vector4& col);

	// フォグの有効と無効を切り替える
	void SetFogEnable(bool distance, bool height);

	// 距離フォグの設定
	void SetDistanceFog(const Math::Vector3& col, float density = 0.001f);

	// 高さフォグの設定
	void SetheightFog(const Math::Vector3& col, float topValue, float bottomValue, float distance);

	void SetShadowCenter(const Math::Vector3& center) { m_shadowCenter = center; }

private:

	void WriteLightParams(); 
	void WriteFogParams();

	KdAmbientParameter m_parameter;

	// 点光源
	std::list<PointLight> m_pointLights;

	// 平行光の影生成用の射影行列
	Math::Matrix m_shadowProj;
	// 平行光源の高さ(実際には存在しない影生成用の仮の位置)
	float		m_dirLightHeight = 0.0f;

	// 影中心（毎フレーム プレイヤー位置を設定推奨）
	Math::Vector3 m_shadowCenter = Math::Vector3::Zero;

	// 変更があるかを判定するフラグ
	bool m_dirtyLightAmb = true;
	bool m_dirtyLightDir = true;
	bool m_dirtyFogDist = true;
	bool m_dirtyFogHeight = true;
};
