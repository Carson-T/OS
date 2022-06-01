#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>

#define BUF_SZ 256
#define TRUE 1
#define FALSE 0

const char* COMMAND_EXIT = "exit";
const char* COMMAND_HELP = "help";
const char* COMMAND_CD = "cd";
const char* COMMAND_IN = "<";
const char* COMMAND_OUT = ">";
const char* COMMAND_PIPE = "|";

 
enum {
	RESULT_NORMAL,
	ERROR_FORK,
	ERROR_COMMAND,
	ERROR_WRONG_PARAMETER,
	ERROR_MISS_PARAMETER,
	ERROR_TOO_MANY_PARAMETER,
	ERROR_CD,
	ERROR_SYSTEM,
	ERROR_EXIT,


	ERROR_MANY_IN,
	ERROR_MANY_OUT,
	ERROR_FILE_NOT_EXIST,
	

	ERROR_PIPE,
	ERROR_PIPE_MISS_PARAMETER
};

char username[BUF_SZ];
char hostname[BUF_SZ];
char curPath[BUF_SZ];
char commands[BUF_SZ][BUF_SZ];

int isCommandExist(const char* command);
void getUsername();
void getHostname();
int getCurWorkDir();
int splitCommands(char command[BUF_SZ]);
int callExit();
int callCommand(int commandNum);
int callCommandWithPipe(int low, int high);
int callCommandWithRedi(int low, int high);
int callCd(int commandNum);

int main() {

	int result = getCurWorkDir();
	if (ERROR_SYSTEM == result) {
		fprintf(stderr, "\e[31;1mError: System error while getting current work directory.\n\e[0m");
		exit(ERROR_SYSTEM);
	}
	getUsername();
	getHostname();


	char com[BUF_SZ];
	while (TRUE) {
		printf("\e[32;1m%s@%s:%s\e[0m$ ", username, hostname,curPath);  

		fgets(com, BUF_SZ, stdin);
		int len = strlen(com);
		if (len != BUF_SZ) {
			com[len-1] = '\0';
		}

		int commandNum = splitCommands(com);
		
		if (commandNum != 0) {  
			if (strcmp(commands[0], COMMAND_EXIT) == 0) {  
				result = callExit();
				if (ERROR_EXIT == result) {
					exit(-1);
				}
			} else if (strcmp(commands[0], COMMAND_CD) == 0) {  
				result = callCd(commandNum);
				switch (result) {
					case ERROR_MISS_PARAMETER:
						fprintf(stderr, "\e[31;1mError: Miss parameter while using command \"%s\".\n\e[0m"
							, COMMAND_CD);
						break;
					case ERROR_WRONG_PARAMETER:
						fprintf(stderr, "\e[31;1mError: No such path \"%s\".\n\e[0m", commands[1]);
						break;
					case ERROR_TOO_MANY_PARAMETER:
						fprintf(stderr, "\e[31;1mError: Too many parameters while using command \"%s\".\n\e[0m"
							, COMMAND_CD);
						break;
					case RESULT_NORMAL:
						result = getCurWorkDir();
						if (ERROR_SYSTEM == result) {
							fprintf(stderr
								, "\e[31;1mError: System error while getting current work directory.\n\e[0m");
							exit(ERROR_SYSTEM);
						} else {
							break;
						}
				}
			} else {
				result = callCommand(commandNum);
				switch (result) {
					case ERROR_FORK:
						fprintf(stderr, "\e[31;1mError: Fork error.\n\e[0m");
						exit(ERROR_FORK);
					case ERROR_COMMAND:
						fprintf(stderr, "\e[31;1mError: Command not exist in myshell.\n\e[0m");
						break;
					case ERROR_MANY_IN:
						fprintf(stderr, "\e[31;1mError: Too many redirection symbol \"%s\".\n\e[0m", COMMAND_IN);
						break;
					case ERROR_MANY_OUT:
						fprintf(stderr, "\e[31;1mError: Too many redirection symbol \"%s\".\n\e[0m", COMMAND_OUT);
						break;
					case ERROR_FILE_NOT_EXIST:
						fprintf(stderr, "\e[31;1mError: Input redirection file not exist.\n\e[0m");
						break;
					case ERROR_MISS_PARAMETER:
						fprintf(stderr, "\e[31;1mError: Miss redirect file parameters.\n\e[0m");
						break;
					case ERROR_PIPE:
						fprintf(stderr, "\e[31;1mError: Open pipe error.\n\e[0m");
						break;
					case ERROR_PIPE_MISS_PARAMETER:
						fprintf(stderr, "\e[31;1mError: Miss pipe parameters.\n\e[0m");
						break;
				}
			}
		}
	}
}

// int isCommandExist(const char* command) {  
// 	if (command == NULL || strlen(command) == 0) return FALSE;

// 	int result = TRUE;
	
// 	int fds[2];
// 	if (pipe(fds) == -1) {
// 		result = FALSE;
// 	} else {

// 		int inFd = dup(STDIN_FILENO);
// 		int outFd = dup(STDOUT_FILENO);

// 		pid_t pid = vfork();
// 		if (pid == -1) {
// 			result = FALSE;
// 		} else if (pid == 0) {

// 			close(fds[0]);
// 			dup2(fds[1], STDOUT_FILENO);
// 			close(fds[1]);

// 			char tmp[BUF_SZ];
// 			sprintf(tmp, "command -v %s", command);
// 			system(tmp);
// 			exit(1);
// 		} else {
// 			waitpid(pid, NULL, 0);

// 			close(fds[1]);
// 			dup2(fds[0], STDIN_FILENO);
// 			close(fds[0]);

// 			if (getchar() == EOF) {  
// 				result = FALSE;
// 			}
			

// 			dup2(inFd, STDIN_FILENO);
// 			dup2(outFd, STDOUT_FILENO);
// 		}
// 	}

// 	return result;
// }

void getUsername() {  
	struct passwd* pwd = getpwuid(getuid());
	strcpy(username, pwd->pw_name);
}

void getHostname() {  
	gethostname(hostname, BUF_SZ);
}

int getCurWorkDir() {  
	char* result = getcwd(curPath, BUF_SZ);
	if (result == NULL)
		return ERROR_SYSTEM;
	else return RESULT_NORMAL;
}

int splitCommands(char command[BUF_SZ]) {  
	int num = 0;
	int i, j;
	int len = strlen(command);

	for (i=0, j=0; i<len; ++i) {
		if (command[i] != ' ') {
			commands[num][j++] = command[i];
		} else {
			if (j != 0) {
				commands[num][j] = '\0';
				++num;
				j = 0;
			}
		}
	}
	if (j != 0) {
		commands[num][j] = '\0';
		++num;
	}

	return num;
}

int callExit() {  
	pid_t pid = getpid();
	if (kill(pid, SIGTERM) == -1) 
		return ERROR_EXIT;
	else return RESULT_NORMAL;
}

int callCommand(int commandNum) {  
	pid_t pid = fork();
	if (pid == -1) {
		return ERROR_FORK;
	} else if (pid == 0) {

		int inFds = dup(STDIN_FILENO);
		int outFds = dup(STDOUT_FILENO);

		int result = callCommandWithPipe(0, commandNum);
		

		dup2(inFds, STDIN_FILENO);
		dup2(outFds, STDOUT_FILENO);
		exit(result);
	} else {
		int status;
		waitpid(pid, &status, 0);
		return WEXITSTATUS(status);
	}
}

int callCommandWithPipe(int low, int high) {  
	// if (low >= high) return RESULT_NORMAL;

	int pipeIdx = -1;
	for (int i=low; i<high; ++i) {
		if (strcmp(commands[i], COMMAND_PIPE) == 0) {
			pipeIdx = i;
			break;
		}
	}

	if (pipeIdx == -1) {  
		return callCommandWithRedi(low, high);
	} else if (pipeIdx+1 == high) {  
		return ERROR_PIPE_MISS_PARAMETER;
	} else if (pipeIdx+1 < high){

        int fds[2];
        if (pipe(fds) == -1) {
            return ERROR_PIPE;
        }
        // int result = RESULT_NORMAL;
        pid_t pid =  fork();
        if (pid == -1) {
            return ERROR_FORK;
        } else if (pid == 0) {  
            close(fds[0]);
            dup2(fds[1], STDOUT_FILENO);  
            close(fds[1]);
            
            exit(callCommandWithRedi(low, pipeIdx));
        } else {  
            int status;
            waitpid(pid, &status, 0);
            int exitCode = WEXITSTATUS(status);
            
            if (exitCode != RESULT_NORMAL) {  
                char line[BUF_SZ];
                close(fds[1]);
                dup2(fds[0], STDIN_FILENO);  
                close(fds[0]);
                while(fgets(line, BUF_SZ, stdin) != NULL) {  
                    fprintf(stderr,"%s", line);  
                }
                
                return exitCode;
            } else if (pipeIdx+1 < high){
                close(fds[1]);
                dup2(fds[0], STDIN_FILENO);  
                close(fds[0]);
                return callCommandWithPipe(pipeIdx+1, high);  
            }
        }
	}

}

int callCommandWithRedi(int low, int high) {  
	// if (!isCommandExist(commands[low])) {  
	// 	return ERROR_COMMAND;
	// }	

	int inNum = 0, outNum = 0;
	char *inFile = NULL, *outFile = NULL;
	int endIdx = high;  

	for (int i=low; i<high; ++i) {
		if (strcmp(commands[i], COMMAND_IN) == 0) {  
			++inNum;
			if (i < high-1)
				inFile = commands[i+1];
			else return ERROR_MISS_PARAMETER;  

			if (endIdx == high) endIdx = i;
		} else if (strcmp(commands[i], COMMAND_OUT) == 0) {  
			++outNum;
			if (i < high-1)
				outFile = commands[i+1];
			else return ERROR_MISS_PARAMETER;  
				
			if (endIdx == high) endIdx = i;
		}
	}

	if (inNum > 1) {  
		return ERROR_MANY_IN;
	} else if (outNum > 1) {  
		return ERROR_MANY_OUT;
	}

	// if (inNum == 1) {
	// }
	// int result = RESULT_NORMAL;
	pid_t pid = fork();
	if (pid == -1) {
		return ERROR_FORK;
	} else if (pid == 0) {

		if (inNum == 1){
            FILE* fp = fopen(inFile, "r");
		    if (fp == NULL)  
                fclose(fp);
			    exit(ERROR_FILE_NOT_EXIST);
            else{
                fclose(fp);
			    freopen(inFile, "r", stdin);
            }
        }
		if (outNum == 1)
			freopen(outFile, "w", stdout);

		char* comm[BUF_SZ];
		for (int i=low; i<endIdx; ++i)
			comm[i] = commands[i];
		comm[endIdx] = NULL;

		execvp(comm[low], comm+low);
		exit(errno);  
	} else {
		int status;
		waitpid(pid, &status, 0);
		int err = WEXITSTATUS(status);  
        if(err){
            if(err == ERROR_FILE_NOT_EXIST)
                return err;
            else{
			    fprintf(stderr,"\e[31;1mError: %s\n\e[0m", strerror(err));
                return ERROR_COMMAND;
            }
		}
	}

	return RESULT_NORMAL;
}

int callCd(int commandNum) {  
	int result = RESULT_NORMAL;

	if (commandNum < 2) {
		result = ERROR_MISS_PARAMETER;
	} else if (commandNum > 2) {
		result = ERROR_TOO_MANY_PARAMETER;
	} else {
		int ret = chdir(commands[1]);
		if (ret) result = ERROR_WRONG_PARAMETER;
	}

	return result;
}