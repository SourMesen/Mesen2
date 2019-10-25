#pragma once
#include "stdafx.h"
#include "NetMessage.h"
#include "../Utilities/sha1.h"

class HandShakeMessage : public NetMessage
{
private:
	static constexpr int CurrentVersion = 100; //Use 100+ to distinguish from Mesen
	uint32_t _emuVersion = 0;
	uint32_t _protocolVersion = CurrentVersion;
	string _playerName;
	string _hashedPassword;
	bool _spectator = false;

protected:
	void Serialize(Serializer &s) override
	{
		s.Stream(_emuVersion, _protocolVersion, _playerName, _hashedPassword, _spectator);
	}

public:
	HandShakeMessage(void* buffer, uint32_t length) : NetMessage(buffer, length) {}

	HandShakeMessage(string playerName, string hashedPassword, bool spectator, uint32_t emuVersion) : NetMessage(MessageType::HandShake)
	{
		_emuVersion = emuVersion;
		_protocolVersion = HandShakeMessage::CurrentVersion;
		_playerName = playerName;
		_hashedPassword = hashedPassword;
		_spectator = spectator;
	}

	string GetPlayerName()
	{
		return _playerName;
	}

	bool IsValid(uint32_t emuVersion)
	{
		return _protocolVersion == CurrentVersion && _emuVersion == emuVersion;
	}

	bool CheckPassword(string serverPassword, string connectionHash)
	{
		return GetPasswordHash(serverPassword, connectionHash) == string(_hashedPassword);
	}

	bool IsSpectator()
	{
		return _spectator;
	}

	static string GetPasswordHash(string serverPassword, string connectionHash)
	{
		string saltedPassword = serverPassword + connectionHash;
		vector<uint8_t> dataToHash = vector<uint8_t>(saltedPassword.c_str(), saltedPassword.c_str() + saltedPassword.size());
		return SHA1::GetHash(dataToHash);
	}
};
