#include "Debugger.h"
#include <iostream>	//library for cin
#include <fstream>	//Input/output stream class to operate on files
#include <string>	//library for string
#include "CPU.h"	//header that define CPU class
#include <signal.h>	//Signal handling software

#define SIZE_OF_BYTE_MEMORY	1<<16	//size of byte memory
#define BASE_OF_HEX	16	//base of hexdecimal
#define MIM_SIZE_OF_SRECORD	10	//minimun size of S-Record
#define START_BIT_OF_COUNT	2	//start bit of count of S-Record
#define ADDRESS_OF_PROGRAM_COUNTER	7	//address of program counter

bool is_running;	//indicate whether debugger should keep running

//Function to display help list
int help_func()
{
	int user_cmd;	//user's command
	std::cout << "\nCommand list:\n"
		<< "1. Load S-Record file\n"
		<< "2. Add a Program Counter break point\n"
		<< "3. Add a CPU clock break point\n"
		<< "4. Delete a Program Counter break point\n"
		<< "5. Delete a clock break point\n"
		<< "6. Display all break point(s)\n"
		<< "7. Display a data from a memory\n"
		<< "8. Display a data from a register\n"
		<< "9. Update a data from a memory\n"
		<< "10. Update a data from a register\n"
		<< "11. Run CPU\n"
		<<"\nChoose a command, type the number of command:	";
	std::cin >> user_cmd;
	return user_cmd;
}

/*
- Invoked when SIGINT (control-C) is detected
- changes state of waiting_for_signal
- signal must be reinitialized
*/
void sigint_hdlr()
{
	is_running = false;
}

//constructor
Debugger::Debugger()	
{
}

//destructor
Debugger::~Debugger()	
{
}

/*
	The function to load S-Record data to memory
	Return loading result true if loading successfully, false if has problem
*/
bool Debugger::load_SRecord(unsigned char* memory, CPU& m_CPU)
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
			std::string type = line.substr(0, 2);	//get the record type:S0,S1,S9	

			if (type == "S1" && record_size >= MIM_SIZE_OF_SRECORD)	// If it is S1 Record and record condition is correct
			{
				unsigned char count = static_cast<unsigned char>(strtol(line.substr(START_BIT_OF_COUNT, 2).c_str(), NULL, BASE_OF_HEX));	//get count from record and convert to integer	magic number??
				unsigned short address = static_cast<unsigned short>(strtol(line.substr(4, 4).c_str(), NULL, BASE_OF_HEX));	//get address from record and convert to integer
				unsigned char sum = count
					+ static_cast<unsigned char>(strtol(line.substr(4, 2).c_str(), NULL, BASE_OF_HEX))
					+ static_cast<unsigned char>(strtol(line.substr(6, 2).c_str(), NULL, BASE_OF_HEX));	//create sum = count + address.ll +address.hh
				for (size_t i = 8; i < line.size() - 2; i += 2)	//start load data to memory
				{
					unsigned char data = static_cast<unsigned char>(strtol(line.substr(i, 2).c_str(), NULL, BASE_OF_HEX));	//convert data to char
					sum += data;
					memory[address] = data;	//load data to memory
					address++;	//update destination address
				}
				sum += (unsigned char)strtol(line.substr(line.size() - 2, 2).c_str(), NULL, BASE_OF_HEX);	//update sum
				if ((int)sum != 255)	//check sum = ff
				{	//if check sum not correct
					rtv = false;	//return false
					std::cout << line << " --check sum not correct\n";
					break;
				}
			}
			else if (type == "S9" && record_size == 10)	// If it is S9 Record and record condition is correct
			{
				unsigned char count = static_cast<unsigned char>(strtol(line.substr(START_BIT_OF_COUNT, 2).c_str(), NULL, BASE_OF_HEX));	//get count from record and convert to integer
				unsigned short PC = static_cast<unsigned short>(strtol(line.substr(4, 4).c_str(), NULL, BASE_OF_HEX));	//get address from record and convert to integer
				unsigned char sum = count
					+ static_cast<unsigned char>(strtol(line.substr(4, 2).c_str(), NULL, BASE_OF_HEX))
					+ static_cast<unsigned char>(strtol(line.substr(6, 2).c_str(), NULL, BASE_OF_HEX))
					+ (unsigned char)strtol(line.substr(line.size() - 2, 2).c_str(), NULL, BASE_OF_HEX);	//create sum = count + PC.ll +PC.hh
				m_CPU.set_register_val(ADDRESS_OF_PROGRAM_COUNTER, PC);	//set Program Counter value
				if ((int)sum != 255)	//check sum = ff
				{	//if check sum not correct
					rtv = false;	//return false
					std::cout << line << " --check sum not correct\n";
					break;
				}
			}
			else if (type == "S0")
			{}
			else
				rtv = false;
		}
	}

	return rtv;
}

/*
	function to run debugger called by main
	accept user's command
*/
void Debugger::run_debugger()
{
	signal(SIGINT, (_crt_signal_t)sigint_hdlr);	//Call signal() - bind sigint_hdlr to SIGINT 
	unsigned char memory[SIZE_OF_BYTE_MEMORY] = {NULL};	//initialize memory
	CPU m_CPU(memory);
	int user_cmd=0;	//user's command
	is_running = true;

	while (is_running)
	{
		// print help and ask user for command
		user_cmd = help_func();

		switch (user_cmd)
		{
		case 1:	//Load S-Record
		{
			bool is_load = false;
			while (!is_load)	//??need comment?
			{
				is_load = load_SRecord(memory, m_CPU);	//Call loading function
				if (!is_load)	//if loading failed
					std::cout << "Loading failed! Try again!\n";
			}
			break;
		}
		case 2:	//add new PC break point
		{
			add_PC_BP();
			break;
		}
		case 3:	//add new clock break point
		{
			add_clk_BP();
			break;
		}
		case 4:	//delete a break point from PC list
		{
			dlt_PC_BP();
			break;
		}
		case 5:	//delete a break point from clock list
		{
			dlt_clk_BP();
			break;
		}
		case 6:	//display all break point list
		{
			display_BP("All Program Counter break point(s):", PC_BP_list);	//Display pc break point list
			display_BP("All clock break point(s):", clk_BP_list);	//Display clock break point list
			break;
		}
		case 7:	//display data from a specific memory
		{
			std::string address;
			std::cout << "Type in the hex value of address of the memory to display: ";
			std::cin >> address;
			printf("Data in that memory is %02lx\n", memory[static_cast<unsigned short>(strtol(address.c_str(), NULL, BASE_OF_HEX))]);	//display hex decimal of the specific memory value
			break;
		}
		case 8:	//display data from a specific register
		{
			int address;
			std::cout << "Type in the address (0-7) of the register to display: ";
			std::cin >> address;
			printf("Data in that register is %02lx\n", m_CPU.get_register_val(address));	//display hex decimal of the specific register value
			break;
		}
		case 9:	//Update a data from a memory
		{
			std::string address, data;
			std::cout << "Type in the hex value of address of the memory to update: ";
			std::cin >> address;
			std::cout << "Type in the two digits hex value of data to update: ";
			std::cin >> data;
			memory[static_cast<unsigned short>(strtol(address.c_str(), NULL, BASE_OF_HEX))] = static_cast<unsigned char>(strtol(data.c_str(), NULL, BASE_OF_HEX));	//update hex decimal of the specific memory value
			break;
		}
		case 10:	//Update a data from a register
		{
			int address;
			std::string data;
			std::cout << "Type in the address (0-7) of the register to update: ";
			std::cin >> address;
			std::cout << "Type in the four digits hex value of data to update: ";
			std::cin >> data;
			m_CPU.set_register_val(address, static_cast<unsigned short>(strtol(data.c_str(), NULL, BASE_OF_HEX)));	//update hex decimal of the specific register value
			break;
		}
		case 11:	//run CPU
		{
			while (is_running)	//fetch-decode-execute cycle
			{
				m_CPU.fetch();
				m_CPU.decode();
				m_CPU.execute();
			}
			break;
		}
		default:	//other wise
			break;
		}
	}
	
	//memeory test output
	/*for (size_t i = 200; i < 500; i++)
	{
		std::cout << i << "\t";
		printf("%02lx\n", memory[i]);
	}*/
}

//add a new break point triggered by program counter	??create new function for these functions?
void Debugger::add_PC_BP()
{
	std::cout << "Type in the Program Counter hex value of the new break point: ";
	std::string PC_value;
	std::cin >> PC_value;
	PC_BP_list.push_back(strtol(PC_value.c_str(), NULL, BASE_OF_HEX));	//convert hex decimal string and push to PC break point list
}

//add a new break point triggered by CPU clock
void Debugger::add_clk_BP()
{
	std::cout << "Type in the CPU clock value of the new break point: ";
	int clk_value;
	std::cin >> clk_value;
	clk_BP_list.push_back(clk_value);	//add to clock break point list
}

//delete a break point triggered by program counter
void Debugger::dlt_PC_BP()
{
	std::cout << "Type in the Program Counter hex value of break point to delete: ";
	std::string PC_value;
	std::cin >> PC_value;
	PC_BP_list.remove(strtol(PC_value.c_str(), NULL, BASE_OF_HEX));	//convert hex decimal string and remove from PC break point list
}

//delete a break point triggered by CPU clock
void Debugger::dlt_clk_BP()
{
	std::cout << "Type in the CPU clock value of break point to delete: ";
	int clk_value;
	std::cin >> clk_value;
	clk_BP_list.remove(clk_value);	//delete from clock break point list
}

//display all break point from a list
void Debugger::display_BP(std::string cmt, std::list<int>& l)
{
	std::cout << std::endl << cmt << std::endl;
	l.sort();
	for (std::list<int>::iterator it = l.begin(); it != l.end(); it++)	//display each element in the list
		std::cout << *it << "\n";
}