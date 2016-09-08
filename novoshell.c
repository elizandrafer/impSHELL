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

#define flag 1
int in, out;
int saved_stdin, saved_stdout;

//NAO ESTA PRONTO. FALTA COMPLEMENTO COM PIPE
void command_TEE(char *arg){

	int fd, len, slen;

    fd = open(arg, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    if(fd == -1) { perror("open"); exit(EXIT_FAILURE); }
 
   	len = tee(STDIN_FILENO, STDOUT_FILENO, INT_MAX, SPLICE_F_NONBLOCK);
/*
   	if (len < 0){
        if(errno == EAGAIN) continue;
        perror("tee");
        exit(EXIT_FAILURE);
    }else if(len == 0) break;
*/
   	printf("%d\n", len);

  	while(len > 0){ slen = splice(STDOUT_FILENO, NULL, fd, NULL, len, SPLICE_F_MOVE); len -= slen; } 
	close(fd);

}

void command_PIPE(char *arg){

}


//esta função redireciona comandos do terminal ou de um arquivo para outro arquivo.
//Ela recebe como entrada o comando de redirecionamento e os argumentos.
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
		//pipeline
		case '|':
			command_PIPE(arg);
		break;
		}

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
	}else if(!strcmp(command, "tee")){
		command_TEE(arg);

	}else return;

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
	while(arg[len]!='\0'){len++;}
 
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

//entra no diretório desejado
//Recebe como entrada o caminho para o diretório
void command_CD(char **path){

	char dir[100];
	if(*path == '\0'){ redirectionDIR("~", dir); }
	else redirectionDIR(*path, dir);
	chdir(dir);

}

//exibe o diretório corrente
//Recebe como entrada:
//1) vazio
//2) redirecionamento arquivo
void command_PWD(char **arg){

	if((*arg) != '\0') redirectionIO(arg[0], arg[1]);
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
	char *buffer;
	int len=0;
	while(arg[len]!='\0'){len++;}

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

//sai do novoshell.c
void command_EXIT() {

	printf("Tchau querida!\n"); exit(0);
}

//Renomeia um arquivo
//Recebe como entrada o nome atual do arquivo e o nome novo
void command_RNM(char **args) {

	char *origem, *destino;
	int i=0;
	
	if(args[2]!='\0'){ printf("use: rnm <nome_atual_do_arquivo> <novo_nome_do_arquivo>\n"); return; }

	origem = args[0];
	destino = args[1];

	if(!rename(origem,destino)) printf("Arquivo renomeado com sucesso!\n");
	else printf("Erro ao renomear arquivo.\n");

}

//remove um arquivo
//recebe como entrada o nome do arquivo a ser removido
void command_RMV(char **args){
	
	char *origem;
	origem = args[0];

	if(args[1]!='\0'){ printf("use: rmv <nome_do_arquivo>\n"); return; } 

	if(!remove(origem)){ printf("Arquivo %s removido.\n", origem); }
	else printf("Erro ao remover arquivo.\n");
	
}

//copia o conteúdo de um arquivo para outro arquivo.
//recebe como entrada o nome do arquivo de origem e o do arquivo de destino.
void command_COPY(char **args) {

	FILE *f1, *f2;
	char *origem, *destino;
	char aux_arquivo[100];

	if(args[2]!='\0'){ printf("Use: cp <nome_do_arquivo> <nome_da_copia>\n"); return; }

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

//separa uma string com um comando em substrings
//recebe o comando e seus argumentos, e separa por espaços, \t ou \n
void parseLine(char *linha, char **argv){

    while(*linha != '\0'){      													   
        while (*linha == ' ' || *linha == '\t' || *linha == '\n') *linha++ = '\0';     
        *argv++ = linha;          													   
        while (*linha != '\0' && *linha != ' ' && *linha != '\t' && *linha != '\n') linha++;
    }
    *argv = '\0';                 														
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

//lê um comando e chama a função correspondente
//recebe o comando (e seus argumentos, quando existentes)
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
	}else{
		printf("Comando não existe\n");
	}

	dup2(saved_stdin, 0);
	dup2(saved_stdout, 1);
	close(saved_stdin);
	close(saved_stdout);
	
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
        	printf("ERRO: Execucao falhou!\n");
            exit(1);
        }
    }else{                                  
          while (wait(&status) != pid)  ;
    }
}

int main() {


	char *linhaComando, *argv[64];

	while(flag){

		printf("~@~%s >", get_current_dir_name());
		linhaComando = readline(" ");
		if(*linhaComando != '\0'){
			add_history(linhaComando);
			parseLine(linhaComando, argv);
			readCommand(argv[0], &argv[1]);
		}

	}

	return 0;
}
