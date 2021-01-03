/*
 * utilFunctions.c
 *
 *  Created on: 26 Feb. 2020
 *      Author: Titus
 */


#include <utilFunctions.h>

Clock_Struct clkStruct;
Clock_Handle clkHandle;
Semaphore_Struct timerSemStruct;
Semaphore_Handle timerSemHandle;

Semaphore_Struct eventGenSemStruct;
Semaphore_Handle eventGenSemHandle;

Semaphore_Struct eventProcessSemStruct;
Semaphore_Handle eventProcessSemHandle;


Types_FreqHz timestampFreq;   /* Timestamp frequency */

Display_Handle hSerial;


/*
 *  ======== clk0Fxn =======
 *  Clock interrupt handler
 */

void clk0Fxn(UArg arg0){
    Semaphore_post(timerSemHandle);
}



void createTimerSemaphore(){

    Semaphore_Params semParams;
    /* Construct a Semaphore object to be use as a resource lock, inital count 1 */
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&timerSemStruct, 0, &semParams);
    /* Obtain instance handle */
    timerSemHandle = Semaphore_handle(&timerSemStruct);
}

void createEventGenSemaphore(){

    Semaphore_Params semParams;
    /* Construct a Semaphore object to be use as a resource lock, inital count 1 */
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&eventGenSemStruct, 0, &semParams);
    /* Obtain instance handle */
    eventGenSemHandle = Semaphore_handle(&eventGenSemStruct);
}

void createEventProcessSemaphore(){

    Semaphore_Params semParams;
    /* Construct a Semaphore object to be use as a resource lock, inital count 1 */
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&eventProcessSemStruct, 0, &semParams);
    /* Obtain instance handle */
    eventProcessSemHandle = Semaphore_handle(&eventProcessSemStruct);
}


void createClockTimer(uint32_t period){
    /* Construct BIOS Objects */
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = period/Clock_tickPeriod;
    clkParams.startFlag = FALSE;

    /* Construct a periodic Clock Instance */
    Clock_construct(&clkStruct, (Clock_FuncPtr)clk0Fxn,
                    0, &clkParams);
}

float timediff(uint32_t t1, uint32_t t2) {
    Timestamp_getFreq(&timestampFreq);

    return ((t2-t1)*1.0)/timestampFreq.lo;
}
