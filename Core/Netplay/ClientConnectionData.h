#pragma once

#include "stdafx.h"

class ClientConnectionData
{
public:
	string Host;
	uint16_t Port;
	string Password;
	bool Spectator;

	ClientConnectionData() {}

	ClientConnectionData(string host, uint16_t port, string password, bool spectator) :
		Host(host), Port(port), Password(password), Spectator(spectator)
	{
	}

	~ClientConnectionData()
	{
	}
};