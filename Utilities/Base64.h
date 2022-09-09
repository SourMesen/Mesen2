#pragma once
#include "pch.h"

class Base64
{
public:
	static string Encode(const vector<uint8_t> data)
	{
		std::string out;

		int val = 0, valb = -6;
		for(uint8_t c : data) {
			val = (val << 8) + c;
			valb += 8;
			while(valb >= 0) {
				out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
				valb -= 6;
			}
		}
		if(valb>-6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
		while(out.size() % 4) out.push_back('=');
		return out;
	}

	static vector<uint8_t> Decode(string in)
	{
		vector<uint8_t> out;

		vector<int> T(256, -1);
		for(int i = 0; i < 64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

		int val = 0, valb = -8;
		for(uint8_t c : in) {
			if(T[c] == -1) break;
			val = (val << 6) + T[c];
			valb += 6;
			if(valb >= 0) {
				out.push_back(val >> valb);
				valb -= 8;
			}
		}
		return out;
	}
};
