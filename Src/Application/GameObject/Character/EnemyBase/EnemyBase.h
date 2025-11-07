#pragma once
#include "../CharacterBase.h"
class Player;
class EnemyBase : public CharacterBase
{
public:

	EnemyBase() = default;
	~EnemyBase() override = default;

	void UpdateAttackCollision(float _radius = 1.f, float _distance = 1.1f,
		int _attackCount = 5, float _attackTimer = 0.3f,
		float _activeBeginSec = 0.0f, float _activeEndSec = 3.0f);


protected:

	void Init() override;
	void Update() override;
	void DrawLit() override;
	void PostUpdate() override;
	void ImGuiInspector() override;
	void JsonInput(const nlohmann::json& _json) override;
	void JsonSave(nlohmann::json& _json) const override;

	void UpdateQuaternion(Math::Vector3& _moveVector) override;


	bool m_Expired = false;				// 敵を消滅させるかどうか
	bool m_isHit = false;				// ヒット判定
	bool m_isAtkPlayer = false;

	int m_getDamage = 0;				// 受けるダメージ量

	float m_attackRadius = 1.5f;		// 攻撃判定の半径
	float m_attackFrame = 0.0f;			// 攻撃判定フレーム

	std::weak_ptr<Player> m_wpPlayer;

	// ジャスト回避成功フラグ
	bool m_justAvoidSuccess = false;

	bool m_hitOnce = false;
	int m_chargeAttackCount = 0;								// 何回ダメージを与えたか
	float m_chargeAttackTimer = 0.0f;							// 経過時間
	bool m_isChargeAttackActive = false;						// 連続攻撃中か

	bool m_invincible = false;					// 無敵判定用
	int m_totalHitCount = 0;					// 累積ヒット回数（無敵判定用）

	// 攻撃の有効時間ウィンドウ（クランプなし）
	float m_attackActiveTime = 0.0f;	// 攻撃開始からの経過時間
	float m_attackActiveBegin = 0.0f;	// 当たり判定が有効になる開始秒
	float m_attackActiveEnd = 3.0f;		// 当たり判定が無効化される終了秒

};