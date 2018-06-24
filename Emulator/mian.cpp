#if 0	//real main
//

#include <iostream>
#include "Debugger.h"

int main()
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
	unsigned short d = 0xff00;

	short f = d << 1;
	std::cout << d+a <<std::endl;

	system("pause");
	return 0;
}
#endif 