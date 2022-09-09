#pragma once

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <mmsystem.h>

#undef min
#undef max

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")


// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <stdio.h>

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <dsound.h>
#include <io.h>
#include <Fcntl.h>

#include <list>
#include <vector>

#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <thread>

using std::list;
using std::vector;
using std::shared_ptr;
using std::unique_ptr;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::thread;
using namespace std::literals::string_literals;