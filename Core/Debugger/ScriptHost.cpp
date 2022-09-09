#include "pch.h"
#include "Debugger/ScriptHost.h"
#include "Debugger/ScriptingContext.h"
#include "Debugger/ScriptingContext.h"
#include "EventType.h"
#include "MemoryOperationType.h"

ScriptHost::ScriptHost(int scriptId)
{
	_scriptId = scriptId;
}

int ScriptHost::GetScriptId()
{
	return _scriptId;
}

string ScriptHost::GetLog()
{
	shared_ptr<ScriptingContext> context = _context.lock();
	return context ? context->GetLog() : "";
}

bool ScriptHost::LoadScript(string scriptName, string scriptContent, Debugger* debugger)
{
	_context.reset(new ScriptingContext(debugger));
	if(!_context->LoadScript(scriptName, scriptContent, debugger)) {
		return false;
	}
	return true;
}

void ScriptHost::ProcessEvent(EventType eventType)
{
	_context->CallEventCallback(eventType);
}
