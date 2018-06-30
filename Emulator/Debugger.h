// The class of debugger
#pragma once

#include<list>	//library for list
#include<map>
#include<vector>

class CPU;
class Memory;

struct device_timetable_info
{
	unsigned char device_num;
	unsigned char data;
	device_timetable_info(unsigned char d_n, unsigned char d) { device_num = d_n; data = d; }
};

class Debugger
{
public:
	Debugger() { clock_limit = 1000; }	//constructor of debugger, set initial clock limit value to 1000
	~Debugger() {}	//destructor

	void run_debugger();	//function to run debugger called by main
	bool load_SRecord(Memory& memory, CPU& m_CPU);	//The function to load S-Record data to memory
	bool load_device_file(Memory& memory, CPU& m_CPU);	//the function to load device file

	void add_PC_BP();	//add a new break point triggered by program counter
	void set_clk_limit();	//add a new break point triggered by CPU clock
	void dlt_PC_BP();	//delete a break point triggered by program counter
	void display_BP(std::string cmt, std::list<int>& l);	//display all break point from a list

	void check_debugger_status(CPU& m_CPU, const unsigned int clock);	//check debugger status in CPU cycle after fetch decode execute
	void check_device_table(Memory& mem, const unsigned int clock);	//check device table to emulate input output device

private:
	std::list<int> PC_BP_list;	//break point list relate to Program Counter 
	unsigned int clock_limit;	//clock limit for debugger running

	/*
		time table to emulate date arrive device
		each map element is a vector of data to load at one time point
		map stores and sorts vectors for multiple time points
	*/
	std::map<unsigned int /*time*/, std::vector<device_timetable_info>> device_timetable;	
};