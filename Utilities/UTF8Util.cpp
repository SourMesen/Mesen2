#include "stdafx.h"
#include "UTF8Util.h"
#include <codecvt>
#include <locale>

namespace utf8 
{
	std::wstring utf8::decode(const std::string &str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		return conv.from_bytes(str);
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