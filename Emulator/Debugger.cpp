#include "Debugger.h"
#include <iostream>
#include <fstream>	// Input/output stream class to operate on files
#include <string>

#define SIZE_OF_MEMORY 65536
#define BASE_OF_HEX 16

Debugger::Debugger()
{
	memory = new unsigned char[SIZE_OF_MEMORY];
}


Debugger::~Debugger()
{
	delete memory;
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
	bool rtv = SRecord_file.is_open();	//  Is filed opened successfully? Yes = true, No = false.
	if (rtv)	// If open successfully, start loading S-Record
	{
		std::string line;
		while (std::getline(SRecord_file, line))	//reading lines from input file
		{
			if (line.size()>=10)
				{
				std::string type = line.substr(0, 2);	//get the record type:S0,S1,S9	
				if (type=="S1")	// If it is S1 Record
				{
					int count = strtol(line.substr(2, 2).c_str(), NULL, BASE_OF_HEX);	//get count from record and convert to integer
					short address = strtol(line.substr(4, 4).c_str(), NULL, BASE_OF_HEX);	//get address from record and convert to integer
					char sum = count + strtol(line.substr(4, 2).c_str(), NULL, BASE_OF_HEX) + strtol(line.substr(6, 2).c_str(), NULL, BASE_OF_HEX);	//create sum = count + address.ll +address.hh
					for (size_t i = 8; i < line.size()-2; i+=2)	//start load data to memory
					{
						unsigned char data = strtol(line.substr(i, 2).c_str(), NULL, BASE_OF_HEX);	//convert data to char
						sum += data;
						memory[address] = data;	//load data to memory
						address++;	//update destination address
					}
					sum += (char)strtol(line.substr(line.size()-2, 2).c_str(), NULL, BASE_OF_HEX);	//update sum
					if ((int)sum != -1)	//check sum = ff
					{	//if check sum not correct
						rtv = false;	//return false
						std::cout << line << " --check sum not correct\n";
						break;
					}
				}
			}
			else
			{
				std::cout << "Record has mistake\n";
				break;
			}
		}
	}

	return rtv;
}

void Debugger::run_debugger()
{

	// print help
	//ask user for command


	bool is_load = false;
	while (!is_load)
	{
		is_load = load_SRecord();
		if (!is_load)
			std::cout << "Loading failed! ";
	}
	
	for (size_t i = 200; i < 500; i++)
	{
		std::cout<<i<<"\t"<<(unsigned int)memory[i]<<std::endl;
	}
}