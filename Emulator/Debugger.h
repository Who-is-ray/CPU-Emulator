// The class of debugger
#pragma once

#include<list>

class CPU;

class Debugger
{
public:
	Debugger();	//constructor
	~Debugger();	//destructor

	void run_debugger();	//function to run debugger called by main
	bool load_SRecord(unsigned char* memory, CPU& m_CPU);	//The function to load S-Record data to memory

	void add_PC_BP();	//add a new break point triggered by program counter
	void add_clk_BP();	//add a new break point triggered by CPU clock
	void dlt_PC_BP();	//delete a break point triggered by program counter
	void dlt_clk_BP();	//delete a break point triggered by CPU clock
	void display_BP(std::string cmt, std::list<int>& l);	//display all break point from a list

private:
	std::list<int> PC_BP_list;	//break point list relate to Program Counter 
	std::list<int> clk_BP_list;	//break point list relate to CPU clock
};