#pragma once
class EffekseerEffectBase;
class EffekseerEffectManager : public KdGameObject
{
public:

	static const uint32_t TypeID;

	EffekseerEffectManager() { m_typeID = TypeID; }
	~EffekseerEffectManager() override = default;


	std::list<std::weak_ptr<EffekseerEffectBase>> m_effectObjList;

private:

};