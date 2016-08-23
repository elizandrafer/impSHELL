#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

/*
int command_LS(char **args){
	char dir[100];
	char *cp;

	if(args[1]==NULL){
		cp = getcwd(dir, 100);
		if(cp!=NULL) printf("DIR >> %s", dir);
	}
}
*/

int main(){
	char dir[100], comando[100], *str;
	DIR *d;
	struct dirent *di;

	printf("> ");
	scanf("%s", comando);
	//strcpy(comando, "ls -al -al -al");

	str = strtok(comando, " ");
	while(str != NULL){
		printf("%s \n", str);
		str = strtok(0, " ");
	}
	

	if(getcwd(dir, sizeof(dir)) != NULL){
		printf("DIR >> %s \n", dir);
		d = opendir(dir);
		if(d){
			printf("Itens do diretorio ...");
			while((di = readdir(d)) != NULL){
				printf("%s \n", di->d_name);
			}
		}

	}else{
		perror(" getcwd error");
	}


	
}