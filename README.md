# Emulator

Assignment 2 of Computer Architecture:
For this assignment, we are asked to design, build and test an emulator for X-Makina Instruction Set Architecture which can emulator the functionality of CPU, registers, memory and device. Also, a loader and a simple debugger are required too. 
Loader is responsible for checking the validity of input file and S-Record. And then load the S-records to the memory. Also loader need to load device file to device time table as well.
Debugger should be able to ask input file to load, start and stop the CPU emulator, display and edit registers, display and edit memory contents and set break point(s). Also it pauses the program when a break point is hit and ask user’s command.
CPU Emulator should emulate CPU’s functionalities include fetch, decode and execute. Device and interrupts need to be emulator as well.

Assignment 3 of Computer Architecture:
For this assignment, we are asked to design, build and test an cache which will be added to the emulator we built in assignment 2. 
Cache is an array of size of 32, which has faster speed than memory when CPU accessing. Two cache organization method will be emulated in this assignment, Associative and Directive mapping.
For associative, we have a dirty bit to indicated if the data has been changed and need to be write back before it got overwrite. Also it has a 5 bit age bits to indicate the recent using age. For write through, it always keep consistence for memory and cache, so every write need to update both memory and cache, so dirty bit is not necessary in write through.
In this assignment, all memory except mem[0] to mem[15] can be store into cache. The reason that device CSR will not be store in cache because it need to keep consistence with cache and memory so it actually not saving to much time.
