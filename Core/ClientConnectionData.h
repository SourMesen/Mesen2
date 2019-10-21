#pragma once

#include "stdafx.h"

class ClientConnectionData
{
public:
	string Host;
	uint16_t Port;
	string Password;
	string PlayerName;
	bool Spectator;

	ClientConnectionData() {}

	ClientConnectionData(string host, uint16_t port, string password, string playerName, bool spectator) :
		Host(host), Port(port), Password(password), PlayerName(playerName), Spectator(spectator)
	{
	}

	~ClientConnectionData()
	{
	}
};