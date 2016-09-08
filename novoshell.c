#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>

#define flag 1
#define WRITE 1
#define READ 0
int in = STDIN_FILENO, out=STDOUT_FILENO, n=0;
int saved_stdin, saved_stdout;

// Tee para chamada sem pipe
void command_TEE(int input, char **argv){
	int i;
	int file;
	FILE *f1, *f2;
	char *firstFile, aux_arquivo[100];

	if (*argv ==NULL)
		return;

	firstFile = *argv;
	argv++;

	if (input == STDIN_FILENO) {
		f1 = fopen(firstFile,"w");
		if(f1==NULL){ printf("Erro!\n"); return; }
		do {
			fgets(aux_arquivo, 100, stdin);
			if (strcmp(aux_arquivo,"end\n") != 0) {
				printf("%s", aux_arquivo);
				fputs(aux_arquivo, f1);
			}
		} while(strcmp(aux_arquivo,"end\n") != 0);	
		fclose(f1);
	}

	f1 = fopen(firstFile,"r");
	if(f1==NULL){ printf("Erro!\n"); return; }
	for (i=0;argv[i] != NULL;i++) {
		f2 = fopen(argv[i],"w");
		if(f2==NULL){ printf("Erro!\n"); return; }

		while(!feof(f1)) {
			fgets(aux_arquivo, 100, f1);
			if (!feof(f1))
				fputs(aux_arquivo, f2);
		}
		fclose(f2);
		rewind(f1);
	}
	fclose(f1);
}

void redirectionIO(char *command, char *arg){

	int len = (unsigned)strlen(command);

	if(len==1){

		switch(*command){
		//arquivo entrada
		case '<':
			in = open(arg, O_RDONLY);
			dup2(in, 0);
			close(in);
		break;
		//arquivo saida
		case '>':
			out = open(arg, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			dup2(out, 1);
			close(out);
		break;

	//arquivo saida. Se existir, adiciona no final arquivo
	}else if(!strcmp(command, ">>")){
	
		out = open(arg, O_WRONLY | O_CREAT | O_APPEND, 0666);
		lseek(out, 0, SEEK_END);
		dup2(out, 1);
		close(out);
	
	//arquivo entrada. Le até palavraX
	}else if(!strcmp(command, "<<")){
		//ainda nao sei como

	//saida padrao e arquivo
	}
}

void redirectionDIR(char *diretorio, char dir[]){

	//diretorio corrente
	if( (diretorio == '\0') || (!strcmp(diretorio, "|")) || (!strcmp(diretorio, "./")) ){
		if(getcwd(dir, 100) == NULL){ perror("getcwd error"); }

	//diretorio home
	}else if(!strcmp(diretorio, "~")){
		struct passwd *passE = getpwuid(getuid());
		strcpy(dir, passE->pw_dir);

	//diretorio acima
	}else if(!strcmp(diretorio, "..")){
		if(getcwd(dir, 100) != NULL){
			*(strrchr(dir, '/') + 1) = 0;
		}else perror("getcwd error");

	//diretorio qualquer
	}else strcpy(dir, diretorio);

}

void command_LS(char **arg) {

	char dir[100];
	DIR *d;
	struct dirent *di;
	int len=0;

	//arg[strlen(arg)-1]='\0';
	while(arg[len]!=NULL) {
		len++;
	}
 
	switch(len){
		case 3: 
			redirectionIO(arg[1], arg[2]);
			redirectionDIR(arg[0], dir);
		break;
		case 2:
			redirectionIO(arg[0], arg[1]);
			redirectionDIR("./", dir);
		break;
		default:
			redirectionDIR(arg[0], dir);
		break;
	}

	printf("# DIR :: %s \n", dir);
	d = opendir(dir);

	if(d){

		printf("Itens do diretorio ...\n");	
		while((di = readdir(d)) != NULL){
			printf("%s\n", di->d_name);
		}
	}

}

void command_CD(char **path){

	char dir[100];
	if(*path == NULL){ redirectionDIR("~", dir); }
	else redirectionDIR(*path, dir);
	chdir(dir);

}

void command_PWD(char **arg){

	if((*arg) != NULL) redirectionIO(arg[0], arg[1]);
	printf("%s\n", get_current_dir_name());

}

//CONTINUAR
void command_CAT(char **arg){

	FILE *f;
	char *buffer;
	int len=0;
	while(arg[len]!=NULL){len++;}

	printf("CAT\n");
/*
	//if(len==1){
		f = fopen(arg[0], "r");
		while(!feof(f)){
			fscanf(f, "%s", buffer);
			printf("%s\n", buffer);
		}
		
		fclose(f);
	//}

	
*/
/*
	if(len==3){ 

		redirectionIO(arg[1], arg[2]); 
	}
	if(len==2){ 
		redirectionIO(arg[0], arg[1]); 

	}

	if((*arg) != '\0'){

		redirectionIO('<', arg[0]);
		redirectionIO('>', )

		f = fopen(arg[0], "r");
		while(!feof(f)){ }

	}*/

}

void command_EXIT() {
	printf("Tchau, querida!\n"); 
	exit(0);
}

void command_RNM(char **args) {

	char *origem, *destino;
	int i=0;
	
	if(args[2]!=NULL){ printf("use: rnm <nome_atual_do_arquivo> <novo_nome_do_arquivo>\n"); return; }

	origem = args[0];
	destino = args[1];

	if(!rename(origem,destino)) printf("Arquivo renomeado com sucesso!\n");
	else printf("Erro ao renomear arquivo.\n");

}

void command_RMV(char **args){
	
	char *origem;
	origem = args[0];

	if(args[1]!=NULL){ printf("use: rmv <nome_do_arquivo>\n"); return; } 

	if(!remove(origem)){ printf("Arquivo %s removido.\n", origem); }
	else printf("Erro ao remover arquivo.\n");
	
}

void command_COPY(char **args) {

	FILE *f1, *f2;
	char *origem, *destino;
	char aux_arquivo[100];

	if(args[2]!=NULL){ printf("Use: cp <nome_do_arquivo> <nome_da_copia>\n"); return; }

	origem = args[0];
	destino = args[1];

	if(!strcmp(origem,destino)){ printf("Erro ao copiar arquivo. Arquivo original e copia tem o mesmo nome.\n"); return; }

	f1 = fopen(origem, "r");
	f2 = fopen(destino, "w");

	if(f1==NULL || f2==NULL){ printf("Erro ao copiar arquivo %s\n", origem); return; }
	while(!feof(f1)) {
		fgets(aux_arquivo, 100, f1);
		fputs(aux_arquivo, f2);
	}

	printf("Arquivo copiado com sucesso!\n");
	fclose(f1);
	fclose(f2);
	
}

void parseLine(char *linha, char **argv){

    while(*linha != '\0') {      													   
        while (*linha == ' ' || *linha == '\t' || *linha == '\n') {
        	*linha = '\0';
        	++linha;
        }
        if(*linha!='\0') {
        	*argv++ = linha;
        	if (*linha == '"') {
        		linha++;
        		while (*linha != '"' && *linha!='\0')
        			linha++;	
        	}
        }
        while (*linha != '\0' && *linha != ' ' && *linha != '\t' && *linha != '\n') {
			linha++;
        }
    }
    *argv = NULL;           														
}

/*
	args = strchr(commandLine, ' ');			//pega a partir 
	if(args!=NULL) {
		args +=1;								//pega depois
		if (strlen(args)==0) {
			args = NULL;
		}
	}
	str = strtok(commandLine, " \n");					
	//tratar dentro das funcoes argumento==NULL

*/

void readCommand(char *str, char **args){

	// str = comando
	// *args = {arg1, arg2, arg3}
	saved_stdin = dup(0);
	saved_stdout = dup(1);

	if(!strcmp(str, "ls")){
		command_LS(*(&args));
	}else if(!strcmp(str, "rnm")){
		command_RNM(*(&args));
	}else if(!strcmp(str, "cp")){
		command_COPY(*(&args));
	}else if(!strcmp(str, "rmv")){
		command_RMV(*(&args));
	}else if(!strcmp(str, "cd")){
		command_CD(*(&args));
	}else if(!strcmp(str, "pwd")){
		command_PWD(*(&args));
	}else if(!strcmp(str, "cat")){
		command_CAT(*(&args));
	}else if(!strcmp(str, "exit")){
		command_EXIT();
	}else if(!strcmp(str, "tee")){
		command_TEE(STDIN_FILENO, args);
	}else{
		printf("Comando não existe\n");
	}

	dup2(saved_stdin, 0);
	dup2(saved_stdout, 1);
	close(saved_stdin);
	close(saved_stdout);
	
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
}

int call_command(int input, int output, int first, int last, char **argv) {
	int pipe_vector[2];
	pid_t pid;

	pipe(pipe_vector);
	pid = fork();

	if (pid==0)	{
		if (first && !last) {
			if (input != STDIN_FILENO) {
				dup2(input, 0);
				close(input);
			}
			dup2(pipe_vector[WRITE], STDOUT_FILENO);
		} else if (!first && !last) {
			dup2(input, STDIN_FILENO);
			dup2(pipe_vector[WRITE], STDOUT_FILENO);
		} else if (!first && last) {
			dup2(input, STDIN_FILENO);
			if (output != STDOUT_FILENO) {
				dup2(output, STDOUT_FILENO);
				close(output);
			}
		}
		
		if (execvp(argv[0], argv) < 0)
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

void cleanup(int n) {
	int i;
	for(i=0;i<n;i++) {
		wait(NULL);
	}
}

void command_PIPE(char *lineCommand) {
	char *argv[64];
	char *command, *next=NULL, *hasRedirectionIn = NULL, *hasRedirectionOut = NULL;
	int input, output, first, last;

	first = 1;
	last = 0;
	input = STDIN_FILENO;
	output = STDOUT_FILENO;

	command = lineCommand;

	// Verifica se ha redirecionamento de entrada ou saida: "<", ">", ">>"
	hasRedirectionIn = strchr(command, '<');
	hasRedirectionOut = strchr(command, '>');
	if (hasRedirectionIn && *(hasRedirectionIn-1) == ' ') {
		*(hasRedirectionIn-1) = '\0';
	}
	if (hasRedirectionOut && *(hasRedirectionOut-1) == ' ') {
		*(hasRedirectionOut-1) = '\0';
	}

	
	// Trata redirecionamento de entrada: "<"
	if (hasRedirectionIn) {
		parseLine(hasRedirectionIn,argv);
		if (!strcmp(argv[0],"<"))
			input = open(argv[1], O_RDONLY);
	}

	next = strchr(command, '|');

	while (next != NULL) {
		*next = '\0';
		parseLine(command, argv);
		if (argv[0] != NULL) {
			if (!strcmp(argv[0],"exit"))
				command_EXIT();
			n+=1;
			input = call_command(input, output, first, last, argv);
		}
		command = next+1;
		next = strchr(command, '|');
		first = 0;
	}
	
	last = 1; // Ultimo comando do pipe

	// Trata redirecionamento de saida: ">" e ">>"
	if (hasRedirectionOut) {
		parseLine(hasRedirectionOut,argv);
		/*
		int i;
		for(i=0;argv[i]!=NULL;i++)
			printf("hasRedirectionIn[%d] %s\n", i, argv[i]);
		*/
		if (!strcmp(argv[0],">")) {
			output = open(argv[1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		} else if (!strcmp(argv[0],">>")) {
			output = open(argv[1], O_WRONLY | O_CREAT | O_APPEND, 0666);
			lseek(output, 0, SEEK_END);
		}
	}


	parseLine(command, argv);
	if (argv[0] != NULL) {
		if (!strcmp(argv[0],"exit"))
			command_EXIT();
		n+=1;
		input = call_command(input, output, first, last,argv);
	}
}

int main() {


	char *linhaComando, *argv[64], *hasPipe;

	while(flag){

		printf("~@~%s >", get_current_dir_name());
		linhaComando = readline(" ");
		add_history(linhaComando);

		hasPipe = strchr(linhaComando, '|');

		if (hasPipe) {
			command_PIPE(linhaComando);
			cleanup(n);
			n=0;
		} else if(*linhaComando != '\0'){
			parseLine(linhaComando, argv);
 			readCommand(argv[0], &argv[1]);
 			/*int i;
 			for(i=0;argv[i]!=NULL;i++) {
 				printf("argv[%d]: %s\n", i, argv[i]);
 			}*/
		}

	}

	return 0;
}