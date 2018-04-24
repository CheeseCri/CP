#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include "ssu_cp.h"

int overwritecheck = 1;//for overwrite. 1 : do overwrite, 0 : do not overwrite
int opt_s = 0, opt_i = 0, opt_l = 0, opt_p = 0,opt_n = 0, opt_r = 0, opt_d = 0;//option's value
int processcount = 0;//for option -d


int main(int argc, char *argv[])
{
	int c;//for option
	int i;
	int srcformat;//save file's format : reg = 0, dir = 1
	char answer;//for -i option
	char buf[PATH_MAX];//get src
	char buf2[PATH_MAX];//get target
	char src[PATH_MAX];//get src's realpath
	char target[PATH_MAX];//get target's realpath

	while((c = getopt(argc, argv,"silpnrd:SILPNRD:")) != -1){//option count
		if(c<97)//if option is uppercase, change it to lowercase
			c = c+32;
		switch (c){
			case 's':
				printf("s option is on\n");
				opt_s++;
				break;
			case 'i':
				printf("i option is on\n");
				opt_i++;
				break;
			case 'l':
				printf("l option is on\n");
				opt_l++;
				break;
			case 'p':
				printf("p option is on\n");
				opt_p++;
				break;
			case 'n':
				printf("n option is on\n");
				opt_p++;
				break;
			case 'r':
				printf("r option is on\n");
				opt_r++;
				break;
			case 'd':
				printf("d option is on\n");
				opt_d++;
				if((processcount = atoi(optarg)) == 0){
					printf("ssu_cp: -d option error, must have [N]\n");
					errorexit();
				}
				if(processcount < 1 || processcount > 9){
					printf("ssu_cp: -d option error, [N] must 1 ~ 9\n");
					errorexit();
				}
				break;
		}
	}

	#ifdef DEBUG //for DEBUG
	printf("----Argv List----\n");
	for(i = 0; i < argc; i++){
		printf("argv[%d] : %s\n",i, argv[i]);
	}
	printf("----Except option----\n");
	for(i = optind; i < argc; i++){
		printf("argv[%d] : %s\n",i,argv[i]);
	}
	#endif

	if(opt_s > 1 || opt_i > 1 || opt_l > 1 || opt_p > 1 || opt_r > 1 || opt_d > 1){//option double time error
		printf("ssu_cp: cannot use same option double times\n");
		errorexit();
	}
	if(opt_r > 0 && opt_d > 0){// when r option and d option are used in same time
		printf("ssu_cp: -r and -d cannot use together\n");
		errorexit();
	}

	if(argc - optind != 2){//argument error print
		printf("ssu_cp: command error\n");
		errorexit();
	}


	strcpy(buf,argv[optind]);//save src
	strcpy(buf2,argv[optind+1]);//save target
	printf("target : %s\n",buf2);
	printf("src : %s\n", buf);


	if(opt_p == 1){// -p option
		printoptp(buf);
	}

	if(opt_i == 1){// -i option
		if(access(buf2, F_OK) == 0){
			printf("overwrite %s (y/n)?",buf2);
			scanf("%s",&answer);
			if(answer == 'y')
				overwritecheck = 1;
			else
				overwritecheck = 0;
		}
	}

	if(opt_n == 1){//-n option
		overwritecheck = 0;
	}

	if(opt_s == 1){//-s option
		if(opt_i != 0 || opt_l != 0 || opt_p != 0 || opt_n != 0 || opt_r != 0 || opt_d != 0){//-s cannot be used with other option
			printf("ssu_cp:-s option cannot be used with other option\n");
			errorexit();
		}
		if(srcformat == 1){//if source is directory, error
			printf("ssu_cp:%s: cannot do symbolic copy to directory\n",buf);
			errorexit();
		}
		else{
			if(symlink(buf, buf2) < 0){//if symlink task error occured, print error
				fprintf(stderr, "ssu_cp:%s->%s: symlink error\n",buf2,buf);
				errorexit();
			}
			else{
				exit(0);
			}
		}	
	}

	realpath(buf,src);//change source path to real path
	realpath(buf2,target);//change target path to real path
	if(!strcmp(src,target)){//if source and target are same, print error
		printf("ssu_cp: input error, source and target are same\n");
		errorexit();
	}
	srcformat = fileformat(src);//get source file's format

	if(opt_r == 1){
		if(srcformat != 1){//for -r option, if source is not directory, error
			printf("ssu_cp:%s: this is not directory file\n",src);
			errorexit();
		}
		directorycp(src,target);//Directory copy function.
	}
	else if(opt_d == 1){
		if(srcformat != 1){//for -d option, if source is not directory, error
			printf("ssu_cp:%s: this is not directory file\n",src);
			errorexit();
		}
		processcp(src, target);//Directory copy function by using fork()
	}
	else{
		cp(src,target);//Copy file
	}
	exit(0);
}

//find out fileformat
//if file is regular, return 0
//if file is directory, return 1
//if file is symbolic, return 2
//else return 3
int fileformat(char *fname){
	struct stat file_info;
	if(lstat(fname, &file_info) < 0) {//not original file, get link file's information
		fprintf(stderr, "ssu_cp:%s: No such file or directory\n",fname);
		errorexit();
	}

	if(S_ISREG(file_info.st_mode))
		return 0;
	else if(S_ISDIR(file_info.st_mode))
		return 1;
	else if(S_ISLNK(file_info.st_mode)){
		return 2;
	}
	else{
		return 3;
	}
}
//print -p option's requirements.
void printoptp(char * src){
	char buff[20];
	struct stat srcstat;
	struct passwd *pw;
	struct group *gw;
	if(lstat(src, &srcstat) < 0) {
		fprintf(stderr, "ssu_cp:%s: No such file or directory\n",src);
		errorexit();
	}
	pw = getpwuid(srcstat.st_uid);
	gw = getgrgid(srcstat.st_gid);
	printf("**********file info**********\n");
	printf("File Name : %s\n", src);
	strftime(buff, 20, "%Y.%m.%d %H:%M:%S",localtime(&srcstat.st_atime));
	printf("Last Access Time : %s\n", buff);
	strftime(buff, 20, "%Y.%m.%d %H:%M:%S",localtime(&srcstat.st_ctime));
	printf("Last Change Time : %s\n", buff);
	strftime(buff, 20, "%Y.%m.%d %H:%M:%S",localtime(&srcstat.st_mtime));
	printf("Last Modified Time : %s\n", buff);
	printf("USER : %s\n", pw->pw_name);
	printf("GROUP : %s\n", gw->gr_name);
	printf("file size : %ld\n", srcstat.st_size);
}
//file copy function
void cp(char * src, char * target){
	int fd;
	int fd2;
	int length;
	char buf[BUFSIZ];
	struct stat srcstat;
	struct utimbuf time_buf;

	if(overwritecheck == 0){//overwrite check
		if(access(target,F_OK) == 0)
			return;
	}

	if((fd = open(src, O_RDONLY)) < 0){
		fprintf(stderr, "ssu_cp:%s: open error\n",src);
		errorexit();
		exit(1);
	}
	
	if((fd2 = open(target, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0){
		fprintf(stderr, "ssu_cp:%s: open erron\n",target);
		errorexit();
		exit(1);
	}

	while((length = read(fd, buf, BUFSIZ)) > 0)//wrtie source's content to target
		write(fd2, buf, length);


	if(opt_l == 1){	//for -l option
		if(stat(src,&srcstat) < 0){
			fprintf(stderr, "ssu_cp:%s: stat error\n", src);
		}
		if(chown(target, srcstat.st_uid, srcstat.st_gid) < 0) {
			fprintf(stderr, "ssu_cp:%s: chown error\n", target);
		}
		if(chmod(target, srcstat.st_mode) < 0){
			fprintf(stderr, "ssu_cp:%s: chmod error\n", target);
		}

		time_buf.actime = srcstat.st_atime;
		time_buf.modtime = srcstat.st_mtime;


		if(utime(target, &time_buf) < 0){
			fprintf(stderr, "ssu_cp:%s: utime error\n",target);
		}

	}
}
//Directory copy function
void directorycp(char *src, char *target){
	struct dirent *dentry;
	char srcbuf[PATH_MAX];
	char tarbuf[PATH_MAX];
	DIR *dirp;

	if(fileformat(src) != 1){//if file's format is not directory
		printf("ssu_cp:%s: [SOURCE] is not directory\n",src);
		errorexit();
	}

	if(access(target,F_OK) < 0){//if target directory does not exist
		if(mkdir(target,0777) < 0){
			fprintf(stderr, "ssu_cp:%s: failed to mkdir",target);
			errorexit();
		}
	}
	else if(access(target,F_OK) == 0){//overwrite check
		if(overwritecheck == 0)
			return;
	}

	if((dirp = opendir(src)) == NULL){//open source directory
		fprintf(stderr, "ssu_cp:%s: opendir error\n",src);
		errorexit();
		exit(1);
	}

	while((dentry = readdir(dirp)) != NULL){//read files in source directory
		if(strcmp(dentry->d_name, ".") == 0 || strcmp(dentry->d_name,"..") == 0)//skip current and parent's directory
			continue;

		sprintf(srcbuf, "%s/%s",src, dentry->d_name);
		sprintf(tarbuf, "%s/%s",target, dentry->d_name);
		if(fileformat(srcbuf) == 1){//if file is directory, call directory copy function again
			directorycp(srcbuf,tarbuf);
		}
		else
			cp(srcbuf, tarbuf);//if file is regular file, call file copy function
	}

}

//Directory copy by using fork()
void processcp(char *src, char *target){
	struct dirent *dentry;
	char srcbuf[PATH_MAX];
	char tarbuf[PATH_MAX];
	DIR *dirp;
	pid_t pid;


	if(fileformat(src) != 1){
		printf("ssu_cp:%s: [SOURCE] is not directory\n",src);
		errorexit();
		exit(1);
	}

	if(access(target,F_OK) < 0){
		if(mkdir(target,0777) < 0){
			fprintf(stderr, "ssu_cp:%s: failed to mkdir",target);
			errorexit();
			exit(1);
		}
	}

	if((dirp = opendir(src)) == NULL){
		fprintf(stderr, "ssu_cp:%s: opendir error\n",src);
		errorexit();
		exit(1);
	}

	while((dentry = readdir(dirp)) != NULL){
		if(strcmp(dentry->d_name, ".") == 0 || strcmp(dentry->d_name,"..") == 0)//skip current and parent's directory.
			continue;

		sprintf(srcbuf, "%s/%s",src, dentry->d_name);
		sprintf(tarbuf, "%s/%s",target, dentry->d_name);
		if(fileformat(srcbuf) == 1){//if file is directory, and processcount is bigger than 0
			if(processcount > 0){
				processcount--;
				pid = fork();
				if(pid == 0){
					directorycp(srcbuf,tarbuf);
					printf("ssu_cp:pid=%d: %s->%s copy done\n",getpid(),srcbuf,tarbuf);
					exit(0);
				}
			}
			else{
				directorycp(srcbuf, tarbuf);
			}
		}
		else
			cp(srcbuf, tarbuf);
	}

}
//if there is error, call this funtion to show manual
void errorexit(){
	printf("ssu_cp error\n"
			"usage in case of file\n"
			"cp [i/n] [-l][-p]\n"
			"or cp [-s][source][target]\n"
			"in case of directory cp [-i/n][-l][-p][-r][-d][N]\n");
	exit(1);
}