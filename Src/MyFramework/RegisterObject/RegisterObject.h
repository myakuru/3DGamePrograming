#pragma once
class RegisterObject
{
public:
	RegisterObject() = default;
	~RegisterObject() = default;

	template<typename T>
	void Register()
	{
		std::string className = typeid(T).name(); // クラス名取得
		uint32_t typeID = T::TypeID;

		auto registerFunc = [typeID]()
			{
				auto obj = std::make_shared<T>();
				obj->m_typeID = typeID;
				return obj;
			};

		KdDebugGUI::Instance().AddLog("RegisterObject: %s, ID: %u\n", className.data(), typeID);

		m_RegisterObject.emplace(typeID, registerFunc);
		m_ClassNameToID.try_emplace(className, typeID); // クラス名とIdもぺ。
	}

	// m_RegisterObjectのゲッター
	const auto& GetRegisterObject() const
	{
		return m_RegisterObject;
	}

	std::map<std::string, uint32_t, std::less<>> m_ClassNameToID; // クラス名からID

private:
	std::map<uint32_t, std::function<std::shared_ptr<KdGameObject>()>> m_RegisterObject;

public:
	// シングルトンインスタンスを取得する関数
	static RegisterObject& GetInstance()
	{
		static RegisterObject instance;
		return instance;
	}
};