#include <stdio.h>
#include <stdlib.h>
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

void command_LS() {
	char dir[100];
	DIR *d;
	struct dirent *di;
	

	if(getcwd(dir, 100) != NULL){
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
void command_RNM(char comando[]) {
	char *str;
	char comando_copia2[100];
	int cont = 0;
	char origem[100];
	char destino[100];
	//printf("%s\n", comando);
	strcpy(comando_copia2, comando);
	str = strtok(comando, " ");


	while(str != NULL){
		str = strtok(NULL, " ");
		cont++;
	}

	if(cont != 3){
		printf("Diferente de 3\n");
		return;
	}

	cont = 0;
	str = strtok(comando_copia2, " ");
	//str = strtok(NULL, " ");

	while(str != NULL){
		
		if (cont == 1) {
			strcpy(origem, str);
		} else if (cont == 2){
			strcpy(destino, str);
		}
		str = strtok(NULL, " ");
		cont++;
	}

	if(!rename(origem,destino)){
		printf("Arquivo renomeado com sucesso!\n");
	}
	else{
		printf("Erro ao renomear arquivo.\n");
	}
}

void command_RMV(char comando[]){
	char *str;
	char comando_copia2[100];
	int cont = 0;
	char origem[100];
	//printf("%s\n", comando);
	strcpy(comando_copia2, comando);
	str = strtok(comando, " ");


	while(str != NULL){
		str = strtok(NULL, " ");
		cont++;
	}

	if(cont != 2){
		printf("Diferente de 2\n");
		return;
	}

	cont = 0;
	str = strtok(comando_copia2, " ");
	//str = strtok(NULL, " ");

	while(str != NULL){
		
		if (cont == 1) {
			strcpy(origem, str);
		}
		str = strtok(NULL, " ");
		cont++;
	}

	if(!remove(origem)){
		printf("Arquivo removido.\n");
	}
	else{
		printf("Erro ao remover arquivo.\n");
	}
}

void command_COPY(char comando[]) {
	FILE *f1, *f2;
	char *str;
	char comando_copia2[100];
	int cont = 0;
	char origem[100];
	char destino[100];
	char aux_arquivo[100];
	//printf("%s.\n", comando);
	strcpy(comando_copia2, comando);
	str = strtok(comando, " ");


	while(str != NULL){
		str = strtok(NULL, " ");
		cont++;
	}

	if(cont != 3){
		printf("Diferente de 3\n");
		return;
	}

	cont = 0;
	str = strtok(comando_copia2, " ");
	//str = strtok(NULL, " ");

	while(str != NULL){
		
		if (cont == 1) {
			strcpy(origem, str);
		} else if (cont == 2){
			strcpy(destino, str);
		}
		str = strtok(NULL, " ");
		cont++;
	}

	if(!strcmp(origem,destino)) {
		printf("Erro ao copiar arquivo\n");
		return;
	}

	//printf("%s\n", origem);
	//printf("%s\n", destino);

	f1 = fopen(origem, "r");
	f2 = fopen(destino, "w");

	if (f1==NULL || f2==NULL) {
		printf("Erro ao copiar arquivo\n");
		return;
	}

	while (!feof(f1)) {
		fgets(aux_arquivo, 100, f1);
		fputs(aux_arquivo, f2);
	}

	fclose(f1);
	fclose(f2);
}

void command_EXIT() {
	exit(0);
}

void exec_command(char comando[]) {
	char *str;
	char comando_copia[100];
	strcpy(comando_copia,comando);
	str = strtok(comando_copia, " ");
	printf("%s\n", str);
	if(str != NULL){
		if (!strcmp(str,"ls")) { // strcmp retorna 0 se as strings são iguais e 1 caso contrário, por isso a lógica é inversa
			command_LS();
		}
		else if (!strcmp(str,"rnm")) {
			command_RNM(comando);
		}
		else if (!strcmp(str,"cp")) {
			command_COPY(comando);
		}else if (!strcmp(str,"rmv")){
			command_RMV(comando);
		} else{
			printf("Comando não existe\n");
		}
	}
}

int main() {
	char dir[100], comando[100], comando_copia[100], *str;
	DIR *d;

	printf("> ");
	gets(comando);
	// strcpy(comando, "ls -al -al -al");

	strcpy(comando_copia,comando);

	str = strtok(comando_copia, "|");
	while(str != NULL){
		printf("%s.\n", str);
		exec_command (str);
		str = strtok(NULL, "|");
	}

	return 0;
}