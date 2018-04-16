#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char buf[BUFSIZ];
	sprintf(buf, "argv[0] : %s, argv[1] : %s",argv[0],argv[1]);
	printf("%s",buf);
}