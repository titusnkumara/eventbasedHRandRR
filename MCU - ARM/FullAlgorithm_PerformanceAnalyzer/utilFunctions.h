/*
 * utilFunctions.h
 *
 *  Created on: 26 Feb. 2020
 *      Author: Titus
 */

#ifndef UTILFUNCTIONS_H_
#define UTILFUNCTIONS_H_

/* Error reporting library */
#include <xdc/runtime/Error.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Clock.h>
#include <xdc/runtime/Types.h>
#include <xdc/runtime/Timestamp.h>
#include <ti/sysbios/knl/Task.h>

#include <ti/display/Display.h>
#include <ti/display/DisplayUart.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTCC26XX.h>


struct event{
    float   timeStamp;
    int     height;
    float   V1;
    float   V2;
};


typedef struct event event_p;
typedef struct event event_r;


/*
 * Timer variables
 */

extern Semaphore_Struct timerSemStruct;
extern Semaphore_Handle timerSemHandle;
extern Clock_Struct clkStruct;
extern Clock_Handle clkHandle;
extern Semaphore_Struct eventGenSemStruct;
extern Semaphore_Handle eventGenSemHandle;
extern Semaphore_Struct eventProcessSemStruct;
extern Semaphore_Handle eventProcessSemHandle;

extern Display_Handle hSerial;



void clk0Fxn(UArg arg0);
void createTimerSemaphore();
void createEventGenSemaphore();
void createEventProcessSemaphore();
void createClockTimer(uint32_t period);
float timediff(uint32_t t1, uint32_t t2);


#endif /* UTILFUNCTIONS_H_ */
