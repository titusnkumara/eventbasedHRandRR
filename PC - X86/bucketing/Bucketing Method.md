



Readme
-------------
The **bucketingProcess** program takes the event information in **eventArray.h** header file. Then the program calculate the respiratory rate and heart rate using  the bucketing algorithm. The output is written to **result.txt** file. 

Use GNU Compiler Collection (GCC) to compile C source code to an executable file. The program works in both Linux and Windows OS.

#### **Compilation**
gcc bucketingProcess.c -Wall -o bucketingProcess

> **Note :** Make sure eventArray.h file is present in the current directory. If not, generate the event file using eventGen program 
> run 
> cat rawValues_TestDataset.txt | ./eventgen
> or
> cat rawValues_VerificationDataset.txt | ./eventgen
> to generage the header file either using test dataset or verification data set.

#### **Usage**

Execute the command in the terminal.

> **Linux :**    ./bucketingProcess > result.txt

> **Windows :**   ./bucketingProcess.exe > result.txt
###
The command writes output to file **result.txt** which has three columns. Column 1 shows the timestamp of the event, column 2 shows the calculated value and the column 3 shows if current row is related to HR or RR (0 for HR and 1 for RR).
> for example
> 715.125000 86.347183 0; represent 86.347183 beats per minute for HR calculation at 715.125 s timestamp.
> 707.409973 16.783445 1; represent 16.783445 breaths per minute for RR calculation at 707.409973 s timestamp.
###
Further the program produce **debugEventProcess.txt** file that has performance metrices. An example output looks as follows,

File open success
Starting timeStamp 0.055000
DAQperiod 719.095032 s
End timeStamp 719.150024
No of events 1045
Elapsed time 4.876221 ms
Time per event 0.004666 ms
Average RR 17.712807
Average HR 84.909912




