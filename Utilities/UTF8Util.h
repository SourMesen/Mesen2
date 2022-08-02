#pragma once

#include <fstream>

namespace utf8 {
	class utf8
	{
	public:
		static std::wstring decode(const std::string &str);
		static std::string encode(const std::wstring &wstr);
		static std::string encode(const std::u16string &wstr);
	};
		
#ifdef _WIN32
	class ifstream : public std::ifstream
	{
	public:
		ifstream(const std::string& _Str, ios_base::openmode _Mode = ios_base::in, int _Prot = (int)ios_base::_Openprot) : std::ifstream(utf8::decode(_Str), _Mode, _Prot) { }
		ifstream() : std::ifstream() { }
		void open(const std::string& _Str, ios_base::openmode _Mode = ios_base::in, int _Prot = (int)ios_base::_Openprot)	{ std::ifstream::open(utf8::decode(_Str), _Mode, _Prot); }
	};

	class ofstream : public std::ofstream
	{
	public:
		ofstream(const std::string& _Str, ios_base::openmode _Mode = ios_base::in, int _Prot = (int)ios_base::_Openprot) : std::ofstream(utf8::decode(_Str), _Mode, _Prot) { }
		ofstream() : std::ofstream() { }
		void open(const std::string& _Str, ios_base::openmode _Mode = ios_base::in, int _Prot = (int)ios_base::_Openprot) { std::ofstream::open(utf8::decode(_Str), _Mode, _Prot); }
	};
#else
	using std::ifstream;
	using std::ofstream;
#endif
}