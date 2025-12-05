#pragma once
#include"../EffekseerEffectBase.h"
class SwordFlash : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;

	SwordFlash() { m_typeID = TypeID; }
	~SwordFlash() override = default;

	void JsonSave(nlohmann::json& _json) const override
	{
		EffekseerEffectBase::JsonSave(_json);
		KdDebugGUI::Instance().AddLog(("Effect Init" + m_guid.ToString() +"\n").data());
		// 追加で保存するパラメータがあればここに記述
	}
	void JsonInput(const nlohmann::json& _json) override
	{
		EffekseerEffectBase::JsonInput(_json);
		KdDebugGUI::Instance().AddLog(("Effect re" + m_guid.ToString() + "\n").data());
		// 追加で読み込むパラメータがあればここに記述
	}

};