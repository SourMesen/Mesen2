// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#if _WIN32 || _WIN64
	#pragma comment(lib, "Core.lib")
	#pragma comment(lib, "Utilities.lib")
	#pragma comment(lib, "Windows.lib")
	#pragma comment(lib, "SevenZip.lib")
	#pragma comment(lib, "Lua.lib")
	#pragma comment(lib, "ws2_32.lib") //Winsock Library
	#define DllExport __declspec(dllexport)
#else
	#define __stdcall
	#define DllExport __attribute__((visibility("default")))
#endif