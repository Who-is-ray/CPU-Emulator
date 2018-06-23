// The class of debugger
#pragma once
#include <string>

#define SIZE_OF_MEMORY 65536

class Debugger
{
public:
	Debugger();
	~Debugger();

	void run_debugger();

	bool load_SRecord();

private:
	std::string memory[SIZE_OF_MEMORY];	//is it ok that memory is an element of debugger??


};

