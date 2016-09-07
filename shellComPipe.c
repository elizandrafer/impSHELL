#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <string.h>
#include <sys/wait.h>

#define flag 1
#define READ 0
#define WRITE 1

pid_t pid;

char *redIO[6] = {">", "<", ">>", "<<", "|", "tee"};
int n = 0;
char *argumentos[1024];

void command_LS(char *arg[]) {
	char dir[100];
	DIR *d;
	struct dirent *di;

	if(arg[1]==NULL){	
		if(getcwd(dir, 100) != NULL) d = opendir(dir);
		else perror("getcwd error");

	}else{
		//diretorio corrente
		if(!strcmp(arg[1], "./")){
			if(getcwd(dir, 100) != NULL) d = opendir(dir);
			else perror("getcwd error");

		//diretorio home
		}else if(!strcmp(arg[1], "~")){
			struct passwd *passE = getpwuid(getuid());
			strcpy(dir, passE->pw_dir);
			d = opendir(dir);

		//diretorio acima
		}else if(!strcmp(arg[1], "..")){
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

void command_RNM(char *args[]) {
	int cont = 0;
	char origem[100];
	char destino[100];

	for (cont=0;args[cont]!=NULL;cont++);

	if(cont != 3){
		printf("use: rnm <nome_atual_do_arquivo> <novo_nome_do_arquivo>\n");
		return;
	}

	strcpy(origem, args[1]);
	strcpy(destino, args[2]);

	if(!rename(origem,destino)){
		printf("Arquivo renomeado com sucesso!\n");
	}
	else{
		printf("Erro ao renomear arquivo.\n");
	}
}

void command_RMV(char *args[]){
	int cont;
	char origem[100];
	
	for (cont=0;args[cont]!=NULL;cont++);

	if(cont != 2){
		printf("use: rmv <nome_do_arquivo>\n");
		return;
	}

	strcpy(origem, args[1]);

	if(!remove(origem)){
		printf("Arquivo %s removido.\n", origem);
	}
	else{
		printf("Erro ao remover arquivo.\n");
	}
}

void command_COPY(char *args[]) {
	FILE *f1, *f2;
	int cont;
	char origem[100];
	char destino[100];
	char aux_arquivo[100];
	
	//printf("%s.\n", args);	
	for (cont=0;args[cont]!=NULL;++cont);

	if(cont != 3){
		printf("Use: cp <nome_do_arquivo> <nome_da_copia>\n");
		return;
	}

	strcpy(origem, args[1]);
	strcpy(destino, args[2]);

	if(!strcmp(origem,destino)) {
		printf("Erro ao copiar arquivo. Arquivo original e copia tem o mesmo nome.\n");
		return;
	}

	printf("Arquivo de origem: %s\n", origem);
	printf("Arquivo de destino: %s\n", destino);

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
	printf("Tchau, querida!\n");
	exit(0);
}

/*void exec_command(char command[]) {
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
}*/

void execCommand(){
	if(argumentos[0]!=NULL){
		printf(".%s.\n", argumentos[0]);
		if(!strcmp(argumentos[0], "ls")){
			command_LS(argumentos);
		}else if(!strcmp(argumentos[0], "rnm")){
			command_RNM(argumentos);
		}else if(!strcmp(argumentos[0], "cp")){
			command_COPY(argumentos);
		}else if(!strcmp(argumentos[0], "rmv")){
			command_RMV(argumentos);
		}else if(!strcmp(argumentos[0], "exit")){
			command_EXIT();
		}else{
			printf("Comando não existe\n");
		}
	}
}

char *remove_spaces_at_begin(char *str) {
	while (*str == ' ') {
		++str;
	}
	return str;
}

int call_command(int input, int first, int last) {
	int pipe_vector[2];

	pipe(pipe_vector);
	pid = fork();


	if (pid==0)	{
		if (first && !last && input==0) {
			dup2(pipe_vector[WRITE], STDOUT_FILENO);
		} else if (!first && !last && input!=0) {
			dup2(input, STDIN_FILENO);
			dup2(pipe_vector[WRITE], STDOUT_FILENO);
		} else if (!first && last && input!=0) {
			dup2(input, STDIN_FILENO);
		}

		//execCommand();
		if (execvp(argumentos[0], argumentos) < 0)
			_exit(EXIT_FAILURE);
	}

	if (input!=0) {
		close(input);
	}

	close(pipe_vector[WRITE]);

	if (last==1)
		close(pipe_vector[READ]);

	return pipe_vector[READ];
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

void split(char *command) {
	command = remove_spaces_at_begin(command);
	char *next = strchr(command, ' ');
	int i = 0;

	while (next != NULL) {
		next[0] = '\0';
		argumentos[i] = command;
		i++;
		command = remove_spaces_at_begin(next+1);
		next = strchr(command, ' ');
	}

	if (command[0] != '\0') {
		argumentos[i] = command;
		next = strchr(command, '\n');
		next[0] = '\0';
		i++;
	}
	argumentos[i]=NULL;
	/*
	printf("..........................\n");
	for(i=0;argumentos[i]!=NULL;i++) {
		printf(".%s.\n", argumentos[i]);
	}
	printf("..........................\n");
	*/
}

int run (int input, int first, int last) {
	if (argumentos[0]!=NULL) {
		n+=1;
		return call_command(input, first, last);
	}
	return 0;
}

void cleanup(int n) {
	int i;
	for(i=0;i<n;i++) {
		wait(NULL);
	}
}

int main() {

	char lineCommand[1024], *command, *next=NULL;
	int input, first, last;

	while(flag){
		printf("> ");
		fgets(lineCommand, 1024, stdin);

		first = 1;
		last = 0;
		input = 0;

		command = lineCommand;
		next = strchr(command, '|');

		while (next != NULL) {
			*next = '\0';
			split(command);
			if (!strcmp(argumentos[0],"exit"))
				command_EXIT();
			input = run(input, first, last);
			command = next+1;
			next = strchr(command, '|');
			first = 0;
		}
		last = 1;
		split(command);
		if (!strcmp(argumentos[0],"exit"))
				command_EXIT();
		input = run(input,first,last);
		cleanup(n);
		n = 0;		
	}

	return 0;
}