#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

int checkFileExsist(char *rqFile){
  DIR *dir;
  struct dirent *reader;
  int isFind = 0;

  if ((dir = opendir("./")) == NULL){
     printf("error: directory can not opened.\n");
  }
  else {
    printf("The file you want: %s\n", rqFile);
    printf("The following are the result: \n");
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

void listFile(){
  DIR *dir;
  struct dirent *reader;
  int fileNumber = 0;
  char *fileName;
  if ((dir = opendir("./")) == NULL){
    printf("error: directory can not opened.\n");
  }
  else {
    fileName = malloc(sizeof(char) );
    while (( reader = readdir(dir)) != NULL){
      if((strcmp(reader->d_name,".") == 0) || ((strcmp(reader->d_name,"..") == 0)) ){
        continue;
      }
      strcat(fileName,reader->d_name);
      strcat(fileName," \n");
      fileNumber++;
    }
    if(fileNumber == 0){
      strcpy(fileName,"No file found in the directory.\n");
    }
      printf("%s", fileName);
  closedir(dir);
  }
}

main() {
  int choice = 0;
  char type[4];
  char *requestFile = malloc(sizeof(char));
  printf("chice(1/2): ");
  scanf("%d",&choice);
  if(choice == 1){
    listFile();
  }else if(choice == 2){
    printf("Which file do you want? ");
    scanf("%s",requestFile);
    if (checkFileExsist(requestFile) == 1){
      strcpy(type,"0xB2");
    }
    else{
      strcpy(type,"0xB3");
    }
    printf("%s\n",type); 
  }
}
