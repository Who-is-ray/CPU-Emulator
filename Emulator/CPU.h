//Class of CPU

#pragma once
#include<string>

#define SIZE_OF_REGISTER 8

class CPU
{
public:
	CPU();
	~CPU();

private:
	std::string* Register;
};

