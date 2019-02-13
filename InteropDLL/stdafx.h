// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#if _WIN32 || _WIN64
	#if _WIN64
		#define ENVIRONMENT64
	#else
		#define ENVIRONMENT32
	#endif
#endif

#if __GNUC__
	#if __x86_64__ || __ppc64__
		#define ENVIRONMENT64
	#else
		#define ENVIRONMENT32
	#endif
#endif

#ifdef _DEBUG
	#define MESEN_LIBRARY_DEBUG_SUFFIX "Debug"
#else 
	#define MESEN_LIBRARY_DEBUG_SUFFIX "Release"
#endif

#ifdef ENVIRONMENT32
	#define MESEN_LIBRARY_SUFFIX "x86.lib"
#else 
	#define MESEN_LIBRARY_SUFFIX "x64.lib"
#endif

#if _WIN32 || _WIN64
	#pragma comment(lib, "Core.lib")
	#pragma comment(lib, "Utilities.lib")
	#pragma comment(lib, "Windows.lib")
	#pragma comment(lib, "SevenZip.lib")
	#pragma comment(lib, "Lua.lib")
	#pragma comment(lib, "../Dependencies/DirectXTK." MESEN_LIBRARY_DEBUG_SUFFIX ".Static." MESEN_LIBRARY_SUFFIX)
	#define DllExport __declspec(dllexport)
#else
	#define __stdcall
	#define DllExport __attribute__((visibility("default")))
#endif