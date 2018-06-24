#include "Debugger.h"
#include <iostream>	//library for cin
#include <fstream>	// Input/output stream class to operate on files
#include <string>	// library for string
#include "CPU.h"

#define SIZE_OF_MEMORY 65536	//size of memory
#define BASE_OF_HEX 16	//base of hexdecimal
#define MIM_SIZE_OF_SRECORD 10	//minimun size of S-Record
#define START_BIT_OF_COUNT 2	//start bit of count of S-Record

int help_func();

//constructor
Debugger::Debugger()	
{
	//memory = new unsigned char[SIZE_OF_MEMORY];	//initialize memory
}

//destructor
Debugger::~Debugger()	
{
	//delete memory;	//delete memory
}

/*
	The function to load S-Record data to memory
	Return loading result true if loading successfully, false if has problem
*/
bool Debugger::load_SRecord(unsigned char* memory)
{
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
			int record_size = line.size();
			if (record_size >= MIM_SIZE_OF_SRECORD)	//check record condition is correct
				{
				std::string type = line.substr(0, 2);	//get the record type:S0,S1,S9	
				if (type=="S1")	// If it is S1 Record
				{
					int count = strtol(line.substr(START_BIT_OF_COUNT, 2).c_str(), NULL, BASE_OF_HEX);	//get count from record and convert to integer	magic number??
					short address = static_cast<short>(strtol(line.substr(4, 4).c_str(), NULL, BASE_OF_HEX));	//get address from record and convert to integer
					char sum = count + static_cast<char>(strtol(line.substr(4, 2).c_str(), NULL, BASE_OF_HEX)) + static_cast<char>(strtol(line.substr(6, 2).c_str(), NULL, BASE_OF_HEX));	//create sum = count + address.ll +address.hh
					for (size_t i = 8; i < line.size()-2; i+=2)	//start load data to memory
					{
						unsigned char data = static_cast<unsigned char>(strtol(line.substr(i, 2).c_str(), NULL, BASE_OF_HEX));	//convert data to char
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
			{	// if line size < minimum size, error
				std::cout << "Record has mistake\n";
				break;
			}
		}
	}

	return rtv;
}

//function to run debugger called by main
void Debugger::run_debugger()
{
	unsigned char* memory = new unsigned char[SIZE_OF_MEMORY];	//initialize memory
	CPU m_CPU(memory);
	int user_cmd=0;	//user's command

	// print help and ask user for command
	user_cmd = help_func();

	switch (user_cmd)
	{
		case 1:	//Load S-Record
		{
			bool is_load = false;
			while (!is_load)	//??need comment?
			{
				is_load = load_SRecord(memory);	//Call loading function
				if (!is_load)	//if loading failed
					std::cout << "Loading failed! Try again!\n";
			}
			break;
		}
		default:
			break;
	}
	
	/*for (size_t i = 200; i < 500; i++)
	{
		std::cout << i << "\t";
		printf("%02lx\n", memory[i]);
	}*/
}

//Function to display help list
int help_func()
{
	int user_cmd;	//user's command
	std::cout << "Command list:\n1. Load S-Record file\n\nChoose a command, type the number of command:	";
	std::cin >> user_cmd;
	return user_cmd;
}