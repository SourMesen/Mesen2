#include "pch.h"
#include "Debugger/ScriptHost.h"
#include "Debugger/ScriptingContext.h"
#include "Debugger/ScriptingContext.h"
#include "Shared/EventType.h"
#include "Shared/MemoryOperationType.h"

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
	shared_ptr<IScriptingContext> context = _context.lock();
	return context ? context->GetLog() : "";
}

static bool endsWithCaseInsensitive(const std::string& str, const std::string& suffix)
{
	if(suffix.size() > str.size())
		return false;

	return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin(), [](char a, char b) {
		return std::tolower(a) == std::tolower(b);
	});
}

bool ScriptHost::LoadScript(string scriptName, string path, string scriptContent, Debugger* debugger)
{
	if(endsWithCaseInsensitive(scriptName, ".py"))
		_context.reset(new PythonScriptingContext(debugger));
	else
		_context.reset(new ScriptingContext(debugger));

	return _context->LoadScript(scriptName, path, scriptContent, debugger);
}

void ScriptHost::ProcessEvent(EventType eventType, CpuType cpuType)
{
	_context->CallEventCallback(eventType, cpuType);
}
