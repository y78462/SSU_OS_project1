#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

#define STDIN_PIPE 0x1
#define STDOUT_PIPE 0x2

#define EXIT_CODE 1
int exec_command(char **command, int pipefd[2], int flags);

/* Splits the string by space and returns the array of tokens
*
*/
//ttop, pps는 ./ttop ./pps로 실행할수있으므로 execvp시 주의
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t' || readChar =='\r'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;

	pid_t child_pid;//자식 프로세스 아이디
	int status; //종료상태 리턴받는 변수
	int ret_val; //exec -> return value
	int pipeloc[5]; //파이프위치저장배열

	//배치파일이 있으면 읽어서 오픈
	FILE* fp;
	if(argc == 2) {
		fp = fopen(argv[1],"r");
		if(fp < 0) {
			printf("File doesn't exists.");
			return -1;
		}
	}

	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		if(argc == 2) { // batch mode
			if(fgets(line, sizeof(line), fp) == NULL) { // file reading finished
				break;	
			}
			line[strlen(line) - 1] = '\0';
		} else { // interactive mode
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}
//		printf("-----Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);
   
       //do whatever you want with the commands, here we just print them

		int pipe_num=0; //pipe 개수
		int token_num=0; //token 개수
		for(i=0;tokens[i]!=NULL;i++){
		  token_num++;
//			printf("-----found token %s (remove this debug output later)\n", tokens[i]);
			if(strcmp(tokens[i],"|")==0)
			{
			  pipeloc[pipe_num] = i;
			  pipe_num++;
			}
		}
//		printf("-----num tokens : %d\n",token_num);
//		printf("-----num pipe : %d\n\n",pipe_num);

		if(token_num ==0)
		  continue;


		//파이프 없을 때 
		if(pipe_num == 0)
		{
		  
		  //명령어 실행
			if((child_pid = fork())<0)
			{//error
		  		fprintf(stderr,"fork error\n");
		  		continue;
			}
			else if(child_pid ==0)
			{
		  	//자식 프로세스가 할일
	          //pps, ttop인지 확인해서 맞으면 token[0]에 "./" 붙여줌 
			  if(strcmp(tokens[0],"pps")==0 || strcmp(tokens[0],"ttop")==0)
			  {
				char buf[MAX_TOKEN_SIZE];
				bzero(buf,sizeof(buf));
				sprintf(buf,"./%s",tokens[0]);
				strcpy(tokens[0],buf);
				//printf("tokens[0] : %s\n",tokens[0]);
			  }
			  if(execvp(tokens[0],tokens)<0)
			  {
				fprintf(stderr,"SSUShell : Incorrect command\n");
				continue;
			  }
			}
			else
			{//부모가 할일
		  		if( wait(&status)!= child_pid)
		  		{
					//fprintf(stderr,"wait error\n");
		  		}
			}
		}
		else
		//파이프 있을 때
		{
	          //pps, ttop인지 확인해서 맞으면 token[0]에 "./" 붙여줌 
			  if(strcmp(tokens[0],"pps")==0 || strcmp(tokens[0],"ttop")==0)
			  {
				char buf[MAX_TOKEN_SIZE];
				bzero(buf,sizeof(buf));
				sprintf(buf,"./%s",tokens[0]);
				strcpy(tokens[0],buf);
				//printf("tokens[0] : %s\n",tokens[0]);
			  }
		  
		  char ***cmdpipe = (char ***)malloc(sizeof(char**) * (pipe_num+1));
		  for(i=0;i<=pipe_num;i++){
		  	*(cmdpipe+i) = (char **)malloc(sizeof(char *)*5);
		  }
		  int k=0, l=0;
		  //명령어를 파이프 기준 앞, 뒤로 자르기부터
		  for(i=0;i<token_num;i++)
		  {
			if(strcmp(tokens[i],"|") != 0)
			{
			  //not pipe
			  *(*(cmdpipe+k)+l) = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
			  strcpy(*(*(cmdpipe+k)+l),tokens[i]);
//			  printf("-----cmdpipe[%d][%d]=%s\n",k,l,*(*(cmdpipe+k)+l));
			  l++;
			}
			else
			{//pipe만남
			  k++;
			  l=0;
			}
		  }

		  int r;
		  int error_flag=0;
		  //printf("\n");
		  int pipefd[pipe_num][2];
		  for(i=0;i<pipe_num; i++)
		  {
			if(pipe(pipefd[i])<0)
			{
			  fprintf(stderr,"pipe error\n");
			  break;
			}
		  }
		  //첫번째 명령어
		 if((r= exec_command(*(cmdpipe),pipefd[0],STDOUT_PIPE))<0)
		 {
		   printf("SSUShell : Incorrect command\n");
		   error_flag =-1;
		   break;
		 }
		 close(pipefd[0][1]);
		 if(error_flag ==-1) {
	     	fprintf(stderr,"SSUShell : Incorrect command\n");
			break;
		  }
		 //중간 명령어 반복
		  for(i=1;i<pipe_num;i++)
		  { 
			int temp_pipefd[] = {pipefd[i-1][0],pipefd[i][1]};
			if((r=exec_command(*(cmdpipe+i),temp_pipefd,STDIN_PIPE|STDOUT_PIPE))<0)
		    {
		     printf("SSUShell : Incorrect command\n");
			 error_flag = -1;
		     break;
		    }
		  }
		  if(error_flag ==-1) {
	     	//fprintf(stderr,"4  SSUShell : Incorrect command\n");
			break;
		  }
		  //마지막 명령어
		  if((r=exec_command(*(cmdpipe+pipe_num),pipefd[pipe_num-1],STDIN_PIPE))<0)
		 {
		   printf("SSUShell : Incorrect commandd\n");
		   error_flag = -1;
		   break;
		 }
		  if(error_flag ==-1)
		  {
		    //fprintf(stderr,"6  SSUShell : Incorrect command\n");
			break;
		  }
		  close(pipefd[pipe_num-1][0]);
		  for(i=0;i<pipe_num-1;i++)
		  {
			close(pipefd[i][0]);
		  }
		  for(i=1;i<pipe_num;i++)
		  {
			close(pipefd[i][1]);
		  }
		  int wstatus;
		  while(wait(&wstatus)>0);

		  for(i=0;i <= pipe_num; i++)
			for(k=0;k<5;k++)
			  free (*(*(cmdpipe+i)+k));
		  for(i=0;i<=pipe_num;i++)
			free(*(cmdpipe+i));
		  free(cmdpipe);
		}//the end 
		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}//while end
	return 0;
}

int exec_command(char **command, int pipefd[2], int flags)
{
  pid_t cpid = fork();
  if(cpid>0)
	return 0;
  else if(cpid<0)
	perror("fork");

  if(flags & STDIN_PIPE)
	if(dup2(pipefd[0],STDIN_FILENO)<0)
	  perror("dup2");
  if(flags & STDOUT_PIPE)
	if(dup2(pipefd[1],STDOUT_FILENO)<0)
	  perror("dup2");

  close(pipefd[0]);
  close(pipefd[1]);

  if(execvp(command[0],command)<0)
  {
	//fprintf(stderr,"EXEC ::: SSUShell : Incorrect command\n");
	return -1;
  }
  return 1;
}

