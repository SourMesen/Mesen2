#pragma once

class Serializer;

enum class DeserializeResult
{
	Success,
	InvalidFile,
	SpecificError,
};

class ISerializable
{
public:
	virtual void Serialize(Serializer &s) = 0;
};

