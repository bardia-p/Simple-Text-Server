// Bardia Parmoun
// 101143006


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/msg.h>

#define TEXT_SIZE 100
#define SENTENCE_SIZE 35
#define WORD_SIZE 35
#define MAX_TEXT 512
#define MICRO_SEC_IN_SEC 1000000

static char text[TEXT_SIZE][SENTENCE_SIZE]; //keeps track of all the sentences in the server
static char responseText[BUFSIZ]; //used to save the response

static int size = 0; //keeps track of the size of the code

struct my_msg_st {
	long int my_msg_type; //keeps track of the type of the message
	char some_text[BUFSIZ]; //keeps track of the message
};

/* finds the sentence from a given command */
void findSentence(char* newCommand, char* sentence, int i){
	int j = 0;
	//removes the next line character
	while (newCommand[i]!='\0'){
		if (newCommand[i]!='\n'){
			sentence[j] = newCommand[i];
			j++;
		}
		i++;
	}
	sentence[j] = '\0';
}

/* finds the word from a given command */
void findWord(char* newCommand, char* word, int i){
	int j = 0;
	//gets rid of the white space after the word
	while (newCommand[i]!='\0' && newCommand[i]!='\n' && newCommand[i]!=' '){
		word[j] = newCommand[i];
		j++;
		i++;
	}
	word[j] = '\0';
}


/* adds the sentence to the text */
int append (char* newSentence){
	//adds the sentence to the end of the text
	//if it goes over the limit it replaces it
	if (strlen(newSentence)<=35){
		strcpy(text[(size++)%TEXT_SIZE], newSentence);
		return 1;
	}
	else{
		return 0;		
	}
}

/* displays all the sentences in the server */
void displayText(char* response, int isClient){
	if (size == 0){
		//if the server is empty
		strcpy(response, "Nothing to display\n");
	}
	else {
		//if the server is calling this it prints all the sentences in the text
		int text_length = size;

		// if the client is running this command it only prints the first 10 lines in the server to not exceed 
		// the maximum size of the message queue limit
		if (isClient && size > 10){
			text_length = 10;
		}

		//adds the period at the end of the sentence
		for (int i=0; i<text_length; i++){
			strcat(response, text[i]);
			strcat(response, ".\n");
		}
	}
}

/* finds the first occurance of the sentence in the text */
int removeSentence (char* sentence){
	int i = 0;	
	int found = 0;	
	for (; i<size; i++){
		//finding the sentence
		if (strcmp(text[i], sentence)==0){
			strcpy(text[i], "");
			found = 1;
			break;
		}
	}
	
	if (found){
		// move the other sentences back
		for (; i<size-1; i++){
			strcpy(text[i], text[i+1]);
		}
		strcpy(text[i],"");
		size--;
	}
	return found;
}


/* remove every occurance of a word for a given sentence */
int removeWordFromSentence (char* sentence, char* word, char* newSentence){
	char * newWord= strtok(sentence, " ");
   	int wordLength = strlen(word);
	int found = 0;

	strcpy(newSentence, "");

   	while(newWord != NULL ) {
		//if the word did not match add it to the new sentence
		if (!strcmp(newWord, word)==0){
			strcat(newSentence, newWord);
			strcat(newSentence, " ");
		}

		else{
			found = 1;
		}

     		newWord = strtok(NULL, " ");
   	}

	//removes the space at the end of the sentence
	if (newSentence[strlen(newSentence)-1]==' '){
		newSentence[strlen(newSentence)-1]='\0';
	}
	return found;
}

/* deletes every occurance of the word in the text */
int deleteWord (char* word){	
	int found =0;

	int sentencesToDelete=0;

	for (int i=0; i<size;i++){
		//goes through every sentence and deletes the words
		char newSentence[SENTENCE_SIZE];

		if (removeWordFromSentence(text[i], word, newSentence)){
			found = 1;
		}
	
		strcpy(text[i], newSentence);

		if (strcmp(newSentence, "")==0){
			sentencesToDelete++;
		}
	}

	for (int i =0; i<sentencesToDelete; i++){
		removeSentence("");
	}
	return found;
}

/* finds the first occurance of the word in the text */
int searchWord (char* word, char* foundSentence){
	char *  foundWord;

	for (int i=0; i<size; i++){
		foundWord = strstr(text[i], word);
		int index = (int) (foundWord - text[i]);

		if (foundWord && (index == 0 || text[i][index-1]==' ') && (index + strlen(word) - 1 == strlen(text[i])-1 || text[i][index+strlen(word)]==' ')){
			strcpy(foundSentence, text[i]);
			return 1;
		}
	}
	return 0;
}

int main()
{
	int running = 1;
	int msgid1; //message queue used to receive data from the client
	int msgid2; //message queue used to send data to the client

	struct my_msg_st send_data; 
	struct my_msg_st receive_data;

	long int msg_to_receive = 0;

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


	char sentence[SENTENCE_SIZE] = ""; //keeps track of the sentence in the command
	char word[WORD_SIZE] = ""; //keeps track of the word in the command
	char buffer[BUFSIZ]; //used to sent the receive message
	char timeVal[BUFSIZ];

	while(running) {	
		//initializes sentence and word
		strcpy(sentence,"");
		strcpy(word, "");
		strcpy(responseText, "");

		//receives a command from the user
		if (msgrcv(msgid1, (void *)&receive_data, BUFSIZ, msg_to_receive, 0) == -1) {
			fprintf(stderr, "msgrcv failed with error: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		
		//if the command is append
		if (strncmp(receive_data.some_text, "append", 6) == 0) {
 			findSentence(receive_data.some_text,sentence, 7); //extracts the sentence from the command
			if (append(sentence)){ //adds the sentence to the list of commands	
				strcpy(responseText,"You added the following sentence: ");
				strcat(responseText, sentence); 
				strcat(responseText, "\n");
			}
			else{
				strcpy(responseText,"Your sentence was too long\n");
			}
		}

	
		//if the command is delete
		else if (strncmp(receive_data.some_text, "delete", 6) == 0) {
			findWord(receive_data.some_text,word, 7); //extracts the word from the command
				
			int didDelete = deleteWord(word);
			
			if (didDelete){ //if the delete operation was successful
				strcpy(responseText, "You deleted all the instances of ");
				strcat(responseText, word); 
				strcat(responseText, "\n");
			} else {
				strcpy(responseText, "Did not find the word in the text :(\n");
			}
		}
	
		//if the command is search
		else if (strncmp(receive_data.some_text, "search", 6) == 0) {
			findWord(receive_data.some_text,word, 7); //extracts the word from the command
		
			int didFind = searchWord(word,sentence); 
			
			if (didFind){ //if search was successful
				strcpy(responseText, "Here is the first occurance of ");
				strcat(responseText, word);
				strcat(responseText, ":\n");
				strcat(responseText, sentence);
				strcat(responseText, "\n");
			} else {
				strcpy(responseText, "Did not find the word in the text :(\n");
			}	
		}

		//if the command is remove
                else if (strncmp(receive_data.some_text, "remove", 6) == 0) {
			findSentence(receive_data.some_text,sentence, 7); //extracts the sentence from the command
				
			int didRemove = removeSentence(sentence); 
		
			if (didRemove){ //if remove was successful
				strcpy(responseText, "You removed the following sentence: ");
				strcat(responseText, sentence);
				strcat(responseText, "\n");
			} else {
				strcpy(responseText, "Could not find the sentence :(\n");
			}
		}

		//if the command is end
                else if (strncmp(receive_data.some_text, "end", 3) == 0) {	
			//prints everything in the server
			displayText(responseText, 0);
			printf("The final version of the text in the server:\n%s\n",responseText);

			strcpy(responseText, "You have been disconnected\n");
				
			running = 0;	
		}
		
		/*	
		// the optional display command used for debugging
		else if (strncmp(receive_data.some_text, "display", 7) == 0){
			strcpy(responseText, "The text in the server as of now: \n");
			displayText(responseText, 1);
		}
		*/

		// if the commands is invalid
		else {	
			strcpy(responseText, "Invalid command\n");
		}

		strcpy(send_data.some_text, responseText);
		send_data.my_msg_type = 1;

		if (msgsnd(msgid2, (void *)&send_data, MAX_TEXT, 0) == -1) {
			fprintf(stderr, "msgsnd failed\n");
			exit(EXIT_FAILURE);
		}
	}

	//disconnecting both message queues
	if (msgctl(msgid1, IPC_RMID, 0) == -1) {
		fprintf(stderr, "msgctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}

	if (msgctl(msgid2, IPC_RMID, 0) == -1) {
		fprintf(stderr, "msgctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
} 
