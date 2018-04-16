#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]){
	int file;

	if((file = access(argv[1], F_OK)) < 0){
		if(mkdir(argv[1],0777) < 0){
			fprintf(stderr, "ssu_cp:%s: mkdir error",argv[1]);
			exit(1);
		}
	}
	else if(file == 0)
		printf("there is directory");

	exit(0);
	
}