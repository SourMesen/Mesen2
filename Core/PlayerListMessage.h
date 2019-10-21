#pragma once
#include "stdafx.h"
#include "NetMessage.h"

class PlayerListMessage : public NetMessage
{
private:
	vector<PlayerInfo> _playerList;

protected:
	virtual void ProtectedStreamState()
	{
		constexpr uint32_t PlayerNameMaxLength = 50;

		uint32_t nameLength = PlayerNameMaxLength + 1;
		char playerName[PlayerNameMaxLength + 1];
		uint8_t playerPort = 0;
		bool isHost = false;

		if(_sending) {
			uint32_t playerCount = (uint32_t)_playerList.size();
			Stream<uint32_t>(playerCount);
			for(uint32_t i = 0; i < playerCount; i++) {
				memset(playerName, 0, nameLength);
				memcpy(playerName, _playerList[i].Name.c_str(), std::min((uint32_t)_playerList[i].Name.size(), PlayerNameMaxLength));
				playerPort = _playerList[i].ControllerPort;

				StreamArray(playerName, nameLength);
				Stream<uint8_t>(playerPort);
				Stream<bool>(isHost);
			}
		} else {
			uint32_t playerCount;
			Stream<uint32_t>(playerCount);
			
			for(uint32_t i = 0; i < playerCount; i++) {
				memset(playerName, 0, nameLength);
				StreamArray(playerName, nameLength);
				Stream<uint8_t>(playerPort);
				Stream<bool>(isHost);

				PlayerInfo playerInfo;
				playerInfo.Name = playerName;
				playerInfo.ControllerPort = playerPort;
				playerInfo.IsHost = isHost;

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