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
	pid_t wpid;
	int status;
	int stdin_cpy = dup(STDIN_FILENO);
	int stdout_cpy = dup(STDOUT_FILENO);
	int newout = -1;
	int newin = -1;

	//go through and do rediretions
	int i = 0;
	while(args[i]!=NULL){
		if(strcmp(args[i], "<") == 0){
			newin = open(args[i+1], O_RDONLY);
			if(newin < 0){
				perror("Opening File");
				exit(EXIT_FAILURE);
			}
			int ret = dup2(newin, STDIN_FILENO);
			if(ret<0){
				perror("dup2 failed");
				exit(EXIT_FAILURE);
			}
			args[i] = NULL;
		}
		else if(strcmp(args[i], ">") == 0){
			newout = open(args[i+1], O_WRONLY|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR);
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
		i++;
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
		if(background){
			printf("%d\n", pid);
			waitpid(-1, &status, WNOHANG);
		} else {
			wpid = wait(&status);
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

	/* Main loop */

	while(1){

		//check and see if any children need to be reaped


		// init per loop variables
		int background = 0;
		char * args[ARGS_LEN];
		int args_count = 0;
		for(int i = 0; i<ARGS_LEN; i++){
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
		} else {
			background = 0;
		}

		if(DEBUG){
			for(int i = 0; i < args_count; i++){
				printf("%s\n", args[i]);
			}
		}

		// default commands 
		if(strcmp(args[0], "exit") == 0){
			//free all mem
			for(int i = 0; i < ARGS_LEN; i++){
				free(args[i]);
			}
			free(arg);
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

		//free line, username, args, arg
		for(int i = 0; i < ARGS_LEN; i++){
			free(args[i]);
		}
		free(arg);
	}

	//free mem
	free(username);
	free(line);

	return(0);
}

