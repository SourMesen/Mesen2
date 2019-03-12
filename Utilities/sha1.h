/*
    sha1.h - header of

    ============
    SHA-1 in C++
    ============

    100% Public Domain.

    Original C Code
        -- Steve Reid <steve@edmweb.com>
    Small changes to fit into bglibs
        -- Bruce Guenter <bruce@untroubled.org>
    Translation to simpler C++ Code
        -- Volker Grabsch <vog@notjusthosting.com>
    Safety fixes
        -- Eugene Hopkinson <slowriot at voxelstorm dot com>
*/

#pragma once

#include <cstdint>
#include <iostream>
#include <string>

class SHA1
{
public:
    SHA1();
    void update(const std::string &s);
    void update(std::istream &is);
    std::string final();
    static std::string GetHash(const std::string &filename);
	 static std::string GetHash(std::istream &stream);
	 static std::string GetHash(vector<uint8_t> &data);
	 static std::string GetHash(uint8_t* data, size_t size);

private:
    uint32_t digest[5];
    std::string buffer;
    uint64_t transforms;
};
