#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>

#define COMSIZE 256
#define TRUE 1
#define FALSE 0

const char* COMMAND_EXIT = "exit";
const char* COMMAND_CD = "cd";
const char* COMMAND_IN = "<";
const char* COMMAND_OUT = ">";
const char* COMMAND_PIPE = "|";

 
enum {
	RESULT_NORMAL=132,

    ERROR_CWD,

	ERROR_FORK,
    ERROR_PIPE,
	ERROR_PIPE_MISS_PARAMETER,

    ERROR_MISS_FILE,
	ERROR_MANY_REDIR,
    ERROR_FILE_NOT_EXIST,
	ERROR_COMMAND,

	
	ERROR_WRONG_PATH,
    ERROR_NUM_PATH
};

char username[COMSIZE];
char hostname[COMSIZE];
char curPath[COMSIZE];
char split_commands[COMSIZE][COMSIZE];

void getUsername();
void getHostname();
int getCurWorkDir();
int splitCommands(char inputCom[COMSIZE]);
int callCommand(int commandNum);
int callCommandWithPipe(int low, int high);
int callCommandWithRedi(int low, int high);
int callCd(int commandNum);

int main() {

	int result = getCurWorkDir();
	if (ERROR_CWD == result) {
		fprintf(stderr, "\033[31;1mError: System error while getting current work directory.\n\033[0m");
		exit(-1);
	}
	getUsername();
	getHostname();


	char inputCom[COMSIZE];
	while (TRUE) {
		printf("\033[41;32;1m%s@%s:%s\033[0m$ ", username, hostname,curPath);  

		fgets(inputCom, COMSIZE, stdin);
		int len = strlen(inputCom);
		if (len != COMSIZE) {
			inputCom[len-1] = '\0';
		}

		int commandNum = splitCommands(inputCom);
		
		if (commandNum != 0) {  
			if (strcmp(split_commands[0], COMMAND_EXIT) == 0) {  
				exit(0);
			} else if (strcmp(split_commands[0], COMMAND_CD) == 0) {  
				result = callCd(commandNum);
				switch (result) {
					case ERROR_NUM_PATH:
						fprintf(stderr, "\033[31;1mError: Wrong numbers of parameter while using cd \"%s\".\n\033[0m"
							, COMMAND_CD);
						break;
					case ERROR_WRONG_PATH:
						fprintf(stderr, "\033[31;1mError: No such path \"%s\".\n\033[0m", split_commands[1]);
						break;
					case RESULT_NORMAL:
						result = getCurWorkDir();
						if (result == ERROR_CWD) {
							fprintf(stderr,"\033[31;1mError: System error while getting current work directory.\n\033[0m");
							exit(-1);
						} else {
							break;
						}
				}
			} else {
				result = callCommand(commandNum);
				switch (result) {
					case ERROR_FORK:
						fprintf(stderr, "\033[31;1mError: Fork error.\n\033[0m");
						exit(ERROR_FORK);
					case ERROR_PIPE:
						fprintf(stderr, "\033[31;1mError: Open pipe error.\n\033[0m");
						break;
					case ERROR_PIPE_MISS_PARAMETER:
						fprintf(stderr, "\033[31;1mError: Miss pipe parameters.\n\033[0m");
						break;

					case ERROR_MISS_FILE:
						fprintf(stderr, "\033[31;1mError: Miss redirect file parameters.\n\033[0m");
						break;
					case ERROR_MANY_REDIR:
						fprintf(stderr, "\033[31;1mError: Too many redirection symbol \"%s\".\n\033[0m", COMMAND_IN);
						break;
					case ERROR_FILE_NOT_EXIST:
						fprintf(stderr, "\033[31;1mError: Input redirection file is not exist.\n\033[0m");
						break;
				}
			}
		}
	}
}

// int isCommandExist(const char* inputCom) {  
// 	if (inputCom == NULL || strlen(inputCom) == 0) return FALSE;

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

// 			char tmp[COMSIZE];
// 			sprintf(tmp, "inputCom -v %s", inputCom);
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
	gethostname(hostname, COMSIZE);
}

int getCurWorkDir() {  
	char* dir = getcwd(curPath, COMSIZE);
	if (dir == NULL)
		return ERROR_CWD;
	else return RESULT_NORMAL;
}

int splitCommands(char inputCom[COMSIZE]) {  
	int num = 0;
	int i, j;
	int len = strlen(inputCom);

	for (i=0, j=0; i<len; ++i) {
		if (inputCom[i] != ' ') {
			split_commands[num][j++] = inputCom[i];
		} else {
			if (j != 0) {
				split_commands[num][j] = '\0';
				++num;
				j = 0;
			}
		}
	}
	if (j != 0) {
		split_commands[num][j] = '\0';
		++num;
	}

	return num;
}

// int callExit() {  
// 	pid_t pid = getpid();
// 	if (kill(pid, SIGTERM) == -1) 
// 		return ERROR_EXIT;
// 	else return RESULT_NORMAL;
// }

int callCommand(int commandNum) {  
    int inFile = dup(STDIN_FILENO);
    int outFile = dup(STDOUT_FILENO);

    int result = callCommandWithPipe(0, commandNum);

    dup2(inFile, STDIN_FILENO);
    dup2(outFile, STDOUT_FILENO);
    return result;
	
}

int callCommandWithPipe(int low, int high) {  
	// if (low >= high) return RESULT_NORMAL;

	int pipeIdx = -1;
	for (int i=low; i<high; i++) {
		if (strcmp(split_commands[i], COMMAND_PIPE) == 0) {
			pipeIdx = i;
			break;
		}
	}

	if (pipeIdx == -1) {  
		return callCommandWithRedi(low, high);
	}else if (pipeIdx+1 == high) {  
		return ERROR_PIPE_MISS_PARAMETER;
    } 
    else if (pipeIdx+1 < high){
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
                char errorContents[COMSIZE];
                close(fds[1]);
                dup2(fds[0], STDIN_FILENO);  
                close(fds[0]);
                while(fgets(errorContents, COMSIZE, stdin) != NULL) {  
                    fprintf(stderr,"%s", errorContents);  
                }
                return exitCode;
            } else{
                close(fds[1]);
                dup2(fds[0], STDIN_FILENO);
                close(fds[0]);
                return callCommandWithPipe(pipeIdx+1, high);  
            }
        }
	}

}

int callCommandWithRedi(int low, int high) {  
	// if (!isCommandExist(split_commands[low])) {  
	// 	return ERROR_COMMAND;
	// }	

	int inFileNum = 0, outFileNum = 0;
	char *inFile = NULL, *outFile = NULL;
	int endIdx = high;  

	for (int i=low; i<high; ++i) {
		if (strcmp(split_commands[i], COMMAND_IN) == 0) {  
			inFileNum++;
			if (i < high-1)
				inFile = split_commands[i+1];
			else return ERROR_MISS_FILE;  

			if (endIdx == high) endIdx = i;
		} else if (strcmp(split_commands[i], COMMAND_OUT) == 0) {  
			outFileNum++;
			if (i < high-1)
				outFile = split_commands[i+1];
			else return ERROR_MISS_FILE;  
				
			if (endIdx == high) endIdx = i;
		}
	}

	if (inFileNum > 1 || outFileNum > 1)
		return ERROR_MANY_REDIR;

	pid_t pid = fork();
	if (pid == -1) {
		return ERROR_FORK;
	} else if (pid == 0) {

		if (inFileNum == 1){
            FILE* fp = fopen(inFile, "r");
		    if (fp == NULL)
			    exit(ERROR_FILE_NOT_EXIST);
            else{
                fclose(fp);
			    freopen(inFile, "r", stdin);
            }
        }
		if (outFileNum == 1)
			freopen(outFile, "w", stdout);

		char* comm[COMSIZE];
		for (int i=low; i<endIdx; ++i)
			comm[i] = split_commands[i];
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
			    fprintf(stderr,"\033[31;1mError: %s\n\033[0m", strerror(err));
                return ERROR_COMMAND;
            }
		}
	}

	return RESULT_NORMAL;
}

int callCd(int commandNum) {  
	if (commandNum == 2) {
		int a = chdir(split_commands[1]);
		if (a)
            return ERROR_WRONG_PATH;
        return RESULT_NORMAL;
	}
    else
		return ERROR_NUM_PATH;
}