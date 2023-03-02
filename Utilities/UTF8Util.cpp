#include "pch.h"
#include "UTF8Util.h"
#include <codecvt>
#include <locale>

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

namespace utf8 
{
	std::wstring utf8::decode(const std::string &str)
	{
#ifdef _MSC_VER
		std::wstring ret;
		int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
		if(len > 0) {
			ret.resize(len);
			MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &ret[0], len);
		}
		return ret;
#else
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		return conv.from_bytes(str);
#endif
	}

	std::string utf8::encode(const std::wstring &wstr)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    	return conv.to_bytes(wstr);
	}

	std::string utf8::encode(const std::u16string &wstr)
	{
		#ifdef _MSC_VER
			std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> conv;
			auto p = reinterpret_cast<const int16_t *>(wstr.data());
			return conv.to_bytes(p, p + wstr.size());
		#else 
			std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
  			return conv.to_bytes(wstr);
		#endif
	}	
}