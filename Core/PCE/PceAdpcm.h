#pragma once
#include "stdafx.h"

class PceCdRom;
class PceScsiBus;

struct PceAdpcmState
{
	uint16_t ReadAddress;
	uint16_t WriteAddress;

	uint16_t AddressPort;
	uint8_t DataPort;

	uint8_t DmaControl;
	uint8_t Control;
	uint8_t PlaybackRate;
	uint8_t FadeTimer;

	uint16_t AdpcmLength;
	bool EndReached;
	bool HalfReached;

	bool Playing;
	bool Reading;
	bool Writing;

	uint8_t ReadBuffer;
	uint8_t ReadClockCounter;
	
	uint8_t WriteBuffer;
	uint8_t WriteClockCounter;

};

class PceAdpcm
{
private:
	PceCdRom* _cdrom;
	PceScsiBus* _scsi;
	PceAdpcmState _state;
	uint8_t* _ram;

	void Reset();
	void SetControl(uint8_t value);

public:
	PceAdpcm(PceCdRom* cdrom, PceScsiBus* scsi);
	~PceAdpcm();

	void Exec();

	void Write(uint16_t addr, uint8_t value);
	uint8_t Read(uint16_t addr);
};