#pragma once

#include <string>
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>
#include <atomic>
#include <cstring>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
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
using std::unordered_map;
using std::unordered_set;
using std::optional;

#ifndef _MSC_VER
	// Some headers have functions marked as `__forceinline` but don't provide the bodies;
	// it fails to compile when LTO is not enabled.
	#if !defined(__MINGW32__) && (defined(__clang__) || defined(__GNUC__)) && defined(HAVE_LTO)
		#define __forceinline __attribute__((always_inline)) inline
	#else 
		#define __forceinline inline
	#endif
#endif
