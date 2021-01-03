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
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ti/sysbios/posix/time.h>
#include <unistd.h>

//#include <xdc/runtime/Timestamp.h>
//#include <xdc/runtime/Types.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SDSPI.h>
// #include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTCC26XX.h>
#include <ti/display/Display.h>
#include <ti/display/DisplayUart.h>
// #include <ti/drivers/Watchdog.h>

#include <dataFile.h>

#include <utilFunctions.h>



#define SAMPLERATE 200

extern UARTCC26XX_Object uartCC26XXObjects[1];

//#define SHOW_ARRAYS 1



static event_p observedEvent;

extern event_p dispatchedEvent;


int dispatchEnabled = 0;

int totalNumOfEvents = 0;

float rawDataAcquired = 0.0;

int eventGenExit = 0;


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
    dispatchedEvent = observedEvent;
    Semaphore_post(eventProcessSemHandle);

    totalNumOfEvents++;

    observedEvent.timeStamp = 0;;
    observedEvent.height = 0;
    observedEvent.V1 = 0;
    observedEvent.V2 = 0;
}

/*
float timediff(uint32_t t1, uint32_t t2) {
    Timestamp_getFreq(&timestampFreq);

    return ((t2-t1)*1.0)/timestampFreq.lo;
}
*/


/* Board Header file *
 *
 *  ======== eventGenTask ========
 */
int eventGenTask(UArg arg0, UArg arg1)
{
    uint32_t t1,t2;
    float elapsed;

    //Timekeeping variables

    float rawValue = 0;
    int dataCounter = 0;
    int delayValue = 10;
    float rawDataBuffer[20] = {0};


    observedEvent.timeStamp = 0;
    observedEvent.height = 0;
    observedEvent.V1 = 0;
    observedEvent.V2 = 0;


    while(dataCounter<delayValue){
        Semaphore_pend(eventGenSemHandle,BIOS_WAIT_FOREVER);
        if(eventGenExit)break; //do not proceed if I don't have data

        //retrive events
        rawValue = rawDataAcquired;
        //Display_printf(hSerial, 0, 0, "%f\r\t", rawValue);

        rawDataBuffer[dataCounter] = rawValue;
        dataCounter ++;
    }

    t1 = Timestamp_get32();
    while(1){
        Semaphore_pend(eventGenSemHandle,BIOS_WAIT_FOREVER);

        if(eventGenExit)break; //do not proceed if I don't have data

        rawValue = rawDataAcquired;

        //Display_printf(hSerial, 0, 0, "%f\r\t", rawValue);

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


        if(value2>value1){
            incrementEvent(value2);
        }else{
            if(dispatchEnabled){
                dispatchEvent(value2,dataCounter);
                dispatchEnabled = 0;
            }
        }
        dataCounter++;

    }

    t2 = Timestamp_get32();
    elapsed = timediff(t1,t2);
    Display_printf(hSerial, 0, 0, "elapsed: %f seconds\n", elapsed);
    Display_printf(hSerial, 0, 0, "Processed events: %d\n", dataCounter);
    Display_printf(hSerial, 0, 0, "Generated events: %d\n", totalNumOfEvents);
    Display_printf(hSerial, 0, 0, "Time per event: %f ms\n", (elapsed*1000)/(dataCounter-delayValue));
    Display_printf(hSerial, 0, 0,"UART speed %d\n",uartCC26XXObjects[0].baudRate);

    return 0;

}


int eventCaptureTask(UArg arg0, UArg arg1){

    int dataCounter = 0;
    Display_Params params;


    // Open display
    UART_init();

    Display_Params_init(&params);
    hSerial = Display_open(Display_Type_UART, &params);
    uartCC26XXObjects[0].baudRate = 921600; //Device specific baud rate

    Task_sleep(10000);

    if (hSerial) {
            Display_printf(hSerial, 0, 0, "EventGen function demo\r\t");
    }

    clkHandle =  Clock_handle(&clkStruct);
    Clock_start(clkHandle);

    while(1){
        Semaphore_pend(timerSemHandle,BIOS_WAIT_FOREVER);
        if(dataCounter<(sizeof(rawEvents)/sizeof(rawEvents[0]))){
        //if(dataCounter<2000){
            rawDataAcquired = rawEvents[dataCounter];
            //Display_printf(hSerial, 0, 0, "%d\r\t", dataCounter);
            Semaphore_post(eventGenSemHandle);
            dataCounter++;

        }else{
            eventGenExit = 1;
            Semaphore_post(eventGenSemHandle);
            Semaphore_post(eventProcessSemHandle);
            Clock_stop(clkHandle);
            return 0;
        }
    }

}
