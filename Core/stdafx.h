#pragma once

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
#include <array>
#include <sstream>
#include <list>
#include <atomic>
#include <thread>
#include <deque>
#include <algorithm>

#include "../Utilities/UTF8Util.h"

#ifndef __MINGW32__
	#ifdef __clang__
		#define __forceinline __attribute__((always_inline)) inline
	#else
		#ifdef __GNUC__
			#define __forceinline __attribute__((always_inline)) inline
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
