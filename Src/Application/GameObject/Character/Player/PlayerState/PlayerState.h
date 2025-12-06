#pragma once
#include"MyFramework/State/StateBase/StateBase.h"
#include"Application/GameObject/Character/Player/PlayerConfig.h"
#include"Application/GameObject/Character/Player/Player.h"
#include"Application/main.h"
#include"MyFramework/Manager/JsonManager/JsonManager.h"

class BossEnemy;
class EffectReference;

class PlayerStateBase : public StateBase
{
	struct StateParameter
	{
		float attackRadius = 1.0f;
		float attackDistance = 1.0f;
		int   attackCount = 1;
		float attackInterval = 0.1f;    // 旧 m_attackTime
		float attackStartTime = 0.0f;
		float attackEndTime = 0.4f;
		Math::Vector3 moveSpeed = Math::Vector3::Zero;
		Math::Vector2 cameraShake = { 0.2f, 0.0f };
		float cameraTime = 0.3f;

		float dashSpeed = 0.7f;                // ダッシュ移動速度
		float blendTime = 0.25f;            // ブレンドエフェクト表示時間
		float animationSpeed = 60.0f;        // アニメーション速度
		float dashSpeedTime = 0.2f;            // ダッシュ移動速度時間
		float changeStateTime = 0.7f;        // 状態遷移までの時間

		// 残像関係
		int afterImageMax = 5;    // 残像発生最大数
		float afterImageInterval = 0.1f; // 残像発生間隔
		Math::Vector4 afterImageColor = { 0.0f,2.0f,2.0f,1.0f }; // 残像色

		void ExposeImGui();
		void LoadJson(const nlohmann::json& pj);
		void SaveJson(nlohmann::json& js) const;
	};

public:
	PlayerStateBase();
	~PlayerStateBase() override;

	void SetPlayer(Player* player) { m_player = player; }

	void ExposeParametersImGui() override {}
	// JSON 読み込み
	virtual void LoadParametersJson(const nlohmann::json& _json) { (void)_json; }
	// 保存
	virtual void SaveParametersJson(nlohmann::json& _json) const { (void)_json; }
	// ImGuiで編集した変数を実行時反映させるための関数
	virtual void ApplyFromConfig(const PlayerStateBase& other) { (void)other; }


protected:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	// 刀の位置を右手に追従するように更新
	virtual void UpdateKatanaPos();

	// 刀と鞘の位置が左手に追従するように更新
	virtual void UpdateUnsheathed();

	// 必殺技入力関連
	bool UpdateSpecialAttackInput();

	// 回避入力関連
	bool UpdateMoveAvoidInput();

	// 攻撃入力関連
	template<typename T>
	bool UpdateAttackInput()
	{
		if (m_lButtonKeyInput)
		{
			m_lButtonKeyInput = false;
			auto state = std::make_shared<T>();
			m_player->ChangeState(state);
			return true;
		}

		return false;
	}

	// 刀を鞘に納める入力関連
	bool UpdateSheathKatanaInput();

	// Eスキル入力関連
	bool UpdateESkillInput();

	Player* m_player = nullptr;

	Math::Vector3 m_prevRootTranslation    = Math::Vector3::Zero;
	Math::Vector3 m_currentRootTranslation = Math::Vector3::Zero;
	Math::Vector3 m_attackDirection      = Math::Vector3::Zero;

	float m_time = 0.0f;

	
	bool m_lButtonKeyInput = false;    // マウス左Buttonが押されているか
	bool m_rButtonKeyInput = false;    // マウス右Buttonが押されているか

	float m_animeTime = 0.0f;        // アニメーション再生時間
	float m_maxAnimeTime = 0.0f;    // アニメーション最大再生時間

	std::weak_ptr<BossEnemy> m_bossEnemy;

	// カメラターゲットオフセットデフォルト値
	const Math::Vector3 m_cameraTargetOffset = { 0.0f,1.0f,-2.5f };
	const Math::Vector3 m_cameraBossTargetOffset = { 0.0f,1.0f,-5.5f };

	// カメラ距離・高さスムーズ係数
	const float m_cameraDistanceSmooth = 8.0f;
	const float m_cameraRotationSmooth = 8.0f;


	std::shared_ptr<KdSoundInstance> m_runSound = nullptr;

	std::weak_ptr<KdGameObject> m_focusTarget;        //    フォーカスターゲット
	float m_focusRemainSec = 0.0f;                    //    フォーカスタイマー
	const float m_focusDurationSec = 10.0f;            // フォーカス継続時間(調整用)
	
	const float DefaultSearchEnemyRadius = 5.0f;    // 既定値

	// 索敵範囲
	float m_searchEnemyRadius = DefaultSearchEnemyRadius;

	std::shared_ptr<KdGameObject>    m_nearestEnemy;
	Math::Vector3                    m_nearestEnemyPos = Math::Vector3::Zero;
	float                            m_minDistSq = std::numeric_limits<float>::max();

	StateParameter m_stateParameter;

	// エフェクト参照（複数）
	std::vector<std::shared_ptr<EffectReference>> m_playerEffects;
};