#pragma once

#ifdef _MSC_VER
#pragma warning( disable : 4100 ) //unreferenced formal parameter
#pragma warning( disable : 4244 ) //conversion from 'x' to 'y', possible loss of data
#pragma warning( disable : 4245 ) //conversion from 'x' to 'y', signed/unsigned mismatch
#endif 

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <memory>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cctype>
#include <memory>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <list>
#include <atomic>
#include <thread>
#include <deque>
#include <algorithm>

#include "../Utilities/UTF8Util.h"

#ifdef _MSC_VER
	#define __noinline __declspec(noinline)
#endif

#ifndef __MINGW32__
	#ifdef __clang__
		#define __forceinline __attribute__((always_inline)) inline
		#define __noinline __attribute__((noinline))
	#else
		#ifdef __GNUC__
			#define __forceinline __attribute__((always_inline)) inline
			#define __noinline __attribute__((noinline))
		#endif
	#endif
#endif

using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::ios;
using std::istream;
using std::ostream;
using std::stringstream;
using utf8::ifstream;
using utf8::ofstream;
using std::list;
using std::max;
using std::string;
using std::atomic_flag;
using std::atomic;
using std::thread;
using std::deque;
