

/*
- Signal handling example
- Catch control C
- sigaction not used because it is not supported

ECED 3403
27 June 16
*/
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <signal.h> /* Signal handling software */
//
//#define FALSE 0
//#define TRUE  1
//
//int waiting_for_signal; /* T|F - indicates whether ^C detected */
//
//void sigint_hdlr()
//{
//	/*
//	- Invoked when SIGINT (control-C) is detected
//	- changes state of waiting_for_signal
//	- signal must be reinitialized
//	*/
//	waiting_for_signal = FALSE;
//	signal(SIGINT, (_crt_signal_t)sigint_hdlr); /* Reinitialize SIGINT */
//}
//
//int main(int argc, char *argv[])
//{
//	/*
//	- Mainline - example of loop waiting for sigint
//	*/
//	long loop_count;
//	int i;
//
//	/* Call signal() - bind sigint_hdlr to SIGINT */
//	signal(SIGINT, (_crt_signal_t)sigint_hdlr);
//
//	for (i = 0; i<5; i++)
//	{
//		printf("Starting %d...\n", i);
//		loop_count = 0;
//		waiting_for_signal = TRUE;
//
//		while (waiting_for_signal)
//			loop_count++;
//
//		printf("All done %ld\n", loop_count);
//		getchar();
//	}
//
//	getchar();
//	return 0;
//}

//#include <signal.h>
//#include <stdio.h>
//void sigint(int a)
//{
//	printf("^C caught\n");
//	signal(SIGINT, sigint);
//}
//int main()
//{
//	signal(SIGINT, sigint);
//	for (;;) {}
//}


////
//
//#include <iostream>
//#include "Debugger.h"
//
////#include <ctype.h>
////#include <signal.h> /* Signal handling software */
//
//bool running;
//
////void sigint_hdlr()
////{
////	/*
////	- Invoked when SIGINT (control-C) is detected
////	- changes state of waiting_for_signal
////	- signal must be reinitialized
////	*/
////	running = false;
////	signal(SIGINT, (_crt_signal_t)sigint_hdlr); /* Reinitialize SIGINT */
////	
////}
//
//int main()
//{
//	Debugger m_debugger;
//
//	//running = true;
//
//	//signal(SIGINT, (_crt_signal_t)sigint_hdlr);
//	//std::string str = "37bD";
//	//long int number;
//	//if (isxdigit(str[0]))
//	//{
//	//	number = strtol(str.c_str(), NULL, 16/*base of hexdecimal*/);
//	//	printf("The hexadecimal number %lx is %ld.\n", number, number);
//	//}
//
//	//while (running)
//	//{}
//
//	system("pause");
//	return 0;
//}