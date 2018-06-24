#if 1
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


#else

#include <iostream>

#include <ctype.h>

int main()
{
	std::string str = "3D";
	char number;
	char a = 255;
	char b = 2;
	int c = 4;
	std::cout << a + b<<std::endl;
	if (isxdigit(str[0]))
	{
		number = strtol(str.c_str(), NULL, 16/*base of hexdecimal*/);
		printf("The hexadecimal number %lx is %ld.\n", number, number);
	}

	system("pause");
	return 0;
}
#endif 