



Readme
-------------
The **bucketingProcess** program takes the event information in **eventArray.h** header file. Then the program calculate the respiratory rate and heart rate using  the bucketing algorithm. The output is written to **result.txt** file. 

Use Android Clang/LLVM toolchain to cross-compile C source code to an executable file for Arm architecture. 


#### **Compilation**

Install Android NDK version with Clang compiler.
https://developer.android.com/ndk/downloads 

This source code is tested with NDK version 20.0.5594570 and compiled to android21 target.

Use the following command to compile the program.

 \<Path to clang.exe file folder>\clang.exe  .\bucketingProcess.c -o bucketingProcess.out -target aarch64-linux-android21 -lm  -mfloat-abi=hard

> **Note :** Make sure eventArray.h file is present in the current directory. If not, generate the event file using eventGen program 
> run 
> cat rawValues_TestDataset.txt | ./eventgen
> or
> cat rawValues_VerificationDataset.txt | ./eventgen
> to generage the header file either using test dataset or verification data set.

#### **Usage**

Connect the Android phone via USB connector. Enable debugging mode. Open the terminal in host PC and push the compiled executable to the device using **adb**. Make sure to send the file to a directory with write permission such as /data/local/tmp. 

> **Push :**   adb push ./bucketingProcess.out /data/local/tmp
> **Open shell:** adb shell
> **Locate file:** cd /data/local/tmp
> **Change permissions:** chmod +x bucketingProcess.out  
> **Run:** ./bucketingProcess.out > result.txt
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
Elapsed time 13.137939 ms
Time per event 0.012572 ms
Average RR 17.712807
Average HR 84.909912
