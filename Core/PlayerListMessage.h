#pragma once
#include "stdafx.h"
#include "NetMessage.h"

class PlayerListMessage : public NetMessage
{
private:
	vector<PlayerInfo> _playerList;

protected:
	void Serialize(Serializer &s) override
	{
		if(s.IsSaving()) {
			uint32_t playerCount = (uint32_t)_playerList.size();
			s.Stream(playerCount);
			for(uint32_t i = 0; i < playerCount; i++) {
				s.Stream(_playerList[i].Name, _playerList[i].ControllerPort, _playerList[i].IsHost);
			}
		} else {
			uint32_t playerCount;
			s.Stream(playerCount);
			
			for(uint32_t i = 0; i < playerCount; i++) {
				PlayerInfo playerInfo;
				s.Stream(playerInfo.Name, playerInfo.ControllerPort, playerInfo.IsHost);
				_playerList.push_back(playerInfo);
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