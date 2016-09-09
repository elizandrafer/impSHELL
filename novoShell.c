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

//NAO ESTA PRONTO. FALTA COMPLEMENTO COM PIPE
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


//esta função redireciona comandos do terminal ou de um arquivo para outro arquivo.
//Ela recebe como entrada o comando de redirecionamento e os argumentos.
void redirectionIO(char *command, char **argv){

	int len = (unsigned)strlen(command);

	if(len==1){

		switch(*command){
			//arquivo entrada
			case '<':
				in = open(argv[0], O_RDONLY);
				dup2(in, 0);
				close(in);
			break;
			//arquivo saida
			case '>':
				out = open(argv[0], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
				dup2(out, 1);
				close(out);
			break;
		}

	//arquivo saida. Se existir, adiciona no final arquivo
	} else if(!strcmp(command, ">>")){
	
		out = open(argv[0], O_WRONLY | O_CREAT | O_APPEND, 0666);
		lseek(out, 0, SEEK_END);
		dup2(out, 1);
		close(out);
	
	//arquivo entrada. Le até palavraX
	//	<< arquivo palavra
	}else if(!strcmp(command, "<<")){
		char buffer[512];
		int line = 1, encontrou = 0;
		int i;

		buffer[0] = '\0';
		in = open(argv[0], O_RDONLY);
		if (in < 0)
			return;

		i=0;
		while (read(in, &buffer[i], 1) == 1 && i<=512) {
		    if (buffer[i] == '\n' || buffer[i] == 0x0/* null caracter*/) {
		        buffer[i] = 0;
		        if (!strncmp(buffer, argv[1], strlen(argv[1]))) {
		        	encontrou = 1;
		            break;
		        }
		        i = 0;
		        line++;
		        continue;
		    }
		    if (i==512) {
				i = 0;
			}
		    i++;
		}

		lseek(in,0,SEEK_SET);

		if (encontrou) {
			//redirecionar a entrada para conteudo do arquivo ate linha tal
			//Como fazer?
			printf("Encontrou %s na linha %d\n", argv[1], line);
		} else {
			// redirecionar a entrada para todo o conteudo do arquivo
			dup2(in, 0);
			close(in);
		}
	}
}

//esta função muda o diretório corrente para algum outro.
//Recebe como entrada o diretório informado e uma string que vai receber o nome do diretório.
void redirectionDIR(char *diretorio, char dir[]){

	//diretorio corrente
	if( (diretorio == '\0') || (!strcmp(diretorio, "./")) ){
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

//esta função exibe o conteúdo de um diretório.
//Recebe como entrada:
//1) argumento vazio
//2) redirecionamento arquivo
//3) diretório redirecionamento arquivo
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
		case 3:  // "ls dir > arq", "ls dir < arq" (invalido), "ls dir >> arq", "ls << arq palavra" (invalido)
			if (strcmp(arg[0],"<<") != 0) {
				redirectionIO(arg[1], arg+2);
				redirectionDIR(arg[0], dir);
			}
		break;
		case 2: // "ls > arq", "ls < arq" ou "ls >> arq"
			redirectionIO(arg[0], arg+1);
			redirectionDIR("./", dir);
		break;
		default: // "ls"
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

//entra no diretório desejado
//Recebe como entrada o caminho para o diretório
void command_CD(char **path){
	char dir[100];
	if(*path == NULL){ 
		redirectionDIR("~", dir); 
	}
	else  {
		redirectionDIR(*path, dir);
	}
	if (chdir(dir) < 0)
		printf("\nNao e possivel entrar no diretorio \"%s\"\n\nPara abrir diretorios cujo nome possui caractere espaço, use aspas.\nPor exemplo: cd \"Área de Trabalho\"\n\n", *path);;

}

//exibe o diretório corrente
//Recebe como entrada:
//1) vazio
//2) redirecionamento arquivo
void command_PWD(char **arg){

	if((*arg) != NULL) redirectionIO(arg[0], arg+1);
	printf("%s\n", get_current_dir_name());

}

//exibe conteúdo de um arquivo
//recebe como entrada
//1) vazio
//2) redirecionamento arquivo
//3) arquivo
//CONTINUAR
void command_CAT(char **arg){

	FILE *f;
	char buffer[100];
	int len=0;
	while(arg[len]!=NULL)
	{
		len++;
	}

	if(len==3) { 
		if (!strcmp(arg[1],">") || !strcmp(arg[1],">>"))
			redirectionIO(arg[1], arg+2);
	}

	if(len>0){
		f = fopen(arg[0], "r");
		if (f==NULL) {
			printf("Impossivel imprimir arquivo\n");
			return;
		}
		while(!feof(f)){
			fgets(buffer,100,f);
			//if (!feof(f))
			printf("%s", buffer);
		}
		printf("\n");
		
		fclose(f);
	}
}

//sai do novoshell.c
void command_EXIT() {

	printf("Tchau!\n"); exit(0);
}

//Renomeia um arquivo
//Recebe como entrada o nome atual do arquivo e o nome novo
void command_RNM(char **args) {

	char *origem, *destino;
	int i=0;
	
	if(args[2]!=NULL){ printf("use: rnm <nome_atual_do_arquivo> <novo_nome_do_arquivo>\n"); return; }

	origem = args[0];
	destino = args[1];

	if(!rename(origem,destino)) printf("Arquivo renomeado com sucesso!\n");
	else printf("Erro ao renomear arquivo.\n");

}

//remove um arquivo
//recebe como entrada o nome do arquivo a ser removido
void command_RMV(char **args){
	
	char *origem;

	if(args[0]==NULL){ printf("use: rmv <nome_do_arquivo>\n"); return; } 

	while (*args!=NULL) {
		origem = *args;
		if(!remove(origem)){ printf("Arquivo %s removido.\n", origem); }
		else printf("Erro ao remover arquivo %s.\n", origem);
		args++;
	}
	
}

//copia o conteúdo de um arquivo para outro arquivo.
//recebe como entrada o nome do arquivo de origem e o do arquivo de destino.
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
		//if (!feof(f1))
			fputs(aux_arquivo, f2);
	}

	printf("Arquivo copiado com sucesso!\n");
	fclose(f1);
	fclose(f2);
	
}

//separa uma string com um comando em substrings
//recebe o comando e seus argumentos, e separa por espaços, \t ou \n
void parseLine(char *linha, char **argv){

    while(*linha != '\0') {      													   
        while (*linha == ' ' || *linha == '\t' || *linha == '\n') {
        	*linha = '\0';
        	++linha;
        }
        if(*linha!='\0') {
        	if (*linha == '"') {
        		linha++; 
        		*argv++ = linha;
	    		while (*linha != '"' && *linha!='\0')
	    			linha++;
	    		*linha = '\0';
	    		linha++;
        	} else {
        		*argv++ = linha;
        	}
        }
        while (*linha != '\0' && *linha != ' ' && *linha != '\t' && *linha != '\n') {
			linha++;
        }
    }
    *argv = NULL;           														
}

//cria um processo filho para o comando     
void execCommand(char **argv){

	int status;
	pid_t pid = fork();

    if(pid < 0){     								
          printf("ERRO: Chamada processo filho falhou!\n");
          exit(1);
    }else if (pid == 0){          
        if(execvp(*argv, argv) < 0) {  
        	printf("O comando \"%s\" nao pode ser executado\n", *argv);
            exit(1);
        }
    }else{                                  
          while (wait(&status) != pid)  ;
    }
}

//lê um comando e chama a função correspondente
//recebe o comando (e seus argumentos, quando existentes)
void readCommand(char **argv){
	
	// cmd = comando
	// *args = {arg1, arg2, arg3}	
	char *cmd = argv[0];
	char **args = &argv[1];

	saved_stdin = dup(0);
	saved_stdout = dup(1);

	if(!strcmp(cmd, "ls")){
		command_LS(*(&args));
	}else if(!strcmp(cmd, "rnm")){
		command_RNM(*(&args));
	}else if(!strcmp(cmd, "cp")){
		command_COPY(*(&args));
	}else if(!strcmp(cmd, "rmv")){
		command_RMV(*(&args));
	}else if(!strcmp(cmd, "cd")){
		command_CD(*(&args));
	}else if(!strcmp(cmd, "pwd")){
		command_PWD(*(&args));
	}else if(!strcmp(cmd, "cat")){
		command_CAT(*(&args));
	}else if(!strcmp(cmd, "exit")){
		command_EXIT();
	}else if(!strcmp(cmd, "tee")){
		command_TEE(STDIN_FILENO, args);
	}else{
		execCommand(argv);
	}

	dup2(saved_stdin, 0);
	dup2(saved_stdout, 1);
	close(saved_stdin);
	close(saved_stdout);
	
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
 			readCommand(argv);
 			/*int i;
 			for(i=0;argv[i]!=NULL;i++) {
 				printf("argv[%d]: %s\n", i, argv[i]);
 			}*/
		}

	}

	return 0;
}
