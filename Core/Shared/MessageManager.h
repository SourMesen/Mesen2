#pragma once

#include "pch.h"

#include "Core/Shared/Interfaces/IMessageManager.h"
#include <unordered_map>
#include "Utilities/SimpleLock.h"

#ifdef _DEBUG
	#define LogDebug(msg) MessageManager::Log(msg);
	#define LogDebugIf(cond, msg) if(cond) { MessageManager::Log(msg); }
#else
	#define LogDebug(msg) 
	#define LogDebugIf(cond, msg)
#endif

class MessageManager
{
private:
	static IMessageManager* _messageManager;
	static std::unordered_map<string, string> _enResources;

	static bool _osdEnabled;
	static SimpleLock _logLock;
	static SimpleLock _messageLock;
	static std::list<string> _log;
	
public:
	static void SetOsdState(bool enabled);

	static string Localize(string key);

	static void RegisterMessageManager(IMessageManager* messageManager);
	static void UnregisterMessageManager(IMessageManager* messageManager);
	static void DisplayMessage(string title, string message, string param1 = "", string param2 = "");

	static void Log(string message = "");
	static void ClearLog();
	static string GetLog();
};
