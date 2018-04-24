#include <stdio.h>
#include <stdlib.h>

int main(int argc, char * argv[]){
	char src[1024];
	char target[1024];

	if(realpath(argv[1],src) == NULL){
		printf("ssu_cp: source path error");
	}
	if(realpath(argv[2],target) == NULL){
		printf("ssu_cp: target path error");
	}

	printf("%s\n%s\n",src, target);

}