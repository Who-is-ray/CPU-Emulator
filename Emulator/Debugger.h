// The class of debugger
#pragma once

class Debugger
{
public:
	Debugger();
	~Debugger();

	void run_debugger();

	bool load_SRecord();

private:
	unsigned char* memory;	//is it ok that memory is an element of debugger??


};

