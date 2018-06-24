#if 1	//real main
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
	std::string str = "3D";
	char number;
	char a = 255;
	char b = 2;
	int c = 4;
	short d = 8;
	short f = d << 1;
	std::cout << f<<std::endl;

	system("pause");
	return 0;
}
#endif 