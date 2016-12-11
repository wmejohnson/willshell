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

void execute(char ** args, int background){
	int b = background;
	int status;
	int stdin_cpy = dup(STDIN_FILENO);
	int stdout_cpy = dup(STDOUT_FILENO);
	int newout;
	int newin;

	//go through and do rediretions
	for(int i = 0; args[i]!=NULL; i++){
		if(strcmp(args[i], "<") == 0){
			printf("new in\n");
			newin = open(args[i+1], O_RDONLY);
			if(newin < 0){
				perror("Opening File");
				exit(EXIT_FAILURE);
			}
			printf("opened\n");
			int ret = dup2(newin, STDIN_FILENO);
			if(ret<0){
				perror("dup2 failed");
				exit(EXIT_FAILURE);
			}
			args[i] = NULL;
		}
		if(strcmp(args[i], ">") == 0){
			newout = open(args[i+1], O_WRONLY|O_APPEND|O_CREAT);
			if(newout < 0){
				perror("Opening File");
				exit(EXIT_FAILURE);
			}
			int ret = dup2(newout, STDOUT_FILENO);
			if(ret<0){
				perror("dup2 failed");
				exit(EXIT_FAILURE);
			}
			args[i] = NULL;

		}
	}

	pid_t pid;
	pid = fork();
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
		if(!b){
			while (wait(&status) != pid);
		}
	}
	
	//restore fd table
	int ret = dup2(stdin_cpy, STDIN_FILENO);
	if(ret<0){
		perror("Error restoring file table");
		exit(EXIT_FAILURE);
	}
	ret = dup2(stdout_cpy, STDOUT_FILENO);
	if(ret<0){
		perror("Error restoring file table");
		exit(EXIT_FAILURE);
	}
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
			args[args_count-1] = NULL;
		}

		if(DEBUG){
			for(int i = 0; i < args_count; i++){
				printf("%s\n", args[i]);
			}
		}

		// default commands 
		if(strcmp(args[0], "exit") == 0){
			//free all mem
			printf("exiting\n");
			break;
		}
		if(strcmp(args[0], "myinfo") == 0){
			printf("parent PID: %i\nPID: %i\n", getppid(), getpid());
			continue;
		}
		if(strcmp(args[0], "cd") == 0){
			if(args_count > 1){
				if(chdir(args[1]) == -1){
					perror("this directory doesn't exist");
				}
			} else {
				if(chdir(getenv("HOME")) == -1){
					perror("this directory doesn't exist");
				}
			}
			continue;
		}	

		//fork and exec 
		if(args_count>0){
			execute(args, background);
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

