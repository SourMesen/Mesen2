#pragma once

#include "stdafx.h"

#include "IMessageManager.h"
#include <unordered_map>
#include "Utilities/SimpleLock.h"

#ifdef _DEBUG
	#define LogDebug(msg) MessageManager::Log(msg);
#else
	#define LogDebug(msg) 
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
