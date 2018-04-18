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

int errorcheck = 0;//for error
int overwritecheck = 1;//for overwrite. 1 : do overwrite, 0 : do not overwrite
int opt_s = 0, opt_i = 0, opt_l = 0, opt_p = 0,opt_n = 0, opt_r = 0, opt_d = 0;
int processcount = 0;//for option -d


int main(int argc, char *argv[])
{
	int c;//for option
	int i;
	int srcformat;//save file's format : reg = 0, dir = 1
	char answer;//for -i option
	char src[PATH_MAX];
	char target[PATH_MAX];

	while((c = getopt(argc, argv,"silpnrd:SILPNRD:")) != -1){//option count
		if(c<97)
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
					errorcheck = 1;
				}
				if(processcount < 1 || processcount > 9){
					printf("ssu_cp: -d option error, [N] must 1 ~ 9\n");
					errorcheck = 1;
				}
				break;
		}
	}

	#ifdef DEBUG
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
		printf("ssu_cp: cannot use same option two times\n");
		errorcheck = 1;
	}
	if(opt_r > 0 && opt_d > 0){// when
		printf("ssu_cp: -r and -d cannot use together\n");
		errorcheck = 1;
	}

	if(argc - optind != 2){
		printf("ssu_cp: command error\n");
		errorcheck = 1;
		exit(1);
	}


	strcpy(src,argv[optind]);//save src
	strcpy(target,argv[optind+1]);//save target
	printf("target : %s\n",target);
	printf("src : %s\n", src);
	srcformat = fileformat(src);


	if(errorcheck == 1){//if error exist, show guide message and exit
		printf("ssu_cp error\n"
			"usage in case of file\n"
			"cp [i/n] [-l][-p]\n"
			"or cp [-s][source][target]\n"
			"in case of directory cp [-i/n][-l][-p][-r][-d][N]\n");
	}

	if(opt_p == 1){// -p option
		printoptp(src);
	}

	if(opt_i == 1){// -i option
		if(access(target, F_OK) == 0){
			printf("overwrite %s (y/n)?",target);
			scanf("%s",&answer);
			if(answer == 'y')
				overwritecheck = 1;
			else
				overwritecheck = 0;
		}
	}

	if(opt_s == 1){
		if(srcformat == 1){
			printf("ssu_cp:%s: cannot do symbolic copy to directory\n",target);
			exit(0);
		}
		else{
			if(symlink(src, target) < 0){
				fprintf(stderr, "ssu_cp:%s->%s: symlink error\n",target,src);
				exit(1);
			}
			else{
				printf("ssu_cp:symlink: %s -> %s\n", target, src);
				exit(0);
			}
		}	
	}

	if(opt_r == 1){
		if(srcformat != 1){
			printf("ssu_cp:%s: this is not directory file\n",src);
		}
		directorycp(src,target);
	}
	else if(opt_d == 1){
		if(srcformat != 1){
			printf("ssu_cp:%s: this is not directory file\n"src);
		}
	}
	else{
		cp(src,target);
	}
	//cp command START
	exit(0);
}

//find out fileformat
//if file is regular, return 0
//if file is directory, return 1
//if file is symbolic, return 2
int fileformat(char *fname){
	struct stat file_info;
	if(lstat(fname, &file_info) < 0) {//not original file, get link file's information
		fprintf(stderr, "ssu_cp:%s: No such file or directory\n",fname);
		errorcheck = 1;
	}

	if(S_ISREG(file_info.st_mode))
		return 0;
	else if(S_ISDIR(file_info.st_mode))
		return 1;
	else if(S_ISLNK(file_info.st_mode)){
		return 2;
	}
	else{
		printf("ssu_cp:%s: unknown file format\n",fname);
		errorcheck = 1;
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
		errorcheck = 1;
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

void cp(char * src, char * target){
	int fd;
	int fd2;
	int length;
	char buf[BUFSIZ];
	struct stat srcstat;
	struct utimbuf time_buf;

	if(overwritecheck == 0){
		if(access(target,F_OK) == 0)
			return;
	}

	if((fd = open(src, O_RDONLY)) < 0){
		fprintf(stderr, "ssu_cp:%s: open error\n",src);
		exit(1);
	}
	
	if((fd2 = open(target, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0){
		fprintf(stderr, "ssu_cp:%s: open erron\n",target);
		exit(1);
	}

	while((length = read(fd, buf, BUFSIZ)) > 0)//cp 
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

void directorycp(char *src, char *target){
	struct dirent *dentry;
	char srcbuf[PATH_MAX];
	char tarbuf[PATH_MAX];
	DIR *dirp;

	if(fileformat(src) != 1){
		printf("ssu_cp:%s: [SOURCE] is not directory\n",src);
		exit(1);
	}

	if(access(target,F_OK) < 0){
		if(mkdir(target,0777) < 0){
			fprintf(stderr, "ssu_cp:%s: failed to mkdir",target);
			exit(1);
		}
	}

	if((dirp = opendir(src)) == NULL){
		fprintf(stderr, "ssu_cp:%s: opendir error\n",src);
		exit(1);
	}

	while((dentry = readdir(dirp)) != NULL){
		if(strcmp(dentry->d_name, ".") == 0 || strcmp(dentry->d_name,"..") == 0)
			continue;

		sprintf(srcbuf, "%s/%s",src, dentry->d_name);
		sprintf(tarbuf, "%s/%s",target, dentry->d_name);
		if(fileformat(srcbuf) == 1){
			directorycp(srcbuf,tarbuf);
		}
		else
			cp(srcbuf, tarbuf);
	}

}


void processcp(char *src, char *target){
	struct dirent *dentry;
	char srcbuf[PATH_MAX];
	char tarbuf[PATH_MAX];
	int process;
	DIR *dirp;
	pid_t pid;

	if(fileformat(src) != 1){
		printf("ssu_cp:%s: [SOURCE] is not directory\n",src);
		exit(1);
	}

	if(access(target,F_OK) < 0){
		if(mkdir(target,0777) < 0){
			fprintf(stderr, "ssu_cp:%s: failed to mkdir",target);
			exit(1);
		}
	}

	if((dirp = opendir(src)) == NULL){
		fprintf(stderr, "ssu_cp:%s: opendir error\n",src);
		exit(1);
	}

	printf("src : %s\n",src);

	for(i = 0; i < processcount; i++){
		pid = fork();
		if(pid < 0){
			fprintf(stderr, "ssu_cp:pid[%s]: process fork error\n",i);
			exit(1);
		}
	}

	while((dentry = readdir(dirp)) != NULL){
		printf("file : %s\n",dentry->d_name);

		if(strcmp(dentry->d_name, ".") == 0 || strcmp(dentry->d_name,"..") == 0)
			continue;

		sprintf(srcbuf, "%s/%s",src, dentry->d_name);
		sprintf(tarbuf, "%s/%s",target, dentry->d_name);
		if(fileformat(srcbuf) == 1){
			if(pid == 0){
				directorycp(srcbuf,tarbuf);
				exit(0);
			}
		}
		else
			cp(srcbuf, tarbuf);
	}

}
