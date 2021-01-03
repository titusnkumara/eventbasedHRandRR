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

Types_FreqHz timestampFreq;   /* Timestamp frequency */
Display_Handle hSerial;


#define SAMPLERATE 200

//#define SHOW_ARRAYS 1

struct event{
    float   timeStamp;
    int     height;
    float   V1;
    float   V2;
};

typedef struct event event_p;


static event_p observedEvent;
int dispatchEnabled = 0;

int totalNumOfEvents = 0;


void incrementEvent(float voltage){

    if(observedEvent.height == 0){
        //first time here
        observedEvent.V1 = voltage;
        dispatchEnabled = 1;
    }
    observedEvent.height++;

}

void dispatchEvent(float voltage, int sampleNumber){
    observedEvent.V2 = voltage;
    observedEvent.timeStamp = sampleNumber/(SAMPLERATE*1.0);

    //Display_printf(hSerial, 0, 0, "%f %d %f %f\n", observedEvent.timeStamp,observedEvent.height,observedEvent.V1,observedEvent.V2);
    totalNumOfEvents++;

    observedEvent.timeStamp = 0;;
    observedEvent.height = 0;
    observedEvent.V1 = 0;
    observedEvent.V2 = 0;
}


float timediff(uint32_t t1, uint32_t t2) {
    Timestamp_getFreq(&timestampFreq);

    return ((t2-t1)*1.0)/timestampFreq.lo;
}



/* Board Header file *
 *
 *  ======== mainThread ========
 */
int mainThread(UArg arg0, UArg arg1)
{
    uint32_t t1,t2;
    float elapsed;

    /*
     * Open display
     */
    Display_Params params;
    UART_init();
    Display_Params_init(&params);
    hSerial = Display_open(Display_Type_UART, &params);

    if (hSerial) {
            Display_printf(hSerial, 0, 0, "EventGen function demo\r\t");
    }
    t1 = Timestamp_get32();

    //Timekeeping variables

    float rawValue = 0;
    int dataCounter = 0;
    int delayValue = 10;
    float rawDataBuffer[20] = {0};


    observedEvent.timeStamp = 0;
    observedEvent.height = 0;
    observedEvent.V1 = 0;
    observedEvent.V2 = 0;

    while(dataCounter<10){
        //retrive events
        rawValue = rawEvents[dataCounter];
        rawDataBuffer[dataCounter] = rawValue;
        dataCounter ++;
    }

    while(dataCounter<(sizeof(rawEvents)/sizeof(rawEvents[0]))){
        rawValue = rawEvents[dataCounter];
        int newdataposition = dataCounter%20;
        int delayPosition = newdataposition - delayValue;
        if(delayPosition<0){
            delayPosition = delayPosition+20;
        }

        //insert data
        rawDataBuffer[newdataposition] = rawValue;

        //do the comparison
        float value2 = rawValue;
        float value1 = rawDataBuffer[delayPosition];

        //printf("%f %f\n",value2,value1);

        if(value2>value1){
            //printf("%d\t%d\n",dataCounter,1);
            incrementEvent(value2);
        }else{
            //printf("%d\t%d\n",dataCounter,0);
            if(dispatchEnabled){
                dispatchEvent(value2,dataCounter);
                dispatchEnabled = 0;
            }
        }

        //printf("%f\n",rawValue);
        dataCounter++;

    }

    t2 = Timestamp_get32();
    elapsed = timediff(t1,t2);
    Display_printf(hSerial, 0, 0, "elapsed: %f seconds\n", elapsed);
    Display_printf(hSerial, 0, 0, "Processed events: %d\n", dataCounter);
    Display_printf(hSerial, 0, 0, "Generated events: %d\n", totalNumOfEvents);
    Display_printf(hSerial, 0, 0, "Time per event: %f ms\n", (elapsed*1000)/dataCounter);
    t1 = Timestamp_get32();
    sleep(1);
    t2 = Timestamp_get32();
    elapsed = timediff(t1,t2);
    Display_printf(hSerial, 0, 0, "second =  %f\n", elapsed);

    return 0;

}
