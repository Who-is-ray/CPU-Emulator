// The class of debugger
#pragma once

#include<list>	//library for list

class CPU;
class Memory;

class Debugger
{
public:
	Debugger() { clock_limit = 1000; }	//constructor of debugger, set initial clock limit value to 1000
	~Debugger() {}	//destructor

	void run_debugger();	//function to run debugger called by main
	bool load_SRecord(Memory& memory, CPU& m_CPU);	//The function to load S-Record data to memory
	bool load_device_file(Memory& memory);	//the function to load device file

	void add_PC_BP();	//add a new break point triggered by program counter
	void set_clk_limit();	//add a new break point triggered by CPU clock
	void dlt_PC_BP();	//delete a break point triggered by program counter
	void display_BP(std::string cmt, std::list<int>& l);	//display all break point from a list

	void check_debugger_status(CPU& m_CPU);	//check debugger status in CPU cycle after fetch decode execute
	void check_device_table() {};	//check device table to emulate input output device

private:
	std::list<int> PC_BP_list;	//break point list relate to Program Counter 
	int clock_limit;	//clock limit for debugger running
};