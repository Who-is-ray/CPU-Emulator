#include "Debugger.h"
#include <iostream>
#include <fstream>	// Input/output stream class to operate on files


Debugger::Debugger()
{
	memory = new std::string[SIZE_OF_MEMORY];
}


Debugger::~Debugger()
{
}

bool Debugger::load_SRecord()
{
	/*
		The function to load S-Record data to memory
	*/
	std::string SRecord_fileName;
	std::cout << "Input the file name of S-Record file: ";	//ask user for file name
	std::cin >> SRecord_fileName;	//get user's input
	std::ifstream SRecord_file(SRecord_fileName);
	bool is_open = SRecord_file.is_open();	// Is filed opened successfully? Yes = true, No = false.
	if (is_open)	// If open successfully, start loading S-Record
	{

	}

	return is_open;
}

void Debugger::run_debugger()
{
	bool is_load = false;
	while (!is_load)
	{
		is_load = load_SRecord();
		if (!is_load)
			std::cout << "Loading failed! ";
	}
	
}