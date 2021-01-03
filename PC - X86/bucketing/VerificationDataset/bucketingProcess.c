#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "eventArray.h"

#define SAMPLERATE 			200
#define RESP_CUT_OFF 		50
#define RR_MAX_PERIOD		1
#define HR_MAX_PERIOD		0.33
#define EVENTARRAYSIZE  	30
#define REFERENCE_ARR_SIZE 	14

#define METHOD_NOTFOUND		0
#define METHOD_MAJORITY		1
#define METHOD_DOUBLE		2
#define METHOD_TRIPLE		3

#define MEAN_BUFF_SIZE		5

#define MISSING_BEAT_THRESH 3

//#define SHOW_ARRAYS 1

struct event{
	float 	timeStamp;
	int 	height;
	float 	V1;
	float 	V2;	
};

typedef struct event event_p;
typedef struct event event_r;

struct scanMethod{
	int index;
	int method;
};

typedef struct scanMethod scan_Method;


FILE *fp = NULL;
FILE *fp_r = NULL;
FILE *fp_w_R = NULL;
FILE *fp_w_P = NULL;

event_p RespEventArray[2];
event_p PulseEventArray[2];

const float referenceArray[REFERENCE_ARR_SIZE] = {0.4,0.45,0.5,0.6,0.7,0.8,0.9,1,1.2,1.6,2,2.4,2.8,3.2};

int counterArray[REFERENCE_ARR_SIZE] = {0};

float meanArray[REFERENCE_ARR_SIZE] = {0};
float eventArray[EVENTARRAYSIZE] = {0};

float meanBuffer[MEAN_BUFF_SIZE] = {0};

event_p timeDiffEvents[2] = {{0}};

float HR_Last_valid = 72.0;

int processedHReventCount = 0;

int lastValueIndex = 0;




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



float processRReventStream(event_r event[]){
	
	float RR = 60./(event[1].timeStamp - event[0].timeStamp);
	event[0] = event[1];
	return RR;
}



/*
Getting the pulse rate
*/


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

float getHR_from_singleMajority(int currentIndex, int counterArray[], float meanArray[]){
	
	int i = currentIndex;
	
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
	float HR = 60/HR_mean_time;
	

	//try to compensate for this
	int forwardCount = i;
	for(forwardCount=i;forwardCount<REFERENCE_ARR_SIZE;forwardCount++){
		//check if any count is larger than 3
		if(counterArray[forwardCount]>MISSING_BEAT_THRESH){
			//check this event for missing beats
			if(meanArray[forwardCount]!=0){
				float differencHR_ = 120/meanArray[forwardCount] - HR;
				if(differencHR_*differencHR_ < 225){
					//15 beats difference
					float HR_compensated_mean = (counterSum*HR_mean_time + meanArray[forwardCount]*counterArray[forwardCount])/(counterSum+2*counterArray[forwardCount]);
					counterSum = counterSum+ 2*counterArray[forwardCount];
					HR_mean_time = HR_compensated_mean;
					HR = 60/HR_mean_time;
				}
				differencHR_ = 180/meanArray[forwardCount] - HR;
				if(differencHR_*differencHR_ < 225){
					//15 beats difference
					float HR_compensated_mean = (counterSum*HR_mean_time + meanArray[forwardCount]*counterArray[forwardCount])/(counterSum+3*counterArray[forwardCount]);
					counterSum = counterSum+ 3*counterArray[forwardCount];
					HR_mean_time = HR_compensated_mean;
					HR = 60/HR_mean_time;
				}
				
				
			}
		}
	}	
	
	return HR;
	
}

float getHR_from_doubleMajority(int currentIndex, int counterArray[], float meanArray[]){
	
	int i = currentIndex;
	
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
	float HR = 60/HR_mean_time;

	//try to compensate for this
	int forwardCount = i;
	for(forwardCount=i;forwardCount<REFERENCE_ARR_SIZE;forwardCount++){
		//check if any count is larger than 3
		if(counterArray[forwardCount]>MISSING_BEAT_THRESH){
			//check this event for missing beats
			if(meanArray[forwardCount]!=0){
				float differencHR_ = 120/meanArray[forwardCount] - HR;
				if(differencHR_*differencHR_ < 225){
					//15 beats difference
					float HR_compensated_mean = (counterSum*HR_mean_time + meanArray[forwardCount]*counterArray[forwardCount])/(counterSum+2*counterArray[forwardCount]);
					counterSum = counterSum+ 2*counterArray[forwardCount];
					HR_mean_time = HR_compensated_mean;
					HR = 60/HR_mean_time;
				}
				differencHR_ = 180/meanArray[forwardCount] - HR;
				if(differencHR_*differencHR_ < 225){
					//15 beats difference
					float HR_compensated_mean = (counterSum*HR_mean_time + meanArray[forwardCount]*counterArray[forwardCount])/(counterSum+3*counterArray[forwardCount]);
					counterSum = counterSum+ 3*counterArray[forwardCount];
					HR_mean_time = HR_compensated_mean;
					HR = 60/HR_mean_time;
				}
				
				
			}
		}
	}	
	
	
	return HR;	
}

float getHR_from_tripleMajority(int currentIndex, int counterArray[], float meanArray[]){

	int i = currentIndex;
	int counterSum = 0;
	float meanSum = 0;
	//get adjecent sum
	counterSum = counterArray[i-2]+counterArray[i-1]+counterArray[i];
	meanSum = counterArray[i-2]*meanArray[i-2]+counterArray[i-1]*meanArray[i-1]+counterArray[i]*meanArray[i];
	
	float HR_mean_time = meanSum/counterSum;
	float HR = 60/HR_mean_time;
	
	
	//try to compensate for this
	int forwardCount = i;
	for(forwardCount=i;forwardCount<REFERENCE_ARR_SIZE;forwardCount++){
		//check if any count is larger than 3
		if(counterArray[forwardCount]>MISSING_BEAT_THRESH){
			//check this event for missing beats
			if(meanArray[forwardCount]!=0){
				float differencHR_ = 120/meanArray[forwardCount] - HR;
				if(differencHR_*differencHR_ < 225){
					//15 beats difference
					float HR_compensated_mean = (counterSum*HR_mean_time + meanArray[forwardCount]*counterArray[forwardCount])/(counterSum+2*counterArray[forwardCount]);
					counterSum = counterSum+ 2*counterArray[forwardCount];
					HR_mean_time = HR_compensated_mean;
					HR = 60/HR_mean_time;
				}
				differencHR_ = 180/meanArray[forwardCount] - HR;
				if(differencHR_*differencHR_ < 225){
					//15 beats difference
					float HR_compensated_mean = (counterSum*HR_mean_time + meanArray[forwardCount]*counterArray[forwardCount])/(counterSum+3*counterArray[forwardCount]);
					counterSum = counterSum+ 3*counterArray[forwardCount];
					HR_mean_time = HR_compensated_mean;
					HR = 60/HR_mean_time;
				}
				
				
			}
		}
	}	
	return HR;
}

void removeSingleEventFromEventArray(float eventArray[], int counterArray[], float meanArray[]){
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
	
}

void addSingleEventToEventArray(float timeDifference, int bucketIndex, float eventArray[], int counterArray[], float meanArray[]){
	
	//bucket index is already found
	float currentAddTotal = counterArray[bucketIndex] * meanArray[bucketIndex];
	//increase the counterArray
	counterArray[bucketIndex]++;
	//change mean
	meanArray[bucketIndex] = (currentAddTotal+timeDifference)/(counterArray[bucketIndex]);
	//add new element to event array
	eventArray[lastValueIndex] = timeDifference;
	lastValueIndex = (lastValueIndex+1)%EVENTARRAYSIZE;	
}


scan_Method getScanMethodAndIndex(int counterArray[]){
	int i=0;
	int val=0;
	scan_Method methodVal1,methodVal2,methodVal3;
	methodVal1.method = METHOD_NOTFOUND;
	methodVal1.index = 0;
	methodVal2.method = METHOD_NOTFOUND;
	methodVal2.index = 0;
	methodVal3.method = METHOD_NOTFOUND;
	methodVal3.index = 0;
	
	
	int singleLargestVal = 0;
	int doubleLargestVal = 0;
	int tripleLargestVal = 0;
	
	//try option 1
	for(i=0;i<REFERENCE_ARR_SIZE;i++){
		val = counterArray[i];
		//printf("sumOfOne %d\n",val);
		if(val>=20 && val>singleLargestVal){
			//printf("Majority found here\n");
			methodVal1.method = METHOD_MAJORITY;
			methodVal1.index = i;
			//This has absolute priority
			return 	methodVal1;
			//singleLargestVal = val;
			//break;
		}
	}
	//Otherwise 
	//try option 2
	for(i=1;i<REFERENCE_ARR_SIZE;i++){
		val = counterArray[i-1]+counterArray[i];
		//printf("sumOfTwo %d\n",val);
		if(val>=(EVENTARRAYSIZE/2) && val>doubleLargestVal){
			//printf("Majority found here\n");
			methodVal2.method = METHOD_DOUBLE;
			methodVal2.index = i;
			doubleLargestVal = val;
			break;
			//return 	methodVal;
		}
		
	}
	//try option 3	
	
	for(i=2;i<REFERENCE_ARR_SIZE;i++){
		val = counterArray[i-2]+counterArray[i-1]+counterArray[i];
		if(val>=(EVENTARRAYSIZE/2) && val>tripleLargestVal){
			methodVal3.method = METHOD_TRIPLE;
			methodVal3.index = i;
			tripleLargestVal = val;
			break;
			//return 	methodVal;
		}
	}
	
	//printf("Sum 1: %d 2: %d 3:%d\n",singleLargestVal,doubleLargestVal,tripleLargestVal);
	
	//check the best method
	//int MaxValue = getMax(singleLargestVal,doubleLargestVal, tripleLargestVal);

	if(doubleLargestVal==0 && tripleLargestVal==0){
		//no method found
		methodVal1.method = METHOD_NOTFOUND;
		methodVal1.index = 0;
		return 	methodVal1;	
	}else if(doubleLargestVal>=tripleLargestVal){
		return 	methodVal2;	
	}else{
		return methodVal3;		
	}
	
	
	//return the default case
	//return methodVal;
}


float getHRfromEvent(event_p event){
	
	timeDiffEvents[1] = event;
	
	float timeDifference = timeDiffEvents[1].timeStamp - timeDiffEvents[0].timeStamp;
	
	//printf("%f %f\n",timeDiffEvents[1].timeStamp,timeDifference);
	
	int bucketIndex = getBucketIndex(timeDifference,referenceArray);
	if(bucketIndex == -1){
		//discard this event
		//printf("%s %f\n","event discarded", timeDifference);
		timeDiffEvents[0] = timeDiffEvents[1];
		return HR_Last_valid;
	}
	//else proceed
	//remove last
	//don't remove anything until count is large enough
	//start removing elements after n elements
	if(processedHReventCount>EVENTARRAYSIZE){
		removeSingleEventFromEventArray(eventArray,counterArray,meanArray);
	}
	
	//Now add the new one
	addSingleEventToEventArray(timeDifference,bucketIndex,eventArray,counterArray,meanArray);
	
	
	//try getting the correct method and index
	scan_Method methodVal = getScanMethodAndIndex(counterArray);
	//printf("Method %d index %d\n",methodVal.method,methodVal.index);
	
	switch(methodVal.method){
		case METHOD_MAJORITY:{
			HR_Last_valid = getHR_from_singleMajority(methodVal.index, counterArray, meanArray);
			//printf("new: %f\n",myHR);
			break;
		}
		case METHOD_DOUBLE:{
			HR_Last_valid = getHR_from_doubleMajority(methodVal.index, counterArray, meanArray);
			//printf("new: %f\n",myHR);
			break;
		}
		case METHOD_TRIPLE:{
			HR_Last_valid = getHR_from_tripleMajority(methodVal.index, counterArray, meanArray);
			//printf("new: %f\n",myHR);
			break;
		}
		default:{
			break;
		}
		
	}
	//printf("%f %f;\n",event.timeStamp,HR_Last_valid);
	timeDiffEvents[0] = timeDiffEvents[1];
	processedHReventCount++;
	return HR_Last_valid;
}



float getMean(float newHR, float meanBuffer[]){
	int i=1;
	float sum = newHR;
	for(i=1;i<MEAN_BUFF_SIZE;i++){
		meanBuffer[i-1] = meanBuffer[i];
		sum = sum+ meanBuffer[i-1];
	}
	meanBuffer[MEAN_BUFF_SIZE-1] = newHR;
	return sum/MEAN_BUFF_SIZE;
}















int main(){
	
	
	
	//Timing variables
    double start, end;
    struct timeval timecheck;
  
	
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
	
	fp = fopen("debugEventProcess.txt", "w");
	//fp_r = fopen("Allevents.txt", "r");
	if(fp==NULL){
		//printf("%d\n",EOF);
		return -1;
	}
	fprintf(fp,"%s\n","File open success");

	//initialize RespEventArray and PulseEventArray
	initializeRespEventArray();
	initializePulseEventArray();
	

	//Measure time
	gettimeofday(&timecheck, NULL);
	start = (long)timecheck.tv_sec * 1000.0 + (long)timecheck.tv_usec / 1000.0;
	
	int eventArraySize = sizeof(myEventArray)/sizeof(myEventArray[0]);
	//scan the first instance
	//scanf("%f %d %f %f;", &event1.timeStamp,&event1.height,&event1.V1,&event1.V2);
	event1.timeStamp = myEventArray[0][0];
	event1.height = myEventArray[0][1];
	event1.V1 = myEventArray[0][2];
	event1.V2 = myEventArray[0][3];
	
	numOfEventsProcessed++;
	float DAQStartTimestamp = event1.timeStamp;
	fprintf(fp,"Starting timeStamp %f\n",DAQStartTimestamp);
	while(numOfEventsProcessed<eventArraySize){
			//printf("%f %d %f %f\n", event2.timeStamp,event2.height,event2.V1,event2.V2);
			//performance variable
			event2.timeStamp = myEventArray[numOfEventsProcessed][0];
			event2.height = myEventArray[numOfEventsProcessed][1];
			event2.V1 = myEventArray[numOfEventsProcessed][2];
			event2.V2 = myEventArray[numOfEventsProcessed][3];
			
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
					//float meanHR = getMean(HR, meanBuffer);
					printf("%f %f %d;\n",tmpEvent.timeStamp,HR,0);
					//printf("%f %d %f %f;\n",tmpEvent.timeStamp,tmpEvent.height, tmpEvent.V1, tmpEvent.V2);
						
					
						
				}
							
			}	
			event1 = mergedEventArray[1];
	};
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
	//fclose(fp_r);
	//fclose(fp_read);
	//fclose(fp_w_P);
	//fclose(fp_w_R);
 
 
	return 0;
}
