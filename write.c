#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

int main(int argc, char * argv[])
{
	int fd;
	int fd2;
	int length;
	char buf[BUFSIZ];
	struct stat srcstat;

	if(argc != 3){
		fprintf(stderr, "usage : %s source target\n", argv[0]);
		exit(1);
	}

	if((fd = open(argv[1], O_RDONLY)) < 0){
		fprintf(stderr, "ssu_cp:%s: open error\n",argv[1]);
		exit(1);
	}

	if(stat(argv[1],&srcstat) < 0){
		fprintf(stderr, "ssu_cp:%s: stat error\n",argv[1]);
		exit(1);
	}

	if((fd2 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, srcstat.st_mode)) < 0){
		fprintf(stderr, "ssu_cp:%s: open erron\n",argv[2]);
		exit(1);
	}

	

	while((length = read(fd, buf, BUFSIZ)) > 0)
		write(fd2, buf, length);

	exit(0);
}
