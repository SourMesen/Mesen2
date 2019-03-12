#pragma once

class Serializer;

class ISerializable
{
public:
	virtual void Serialize(Serializer &s) = 0;
};

