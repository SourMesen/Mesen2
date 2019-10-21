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
	char* _playerName = nullptr;
	uint32_t _playerNameLength = 0;
	char* _hashedPassword = nullptr;
	uint32_t _hashedPasswordLength = 0;
	bool _spectator = false;

protected:
	virtual void ProtectedStreamState()
	{
		Stream<uint32_t>(_emuVersion);
		Stream<uint32_t>(_protocolVersion);
		StreamArray((void**)&_playerName, _playerNameLength);
		StreamArray((void**)&_hashedPassword, _hashedPasswordLength);
		Stream<bool>(_spectator);
	}

public:
	HandShakeMessage(void* buffer, uint32_t length) : NetMessage(buffer, length) {}

	HandShakeMessage(string playerName, string hashedPassword, bool spectator, uint32_t emuVersion) : NetMessage(MessageType::HandShake)
	{
		_emuVersion = emuVersion;
		_protocolVersion = HandShakeMessage::CurrentVersion;
		CopyString(&_playerName, _playerNameLength, playerName);
		CopyString(&_hashedPassword, _hashedPasswordLength, hashedPassword);
		_spectator = spectator;
	}

	string GetPlayerName()
	{
		return string(_playerName);
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
