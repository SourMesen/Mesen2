#include "pch.h"
#include "WS/WsCpuPrefetch.h"
#include "WS/WsMemoryManager.h"
#include "WS/WsCpu.h"

WsCpuPrefetch::WsCpuPrefetch(WsCpu* cpu, WsMemoryManager* memoryManager)
{
	_cpu = cpu;
	_memoryManager = memoryManager;
}

bool WsCpuPrefetch::IsFull()
{
	return _size >= 16;
}

void WsCpuPrefetch::PushByte(uint8_t value)
{
	_data[(_writePos++) & 0x0F] = value;
	_fetchIp++;
	_size++;
}

void WsCpuPrefetch::Prefetch()
{
	_cpu->ProcessCpuCycle();

	_waitCycles++;

	if(IsFull()) {
		return;
	}

	uint32_t addr = ((_fetchCs << 4) + _fetchIp) & 0xFFFFF;
	uint8_t cycles = _memoryManager->GetWaitStates(addr);
	if(_waitCycles < cycles) {
		return;
	}

	_waitCycles = 0;

	bool isWordBus = _memoryManager->IsWordBus(addr);

	//TODOWS debugger can't see these reads
	PushByte(_memoryManager->InternalRead(addr));
	if(!IsFull() && isWordBus && (_fetchIp & 0x01)) {
		PushByte(_memoryManager->InternalRead(((_fetchCs << 4) + _fetchIp)) & 0xFFFFF);
	}
}

void WsCpuPrefetch::ProcessRep(uint8_t opCode)
{
	if(IsFull()) {
		_writePos--;
		_fetchIp--;
		_size--;
	}

	_readPos--;
	_size++;
	_data[_readPos & 0x0F] = opCode;
}

uint8_t WsCpuPrefetch::Read()
{
	while(_size < 2) {
		Prefetch();
	}

	_size--;
	return _data[(_readPos++) & 0x0F];
}

void WsCpuPrefetch::Clear(uint16_t cs, uint16_t ip)
{
	_fetchCs = cs;
	_fetchIp = ip;
	_size = 0;
	_readPos = 0;
	_writePos = 0;
	_waitCycles = 0;
}

void WsCpuPrefetch::Serialize(Serializer& s)
{
	SV(_fetchCs);
	SV(_fetchIp);
	SV(_readPos);
	SV(_writePos);
	SV(_size);
	SV(_waitCycles);
	SVArray(_data, 16);
}
