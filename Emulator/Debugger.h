// The class of debugger
#pragma once

#include<list>	//library for list
#include<map>	//library for map

class CPU;
class Memory;

struct interrput_info
{

};

class Debugger
{
public:
	Debugger() {}	//constructor
	~Debugger() {}	//destructor

	void run_debugger();	//function to run debugger called by main
	bool load_SRecord(Memory& memory, CPU& m_CPU);	//The function to load S-Record data to memory

	void add_PC_BP();	//add a new break point triggered by program counter
	void add_clk_BP();	//add a new break point triggered by CPU clock
	void dlt_PC_BP();	//delete a break point triggered by program counter
	void dlt_clk_BP();	//delete a break point triggered by CPU clock
	void display_BP(std::string cmt, std::list<int>& l);	//display all break point from a list

	void check_debugger_status(CPU& m_CPU);	//check debugger status in CPU cycle after fetch decode execute
	void check_device_table() {};	//check device table to emulate input output device	??only input device have interrupe?
	void check_interrupt();	//check interrput to emulate interrupt

private:
	std::list<int> PC_BP_list;	//break point list relate to Program Counter 
	std::list<int> clk_BP_list;	//break point list relate to CPU clock
	std::map<unsigned char, struct interrput_info> interrput_list;	//list of interrupt(s)
};