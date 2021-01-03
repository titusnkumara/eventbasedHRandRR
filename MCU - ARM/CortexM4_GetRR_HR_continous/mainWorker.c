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

/* Driver Header files */
#include <ti/drivers/GPIO.h>
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SDSPI.h>
// #include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>
#include <ti/display/Display.h>
#include <ti/display/DisplayUart.h>
// #include <ti/drivers/Watchdog.h>

#include "dataFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

Types_FreqHz freq1;   /* Timestamp frequency */
Types_FreqHz freq2;   /* BIOS frequency */
Float        factor;  /* Clock ratio cpu/timestamp */



#define SAMPLERATE      200
#define RESP_CUT_OFF    50
#define RR_MAX_PERIOD   1
#define HR_MAX_PERIOD   0.33
#define HR_EV_BUF_SIZE  40

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



//Event buffers for HR related tasks
float HR_buffer[HR_EV_BUF_SIZE-1];
float newHREvent_buffer[HR_EV_BUF_SIZE-1];
int harmonic_buffer[HR_EV_BUF_SIZE-1];


float timediff(uint32_t t1, uint32_t t2) {
    Timestamp_getFreq(&freq1);

    return ((t2-t1)*1.0)/freq1.lo;
}



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



/*

This section is for processing events


*/

int diff_getHR(event_p event_buffer[],float HR_buffer[],int bufferSize){

    int i;
    event_p tmpEvent1,tmpEvent2;

    for(i=1;i<bufferSize;i++){
        tmpEvent2 = event_buffer[i];
        tmpEvent1 = event_buffer[i-1];
        HR_buffer[i-1] = (tmpEvent2.timeStamp - tmpEvent1.timeStamp);
    }
    return bufferSize-1;
}


/*
This section tries to get median
*/
typedef struct floatList {
    float *list;
    int   size;
} *FloatList;

int floatcmp( const void *a, const void *b) {
    if (*(const float *)a < *(const float *)b) return -1;
    else return *(const float *)a > *(const float *)b;
}

float median( FloatList fl )
{
    qsort( fl->list, fl->size, sizeof(float), floatcmp);
    return 0.5 * ( fl->list[fl->size/2] + fl->list[(fl->size-1)/2]);
}



float get_mean(float a[], int m) {
    float sum=0;
    int i;
    for(i=0; i<m; i++)
        sum+=a[i];
    return((float)sum/m);
}


float processEventStream(event_p event_buffer[],int bufferSize){

    int HR_No_OF_events = 0;
    int i,j;
    float harmonic0;
    float harmonic1;
    float harmonic2;
    float harmonic3;
    float harmonic4;
    float harmonic1_2;

    int noOfNewEvents = 0;
    float newEventPeriod = 0;

    //convert into HR arrays
    HR_No_OF_events = diff_getHR(event_buffer,HR_buffer,bufferSize);


    #ifdef SHOW_ARRAYS
    displayfloatArray(HR_buffer,HR_No_OF_events);
    #endif

    //use median function

    struct floatList floatlist = { HR_buffer, HR_No_OF_events};
    float medianVal = median(&floatlist);



    //Assign harmonics
    harmonic0 = 1*medianVal;
    harmonic1 = 2*medianVal;
    harmonic2 = 3*medianVal;
    harmonic3 = 4*medianVal;
    harmonic4 = 5*medianVal;
    harmonic1_2 = 0.5*medianVal;

    //Assign harmonics
    //tmp harmonics arrays
    float tmpHarmonics[6] = {0};

    for(i=0;i<HR_No_OF_events;i++){
        float currentPeriod = HR_buffer[i];
        tmpHarmonics[0] = (currentPeriod - harmonic0)*(currentPeriod - harmonic0);
        tmpHarmonics[1] = (currentPeriod - harmonic1)*(currentPeriod - harmonic1);
        tmpHarmonics[2] = (currentPeriod - harmonic2)*(currentPeriod - harmonic2);
        tmpHarmonics[3] = (currentPeriod - harmonic3)*(currentPeriod - harmonic3);
        tmpHarmonics[4] = (currentPeriod - harmonic4)*(currentPeriod - harmonic4);
        tmpHarmonics[5] = (currentPeriod - harmonic1_2)*(currentPeriod - harmonic1_2);


        //find the minimum value
        int minIndex = 0;
        float minValue = tmpHarmonics[0];
        for(j=1;j<6;j++){
            if(tmpHarmonics[j]<minValue){
                minIndex = j;
                minValue = tmpHarmonics[j];
            }
        }

        //for any case index != 0


        switch(minIndex){

            case 0:{
                newEventPeriod = currentPeriod;
                noOfNewEvents = noOfNewEvents+1;
                newHREvent_buffer[noOfNewEvents-1] = newEventPeriod;
                harmonic_buffer[i] = 0;
                }
                break;
            //harmonics 1
            case 1:{
                newEventPeriod = currentPeriod/2;
                noOfNewEvents = noOfNewEvents+2;
                newHREvent_buffer[noOfNewEvents-1] = newEventPeriod;
                newHREvent_buffer[noOfNewEvents-2] = newEventPeriod;

                harmonic_buffer[i] = 1;

                }
                break;
            case 2:{
                newEventPeriod = currentPeriod/3;
                noOfNewEvents = noOfNewEvents+3;
                newHREvent_buffer[noOfNewEvents-1] = newEventPeriod;
                newHREvent_buffer[noOfNewEvents-2] = newEventPeriod;
                newHREvent_buffer[noOfNewEvents-3] = newEventPeriod;

                harmonic_buffer[i] = 2;
                }
                break;
            case 3:{
                newEventPeriod = currentPeriod/4;
                noOfNewEvents = noOfNewEvents+4;
                newHREvent_buffer[noOfNewEvents-1] = newEventPeriod;
                newHREvent_buffer[noOfNewEvents-2] = newEventPeriod;
                newHREvent_buffer[noOfNewEvents-3] = newEventPeriod;
                newHREvent_buffer[noOfNewEvents-4] = newEventPeriod;
                harmonic_buffer[i] = 3;
                }
                break;
            case 4:{
                newEventPeriod = currentPeriod/5;
                noOfNewEvents = noOfNewEvents+5;
                newHREvent_buffer[noOfNewEvents-1] = newEventPeriod;
                newHREvent_buffer[noOfNewEvents-2] = newEventPeriod;
                newHREvent_buffer[noOfNewEvents-3] = newEventPeriod;
                newHREvent_buffer[noOfNewEvents-4] = newEventPeriod;
                newHREvent_buffer[noOfNewEvents-5] = newEventPeriod;
                harmonic_buffer[i] = 4;
                }
                break;
            case 5:{
                //reject this case   but I get better results when not rejected
                newEventPeriod = currentPeriod;
                noOfNewEvents = noOfNewEvents+1;
                newHREvent_buffer[noOfNewEvents-1] = newEventPeriod;
                harmonic_buffer[i] = 0;
                }
                break;
            default:{
                }
                break;
        }

    }
    //Now print the status of the new arrays
    #ifdef SHOW_ARRAYS
    displayfloatArray(newHREvent_buffer,noOfNewEvents);
    #endif

    float meanValue = get_mean(newHREvent_buffer,noOfNewEvents);
    return 60.0/meanValue;



}

float processRReventStream(event_r event[]){

    float RR = 60./(event[1].timeStamp - event[0].timeStamp);
    event[0] = event[1];
    return RR;
}







/* Board Header file *
 *
 *  ======== mainThread ========
 */
int mainThread(UArg arg0, UArg arg1)
{

    /*
     * Open display
     */
    Display_Params params;
    UART_init();
    Display_Params_init(&params);
    Display_Handle hSerial = Display_open(Display_Type_UART, &params);

    if (hSerial) {
            Display_printf(hSerial, 0, 0, "Hello Serial!");
    }




    //float timeStamp = 0;
    //int pulseHeight = 0;
    //float V1 = 0;
    //float V2 = 0;
    event_p eventBuffer[HR_EV_BUF_SIZE];
    event_r RespEventBuffer[2];

    RespEventBuffer[0].timeStamp = 0;
    RespEventBuffer[0].height = 0;

    //Timing variables
    uint32_t t1, t2;
    float elapsed;

    //Timekeeping variables
    event_p event1;
    event_p event2;
    event_p processEvent;

    //float bufferTimePeriod = 15;
    int bufferIndex = 0;
    //float bufferStartTime = 0;
    //float bufferEndTime = bufferStartTime+bufferTimePeriod;
    event_p tmpEvent;
    event_p processedEvent;


    //int numberOfEventsInBuffer = 0;
    processedEvent.height = 0;

    int totalEventCount = 0;
    int bufferedEventNumber = 30;


    //filtered event array
    event_p * mergedEventArray;
    event_p rawEventArray[2];


    //performance variables
    int numOfEventsProcessed = 0;



    /*
    fp = fopen("AllEvents.txt", "r");
    fp_w_R = fopen("RespEvents.txt", "w");
    fp_w_P = fopen("PulseEvents.txt", "w");

    if(fp==NULL || fp_w_R==NULL || fp_w_P==NULL){
    printf("%s\n","IO fail");
        return -1;
    }
    printf("%s\n","File open success");
    */

    //initialize RespEventArray and PulseEventArray
    initializeRespEventArray();
    initializePulseEventArray();

    uint32_t arraySize = sizeof(eventDataArray) / sizeof(eventDataArray[0]);

    //Measure time
    t1 = Timestamp_get32();

    //scan the first instance
    //fscanf(fp, "%f %d %f %f", &event1.timeStamp,&event1.height,&event1.V1,&event1.V2);
    event1.timeStamp = eventDataArray[0][0];
    event1.height = (int)eventDataArray[0][1];
    event1.V1 = eventDataArray[0][2];
    event1.V2 = eventDataArray[0][3];
    float eventTotal = 0;
    numOfEventsProcessed++;
    //printf("%s","{");
    while(numOfEventsProcessed<=arraySize){

            //printf("{%f,%d,%f,%f},\n", event2.timeStamp,event2.height,event2.V1,event2.V2);
            event2.timeStamp = eventDataArray[numOfEventsProcessed][0];
            event2.height = (int)eventDataArray[numOfEventsProcessed][1];
            event2.V1 = eventDataArray[numOfEventsProcessed][2];
            event2.V2 = eventDataArray[numOfEventsProcessed][3];
            //call filter function
            rawEventArray[0] = event1;
            rawEventArray[1] = event2;
            mergedEventArray = eventRespirationMergeFilter(rawEventArray);

            //this is the event I am going to separate
            processEvent = mergedEventArray[0];
            //Display_printf(hSerial, 0, 0, "%f,%d,%f,%f\n", processEvent.timeStamp,processEvent.height,processEvent.V1,processEvent.V2);

            if(processEvent.height>=RESP_CUT_OFF){
                //write resp events
                event_r tmpRevent = timingFilterRespiration(processEvent);

                if(tmpRevent.height>0){
                    RespEventBuffer[1]= tmpRevent;
                    float RR = processRReventStream(RespEventBuffer);
                    Display_printf(hSerial, 0, 0, "%f %f %d;\r\t",tmpRevent.timeStamp,RR,1);
                    eventTotal = eventTotal+RR;
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


                    //first fill 30 events
                    if(totalEventCount<bufferedEventNumber){
                        //add this event
                        eventBuffer[bufferIndex] =  tmpEvent;
                        bufferIndex ++;
                        totalEventCount++;
                    }else{
                        //shift everything back
                        int tmpIndex = 1;
                        for(tmpIndex=1;tmpIndex<bufferedEventNumber;tmpIndex++){
                            eventBuffer[tmpIndex-1] = eventBuffer[tmpIndex];
                        }
                        eventBuffer[bufferedEventNumber-1] =  tmpEvent;
                        //process 30
                        float HR = processEventStream(eventBuffer,bufferedEventNumber);
                        eventTotal = eventTotal+HR;
                        Display_printf(hSerial, 0, 0,"%f %f %d;\r\t",tmpEvent.timeStamp,HR,0);


                    }

                }

            }
            event1 = mergedEventArray[1];
            //performance variable
            numOfEventsProcessed++;
    };

    //printf("%s","}");
    //Measure time
    t2 = Timestamp_get32();
    elapsed = timediff(t1,t2);
    Display_printf(hSerial, 0, 0, "elapsed: %f seconds\n", elapsed);
    Display_printf(hSerial, 0, 0, "Time per event %f ms\n",(elapsed*1000.0)/numOfEventsProcessed);
    Display_printf(hSerial, 0, 0, "NoOfEvents = %d\n",numOfEventsProcessed);
    Display_printf(hSerial, 0, 0, "dummyTotal: %f\n", eventTotal);
    //I will have to discard the last event
    /*
    fclose(fp);
    fclose(fp_w_P);
    fclose(fp_w_R);
    */

    t1 = Timestamp_get32();
    sleep(1);
    t2 = Timestamp_get32();
    elapsed = timediff(t1,t2);
    Display_printf(hSerial, 0, 0, "second =  %f\n", elapsed);

    return 0;











    /*
    uint32_t i;
    for(i=0;i<arraySize;i++){
        Display_printf(hSerial, 0, 0, "%f %d %f %f\n",eventDataArray[i][0],eventDataArray[i][1],eventDataArray[i][2],eventDataArray[i][3]);
    }

    while (1) {
        sleep(1);
        Display_printf(hSerial, 0, 0, "Hello Serial!");
    }
    */
}
