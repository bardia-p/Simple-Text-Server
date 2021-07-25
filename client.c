#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/time.h>

#define MAX_TEXT 512

struct my_msg_st {
	long int my_msg_type; //keeps track of the type of the message
	char some_text[MAX_TEXT];
};

int main()
{
	int running = 1;
	struct my_msg_st send_data; //used to send data to the server
	struct my_msg_st receive_data; //used to receive data from the server
	int msgid1;
	int msgid2;
	long int msg_to_receive = 0;

	char buffer[BUFSIZ];

	msgid1 = msgget((key_t)7777, 0666 | IPC_CREAT);

	if (msgid1 == -1) {
		fprintf(stderr, "msgget failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	msgid2 = msgget((key_t)8888, 0666 | IPC_CREAT);

	if (msgid2 == -1) {
		fprintf(stderr, "msgget failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	struct timeval start, end;

	double timeVals[7] = {0,0,0,0,0,0,0};
	double numCommands[7] = {0,0,0,0,0,0,0};

	int commandIndex = -1;
	double time = 0;

	while(running) {
		printf("Enter a new command: ");
		fgets(buffer, BUFSIZ, stdin);
		send_data.my_msg_type = 1;
		commandIndex = -1;

		strcpy(send_data.some_text, buffer);
	
		//sends the command to the server
		if (msgsnd(msgid1, (void *)&send_data, MAX_TEXT, 0) == -1) {
			fprintf(stderr, "msgsnd failed\n");
			exit(EXIT_FAILURE);
		}


		gettimeofday(&start, NULL);

		//receives the response of the server
		if (msgrcv(msgid2, (void *)&receive_data, BUFSIZ, msg_to_receive, 0) == -1) {
			fprintf(stderr, "msgrcv failed with error: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		
		gettimeofday(&end, NULL);
		
		if (strncmp(buffer, "append", 6) == 0) {
 			commandIndex = 0;
		}
		
		//if the command is delete
		else if (strncmp(buffer, "delete", 6) == 0) {
			commandIndex = 1;
		}
	
		//if the command is search
		else if (strncmp(buffer, "search", 6) == 0) {
			commandIndex = 2;		
		}

		//if the command is remove
                else if (strncmp(buffer, "remove", 6) == 0) {
 			commandIndex = 3;	
		}

		//if the command is end
                else if (strncmp(buffer, "end", 3) == 0) {
			commandIndex = 4;
			running = 0;		
		}

		// if the commands is invalid
		else {
			commandIndex = 5;
		}
		
		//prints the command that received from the server
		printf("%s", receive_data.some_text);


		time =  (end.tv_sec-start.tv_sec)*1000000 + end.tv_usec - start.tv_usec;
		
		//adds the total time of the command
		timeVals[commandIndex]+=time;
		numCommands[commandIndex]++;
		
		//adds the total time of all the commands
		timeVals[6]+=time;
		numCommands[6]++;

		printf("The average time that it took to run the command: %.2f\n\n", timeVals[commandIndex]/numCommands[commandIndex]);		
	
		if (commandIndex == 4){
			printf("The average time to run all of the commands in the server: %.2f\n", timeVals[6]/numCommands[6]);
		}
	}

	exit(EXIT_SUCCESS);
}
