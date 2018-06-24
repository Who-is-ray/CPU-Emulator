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
	unsigned char b = 2;
	int c = 4;
	unsigned short d = a;
	a = d >> 8;
	short f = d << 1;
	std::cout << d <<std::endl;

	system("pause");
	return 0;
}
#endif 