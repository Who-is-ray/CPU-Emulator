#include "Debugger.h"	//header that define debugger class
#include "CPU.h"	//header that define CPU class
#include <iostream>	//library for cin
#include <fstream>	//Input/output stream class to operate on files
#include <string>	//library for string
#include <signal.h>	//Signal handling software

#define BASE_OF_HEX	16	//base of hexdecimal
#define MIM_SIZE_OF_SRECORD	10	//minimun size of S-Record
#define ADDRESS_OF_PROGRAM_COUNTER	7	//address of program counter
#define CORRECT_SUM	0xff

bool cpu_is_running;		//flag that indicate whether cpu should keep running
bool debugger_is_running;	//flag that indicate whether debugger should keep running

//Function to display help list
int help_func()
{
	int user_cmd;	//user's command
	std::cout << "\nCommand list:\n"
		<< " 0: Load device file\n"
		<< " 1: Load S-Record file\n"
		<< " 2: Add a Program Counter break point\n"
		<< " 3: Set clock limit\n"
		<< " 4: Delete a Program Counter break point\n"
		<< " 5: Display clock limit\n"
		<< " 6: Display all break point(s)\n"
		<< " 7: Display a range of data from a memory\n"
		<< " 8: Display register\n"
		<< " 9: Update data from a memory\n"
		<< "10: Update a data from a register\n"
		<< "11: Run CPU\n"
		<< "12: Exit\n"
		<<"Choose a command, type the number of command:	";
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
	cpu_is_running = false;	//pause CPU's running
	signal(SIGINT, (_crt_signal_t)sigint_hdlr); /* Reinitialize SIGINT */
}

/*
	check debugger status in CPU cycle after fetch decode execute
	m_CPU - CPU currently running
*/
void Debugger::check_debugger_status(CPU& m_CPU, const unsigned int clock)
{
	unsigned short PC = m_CPU.get_register_val(ADDRESS_OF_PROGRAM_COUNTER);	//get PC value
	//printf("PC = %4lx\n", m_CPU.get_register_val(7));	//program counter test print out	!!
	//std::cout << "clock = " << clock << "\n";	//clock test print out

	if (std::find(PC_BP_list.begin(), PC_BP_list.end(), PC) != PC_BP_list.end())	//if found a break point matchs current PC value
	{
		cpu_is_running = false;
		printf("Catched break point %4lx\n", PC);
	}

	if (clock>=clock_limit)	//if reached clock limit
	{
		cpu_is_running = false;
		printf("Exceed clock limit %d\n", clock_limit);
	}
}

//check device table to emulate input output device
void Debugger::check_device_table(Memory& mem, const unsigned int clock)
{
	std::map<unsigned int /*time*/, std::vector<device_timetable_info>>::iterator it= device_timetable.begin();
	while (it !=device_timetable.end())	//start check data comes in at this time
	{
		if (it->first <= clock)	//if there is data need to load
		{
			for (size_t i = 0; i < it->second.size(); i++)	//check each data need to load at this time
			{
				mem.m_memory.byte_mem[it->second[i].device_num * 2 + 1] = it->second[i].data;	//load data to device CSR.data
				update_CSR(WRITE, WORD, it->second[i].device_num * 2, mem);	//update device CSR
			}
			int time = it->first;
			it++;
			device_timetable.erase(time);	//finished loading, remove request from timetable
		}
		else
			it++;
	}
}

/*
	The function to load S-Record data to memory
	Return loading result true if loading successfully, false if has problem
	memory - memory to load to
	m_CPU - CPU to load to
*/
bool Debugger::load_SRecord(Memory& memory, CPU& m_CPU)
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
				unsigned char count = static_cast<unsigned char>(strtol(line.substr(2, 2).c_str(), NULL, BASE_OF_HEX));	//get count(start from bit 2 and has 2-bit size) from record and convert to integer	magic number??
				unsigned short address = static_cast<unsigned short>(strtol(line.substr(4, 4).c_str(), NULL, BASE_OF_HEX));	//get address(start from bit 4 and has 4-bit size) from record and convert to integer
				unsigned char sum = count
					+ static_cast<unsigned char>(strtol(line.substr(4, 2).c_str(), NULL, BASE_OF_HEX))
					+ static_cast<unsigned char>(strtol(line.substr(6, 2).c_str(), NULL, BASE_OF_HEX));	//create sum = count + address.ll +address.hh
				for (size_t i = 8; i < line.size() - 2; i += 2)	//start load data to memory
				{
					unsigned char data = static_cast<unsigned char>(strtol(line.substr(i, 2).c_str(), NULL, BASE_OF_HEX));	//convert data(has 2-bit size) to char
					sum += data;
					memory.m_memory.byte_mem[address] = data;	//load data to memory
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
				unsigned char count = static_cast<unsigned char>(strtol(line.substr(2, 2).c_str(), NULL, BASE_OF_HEX));	//get count from record and convert to integer
				unsigned short PC = static_cast<unsigned short>(strtol(line.substr(4, 4).c_str(), NULL, BASE_OF_HEX));	//get address from record and convert to integer
				unsigned char sum = count
					+ static_cast<unsigned char>(strtol(line.substr(4, 2).c_str(), NULL, BASE_OF_HEX))
					+ static_cast<unsigned char>(strtol(line.substr(6, 2).c_str(), NULL, BASE_OF_HEX))
					+ (unsigned char)strtol(line.substr(line.size() - 2, 2).c_str(), NULL, BASE_OF_HEX);	//create sum = count + PC.ll +PC.hh
				m_CPU.set_register_val(ADDRESS_OF_PROGRAM_COUNTER, PC);	//set Program Counter value
				if ((int)sum != CORRECT_SUM)	//if check sum not equal to ff
				{	//if check sum not correct
					rtv = false;	//return false
					std::cout << line << " --check sum not correct\n";
					break;
				}
			}
			else if (type == "S0")	//do nothing for S0 record
			{}
			else
				rtv = false;
		}
	}
	if (!rtv)	//if loading failed
		std::cout << "Loading failed! Try again!\n";
	return rtv;
}

/*
	the function to load device file
*/
bool Debugger::load_device_file(Memory& memory, CPU& m_CPU)
{
	std::string Device_fileName;
	std::cout << "Input the file name of device file: ";	//ask user for file name
	std::cin >> Device_fileName;	//get user's input
	FILE* Device_file;
	Device_file = fopen(Device_fileName.c_str(), "r");
	bool rtv = false;
	if (Device_file != nullptr)	// If open successfully, start loading S-Record
	{
		unsigned int in_out = 0, dev_num = 0;
		unsigned int process_time = 0;
		while (dev_num<8)	//reading the first 8 lines from input file for device initialization
		{	
			fscanf(Device_file, "%u\t%u\t%u", &dev_num, &in_out, &process_time);
			if (in_out == 1)	//if is input device
				memory.m_memory.byte_mem[dev_num * 2] = CSR_SET_INPUT;	//set CSR.IO
			else	//if is output device
			{
				memory.m_memory.byte_mem[dev_num * 2] &= CSR_SET_OUTPUT;	//clear CSR.IO
				m_CPU.device_process_time[dev_num] = process_time;	//add device's processing time to processing time table
			}
			dev_num++;	//load initialization info for next device
		}

		unsigned int time = 0;
		unsigned char data = 0;
		while (fscanf(Device_file, "%u	%u	%c", &time, &dev_num, &data) != EOF)	//loading data for device
		{
			device_timetable[time].emplace_back(dev_num, data);
		}
		rtv = true;
	}
	fclose(Device_file);
	if (!rtv)	//if loading failed
		std::cout << "Loading failed! Try again!\n";
	return rtv;
}

/*
	function to run debugger called by main
	accept user's command
*/
void Debugger::run_debugger()
{
	signal(SIGINT, (_crt_signal_t)sigint_hdlr);	//Call signal() - bind sigint_hdlr to SIGINT 
	unsigned int clock = 0;	//Initialize clock, the reason I put clock here is because clock usually are located outside CPU, such us matherboard
	Memory mem(clock);	//initialize memory
	CPU m_CPU(mem, clock);
	int user_cmd=0;	//user's command
	debugger_is_running = true;

	while (debugger_is_running)
	{
		// print help and ask user for command
		user_cmd = help_func();

		switch (user_cmd)
		{
		case 0:	//load device file
		{
			bool is_load = false;
			while (!is_load)	//if haven't load
			{
				is_load = load_device_file(mem, m_CPU);	//Call loading function
			}
			break;
		}
		case 1:	//Load S-Record
		{
			bool is_load = false;
			while (!is_load)	//if haven't load
			{
				is_load = load_SRecord(mem, m_CPU);	//Call loading function
			}
			break;
		}
		case 2:	//add new PC break point
		{
			add_PC_BP();
			break;
		}
		case 3:	//set new clock limit
		{
			set_clk_limit();
			break;
		}
		case 4:	//delete a break point from PC list
		{
			dlt_PC_BP();
			break;
		}
		case 5:	//display clock limit
		{
			std::cout << "Clock limit is: " << clock_limit << std::endl;
			break;
		}
		case 6:	//display all break point list
		{
			display_BP("All Program Counter break point(s):", PC_BP_list);	//Display pc break point list
			break;
		}
		case 7:	//display data from a specific memory
		{
			std::string start_address, end_address;
			//!!if(address>)
			std::cout << "Type in the hex value of starting address of the memory to display: ";
			std::cin >> start_address;
			std::cout << "Type in the hex value of ending address of the memory to display: ";
			std::cin >> end_address;
			for (int i = strtol(start_address.c_str(), NULL, BASE_OF_HEX); i <= strtol(end_address.c_str(), NULL, BASE_OF_HEX); i += 2)
				printf("Data in memory[%04lx] is %04lx\n", i, mem.m_memory.word_mem[static_cast<unsigned short>(i) >> 1]);	//display hex decimal of the specific memory value
			break;
		}
		case 8:	//display register
		{
			for (int i = 0; i < 8; i++)
				printf("register[%d] = %04lx\n", i, m_CPU.get_register_val(i));	//display hex decimal of the specific register value
			break;
		}
		case 9:	//Update a data from a memory
		{
			std::string address, data;
			std::cout << "Type in the hex value of address of the memory to update: ";
			std::cin >> address;
			std::cout << "Type in the two digits hex value of data to update: ";
			std::cin >> data;
			mem.m_memory.byte_mem[static_cast<unsigned short>(strtol(address.c_str(), NULL, BASE_OF_HEX))] = static_cast<unsigned char>(strtol(data.c_str(), NULL, BASE_OF_HEX));	//update hex decimal of the specific memory value
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
			cpu_is_running = true;
			while (cpu_is_running)	//fetch-decode-execute cycle
			{
				m_CPU.fetch();
				m_CPU.decode();
				m_CPU.execute();
				check_device_table(mem, clock);
				m_CPU.check_interrupt();
				check_debugger_status(m_CPU, clock);
			}
			break;
		}
		case 12:	//exit debugger
		{
			debugger_is_running = false;
			break;
		}
		default:	//other wise
			break;
		}
	}
	
	////memeory test output!!
	//for (size_t i = 250; i < 400; i++)
	//{
	//	printf("mem[%04lx]\t%02lx\n", i, mem.m_memory.byte_mem[i]);
	//}
}

//add a new break point triggered by program counter
void Debugger::add_PC_BP()
{
	std::cout << "Type in the Program Counter hex value of the new break point: ";
	std::string PC_value;
	std::cin >> PC_value;
	PC_BP_list.push_back(strtol(PC_value.c_str(), NULL, BASE_OF_HEX));	//convert hex decimal string and push to PC break point list
}

//add a new break point triggered by CPU clock
void Debugger::set_clk_limit()
{
	std::cout << "Type in the CPU clock value of the new break point: ";
	int clk_value;
	std::cin >> clk_value;
	clock_limit = clk_value;	//update clock limit
}

//delete a break point triggered by program counter
void Debugger::dlt_PC_BP()
{
	std::cout << "Type in the Program Counter hex value of break point to delete: ";
	std::string PC_value;
	std::cin >> PC_value;
	PC_BP_list.remove(strtol(PC_value.c_str(), NULL, BASE_OF_HEX));	//convert hex decimal string and remove from PC break point list
}

/*
	display all break point from a list
	cmt - comment to display
	l - list to display
*/
void Debugger::display_BP(std::string cmt, std::list<int>& l)
{
	std::cout << std::endl << cmt << std::endl;
	l.sort();
	for (std::list<int>::iterator it = l.begin(); it != l.end(); it++)	//display each element in the list
		printf("%4lx\n", *it);
}

//update CSR after process with CSR.data
void Debugger::update_CSR(ACTION rw, SIZE bw, unsigned short address, Memory& mem)
{
	if (bw = WORD)	//if processing word size data, process with CSR.data
	{
		if (rw == WRITE)	//if reading from input device's control/status register
			mem.m_memory.byte_mem[address] |= (mem.m_memory.byte_mem[address] & CSR_DBA) > 0 ? CSR_OF : CSR_DBA;	//set DBA, set OF if DBA was set
	}
}