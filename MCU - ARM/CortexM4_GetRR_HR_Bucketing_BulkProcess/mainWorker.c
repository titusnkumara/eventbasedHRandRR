/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== empty.c ========
 */

/* For usleep() */
#include <dataFile.h>
#include <dataFile.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>

#include <xdc/runtime/Timestamp.h>
#include <xdc/runtime/Types.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SDSPI.h>
// #include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>
#include <ti/display/Display.h>
#include <ti/display/DisplayUart.h>
#include <ti/drivers/uart/UARTCC26XX.h>
// #include <ti/drivers/Watchdog.h>

#include "dataFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

Types_FreqHz freq;   /* Timestamp frequency */
Float        factor;  /* Clock ratio cpu/timestamp */
extern UARTCC26XX_Object uartCC26XXObjects[1];


float timediff(uint32_t t1, uint32_t t2) {
    Timestamp_getFreq(&freq);

    return ((t2-t1)*1.0)/freq.lo;
}




#define SAMPLERATE          200
#define RESP_CUT_OFF        50
#define RR_MAX_PERIOD       1
#define HR_MAX_PERIOD       0.33
#define EVENTARRAYSIZE      30
#define REFERENCE_ARR_SIZE  13

//#define SHOW_ARRAYS 1

struct event{
    float   timeStamp;
    int     height;
    float   V1;
    float   V2;
};

typedef struct event event_p;
typedef struct event event_r;


FILE *fp = NULL;
FILE *fp_w_R = NULL;
FILE *fp_w_P = NULL;

event_p RespEventArray[2];
event_p PulseEventArray[2];



void displayEventBuffer(event_p buffer[],int bufferSize){

    int i = 0;
    event_p tmpEvent;
    for(i=0;i<bufferSize;i++){
        tmpEvent = buffer[i];
        printf("%f ",tmpEvent.timeStamp);

    }
    putchar('\n');

}


void displayfloatArray(float buffer[],int bufferSize){

    int i = 0;
    for(i=0;i<bufferSize;i++){
        printf("%f ",buffer[i]);
    }
    putchar('\n');

}

void displayIntArray(int buffer[],int bufferSize){

    int i = 0;
    for(i=0;i<bufferSize;i++){
        printf("%d ",buffer[i]);
    }
    putchar('\n');

}

event_p* eventRespirationMergeFilter(event_p rawEventArray[]){
    float P;

    //temporary event vairables
    float T;
    int H;
    float V1;
    float V2;
    event_p event1;
    event_p event2;

    event_p newEvent;


    //get events
    event1 = rawEventArray[0];
    event2 = rawEventArray[1];

    //assign event details
    int H1      = event1.height;
    int H2      = event2.height;

    float T1    = event1.timeStamp;
    float T2    = event2.timeStamp;

    float V1_1  = event1.V1;
    float V1_2  = event2.V1;

    float V2_1  = event1.V2;
    float V2_2  = event2.V2;


    if(H2>=20 && H1>=20){
        //check period
        P = T2 - (H2*1.0)/SAMPLERATE - T1;

        if(P<(SAMPLERATE*0.1)){

            H = (int)(P*SAMPLERATE);
            T = T2 - (H2*1.0)/SAMPLERATE;
            V1 = V2_1;
            V2 = V1_2;

            newEvent.timeStamp = T;
            newEvent.height = H;
            newEvent.V1 = V1;
            newEvent.V2 = V2;

            //replace event as event1
            rawEventArray[0] = newEvent;

            //change event 2
            H = H1+H2;
            T = T2;
            V1 = V1_1;
            V2 = V2_2;

            //assign event2
            newEvent.timeStamp = T;
            newEvent.height = H;
            newEvent.V1 = V1;
            newEvent.V2 = V2;

            rawEventArray[1] = newEvent;

            return rawEventArray;

        }else{
            //in this case I assume I have missed all pulse events in between
            //these are fast respirations
            return rawEventArray;
        }
    }else{
        //just return back the original array
        return rawEventArray;
    }

    //return rawEventArray;

}


event_r timingFilterRespiration(event_p newEvent){
    event_p event1;
    event_p event2;

    event_r returnEvent;
    returnEvent.height = 0;

    //check for validity
    if(newEvent.height<RESP_CUT_OFF || newEvent.timeStamp<0){
        return returnEvent;
    }

    //event1 is the last valid event
    event1 = RespEventArray[1];
    event2 = newEvent;

    //decide if I should keep the old event or this event
    float timeDiff = event2.timeStamp - event1.timeStamp;
    if(timeDiff<=RR_MAX_PERIOD){
        //decide which one to take
        if(event2.height > event1.height){
            //keep event 2,
            //delete event 1
            RespEventArray[1] = event2;
        }else{
            //delete event 2
            RespEventArray[1] = event1;
        }

    }else{
        //fprintf(fp_w_R, "%f %d %f %f\n", RespEventArray[0].timeStamp,RespEventArray[0].height,RespEventArray[0].V1,RespEventArray[0].V2);
        returnEvent.timeStamp = RespEventArray[0].timeStamp;
        returnEvent.height = RespEventArray[0].height;
        returnEvent.V1 = RespEventArray[0].V1;
        returnEvent.V2 = RespEventArray[0].V2;

        RespEventArray[0] = RespEventArray[1];
        RespEventArray[1] = newEvent;
        return returnEvent;
    }
    return returnEvent;

}



event_p timingFilterPulse(event_p newEvent){


    event_p returnEvent;
    event_p event1;
    event_p event2;

    //event1 is the last valid event
    event1 = PulseEventArray[1];
    event2 = newEvent;

    //decide if I should keep the old event or this event
    float timeDiff = event2.timeStamp - event1.timeStamp;

    if(timeDiff<=HR_MAX_PERIOD){
        //decide which one to take
        if(event2.height > event1.height){
            //keep event 2,
            //delete event 1
            PulseEventArray[1] = event2;
        }else{
            //delete event 2
            PulseEventArray[1] = event1;
        }

    }else{
        //prepare return event
        returnEvent.timeStamp = PulseEventArray[0].timeStamp;
        returnEvent.height = PulseEventArray[0].height;
        returnEvent.V1 = PulseEventArray[0].V1;
        returnEvent.V2 = PulseEventArray[0].V2;

        PulseEventArray[0] = PulseEventArray[1];
        PulseEventArray[1] = newEvent;

        return returnEvent;

    }
    returnEvent.height = 0;
    return returnEvent;


}

void initializeRespEventArray(){

    RespEventArray[0].timeStamp = 0;
    RespEventArray[0].height = 0;
    RespEventArray[0].V1 = 0;
    RespEventArray[0].V2 = 0;
    RespEventArray[1].timeStamp = 0;
    RespEventArray[1].height = 0;
    RespEventArray[1].V1 = 0;
    RespEventArray[1].V2 = 0;

}

void initializePulseEventArray(){

    PulseEventArray[0].timeStamp = 0;
    PulseEventArray[0].height = 0;
    PulseEventArray[0].V1 = 0;
    PulseEventArray[0].V2 = 0;
    PulseEventArray[1].timeStamp = 0;
    PulseEventArray[1].height = 0;
    PulseEventArray[1].V1 = 0;
    PulseEventArray[1].V2 = 0;

}



float processRReventStream(event_r event[]){

    float RR = 60./(event[1].timeStamp - event[0].timeStamp);
    event[0] = event[1];
    return RR;
}



/*
Getting the pulse rate
*/

const float referenceArray[REFERENCE_ARR_SIZE] = {0.4,0.5,0.6,0.7,0.8,0.9,1,1.2,1.6,2,2.4,2.8,3.2};

int counterArray[REFERENCE_ARR_SIZE] = {0};

float meanArray[REFERENCE_ARR_SIZE] = {0};
float eventArray[EVENTARRAYSIZE] = {0};

event_p timeDiffEvents[2] = {0};

float HR_Last_valid = 72.0;

int processedHReventCount = 0;

int lastValueIndex = 0;


int getBucketIndex(float value,const float referenceArray[]){

    int localCounter = 0;
    while(localCounter<REFERENCE_ARR_SIZE && value>referenceArray[localCounter]){
        localCounter++;
    }
    if(localCounter>=REFERENCE_ARR_SIZE){
        //not suitable for bucketing, discard
        return -1;
    }else{
        return localCounter;
    }
}


float getHRfromEvent(event_p event){

    timeDiffEvents[1] = event;

    float timeDifference = timeDiffEvents[1].timeStamp - timeDiffEvents[0].timeStamp;

    //printf("%f %f\n",timeDiffEvents[1].timeStamp,timeDifference);

    int bucketIndex = getBucketIndex(timeDifference,referenceArray);
    if(bucketIndex == -1){
        //discard this event
        return HR_Last_valid;
    }
    //else proceed

    if(processedHReventCount<0){
        //Just add these values to correct buckets
        //first find which index it matches
        //printf("REF %f %d %f\n",timeDifference,bucketIndex,referenceArray[bucketIndex]);

        //get current total to calculate new mean
        float currentTotal = counterArray[bucketIndex] * meanArray[bucketIndex];
        //increase the counterArray
        counterArray[bucketIndex]++;
        //change mean
        meanArray[bucketIndex] = (currentTotal+timeDifference)/(counterArray[bucketIndex]);
        //add new element to event array
        eventArray[processedHReventCount] = timeDifference;

    }else{
        /*

        if(processedHReventCount==30){
            displayfloatArray(meanArray,REFERENCE_ARR_SIZE);
            displayIntArray(counterArray,REFERENCE_ARR_SIZE);
            displayfloatArray(eventArray,EVENTARRAYSIZE);

        }
        */
        //remove last
        //find the value
        float removeValue = eventArray[lastValueIndex];

        //find the index
        int removeIndex = getBucketIndex(removeValue,referenceArray);
        //do the removing part

        float currentTotal = counterArray[removeIndex] * meanArray[removeIndex];
        counterArray[removeIndex]--;
        if(counterArray[removeIndex]>0){
            //I have non zero elements here after removing
            meanArray[removeIndex] = (currentTotal - removeValue)/counterArray[removeIndex];
        }else{
            //make mean zero
            meanArray[removeIndex] = 0;
        }


        //Now add the new one
        //bucket index is already found
        float currentAddTotal = counterArray[bucketIndex] * meanArray[bucketIndex];
        //increase the counterArray
        counterArray[bucketIndex]++;
        //change mean
        meanArray[bucketIndex] = (currentAddTotal+timeDifference)/(counterArray[bucketIndex]);
        //add new element to event array
        eventArray[lastValueIndex] = timeDifference;
        lastValueIndex = (lastValueIndex+1)%EVENTARRAYSIZE;



        //Now do the estimation - needs to be a function
        int majorityFound = 0;
        int i;
        for(i=0;i<REFERENCE_ARR_SIZE;i++){
            int val = counterArray[i];
            if(val>=(EVENTARRAYSIZE/2)){
                //printf("Majority found here\n");
                majorityFound = 1;
                int counterSum = 0;
                float meanSum = 0;
                //get adjecent sum
                if(i==0){
                    //discard left one
                    counterSum = counterArray[i]+counterArray[i+1];
                    meanSum = counterArray[i]*meanArray[i]+counterArray[i+1]*meanArray[i+1];
                }else if(i==(REFERENCE_ARR_SIZE-1)){
                    //discard last one
                    counterSum = counterArray[i-1]+counterArray[i];
                    meanSum = counterArray[i-1]*meanArray[i-1]+counterArray[i]*meanArray[i];
                }else{
                    counterSum = counterArray[i-1]+counterArray[i]+counterArray[i+1];
                    meanSum = counterArray[i-1]*meanArray[i-1]+counterArray[i]*meanArray[i]+counterArray[i+1]*meanArray[i+1];
                }
                float HR_mean_time = meanSum/counterSum;
                HR_Last_valid = 60/HR_mean_time;
                break;


            }


        }

        int sumOfTwoFound = 0;
        if(!majorityFound){
            //try option 2
            for(i=1;i<REFERENCE_ARR_SIZE;i++){
                int val = counterArray[i-1]+counterArray[i];
                if(val>=(EVENTARRAYSIZE/2)){
                //printf("Majority found here\n");
                sumOfTwoFound = 1;
                int counterSum = 0;
                float meanSum = 0;
                //get adjecent sum
                if(i==(REFERENCE_ARR_SIZE-1)){
                    //discard last one
                    counterSum = counterArray[i-1]+counterArray[i];
                    meanSum = counterArray[i-1]*meanArray[i-1]+counterArray[i]*meanArray[i];
                }else{
                    counterSum = counterArray[i-1]+counterArray[i]+counterArray[i+1];
                    meanSum = counterArray[i-1]*meanArray[i-1]+counterArray[i]*meanArray[i]+counterArray[i+1]*meanArray[i+1];
                }
                float HR_mean_time = meanSum/counterSum;
                HR_Last_valid = 60/HR_mean_time;
                break;


            }


            }

        }

        if(!majorityFound && !sumOfTwoFound){
            //try option 3
            for(i=2;i<REFERENCE_ARR_SIZE;i++){
                int val = counterArray[i-2]+counterArray[i-1]+counterArray[i];
                if(val>=(EVENTARRAYSIZE/2)){
                    //printf("Majority found here\n");
                    sumOfTwoFound = 1;
                    int counterSum = 0;
                    float meanSum = 0;
                    //get adjecent sum
                    counterSum = counterArray[i-2]+counterArray[i-1]+counterArray[i];
                    meanSum = counterArray[i-2]*meanArray[i-2]+counterArray[i-1]*meanArray[i-1]+counterArray[i]*meanArray[i];

                    float HR_mean_time = meanSum/counterSum;
                    HR_Last_valid = 60/HR_mean_time;


                    //try to compensate for this
                    int forwardCount = i;
                    for(forwardCount=i;forwardCount<REFERENCE_ARR_SIZE;forwardCount++){
                        //check if any count is larger than 3
                        if(counterArray[forwardCount]>3){
                            //check this event for missing beats
                            if(meanArray[forwardCount]!=0){
                                float differencHR_ = 120/meanArray[forwardCount] - HR_Last_valid;
                                if(differencHR_*differencHR_ < 225){
                                    //15 beats difference
                                    float HR_compensated_mean = (counterSum*HR_mean_time + meanArray[forwardCount]*counterArray[forwardCount])/(counterSum+2*counterArray[forwardCount]);
                                    counterSum = counterSum+ 2*counterArray[forwardCount];
                                    HR_mean_time = HR_compensated_mean;
                                    HR_Last_valid = 60/HR_mean_time;
                                }
                                differencHR_ = 180/meanArray[forwardCount] - HR_Last_valid;
                                if(differencHR_*differencHR_ < 225){
                                    //15 beats difference
                                    float HR_compensated_mean = (counterSum*HR_mean_time + meanArray[forwardCount]*counterArray[forwardCount])/(counterSum+3*counterArray[forwardCount]);
                                    counterSum = counterSum+ 3*counterArray[forwardCount];
                                    HR_mean_time = HR_compensated_mean;
                                    HR_Last_valid = 60/HR_mean_time;
                                }


                            }
                        }
                    }


                    break;
                }
            }

        }




        //estimation done
        //printf("REF %f %d %f\n",timeDifference,bucketIndex,referenceArray[bucketIndex]);
    }

    //printf("%f %f;\n",event.timeStamp,HR_Last_valid);
    timeDiffEvents[0] = timeDiffEvents[1];
    processedHReventCount++;
    return HR_Last_valid;
}





















/* Board Header file *
 *
 *  ======== mainThread ========
 */
int mainThread(UArg arg0, UArg arg1)
{

    float elapsed = 0;
    uint32_t t1,t2;

    /*
     * Open display
     */
    Display_Params params;
    UART_init();
    Display_Params_init(&params);
    Display_Handle hSerial = Display_open(Display_Type_UART, &params);

    //uartCC26XXObjects[0].baudRate = 921600; //Device specific baud rate

    Task_sleep(10000);

    if (hSerial) {
            Display_printf(hSerial, 0, 0, "Hello Serial!");
    }
    //Measure time
    t1 = Timestamp_get32();



    //Timekeeping variables
    event_p event1;
    event_p event2;
    event_p processEvent;

    event_p tmpEvent;
    event_p processedEvent;

    event_r RespEventBuffer[2];

    RespEventBuffer[0].timeStamp = 0;
    RespEventBuffer[0].height = 0;

    processedEvent.height = 0;

    //filtered event array
    event_p * mergedEventArray;
    event_p rawEventArray[2];

    //performance variables
    int numOfEventsProcessed = 0;

    //initialize RespEventArray and PulseEventArray
    initializeRespEventArray();
    initializePulseEventArray();
    uint32_t arraySize = sizeof(eventDataArray) / sizeof(eventDataArray[0]);


    //scan the first instance
    event1.timeStamp = eventDataArray[0][0];
    event1.height = (int)eventDataArray[0][1];
    event1.V1 = eventDataArray[0][2];
    event1.V2 = eventDataArray[0][3];

    numOfEventsProcessed++;
    float DAQStartTimestamp = event1.timeStamp;
    //Display_printf(hSerial,0,0,"Starting timeStamp %f\n",DAQStartTimestamp);

    while(numOfEventsProcessed<arraySize){
            event2.timeStamp = eventDataArray[numOfEventsProcessed][0];
            event2.height = (int)eventDataArray[numOfEventsProcessed][1];
            event2.V1 = eventDataArray[numOfEventsProcessed][2];
            event2.V2 = eventDataArray[numOfEventsProcessed][3];
            //printf("%f %d %f %f\n", event2.timeStamp,event2.height,event2.V1,event2.V2);
            //performance variable
            numOfEventsProcessed++;

            //call filter function
            rawEventArray[0] = event1;
            rawEventArray[1] = event2;
            mergedEventArray = eventRespirationMergeFilter(rawEventArray);

            //this is the event I am going to separate
            processEvent = mergedEventArray[0];

            if(processEvent.height>=RESP_CUT_OFF){
                //write resp events
                event_r tmpRevent = timingFilterRespiration(processEvent);

                if(tmpRevent.height>0){
                    RespEventBuffer[1]= tmpRevent;
                    float RR = processRReventStream(RespEventBuffer);
                    //Display_printf(hSerial,0,0,"%f %f %d;\r\t",tmpRevent.timeStamp,RR,1);
                    //printf("%f %d %f %f;\n",tmpRevent.timeStamp,tmpRevent.height, tmpRevent.V1, tmpRevent.V2);
                }

            }else{
                //write pulse events
                processedEvent = timingFilterPulse(processEvent);

                //check if this is valid
                if(processedEvent.height>0){


                    //create Tmp event
                    tmpEvent.timeStamp = processedEvent.timeStamp;
                    tmpEvent.height = processedEvent.height;
                    tmpEvent.V1 = processedEvent.V1;
                    tmpEvent.V2 = processedEvent.V2;



                    float HR = getHRfromEvent(tmpEvent);
                    Display_printf(hSerial,0,0,"%f %f %d;\r\t",tmpEvent.timeStamp,HR,0);
                    //printf("%f %d %f %f;\n",tmpEvent.timeStamp,tmpEvent.height, tmpEvent.V1, tmpEvent.V2);



                }

            }
            event1 = mergedEventArray[1];
    };


    //printf("%s","}");
    //Measure time
    t2 = Timestamp_get32();
    elapsed = timediff(t1,t2);
    Display_printf(hSerial, 0, 0, "elapsed: %f seconds\n", elapsed);
    Display_printf(hSerial, 0, 0, "Time per event %f ms\n",(elapsed*1000.0)/numOfEventsProcessed);
    Display_printf(hSerial, 0, 0, "NoOfEvents = %d\n",numOfEventsProcessed);
    //Display_printf(hSerial, 0, 0, "dummyTotal: %f\n", eventTotal);
    //I will have to discard the last event
    /*
    fclose(fp);
    fclose(fp_w_P);
    fclose(fp_w_R);
    */

    t1 = Timestamp_get32();
    Task_sleep(10000);
    t2 = Timestamp_get32();
    elapsed = timediff(t1,t2);
    Display_printf(hSerial, 0, 0, "second =  %f\n", elapsed);

    return 0;

}
