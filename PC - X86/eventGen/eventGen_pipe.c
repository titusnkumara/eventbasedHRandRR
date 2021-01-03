#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>


#define SAMPLERATE 200

//#define SHOW_ARRAYS 1

struct event{
	float 	timeStamp;
	int 	height;
	float 	V1;
	float 	V2;	
};

typedef struct event event_p;
typedef struct event event_r;


static event_p observedEvent;
int dispatchEnabled = 0;


//FILE *fp = NULL;
FILE *fp_w = NULL;
FILE *fp_header = NULL;



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
	
	printf("%f %d %f %f\n",observedEvent.timeStamp,observedEvent.height,observedEvent.V1,observedEvent.V2);
	
	fprintf(fp_w, "{%f,%d,%f,%f},\n", observedEvent.timeStamp,observedEvent.height,observedEvent.V1,observedEvent.V2);
	fprintf(fp_header,"{%f,%d,%f,%f},\n", observedEvent.timeStamp,observedEvent.height,observedEvent.V1,observedEvent.V2);
	
	observedEvent.timeStamp = 0;;
	observedEvent.height = 0;
	observedEvent.V1 = 0;
	observedEvent.V2 = 0;
}




int main(){

	//Timekeeping variables	
	float rawValue = 0;
	int dataCounter = 0;
	int delayValue = 10;
	float rawDataBuffer[20] = {0};
	
	
	observedEvent.timeStamp = 0;
	observedEvent.height = 0;
	observedEvent.V1 = 0;
	observedEvent.V2 = 0;

	//fp = fopen("rawValues.txt", "r");
	fp_w = fopen("debugEventgen.log", "w");
	fp_header = fopen("eventArray.h", "w");
	
	if(fp_w==NULL || fp_header==NULL){
		printf("%d\n", EOF);
		return -1;
	}
	fprintf(fp_w,"%s\n","File open success");
	
	fprintf(fp_header,"%s","const float myEventArray[][4] = {");
	
	fprintf(fp_w,"%s","{");
	
	while(dataCounter<10){
		if(scanf("%f", &rawValue)!= -1){
			rawDataBuffer[dataCounter] = rawValue;
			dataCounter ++;
		}
	}
	
	while(scanf("%f", &rawValue)!= -1){
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
		
	};
	fprintf(fp_w,"%s","};");
	fprintf(fp_header,"};");
    //fclose(fp);
	fclose(fp_w);
	fclose(fp_header);
	return 0;
}
