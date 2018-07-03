//Class of CPU
#pragma once
#include<string>	//library for using std::string and string functions
#include<deque>	//library for queue
#include<map>	//library for map

#define SIZE_OF_REGISTER	8
#define SIZE_OF_CONST_TABLE	8

class Memory;

struct output_data_info
{
	unsigned char data;	//output char
	unsigned int device_num;	//output device number
	output_data_info(unsigned char d, unsigned int n) { data = d; device_num = n; }	//constructor of struct output_data_info
};

class CPU
{
public:
	CPU(Memory& memory, unsigned int& clock);	//constructor
	~CPU() {}	//destructor

	unsigned short get_register_val(int loc) { return Register_file[loc]; }	//get a specific register value
	void set_register_val(int address, unsigned short val) { Register_file[address] = val; }	//set a specific register value
	unsigned char get_current_priority();	//get current priority

	void fetch();	//CPU fetch function, emulate fetch routine
	void decode();	//CPU decode function, emulate decode routine
	void execute();	//CPU execute function, emulate execute routine
	void check_interrupt();	//check interrput to emulate interrupt

	std::map<unsigned char /*device number*/, unsigned char /*processing time*/>device_process_time;	//map stores each output device's processing time

private:
	unsigned short Register_file[SIZE_OF_REGISTER] = {NULL};	//register file
	Memory& m_mem;	//memory pointer
	unsigned int& m_clock;	//system clock
	unsigned short MAR;	//memory address register
	unsigned short MDR;	//memory data register
	unsigned short IR;	//instruction register
	unsigned const short const_table[SIZE_OF_CONST_TABLE] = { 0,1,2,4,8,32,48,0xffff };	//constant table that store the constand value

	void write_byte_to_dst(unsigned short& dst, unsigned char data);	//write a byte size data to destination lower byte
	short get_relative_offset();	//return relative offset value from instruction (LDR STR)
	short get_offset();	//return offset value from instruction	(BEQ/BZ to BAL)
	void ModifyStatusFlags(unsigned int result, unsigned int DST_Data, unsigned int SRC_Data, unsigned int carry_bit, unsigned int sign_bit);	//updat psw register
	void push_to_stack(unsigned short data_to_push);	//push a data to stack pointer
	void pull_from_stack(unsigned short& data_to_poll);	//pull a data from stack pointer

	/*
		queue of interrupt(s), this is the queue of maps, each map stores and sorts priority and device number of interrupt at one time point
		because there could be multiple interrupt need to be process at same time
	*/
	std::deque<std::map<unsigned char /*priority*/, unsigned char /*device number*/>> interrput_queue;
	std::map<unsigned int /*time to output*/, std::deque<output_data_info>> output_list;	//list to store output data
};