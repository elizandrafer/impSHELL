#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <string.h>
<<<<<<< HEAD
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#define flag 1

char *redIO[6] = {"<", "<<", ">", ">>", "|", "tee"};

void redirectionINPUT(char *argIN, char *command){
	
	int i=0, in, out;

	while(strcmp(redIO[i], command) && i!=6) i++;
	in = open(argIN, O_RDONLY);
	dup2(in, 0);
	close(in);

	//execCommand();

}

void redirectionOUTPUT(char *argOUT, char *command){

	int i=0, in, out;

	while(strcmp(redIO[i], command) && i!=6) i++;
	out = open(argOUT, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);

}
=======
#include <sys/wait.h>
#define flag 1

char *redIO[6] = {">", "<", ">>", "<<", "|", "tee"};
int n = 0;
>>>>>>> b877e608571b8ee825cbb3fa16ea4c07def0965d

void command_LS(char arg[]) {
	char dir[100];
	DIR *d;
	struct dirent *di;
	printf("aquixx\n");

	if(arg==NULL){	
		if(getcwd(dir, 100) != NULL) d = opendir(dir);
		else perror("getcwd error");

	}else{
		arg[strlen(arg)-1]='\0';

		//diretorio corrente
		if(!strcmp(arg, "./")){
			if(getcwd(dir, 100) != NULL) d = opendir(dir);
			else perror("getcwd error");

		//diretorio home
		}else if(!strcmp(arg, "~")){
			struct passwd *passE = getpwuid(getuid());
			strcpy(dir, passE->pw_dir);
			d = opendir(dir);

		//diretorio acima
		}else if(!strcmp(arg, "..")){
			if(getcwd(dir, 100) != NULL){
				*(strrchr(dir, '/') + 1) = 0;
				d = opendir(dir);
			}else perror("getcwd error");
		}
	} 



	printf("DIR >> %s \n", dir);

	if(d){
		printf("Itens do diretorio ...\n");
		while((di = readdir(d)) != NULL){
			printf("%s \n", di->d_name);
		}
	}

}
void command_RNM(char args[]) {
	char *str;
	char args_copy[100];
	int cont = 0;
	char origem[100];
	char destino[100];

	strcpy(args_copy, args);
	
	str = strtok(args, " ");
	while(str != NULL){
		str = strtok(NULL, " \n");
		cont++;
	}

	if(cont != 2){
		printf("use: rnm <nome_atual_do_arquivo> <novo_nome_do_arquivo>\n");
		return;
	}

	cont = 0;
	str = strtok(args_copy, " \n");
	while(str != NULL){
		if (cont == 0) {
			strcpy(origem, str);
		} else if (cont == 1){
			strcpy(destino, str);
		}
		str = strtok(NULL, " \n");
		cont++;
	}

	if(!rename(origem,destino)){
		printf("Arquivo renomeado com sucesso!\n");
	}
	else{
		printf("Erro ao renomear arquivo.\n");
	}
}

void command_RMV(char args[]){
	char *str;
	char args_copy[100];
	int cont = 0;
	char origem[100];
	
	strcpy(args_copy, args);
	
	str = strtok(args, " ");
	while(str != NULL){
		str = strtok(NULL, " ");
		cont++;
	}

	if(cont != 1){
		printf("use: rmv <nome_do_arquivo>\n");
		return;
	}

	cont = 0;
	str = strtok(args_copy, " \n");
	while(str != NULL){
		strcpy(origem, str);
		str = strtok(NULL, " \n");
	}

	if(!remove(origem)){
		printf("Arquivo %s removido.\n", origem);
	}
	else{
		printf("Erro ao remover arquivo.\n");
	}
}

void command_COPY(char args[]) {
	FILE *f1, *f2;
	char *str;
	char args_copy[100];
	int cont = 0;
	char origem[100];
	char destino[100];
	char aux_arquivo[100];
	
	//printf("%s.\n", args);
	strcpy(args_copy, args);
	
	str = strtok(args, " \n");
	while(str != NULL){
		str = strtok(NULL, " ");
		cont++;
	}

	if(cont != 2){
		printf("Use: cp <nome_do_arquivo> <nome_da_copia>\n");
		return;
	}

	cont = 0;
	str = strtok(args_copy, " \n");
	//str = strtok(NULL, " ");

	while(str != NULL){
		
		if (cont == 0) {
			strcpy(origem, str);
		} else if (cont == 1){
			strcpy(destino, str);
		}
		str = strtok(NULL, " \n");
		cont++;
	}

	if(!strcmp(origem,destino)) {
		printf("Erro ao copiar arquivo. Arquivo original e copia tem o mesmo nome.\n");
		return;
	}

	//printf("Arquivo de origem: %s\n", origem);
	//printf("Arquivo de destino: %s\n", destino);

	f1 = fopen(origem, "r");
	f2 = fopen(destino, "w");

	if (f1==NULL || f2==NULL) {
		printf("Erro ao copiar arquivo %s\n", origem);
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
	printf("Tchau querida!\n");
	exit(0);
}

void exec_command(char command[]) {
	char *str;
	char command_copy[100];
	strcpy(command_copy,command);
	str = strtok(command_copy, " ");
	printf("%s\n", str);
	if(str != NULL){
		if (!strcmp(str,"ls")) { // strcmp retorna 0 se as strings são iguais e 1 caso contrário, por isso a lógica é inversa
			command_LS("./");
		}
		else if (!strcmp(str,"rnm")) {
			command_RNM(command);
		}
		else if (!strcmp(str,"cp")) {
			command_COPY(command);
		}else if (!strcmp(str,"rmv")){
			command_RMV(command);
		} else{
			printf("Comando não existe\n");
		}
	}
}

<<<<<<< HEAD
void parseLine(char *linha, char **argv){

    while(*linha != '\0'){      													   
        while (*linha == ' ' || *linha == '\t' || *linha == '\n') *linha++ = '\0';     
        *argv++ = linha;          													   
        while (*linha != '\0' && *linha != ' ' && *linha != '\t' && *linha != '\n') linha++;
    }
    *argv = '\0';                 														
}

void readCommand(char linhaComando[]){
=======
void readCommand(char commandLine[]){

	char *str, *args=NULL;
>>>>>>> b877e608571b8ee825cbb3fa16ea4c07def0965d

	args = strchr(commandLine, ' ');			//pega a partir 
	if(args!=NULL) {
		args +=1;	//pega depois
		if (strlen(args)==0) {
			args = NULL;
		}
	}
	str = strtok(commandLine, " \n");						//pega antes
	//tratar dentro das funcoes argumento==NULL

	if(str != NULL){
		printf(".%s.\n", args);
		if(!strcmp(str, "ls")){
			command_LS(args);
		}else if(!strcmp(str, "rnm")){
			command_RNM(args);
		}else if(!strcmp(str, "cp")){
			command_COPY(args);
		}else if(!strcmp(str, "rmv")){
			command_RMV(args);
		}else if(!strcmp(str, "exit")){
			command_EXIT();
		}else{
			printf("Comando não existe\n");
		}
	}
}

<<<<<<< HEAD
}
     
void execCommand(char **argv){

	int status;
	pid_t pid = fork();

    if(pid < 0){     								
          printf("ERRO: Chamada processo filho falhou!\n");
          exit(1);
    }else if (pid == 0){          
        if(execvp(*argv, argv) < 0) {  
        	printf("ERRO: Execucao falhou!\n");
            exit(1);
        }
    }else{                                  
          while (wait(&status) != pid)  ;
    }
=======
char *remove_spaces_at_begin(char *str) {
	while (*str == ' ') {
		++str;
		printf("Entrei\n");
	}
	return str;
}
>>>>>>> b877e608571b8ee825cbb3fa16ea4c07def0965d

int call_command(char *command, int input, int first, int last) {
	command = remove_spaces_at_begin(command);
	printf("command: .%s.\n", command);
	readCommand(command);
}

/*
	Uteis (Sobre pipe e dup2):

	Enviado pela Ana:
		http://stackoverflow.com/questions/15673333/what-is-the-proper-way-to-pipe-when-making-a-shell-in-c
	Enviado pela Eli:
		http://www.cs.loyola.edu/~jglenn/702/S2005/Examples/dup2.html
	Exemplo comentado de Pipe em C (em portugues):
		http://www.programacaoprogressiva.net/2014/09/Pipes-em-C-Comunicao-entre-Processos-IPC-Interprocess-Communication.html
	Sobre dup e dup2 (em portugues):
		https://www.vivaolinux.com.br/dica/Utilizando-as-chamadas-de-sistema-dup()-e-dup2()
	Melhor exemplo de pipe para shell:
		https://gist.github.com/parse/966049

	// Continuar do ´ultimo link

*/

void cleanup(int n) {
	int i;
	for(i=0;i<n;i++) {
		wait(NULL);
	}
}

int main() {

<<<<<<< HEAD
	char linhaComando[1024], *argv[64], *str;

	while(flag){
		printf("> ");
		fgets(linhaComando, 1014, stdin);
		parseLine(linhaComando, argv);
		readCommand(argv);
=======
	char linhaComando[1024], *command, *next=NULL;
	int input, first, last;

	while(flag){
		printf("> ");
		fgets(linhaComando, 1024, stdin);

		first = 1;
		last = 0;
		input = 0;

		command = linhaComando;
		next = strchr(command, '|');

		while (next != NULL) {
			*next = '\0';
			input = call_command(command, input, first, last);
			command = next+1;
			next = strchr(command, '|');
			first = 0;
		}
		last = 1;
		input = call_command(command,input,first,last);
		cleanup(n);
		n = 0;		
>>>>>>> b877e608571b8ee825cbb3fa16ea4c07def0965d
	}

	return 0;
}