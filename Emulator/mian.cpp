#if 1	//real main
//

#include <iostream>
#include "Debugger.h"

int main()	//starting point of the program
{
	Debugger m_debugger;
	m_debugger.run_debugger();

	system("pause");
	return 0;
}


#else	//test

#include <iostream>

#include <ctype.h>

int main()
{
	char number;
	unsigned char a = 0b00111100;
	unsigned char b = 0b01100110;
	unsigned char c = ~b;
	unsigned char d = ~a;
	bool e = b;
	unsigned short f = (a&b);
	short x = 0xff00;
	unsigned short	y = 0xf0f0;
	y+=x;
	std::cout << f <<std::endl;

	system("pause");
	return 0;
}
#endif 