#pragma once

class AttacEffect1;

class EffekseerEffectManager
{
public:
	EffekseerEffectManager() = default;
	~EffekseerEffectManager() = default;

private:

	void Init();

	std::shared_ptr<AttacEffect1> m_attacEffect1;

};