/*
 * main.c
 *
 *  Created on: Jan 15, 2014
 *      Author: Sean
 */
#include "all.h"

tree *msg_tree;
sem_t semaphore, can_semaphore, mutex, main_sem;
FILE *f;
char logString[150];
unsigned int errors = 0;
extern int keepRunning;
time_t startTime;

void my_handler(int dummy) {
    keepRunning = 0;
}

/*
 * Thread dedicated to sending out the CANcorder heartbeat
 */
void *txcanthread(int cansock) {

	struct can_frame txmsg;
	while(keepRunning)
	{
		sleep(1);
		txmsg.can_id = 0xAA; 		// CANcorder heartbeat address
		txmsg.can_dlc = 0; 	// No data associated with the heartbeat

		if(write(cansock, &txmsg, sizeof(struct can_frame)) < 0)
		{
			printf("Error writing heartbeat\n");
		}
	}
	return NULL;
}


int main()
{
	startTime = time(0);
	int s;

	msg_tree = initialize_msg_avl();		// Initialize trees that will store parsed data from .dbc file

	char *fileName = "RW3.dbc";			// Your .dbc file
	parseFile(fileName);	// Parse the file

	if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("Error while opening socket");
		return -1;
	}
	sem_init(&mutex, 0, 1);
	sem_init(&can_semaphore, 0, 1);
	sem_init(&semaphore, 0, 0);
	sem_init(&main_sem, 0, 0);

	pthread_t interceptor, translator;
	pthread_t txthread;
	pthread_create(&interceptor, NULL, can_interceptor_thread, s);
	pthread_create(&translator, NULL, translate_thread, NULL);
	pthread_create(&txthread, NULL, txcanthread, s);
	sem_wait(&main_sem);

	return 0;
}
