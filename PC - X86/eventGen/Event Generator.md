



Readme
-------------
The **eventGen** program takes the raw input values and converts values to events. 

Use GNU Compiler Collection (GCC) to compile C source code to an executable file. The program works in both Linux and Windows OS.

#### **Command**
gcc -Wall eventGen_pipe.c -o eventGen

#### **Usage**
Event generator program takes the raw data as the standard input and produces events on the standard output. Further, the program produces two files for debugging purposes. The produced file  **eventArray.h** consists a C array **const float myEventArray[][4]** which contain the event data as importable header file. This allows loading the generated events as a static array to evaluate performance neglecting the IO delay. The second file  **debugEventgen.log** currently produces the same event array for debugging purposes however can be used to print debugging information if necessary. 

execute the command in the terminal. In Linux system use cat command to print raw data. In Windows system use type command to print data.

> **Linux :**  cat rawValues.txt | ./eventGen 

> **Windows :**  type rawValues.txt | ./eventGen.exe 