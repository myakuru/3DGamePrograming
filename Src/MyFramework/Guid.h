#pragma once

#include <string>

#pragma comment(lib ,"rpcrt4.lib")
#include <rpc.h>

class Guid
{
public:

	Guid()
	{
		NewGuid();
	}

	Guid(const UUID& strGuid)
	{
		m_guid = strGuid;
	}

	// 新しいGUIDを作成する
	void NewGuid()
	{
		std::ignore = UuidCreate(&m_guid);
	}

	std::string ToString() const
	{
		std::string ret;
		RPC_CSTR String;
		if (UuidToStringA(&m_guid, &String) == RPC_S_OK)
		{
			ret = (char*)String;
		}
		return ret;
	}

	void FromString(const std::string& strGuid)
	{
		std::ignore = UuidFromStringA((RPC_CSTR)strGuid.c_str(), &m_guid);
	}

	const UUID& GetUUID() const { return m_guid; }

private:
	UUID m_guid = {};
};