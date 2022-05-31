#include<stdio.h>
#include<unistd.h>
#define BUF_SZ 256
char commands[BUF_SZ][BUF_SZ];

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

int main(){
    // char s[] = "ps | qe";
    // int a = splitCommands(s);
    // printf("%s",commands[4]);
    // return 0;
    FILE*p = freopen("q.txt","r",stdin);
    printf("%d",p);
    return 0;
}