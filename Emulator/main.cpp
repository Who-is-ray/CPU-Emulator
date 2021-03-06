#if 1	//debug
/*
The X - Makina Project
- Ray Su
*/
#include "Debugger.h"	//the header file of debugger class

/*
	starting point of the program
	create debugger object and run debugger
*/
int main()
{
	Debugger m_debugger;
	m_debugger.run_debugger();
	return 0;
}


#else	//test

#include <iostream>

#include <ctype.h>
#include<queue>	//library for queue
#include<map>	//library for map

struct output_data_info
{
	unsigned char data;	//output char
	unsigned char device_num;	//output device number
	output_data_info(unsigned char d, unsigned char n) { data = d; device_num = n; }	//constructor of struct output_data_info
};

int main()
{
	char number;
	unsigned char a = 0b00111100;
	unsigned char b = 0b01100110;
	unsigned char c = ~b;
	unsigned char d = ~a;
	bool e = b;
	unsigned short f = 0xffff-32;
	short x = 0xff00;
	unsigned short	y = 0xf0f0;
	y+=x;
	unsigned char t = 100;
	std::map<int, std::queue<output_data_info>>::reverse_iterator it;
	std::map<int, std::queue<output_data_info>> t_map;
	for (int i = 0; i < 10; i++)
	{
		output_data_info info1(t + 1, t + 11);
		t_map[i].emplace(info1);
	}
	for (it = t_map.rbegin(); it != t_map.rend(); ++it)
	{
		std::cout << it->first << "\n";
	}
	std::cout << f <<std::endl;

	system("pause");
	return 0;
}
#endif 