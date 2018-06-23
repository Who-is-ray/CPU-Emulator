#include "Debugger.h"
#include <iostream>
#include <fstream>	// Input/output stream class to operate on files


Debugger::Debugger()
{
	std::cout << "aaaaa";
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
	std::cout << "Input the file name of S-Record: ";
	std::cin >> SRecord_fileName;
	//std::ifstream SRecord_file(SRecord_fileName);

	return true;
}

void Debugger::run_debugger()
{

}