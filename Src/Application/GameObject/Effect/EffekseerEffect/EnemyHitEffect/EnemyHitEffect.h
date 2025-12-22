#include"../EffekseerEffectBase.h"
class RedEnemy;
class KdEffekseerObject;
class BossEnemy;

class EnemyHitEffect : public EffekseerEffectBase
{
public:
	static const uint32_t TypeID;
	EnemyHitEffect() { m_typeID = TypeID; }
	~EnemyHitEffect() override = default;

	void Update() override;
	void EffectUpdate() override;

	// 指定エネミーに対してエフェクトを即時再生
	void PlayForEnemy(const std::shared_ptr<RedEnemy>& enemy);
	void PlayForBossEnemy(const std::shared_ptr<BossEnemy>& enemy);

private:

	std::vector<std::weak_ptr<RedEnemy>> m_enemies;

	// 複数同時再生を管理
	std::vector<std::weak_ptr<KdEffekseerObject>> m_effects;
};