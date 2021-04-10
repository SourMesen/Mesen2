#pragma once
#include "stdafx.h"
#include "Shared/MessageManager.h"
#include "Netplay/NetMessage.h"
#include "Utilities/FolderUtilities.h"

class GameInformationMessage : public NetMessage
{
private:
	string _romFilename;
	string _sha1Hash;
	uint8_t _controllerPort = 0;
	bool _paused = false;

protected:
	void Serialize(Serializer &s) override
	{
		s.Stream(_romFilename, _sha1Hash, _controllerPort, _paused);
	}

public:
	GameInformationMessage(void* buffer, uint32_t length) : NetMessage(buffer, length) { }

	GameInformationMessage(string filepath, string sha1Hash, uint8_t port, bool paused) : NetMessage(MessageType::GameInformation)
	{
		_romFilename = FolderUtilities::GetFilename(filepath, true);
		_sha1Hash = sha1Hash;
		_controllerPort = port;
		_paused = paused;
	}
	
	uint8_t GetPort()
	{
		return _controllerPort;
	}

	string GetRomFilename()
	{
		return _romFilename;
	}

	string GetSha1Hash()
	{
		return _sha1Hash;
	}

	bool IsPaused()
	{
		return _paused;
	}
};