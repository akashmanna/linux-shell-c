#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

// UNIVERSAL DECLARATIONS
#define INPUT_BUFFER_SIZE 256
#define N_MAX_CMD 16
#define MAX_CMD_SIZE 64
#define MAX_OPERATOR_BUF_SIZE 16
#define MAX_BUF_SIZE 1024
#define MAX_DIRECTORY_BUF 1024


char buf[MAX_BUF_SIZE];

void printExitStatus(int status,int process_id);
int createChild (int fromOp, char *fromPtr, int toOp, char *toPtr, int pipecmd, char **command_tokens, int p[], int q[], int r[]);
void quitShell();
int cd(char *args[]);


int cd(char *args[]) {
  if (args[1] == NULL) {
    fprintf(stderr, "Bad Argument to cd\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("Error in CD");
    }
  }
  return 1;
}

void quitShell() {
    exit(0);
}

void help() {
	printf("**************************************\n");
	printf("*                                    *\n");
	printf("*          LINUX - SHELL.c           *\n");
	printf("* NETWORK PROGRAMMING - Assignment 1 *\n");
	printf("*                                    *\n");
	printf("**************************************\n");
}

int createChild(int fromOp, char *fromPtr, int toOp, char *toPtr, int pipecmd, char* command_tokens[], int p[], int q[], int r[]){
    pid_t pid,id;
    int status,fd1;

    //CD
    if(strcmp(command_tokens[0], "cd")==0) {
    	cd(command_tokens);
    	return 1;
    }

    //QUIT
    if(strcmp(command_tokens[0], "quit")==0) {
    	quitShell();
    	return 0;
    }

    if ((pid = fork ()) < 0) {
        perror ("Error in Forking");
        return 1;
    } else if (pid == 0) {
       // printf ("Executing %s\n", command_tokens[0]);
    	
    	// OP = >
        if(toOp == 1) {
        	fd1 = open(toPtr, O_CREAT|O_WRONLY, 0666);
        	dup2(fd1, 1);
        }

        // OP = >>
        else if (toOp == 2) {
            fd1 = open(toPtr, O_CREAT|O_APPEND|O_WRONLY, 0666);
            dup2(fd1, 1);
        }

        // OP = |
        else if(toOp==3) {
        	dup2(q[1],1);
        	if(pipecmd!=1)
        		close(p[0]);
        }
        
        // OP = <
        if (fromOp == 1) {
            fd1 = open(fromPtr, O_RDONLY);
            dup2(fd1, 0);
        }
        
        if (pipecmd==1) {
        	dup2(p[0], 0);         	
        }

        execvp(command_tokens[0], command_tokens);
        perror ("Error in execvp");
		return 1;
    
    } else if (pid > 0) {
        id = wait (&status);
        
        if(toOp==3){
        	close(q[1]);
        }

		printExitStatus(status,id);
		if (WIFEXITED (status))
    		return 0;
		else
        	return 1;
    }

}

void printExitStatus(int status, int process_id) {
    if (WIFEXITED (status)) // NORMAL TERMINATION
        printf ("[Normal Termination] Child PID: %d, Exit Status: %d\n",process_id, WEXITSTATUS (status));
    else if (WIFSIGNALED (status)) // SIGNAL
        printf ("[Signal] Child PID: %d, SigNo: %d \n", process_id, WTERMSIG(status));
    else if (WIFSTOPPED (status))
        printf ("[Stop Signal] Child PID: %d SigNo: %d\n",process_id, WSTOPSIG (status));
}


int main() {
    char *copy;
    int counter = 0;
    pid_t pid[50];
    pid_t id;
    int process_no = 0, status;
    const char delim[] = " \t\r\n\v\f";
    char cwd[MAX_DIRECTORY_BUF];
    help();
    while(1){
    	getcwd(cwd, sizeof(cwd));
        printf("[ASH]%s > ", cwd);
        int children = 0;
        int n=0;
        char input[INPUT_BUFFER_SIZE];
        fgets(input, INPUT_BUFFER_SIZE, stdin);
        input[strlen(input)-1] = '\0';
        int p[2], q[2], r[2], s[2];
        pipe(p);
        pipe(q);
        pipe(r);
        pipe(s);

        char op[MAX_OPERATOR_BUF_SIZE][4];
        memset(op, 0, sizeof(op));          
        
        char commands[N_MAX_CMD][MAX_CMD_SIZE];    
        memset(commands, 0, sizeof(commands));               
        
        int i=0, j=0, k=0, start=0, end=0;        
        //k is for com counter,j is for delim counter
        
        while(input[i] != '\0' && input[i]!='\n') {
            if(input[i]=='<' || input[i]=='>' || input[i]=='|' || input[i]==','){
                end = i;

                if(input[i+1] != '\0' && input[i+1] == '>') {
                    strcpy(op[j++],">>");
                    i++;
                }

                else if(input[i+1] != '\0' && input[i+1] == '|') { 
                    if(input[i+2] != '\0' && input[i+2]=='|'){
                        strcpy(op[j++],"|||");
                        i += 2;
                    }
                    else{
                        strcpy(op[j++],"||");
                        i++;
                    }
                }

                else {
                    op[j++][0] = input[i];;
                    op[j][1] = '\0';
                }

                int a,b;
                for (a = start, b = 0; a < end; a++,b++) {
                    commands[k][b] = input[a];
                }
                k++;
                start = i + 1;
            }
            i++;
        }

        int a,b;
        for (a = start, b = 0; a < strlen(input); a++,b++) {
            commands[k][b] = input[a];
        }

        // PRINT CONTENTS OF commands
        // int temp=0;
        // printf("CONTENTS OF commands:\n");
        // while(*commands[temp]) {
        //     printf("%d %s\n", temp, commands[temp]);
        //     temp++;
        // }
        

        int op_num = j;
        int op_counter = 0;
        int com_num = k;
        int pipeOp_i = 0;
        int pipecmd = 0;
        int pipe_flag = 0;
        int char_read = 0;
        int char_write, id; //pipecmd->'take input from pipe'
        
        i=0;
// Command tokenization
        int token_no = 0;
        char *command_tokens[100];
        memset(command_tokens, 0, sizeof(command_tokens));
        command_tokens[token_no] = strtok(commands[0],delim);
        while(command_tokens[token_no] != NULL){
             token_no++;
             command_tokens[token_no] = strtok(NULL,delim);
        }

        //PRINT CONTENTS OF command_tokens
        // int temp=0;
        // printf("CONTENTS OF command_tokens:\n");
        // while(command_tokens[temp]) {
        //     printf("%d %s\n", temp, command_tokens[temp]);
        //     temp++;
        // }

// Command tokenization -end-


        int flag = 0;
        int pipe_count = 0;
        memset(buf, 0, sizeof(buf));
        
	while(pipeOp_i!=-1) { 
		dup2(q[0],p[0]);
		dup2(q[1],p[1]);
		pipe(q);
		pipe(r);

		if(flag == 1) {
			pipecmd = 1;
			i = pipeOp_i+1;
			id = fork();

			//CHILD
			if(id == 0) {
				if(pipe_count > 1){
					char_read = read(p[0], buf, MAX_BUF_SIZE);
            		buf[char_read+1] = '\0';
            	}
            	char_write = write(r[1], buf, char_read);
            	char_write = write(s[1], buf, char_read);
            	close(r[1]);
            	close(s[1]);
            	exit(0);
            }

            //PARENT
            else if(id>0){
            	id = wait (&status);
            	if(pipe_count!=1){
            		char_read = read(s[0], buf, MAX_BUF_SIZE);
            		close(r[1]);
            		close(p[0]);
					dup2(r[0],p[0]);
					close(p[1]);
					dup2(r[1],p[1]);		
				}
				pipe_count=0;
				//printf("parent executed");
            }
		}
		
        pipeOp_i=-1;
        if(op_num == 0)
        	children = createChild(0, NULL, 0, NULL, pipecmd, command_tokens, p, q, r);
        else{
        	int j=0, fromOp=0, toOp=0, fromOp_i=0, toOp_i=0, op_comma=0;			
            //fromOp-> '<'
            //toOp-> 1-'>', 2-'>>', 3-'|'

        	while(i < op_num){
        		if(op[i][j] == '<') {
        			fromOp = 1;
        			fromOp_i = i;
        		}
        		else if(op[i][j] == '>') {
        			if(op[i][j+1] == '>')
        				toOp = 2;
        			else
        				toOp = 1;
        			toOp_i = i;
        		}
        		else if(op[i][j] == '|'){
        			toOp = 3;
        			pipe_flag = 1;
        			pipeOp_i = i;
        			if(op[i][j+1] == '|'){
        				if(op[i][j+2] == '|')
        					pipe_count = 3;
        				else
        					pipe_count=2;
        			}
        			else
        				pipe_count=1;
        			break;
        		}

        		else if(op[i][j]==','){
        			pipecmd=1;
        			pipeOp_i=i;
        			break;
        		}
        			
        		i++;
        	}

        	char *fromPtr,*toPtr;
        	if(fromOp==1)
        		fromPtr = strtok(commands[fromOp_i+1], delim);
        	else
        		fromPtr = NULL;
        		
        	if(toOp==1||toOp==2)
        		toPtr = strtok(commands[toOp_i+1], delim);
        	else
        		toPtr = NULL;
        	
        	//printf("fromOp=%d,toOp=%d,pipecmd=%d,pipeOp_i=%d,file1=%s,file2=%s\n,pipe_count=%d",fromOp,toOp,pipecmd,pipeOp_i,fromPtr,toPtr,pipe_count);	
        	children = createChild(fromOp, fromPtr, toOp, toPtr, pipecmd, command_tokens, p, q, r);
    	}      

        token_no = 0;
        memset(command_tokens, 0, sizeof(command_tokens));
        command_tokens[token_no]  = strtok(commands[pipeOp_i+1],delim);
        // printf("this is : %s\n",command_tokens[token_no]);
        while(command_tokens[token_no] != NULL){
             //printf("this is : %s\n",command_tokens[token_no]);
             token_no++;
             command_tokens[token_no] = strtok(NULL,delim);
        }
//      Command tokenization ends
		flag=1;
	}

    }
}




