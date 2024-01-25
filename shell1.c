#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#define MAX_LINE 80


char history[10][MAX_LINE];
int count = 0;


void displayHistory()
{
    
    printf("Shell command history:\n");
    
    int i;
    int j = 0;
    int histCount = count;
    
    for (i = 0; i<10;i++)
    {
        printf("%d.  ", histCount);
        while (history[i][j] != '\n' && history[i][j] != '\0')
        {	
            printf("%c", history[i][j++]);
        }
        printf("\n");
        j = 0;
        histCount--;
        if (histCount ==  0)
            break;
    }
    printf("\n");
} 



int get_Tok(char inputBuffer[], char *args[],int *background)
{
   	int length;
    	int i;     
    	int start;  
    	int ct = 0; 
    	int hist;

 	length = read(STDIN_FILENO, inputBuffer, MAX_LINE);// 0  if EOD reached , -1 if error ,otherwise number of bytes.
		
 
   
    start = -1;
    if (length == 0)
        exit(0);   //end of command
    if (length < 0)
    {
        printf("Command not read\n");
        exit(-1);  //terminate
    }
    
   //examine each character
    for (i=0;i<length;i++)
    {
        switch (inputBuffer[i])
        {
            case ' ':
            case '\t' :               
                if(start != -1)
                {
                    args[ct] = &inputBuffer[start];    
                    ct++;
                }
                inputBuffer[i] = '\0'; 
                start = -1;
                break;
                
            case '\n':               
                if (start != -1)
                {
                    args[ct] = &inputBuffer[start];
                    ct++;
                }
                inputBuffer[i] = '\0';
                args[ct] = NULL; 
                break;
                
            default :           
                if (start == -1)
                    start = i;
                if (inputBuffer[i] == '&')
                {
                    *background  = 1;
                    inputBuffer[i] = '\0';
                }
        }
    }
    
    args[ct] = NULL; //if the input line was > 80
if(args[0]==NULL) return -1;
if(strcmp(args[0],"cd")==0){
	if(chdir(args[1])!=0) printf("\nInvalid\n");
	else{
		char cwd[1024];
		if(getcwd(cwd,sizeof(cwd))!=NULL){
			printf("Current directory %s\n", cwd);
		}else{
			perror("getcwd");
		}	
	
	}
}
else if(strcmp(args[0],"exit")==0) exit(0);
else if(strcmp(args[0],"history")==0)
        {		
               if(count>0)
		{
		
                displayHistory();
		}
		else
		{
		printf("\nNo Commands in the history\n");
		}
		return -1;
        }

	else if (args[0][0]-'!' ==0)
	{	int x = args[0][1]- '0'; 
		int z = args[0][2]- '0'; 
		
		if(x>count) //second letter check
		{
		printf("\nNo Such Command in the history\n");
		strcpy(inputBuffer,"Wrong command");
		} 
		else if (z!=-48)
		{
		printf("\nNo Such Command in the history. Enter <=!9 (buffer size is 10 along with current command)\n");
		strcpy(inputBuffer,"Wrong command");
		}
		else
		{

			if(x==-15)//Checking for '!!',ascii value of '!' is 33.
			{	 strcpy(inputBuffer,history[0]);  // this will be your 10 th(last) command
			}
			else if(x==0) //Checking for '!0'
			{	 printf("Enter proper command");
				strcpy(inputBuffer,"Wrong command");
			}
			
			else if(x>=1) //Checking for '!n', n >=1
			{
				strcpy(inputBuffer,history[count-x]);

			}
			
		}
	}
 for (i = 9;i>0; i--) 
       	strcpy(history[i], history[i-1]);
    
    strcpy(history[0],inputBuffer);
    count++;
	if(count>10)
	{ count=10;
	}
}

void setup(char inputBuffer[], char *args[] ,int *background){
    	int pid,tpid;
   
        if(-1!=get_Tok(inputBuffer,args,background)) // get next command  
	{
		pid = fork();
        
        	if (pid < 0)//if pid is less than 0, forking fails
        	{
            
            		printf("Fork failed.\n");
            		exit (1);
        	}
        
       		 else if (pid == 0)//if pid ==0
        	{
            		
            		if (execvp(args[0], args) == -1 && strcmp(args[0],"history")!=0 && strcmp(args[0],"cd")!=0 && strcmp(args[0],"exit")!=0)
           	 	{	
		
                		printf("Error executing command\n");
            		}
       		 }
        
        	else
        	{
           	 	if (*background == 0)
           		 {
                		wait(NULL);
           		 }
        	}
   	 }
}


int main(void)
{
    char inputBuffer[MAX_LINE]; 
    int background; 
    char *args[MAX_LINE/2 + 1];
    int should_run =1;
    

   
    
    while (1) 
    {            
        background = 0; 
        printf("siuuu>");
        fflush(stdout);
	setup(inputBuffer, args, &background);
     }
}
