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
	unsigned char a = 255;
	short b = 1;
	int c = 4;
	unsigned short d = a;
	bool e = b;
	short f = d << 1;
	short x = 0xff00;
	unsigned short	y = 0xf0f0;
	y+=x;
	std::cout << x*b <<std::endl;

	system("pause");
	return 0;
}
#endif 