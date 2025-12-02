#pragma once
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"

class AetheriusEnemy;
class AetheriusEnemy_Attack_3Effect : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;

	AetheriusEnemy_Attack_3Effect() { m_typeID = TypeID; }
	~AetheriusEnemy_Attack_3Effect() override = default;

	// 指定エネミーに対してエフェクトを即時再生
	void PlayForEnemy(const std::shared_ptr<AetheriusEnemy>& enemy);

private:

	void Update() override;
	void EffectUpdate() override;

	std::vector<std::weak_ptr<AetheriusEnemy>> m_targetEnemies;

	// 複数同時再生を管理
	std::vector<std::weak_ptr<KdEffekseerObject>> m_effects;

};