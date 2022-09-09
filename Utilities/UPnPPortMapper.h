#pragma once

#include "pch.h"

using std::wstring;

enum class IPProtocol
{
	TCP = 0,
	UDP = 1
};

class UPnPPortMapper
{
private:
	static vector<wstring> GetLocalIPs();

public:
	static bool AddNATPortMapping(uint16_t internalPort, uint16_t externalPort, IPProtocol protocol);
	static bool RemoveNATPortMapping(uint16_t externalPort, IPProtocol protocol);
};