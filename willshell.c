#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>

#define DEBUG 0
#define LINE_SIZE 1024
#define ARGS_LEN 100

/* Helper functions */

void will_malloc(char ** str, int size){
	*str = (char *) malloc(sizeof(char*) * size);
	if(*str == NULL) {
		printf("malloc error\n");
		exit(1);
	}
}

void sig_handler(int sig){
	signal(sig, sig_handler);

	//some other shite
}

void execute(char ** args){

	pid_t pid;
	pid = fork();
	int status;

	if(pid<0){
		perror("fork problem");
		exit(EXIT_FAILURE);
	}

	if(pid==0){
		//child
		if (execvp(args[0], args) < 0) {
			perror("problem with exec.");
			exit(EXIT_FAILURE);
		}
	} else {
		//parent
		while (wait(&status) != pid);
	}
	
	//restore fd table
}

/* Main Function */

int main(){
	
	/* Initialize Variables */
	char * username = strdup(getenv("USER"));
	char * line;
	will_malloc(&line, LINE_SIZE);
	strcat(username, " -- ");
	int background = 0;

	/* Main loop */

	while(1){
		
		// init per loop variables
		char * args[ARGS_LEN];
		int args_count = 0;
		for(int i = 0; i < ARGS_LEN; i++){
			will_malloc(&args[i], 100);
		}

		// read a line
		line = readline(username);
		if(line!=NULL){
			add_history(line);
		} else {
			continue;
		}

		// parse line into args
		
		char * arg;
		// get first token
		if((arg = strtok(line," ")) == NULL){
			continue;
		}

		// do the rest 
		while( arg != NULL ){	
			args[args_count] = strdup(arg);
			arg = strtok(NULL," ");
			args_count++;	
		}	

		args[args_count] = NULL;
		
		// background
		if(strcmp(args[args_count-1], "&") == 0){
			background = 1;
		}

		if(DEBUG){
			for(int i = 0; i < args_count; i++){
				printf("%s\n", args[i]);
			}
		}

		// default commands 
		if(strcmp(args[0], "exit") == 0){
			//free all mem
			break;
		}
		if(strcmp(args[0], "myinfo") == 0){
			printf("parent PID: %i\nPID: %i\n", getppid(), getpid());
			continue;
		}
		if(strcmp(args[0], "cd") == 0){
			if(args_count > 1){
				chdir(args[1]);
			} else {
				chdir(getenv("HOME"));
			}
			continue;
		}	

		//fork and exec 
		if(args_count>0){
			execute(args);
		}

		//free mem
		for(int i = 0; i < ARGS_LEN; i++){
			free(args[i]);
		}
		free(arg);
	}

	//free mem

	return(0);
}

