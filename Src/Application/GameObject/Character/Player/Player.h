#pragma once
#include "../CharacterBase.h"
#include"PlayerConfig.h"
class Katana;
class WeaponKatanaScabbard;
class PlayerStateBase;
class AetheriusEnemy;
class Player :public CharacterBase
{
public:

	// クラスごとに一意なTypeIDを持たせる
	static const uint32_t TypeID;
	Player() { m_typeID = TypeID; AddTag(ObjTag::PlayerLike); }
	~Player() override = default;

	void Init() override;
	void PreUpdate() override;
	void PostUpdate() override;
	void DrawLit() override;
	void DrawRimLight() override;
	void Update() override;

	// 攻撃の当たり判定(攻撃半径、攻撃距離、攻撃回数、攻撃間隔、カメラシェイクの強さ、カメラシェイクの時間、当たり判定が有効な開始秒・終了秒)
	// 開始 > 終了なら入れ替え
	void UpdateAttackCollision(float _radius = 10.f, float _distance = 1.1f,
		int _attackCount = 5, float _attackTimer = 0.3f,
		Math::Vector2 _cameraShakePow = { 0.3f, 0.3f }, float _cameraTime = 0.3f,
		float _activeBeginSec = 0.0f, float _activeEndSec = 3.0f);

	// 当たり判定リセット
	void ResetAttackCollision()
	{
		m_chargeAttackCount = 0;
		m_chargeAttackTimer = 0.0f;
		m_isChargeAttackActive = false;
		m_onceEffect = false;

		// 時間ウィンドウもリセット
		m_attackActiveTime = 0.0f;
		m_attackActiveBegin = 0.0f;
		m_attackActiveEnd = 3.0f;
	}

	void ImGuiInspector() override;
	void JsonInput(const nlohmann::json& _json) override;
	void JsonSave(nlohmann::json& _json) const override;

	void StateInit();
	void ChangeState(std::shared_ptr<PlayerStateBase> _state);
	void UpdateMoveDirectionFromInput();

	const Math::Vector3& GetMoveDirection() const { return m_moveDirection; }
	Math::Vector3 GetLastMoveDirection() const { return m_lastMoveDirection; }
	void SetMoveDirection(const Math::Vector3& _moveDirection) { m_moveDirection = _moveDirection; }

	KdModelWork* GetModelWork() { return m_modelWork.get(); }

	const std::weak_ptr<Katana>& GetKatana() const { return m_katana; }
	const std::weak_ptr<WeaponKatanaScabbard>& GetScabbard() const { return m_scabbard; }
	const std::list<std::weak_ptr<KdGameObject>>& GetEnemyLike() const { return m_enemyLike; }

	void SetAtkPlayer(bool flg) { m_isAtkPlayer = flg; }

	PlayerConfig& GetPlayerConfig() { return m_playerConfig; }

	// ダメージを受けた時の処理
	void TakeDamage(int damage);

	bool m_onceEffect = false;

	void SetAvoidFlg(bool _flg) { m_nowAvoid = _flg; }
	bool GetAvoidFlg() const { return m_nowAvoid; }

	void SetAvoidStartTime(float time) { m_avoidStartTime = time; }
	float GetAvoidStartTime() const { return m_avoidStartTime; }


	const Math::Vector2& GetCameraShakePower() const { return m_cameraShakePower; }
	float GetCameraShakeTime() const { return m_cameraShakeTime; }


	// 残像処理

	// 更新/描画フック
	void CaptureAfterImage();
	void DrawAfterImages();

	// 残像の状態設定(残像を使用するか？、残像の最台数、残像を何秒間表示するか、カラー)
	void AddAfterImage(bool _flg = false, int _max = 0, float _nterval = 0, const Math::Color& _color = { 0,0,0,0 }, float dissever = 0.0f)
	{
		m_afterImageEnable = _flg;
		m_afterImageMax = _max;
		m_afterImageInterval = _nterval;
		m_afterImageColor = _color;
		m_dissever = dissever;
	}

	// 残像のリセット
	void ResetAfterImage()
	{
		m_afterImages.clear();
		m_afterImageCounter = 0;
	}

	void SetHitCheck(bool _isHit)
	{
		m_isHit = _isHit;
	}

	bool GetHitCheck() const
	{
		return m_isHit;
	}

	// ジャスト回避成功フラグの取得
	bool GetJustAvoidSuccess() const { return m_justAvoid; }

	void SetJustAvoidSuccess(bool _flg) { m_justAvoid = _flg; }

	// 無敵状態の取得
	bool GetInvincible() const { return m_invincible; }

	void SetInvincible(bool _flg) { m_invincible = _flg; }

	// Skill使用中かどうか
	bool GetUseSkill() const { return m_useSkill; }

	// 必殺技が打てるかどうか
	bool GetUseSpecial() const { return m_useSpecial; }

	// キャラクターデーターの取得
	const CharacterData& GetStatus() const
	{
		return *m_characterData;
	}

private:

	void ApplyHorizontalMove(const Math::Vector3& inputMove, float deltaTime);
	void ApplyPushWithCollision(const Math::Vector3& rawPush);
	void ApplyVerticalMove(float deltaY);


	Math::Vector3 m_moveDirection = Math::Vector3::Zero;		// 移動方向
	Math::Vector3 m_lastMoveDirection = Math::Vector3::Zero;	// 最後に移動した方向
	bool m_nowAvoid = false;									// 回避中かどうか
	float m_attackBossEnemyRadius = 2.0f;						// ボスに攻撃する時の当たり判定の半径
	float m_avoidStartTime = 0.0f;								// 回避開始タイム
	int m_chargeAttackCount = 0;								// 何回ダメージを与えたか
	float m_chargeAttackTimer = 0.0f;							// 経過時間
	bool m_isChargeAttackActive = false;						// 連続攻撃中か
	Math::Vector2 m_cameraShakePower = Math::Vector2::Zero;		// カメラシェイクの強さ
	float m_cameraShakeTime = 0.0f;								// カメラシェイクの時間
	PlayerConfig m_playerConfig;								// プレイヤーの設定
	bool m_isAtkPlayer = false;									// プレイヤーと敵が接触したか どうか

	// 参照
	std::list<std::weak_ptr<KdGameObject>> m_enemyLike;

	// 刀
	std::weak_ptr<Katana> m_katana;
	std::weak_ptr<WeaponKatanaScabbard> m_scabbard;

	// 残像関連
	struct AfterImageFrame
	{
		std::vector<Math::Matrix> nodeWorlds; // 各ノードの worldTransform（モデルローカル空間）
		Math::Matrix ownerWorld = Math::Matrix::Identity; // その時点の m_mWorld
		float alpha = 1.0f; // このフレームの不透明度
	};

	// 残像設定・状態
	bool m_afterImageEnable = false;
	int  m_afterImageMax = 1;
	float  m_afterImageInterval = 0.1f; // 何フレームごとに保存するか
	float  m_afterImageCounter = 0.0f;	// カウンタ
	Math::Color m_afterImageColor = { 0,1,1,0.1f }; // 基本色（半透明白）

	bool m_isHit = false;						// ヒット判定用
	bool m_invincible = false;					// 無敵判定用
	bool m_justAvoid = false;					// ジャスト回避判定用
	float m_unScaledeltaTime = 0.0f;			// デフォルトのdeltaTimeを保存する変数

	std::deque<AfterImageFrame> m_afterImages;

	// 描画用テンポラリ Work（元モデルと同じ Data を参照）
	std::unique_ptr<KdModelWork> m_afterImageWork;

	// 攻撃の有効時間ウィンドウ
	float m_attackActiveTime = 0.0f;	// 攻撃開始からの経過時間
	float m_attackActiveBegin = 0.0f;	// 当たり判定が有効になる開始秒
	float m_attackActiveEnd = 3.0f;		// 当たり判定が無効化される終了秒

	bool m_useSkill = false;			// スキル使用中かどうか
	bool m_useSpecial = false;			// スペシャル使用中かどうか

	Math::Vector3 m_pointLightColor = { 0.4f, 0.5f, 0.5f }; // プレイヤーのポイントライトの色
	float m_pointLightRadius = 2.0f;                     // プレイヤーのポイントライトの半径
	Math::Vector3 m_pointLightOffset = { 0.0f, 1.5f, 0.0f }; // プレイヤーのポイントライトのオフセット位置
	bool m_pointLightOn = true;                            // プレイヤーのポイントライトのオンオフ

};