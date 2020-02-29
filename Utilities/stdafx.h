#pragma once

#include <string>
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <vector>
#include <atomic>
#include <cstring>
#include <algorithm>

#include "UTF8Util.h"

using std::shared_ptr;
using std::unique_ptr;
using utf8::ifstream;
using utf8::ofstream;
using std::ostream;
using std::istream;
using std::string;
using std::vector;
using std::atomic;
using std::atomic_flag;

#ifndef __MINGW32__
	#ifdef __clang__
		#define __forceinline __attribute__((always_inline)) inline
	#else
		#ifdef __GNUC__
			#define __forceinline __attribute__((always_inline)) inline
		#endif
	#endif
#endif