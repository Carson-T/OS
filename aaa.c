#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#define BUF_SZ 256
char commands[BUF_SZ][BUF_SZ];


enum{
    AAA,
    BBB
};

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

	return 0;
}

// int a(){
//     pid_t pid = fork();
//     if(pid == 0){
//         exit(0);
//     }
//     else{

//     }
// }
int a();
int main(){
    int x = 1;
    a();
    return 0;
}
int a(){
    printf("%d",x);
}