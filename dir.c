#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "dir.h"

int checkFileExsist(char *target_dir, char *rqFile) {
	DIR *dir;
	struct dirent *reader;
	int isFind = 0;

	if ((dir = opendir(target_dir)) == NULL){
		printf("error: directory can not opened.\n");
		closedir(dir);
		exit(1);
	} else {
		// printf("The file you want: %s\n", rqFile);
		// printf("The following are the result: \n");
		while (( reader = readdir(dir)) != NULL){
			if(strcmp(reader->d_name,rqFile) == 0){
				isFind = 1;
				break;
			}
		}
	}
	closedir(dir);
	return isFind;
}

char *listFile(){
	DIR *dir;
	struct dirent *reader;
	int fileNumber = 0;
	char *fileName;

	if ((dir = opendir("./data/")) == NULL){
		printf("error: directory can not opened.\n");
	} else {
		fileName = calloc(sizeof(char)*1024,1);
		while (( reader = readdir(dir)) != NULL){
			if((strcmp(reader->d_name,".") == 0) || ((strcmp(reader->d_name,"..") == 0)) ) continue;
			strcat(fileName,reader->d_name);
			strcat(fileName," \n");
			fileNumber++;
		}
		closedir(dir);

		if(fileNumber == 0){
			strcpy(fileName,"No file found in the directory.\n");
			return fileName;
		} else {
			char *result = malloc(sizeof(char)*strlen(fileName)+1);
			memcpy(result,fileName,strlen(fileName));

			free(fileName);
			return result;
		}
		
	}
}
