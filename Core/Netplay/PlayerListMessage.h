#pragma once
#include "pch.h"
#include "Netplay/NetMessage.h"

class PlayerListMessage : public NetMessage
{
private:
	vector<PlayerInfo> _playerList;

protected:
	void Serialize(Serializer &s) override
	{
		if(s.IsSaving()) {
			uint32_t playerCount = (uint32_t)_playerList.size();
			SV(playerCount);
			for(uint32_t i = 0; i < playerCount; i++) {
				SVI(_playerList[i].ControllerPort.Port);
				SVI(_playerList[i].ControllerPort.SubPort);
				SVI(_playerList[i].IsHost);
			}
		} else {
			uint32_t playerCount = 0;
			SV(playerCount);
			_playerList.resize(playerCount);

			for(uint32_t i = 0; i < playerCount; i++) {
				SVI(_playerList[i].ControllerPort.Port);
				SVI(_playerList[i].ControllerPort.SubPort);
				SVI(_playerList[i].IsHost);
			}
		}
	}

public:
	PlayerListMessage(void* buffer, uint32_t length) : NetMessage(buffer, length) { }

	PlayerListMessage(vector<PlayerInfo> playerList) : NetMessage(MessageType::PlayerList)
	{
		_playerList = playerList;
	}

	vector<PlayerInfo> GetPlayerList()
	{
		return _playerList;
	}
};