#pragma once
#include "stdafx.h"
#include "MessageManager.h"
#include "NetMessage.h"
#include "../Utilities/FolderUtilities.h"

class GameInformationMessage : public NetMessage
{
private:
	char* _romFilename = nullptr;
	uint32_t _romFilenameLength = 0;
	char _sha1Hash[40];
	uint8_t _controllerPort = 0;
	bool _paused = false;

protected:
	virtual void ProtectedStreamState()
	{
		StreamArray((void**)&_romFilename, _romFilenameLength);
		StreamArray((void**)&_sha1Hash, 40);
		Stream<uint8_t>(_controllerPort);
		Stream<bool>(_paused);
	}

public:
	GameInformationMessage(void* buffer, uint32_t length) : NetMessage(buffer, length) { }

	GameInformationMessage(string filepath, string sha1Hash, uint8_t port, bool paused) : NetMessage(MessageType::GameInformation)
	{
		CopyString(&_romFilename, _romFilenameLength, FolderUtilities::GetFilename(filepath, true));
		memcpy(_sha1Hash, sha1Hash.c_str(), 40);
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
		return string(_sha1Hash, _sha1Hash+40);
	}

	bool IsPaused()
	{
		return _paused;
	}
};