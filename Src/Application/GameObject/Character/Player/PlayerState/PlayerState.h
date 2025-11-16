#pragma once
#include"MyFramework/State/StateBase/StateBase.h"
#include"Application/GameObject/Character/Player/PlayerConfig.h"
#include"Application/GameObject/Character/Player/Player.h"
#include"Application/Data/CharacterData/CharacterData.h"

class BossEnemy;
class PlayerStateBase : public StateBase
{
public:
	PlayerStateBase() = default;
	~PlayerStateBase() override = default;

	void SetPlayer(Player* player) { m_player = player; }

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

	// ため攻撃入力関連
	bool UpdateChargeAttackInput();

	Player* m_player = nullptr;

	Math::Vector3 m_prevRootTranslation    = Math::Vector3::Zero;
	Math::Vector3 m_currentRootTranslation = Math::Vector3::Zero;
	Math::Vector3 m_attackDirection      = Math::Vector3::Zero;

	float m_time = 0.0f;

	// マウス左Buttonが押されているか
	bool m_lButtonKeyInput = false;
	// マウス右Buttonが押されているか
	bool m_rButtonKeyInput = false;

	float m_animeTime = 0.0f;
	float m_maxAnimeTime = 0.0f;

	std::weak_ptr<BossEnemy> m_bossEnemy;

	Math::Vector3 m_cameraTargetOffset = { 0.0f,1.0f,-2.5f };

	Math::Vector3 m_cameraBossTargetOffset = { 0.0f,1.0f,-5.5f };

	std::shared_ptr<KdSoundInstance> m_runSound = nullptr;


	std::weak_ptr<KdGameObject> m_focusTarget;
	float m_focusRemainSec = 0.0f;
	const float m_focusDurationSec = 0.1f;
	const float m_focusMaxDistSq = 50.0f * 50.0f;

	std::shared_ptr<KdGameObject>	m_nearestEnemy;
	Math::Vector3					m_nearestEnemyPos = Math::Vector3::Zero;
	float							m_minDistSq = std::numeric_limits<float>::max();

	CharacterData m_playerData;

};