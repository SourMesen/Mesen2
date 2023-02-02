#pragma once

#if _WIN32 || _WIN64
	#pragma comment(lib, "ws2_32.lib") //Winsock Library
	#define DllExport __declspec(dllexport)
#else
	#define __stdcall
	#define DllExport __attribute__((visibility("default")))
#endif