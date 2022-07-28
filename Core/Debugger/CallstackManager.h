#pragma once
#include "stdafx.h"
#include "DebugTypes.h"

class Debugger;
class Profiler;
class IConsole;

class CallstackManager
{
private:
	Debugger* _debugger;
	deque<StackFrameInfo> _callstack;
	unique_ptr<Profiler> _profiler;

public:
	CallstackManager(Debugger* debugger, IConsole* console);
	~CallstackManager();

	void Push(AddressInfo& src, uint32_t srcAddr, AddressInfo& dest, uint32_t destAddr, AddressInfo& ret, uint32_t returnAddress, StackFrameFlags flags);
	void Pop(AddressInfo& dest, uint32_t destAddr);

	void GetCallstack(StackFrameInfo* callstackArray, uint32_t &callstackSize);
	int32_t GetReturnAddress();
	Profiler* GetProfiler();

	void Clear();
};