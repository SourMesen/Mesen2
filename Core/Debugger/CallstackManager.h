#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"

class Debugger;
class Profiler;
class IDebugger;

class CallstackManager
{
private:
	Debugger* _debugger;
	deque<StackFrameInfo> _callstack;
	unique_ptr<Profiler> _profiler;

public:
	CallstackManager(Debugger* debugger, IDebugger* cpuDebugger);
	~CallstackManager();

	void Push(AddressInfo& src, uint32_t srcAddr, AddressInfo& dest, uint32_t destAddr, AddressInfo& ret, uint32_t returnAddress, StackFrameFlags flags);
	void Pop(AddressInfo& dest, uint32_t destAddr);

	__forceinline bool IsReturnAddrMatch(uint32_t destAddr)
	{
		if(_callstack.empty()) {
			return false;
		}

		for(auto itt = _callstack.rbegin(); itt != _callstack.rend(); itt++) {
			if((*itt).Return == destAddr) {
				return true;
			}
		}

		return false;
	}

	void GetCallstack(StackFrameInfo* callstackArray, uint32_t &callstackSize);
	int32_t GetReturnAddress();
	Profiler* GetProfiler();

	void Clear();
};