/*
 * Copyright (c) 2016-2017, Texas Instruments Incorporated
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
 *  ======== main_tirtos.c ========
 */
#include <stdint.h>

/* RTOS header files */
/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

#include <ti/display/Display.h>
#include <ti/display/DisplayUart.h>


/* Example/Board Header files */
#include "Board.h"

#include <utilFunctions.h>


extern int eventGenTask(UArg arg0, UArg arg1);
extern int eventCaptureTask(UArg arg0, UArg arg1);
extern int eventProcessorTask(UArg arg0, UArg arg1);



/* Stack size in bytes */
#define TASKSTACKSIZE    1024
#define E_GEN_TASKSTACKSIZE    1024
#define E_PROC_TASKSTACKSIZE    2048

uint32_t eventProcessTaskPriority = 2;
uint32_t eventGenTaskPriority = 3;
uint32_t eventCaptureTaskPriority = 3;


Task_Struct taskeventGenStruct;
Char taskeventGenStack[E_GEN_TASKSTACKSIZE];

Task_Struct taskeventCaptureStruct;
Char taskeventCaptureStack[TASKSTACKSIZE];

Task_Struct taskeventProcessorStruct;
Char taskeventProcessorStack[E_PROC_TASKSTACKSIZE];

/*
 *  ======== main ========
 */
int main(void)
{
    Task_Params taskParamseventGen;
    Task_Params taskParamseventCapture;
    Task_Params taskParamseventProcess;
    /* Call driver init functions */
    Board_initGeneral();
    Display_init();

    /* Construct eventCapture Task  thread */
    Task_Params_init(&taskParamseventCapture);
    taskParamseventCapture.stackSize = TASKSTACKSIZE;
    taskParamseventCapture.stack = &taskeventCaptureStack;
    taskParamseventCapture.priority = eventCaptureTaskPriority;
    Task_construct(&taskeventCaptureStruct, (Task_FuncPtr)eventCaptureTask, &taskParamseventCapture, NULL);

    /* Construct eventGen Task  thread */
    Task_Params_init(&taskParamseventGen);
    taskParamseventGen.stackSize = E_GEN_TASKSTACKSIZE;
    taskParamseventGen.stack = &taskeventGenStack;
    taskParamseventGen.priority = eventGenTaskPriority;
    Task_construct(&taskeventGenStruct, (Task_FuncPtr)eventGenTask, &taskParamseventGen, NULL);




    /* Construct eventProcessor Task  thread */
    Task_Params_init(&taskParamseventProcess);
    taskParamseventProcess.stackSize = E_PROC_TASKSTACKSIZE;
    taskParamseventProcess.stack = &taskeventProcessorStack;
    taskParamseventProcess.priority = eventProcessTaskPriority;
    Task_construct(&taskeventProcessorStruct, (Task_FuncPtr)eventProcessorTask, &taskParamseventProcess, NULL);

    //create timer semaphore
    createTimerSemaphore();
    createEventGenSemaphore();
    createEventProcessSemaphore();
    createClockTimer(5000);

    BIOS_start();

    return (0);
}
