#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include "eventArray.h"

#define SAMPLERATE 		200
#define RESP_CUT_OFF 	50
#define RR_MAX_PERIOD	1
#define HR_MAX_PERIOD	0.33
#define HR_EV_BUF_SIZE	40

//#define SHOW_ARRAYS 1

struct event{
	float 	timeStamp;
	int 	height;
	float 	V1;
	float 	V2;	
};

typedef struct event event_p;
typedef struct event event_r;


FILE *fp = NULL;
//FILE *fp_w_R = NULL;
//FILE *fp_w_P = NULL;

event_p RespEventArray[2];
event_p PulseEventArray[2];



//Event buffers for HR related tasks
float HR_buffer[HR_EV_BUF_SIZE-1];
float newHREvent_buffer[HR_EV_BUF_SIZE-1];
int harmonic_buffer[HR_EV_BUF_SIZE-1];

/*
double timediff(clock_t t1, clock_t t2) {
    double elapsed;
    elapsed = ((double)t2 - t1) / (CLOCKS_PER_SEC*1.0) * 1000;
    return elapsed;
}
*/


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
	int H1		= event1.height;
	int H2		= event2.height;
	
	float T1 	= event1.timeStamp;
	float T2 	= event2.timeStamp;
	
	float V1_1	= event1.V1;
	float V1_2	= event2.V1;
	
	float V2_1	= event1.V2;
	float V2_2	= event2.V2;


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
		
	return rawEventArray;
	
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
				//reject this case	 but I get better results when not rejected
				
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




int main(){
	
	
	//float timeStamp = 0;
	//int pulseHeight = 0;
	//float V1 = 0;
	//float V2 = 0;
	event_p eventBuffer[HR_EV_BUF_SIZE];
	event_r RespEventBuffer[2];
	
	RespEventBuffer[0].timeStamp = 0;
	RespEventBuffer[0].height = 0;
	
	//Timing variables
    double start, end;
    struct timeval timecheck;
  
	
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
	
	
	

	fp = fopen("debugEventProcess.txt", "w");
	//fp_r = fopen("Allevents.txt", "r");
	if(fp==NULL){
		//printf("%d\n",EOF);
		return -1;
	}
	fprintf(fp,"%s\n","File open success");
	
	//Measure time
	gettimeofday(&timecheck, NULL);
	start = (long)timecheck.tv_sec * 1000.0 + (long)timecheck.tv_usec / 1000.0;
	
	//initialize RespEventArray and PulseEventArray
	initializeRespEventArray();
	initializePulseEventArray();
	
    int eventArraySize = sizeof(myEventArray)/sizeof(myEventArray[0]);
	
	//Measure time
	//t1 = clock();
	//scan the first instance
	event1.timeStamp = myEventArray[0][0];
	event1.height = myEventArray[0][1];
	event1.V1 = myEventArray[0][2];
	event1.V2 = myEventArray[0][3];
	
	numOfEventsProcessed++;
	float DAQStartTimestamp = event1.timeStamp;
	fprintf(fp,"Starting timeStamp %f\n",DAQStartTimestamp);
	while(numOfEventsProcessed<eventArraySize){
		
			event2.timeStamp = myEventArray[numOfEventsProcessed][0];
			event2.height = myEventArray[numOfEventsProcessed][1];
			event2.V1 = myEventArray[numOfEventsProcessed][2];
			event2.V2 = myEventArray[numOfEventsProcessed][3];		
		
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
					printf("%f %f %d;\n",tmpRevent.timeStamp,RR,1);
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
						printf("%f %f %d;\n",tmpEvent.timeStamp,HR,0);
						
						
					}
						
				}
							
			}	
			event1 = mergedEventArray[1];
	};
	//Measure time
	//Measure time
	gettimeofday(&timecheck, NULL);
	end = (long)timecheck.tv_sec * 1000.0 + (long)timecheck.tv_usec / 1000.0;
	
	float DAQperiod = event2.timeStamp - DAQStartTimestamp;
	fprintf(fp,"DAQperiod %f s\n", DAQperiod);
	fprintf(fp,"End timeStamp %f\n",event2.timeStamp);
	fprintf(fp,"No of events %d\n",numOfEventsProcessed);
	fprintf(fp,"Elapsed time %lf ms\n", (end - start));
	fprintf(fp,"Time per event %lf ms\n", (end - start)/numOfEventsProcessed);
	//I will have to discard the last event
    fclose(fp);
	//fclose(fp_w_P);
	//fclose(fp_w_R);
	return 0;
}
