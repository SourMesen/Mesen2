#pragma once
#include "stdafx.h"
#include "DebugTypes.h"

class Debugger;

class CallstackManager
{
private:
	Debugger* _debugger;
	deque<StackFrameInfo> _callstack;

public:
	CallstackManager(Debugger* debugger);

	void Push(uint32_t srcAddr, uint32_t destAddr, uint32_t returnAddress, StackFrameFlags flags);
	void Pop(uint32_t destAddr);

	void GetCallstack(StackFrameInfo* callstackArray, uint32_t &callstackSize);
};