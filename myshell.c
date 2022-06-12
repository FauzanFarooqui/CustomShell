/*
Fauzan Nayeem Farooqui
BT19CSE029
2/9/2021
Course: Operating Systems
Assignment: 1
//Note: The template is exactly as given, so no extra functions were used. Control flow in main() to redirect 
	//	to command executer is also retained.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()

char** parseInput(char* command, int* ntoks, char* com_type)
{ 
	// This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
	char** tokens = (char**)malloc(sizeof(char*)*256); //256 - MAXSIZE, same as main()
    int i, token_index = 0, flag=0;
    char command_type; 
    
	if(command[0]=='&' || command[0]=='#' || command[0]=='>') //these tokens shouldn't appear as first argument.
    {
		command_type = 'i'; //incorrect input
		flag = 1;
	}
    else if(command[0]==NULL) //the apparent warning given here can be safely ignored, as it's the only way to serve the purpose while keeping it safely working
    {
		command_type = 'e'; //empty
		flag = 1;
	}
	else
	for(i = 1; i<strlen(command); i++) 	//skipping first character as already checked and shouldn't be any of these tokens atleast
		if(command[i]=='&' && command[i+1]=='&')
			if(flag==0 || command_type=='p') 
			{
				command_type = 'p';
				flag = 1;
			}
			else	//i.e the command should not have already been detected as any other type, or else it would be incorrect format 
				flag = 2;

		else if(command[i]=='#' && command[i+1]=='#')
			if(flag==0 || command_type=='s') 
			{
				command_type = 's';
				flag = 1;
			}
			else	
				flag = 2;
		else if(command[i]=='>')
			if(flag==0 || command_type=='p') 
			{
				command_type = 'o';
				flag = 1;
			}
			else	
				flag = 2;

	if(flag!=1)		
		if(flag==0) 
		{
			command_type = 'c';
			flag = 1;
		}
		else
			command_type = 'i';


	while ( (*(tokens+token_index) = strsep(&command," ")) != NULL)
		token_index++;
	
    *ntoks = token_index; //token_index already incremented by 1 before exiting loop, it's the number of tokens in the command.
	*com_type = command_type;
    return tokens;

}

void executeCommand(char** tokens)
{	// This function will fork a new process to execute a command
	if(strcmp(tokens[0], "cd")==0)
	{
		if(chdir(tokens[1])==-1)
        	printf("Shell: Incorrect command\n");
		return;
	}
	int rc = fork();
	if(rc<0)  //fork failed
		exit(0);
	else if(rc>0) //parent process - waiting for child to finish executing or detect status change (for SIGTSTP)
			waitpid(rc, NULL, WUNTRACED); //normal wait() cannot handle/detect SIGTSTP. However, waitpid() detects even process changes and returns.
	else			//child process, wherein we are executing the actual command
	{	
		signal(SIGINT, SIG_DFL);	// Handling ^C signal in child process
    	signal(SIGTSTP, SIG_DFL); //Here, we may handle normally BUT parent process detects this process change and finishes it.  
		
		if(execvp(tokens[0], tokens) == -1)	//returns -1 if something went wrong. or else executes safely
		{
			printf("Shell: Incorrect command\n");
			exit(1);
		}
	}
}

void executeParallelCommands(char** tokens, int ntoks)
{
	// This function will run multiple commands in parallel
	int rc, i, sub_com_i=0; //latter is the index of the first token of the current sub-command
	
	for(i=0; i<=ntoks; i++)
	{
		if(i==ntoks || strcmp(tokens[i], "&&")==0)
		{	
			tokens[i]=NULL;
		
			 //Start command execution - almost same as executeCommand() function
				if(strcmp(tokens[sub_com_i], "cd")==0)
				{
					if(chdir(tokens[sub_com_i+1])==-1)
						printf("Shell: Incorrect command\n");
					sub_com_i=i+1; //next command starts after &&.
					continue;
				}
				rc = fork();
				if(rc<0)  //fork failed
					exit(0);
				else if(rc==0)			//child process, wherein we are executing the actual command
				{	
					signal(SIGINT, SIG_DFL);	// Handling ^C signal in child process
					signal(SIGTSTP, SIG_IGN); //Can't catch ^Z during parallel execution as we are not making the parent wait, or else it will become sequential execution.
					
					if(execvp(tokens[sub_com_i], &tokens[sub_com_i]) == -1)	//returns -1 if something went wrong. or else executes safely
					{
						printf("Shell: Incorrect command\n");
						exit(1);
					}
				}
			 //end command execution
			sub_com_i=i+1; //next command starts after &&
			 	
		}
	}
	
	wait(NULL); //makes sure all child processes have been waited for and terminated
	
}

void executeSequentialCommands(char** tokens, int ntoks)
{	
	// This function will run multiple commands in sequential order
	int i, sub_com_i=0; //latter is the sub-command token index
	char **sub_com_toks = (char**)malloc(sizeof(char*)*256); //256 - MAXSIZE, same as main(). This stores the tokens of 1 whole sub-command
	for(i=0; i<=ntoks; i++)
	{ //we purposefully make the loop evaluate at i=ntoks also - this iteration is to execute the final command, or else it executes only when encounterting th next ##
		if(i!=ntoks && strcmp(tokens[i], "##")!=0) //checking i!=ntoks as first condition is important, or else tokens[ntoks] will be evaluated, which is out of bounds
		{
			sub_com_toks[sub_com_i] = tokens[i];
			sub_com_i++;
		}
		else
		{
			sub_com_toks[sub_com_i]=NULL;
			executeCommand(sub_com_toks);
			sub_com_i=0;
		}
	}

	free(sub_com_toks);
}

void executeCommandRedirection(char** tokens, int ntoks)
{	// This function will run a single command with output redirected to an output file specificed by user
	int i, op_file_i=-1;
	for(i=0; i<ntoks && op_file_i==-1; i++)
		if(strcmp(tokens[i], ">")==0)
			op_file_i = i+1;		//find the index of the output file name token

	if(tokens[op_file_i]==NULL)
        printf("Shell: Incorrect command\n");
	else
	{
		tokens[i-1] = NULL; //we shouldn't pass the '>' token to execvp() as it's not an executable file/command.
		
		int rc = fork();
		if(rc<0)  //fork failed
			exit(0);
		else if(rc>0) //parent process - waiting for child to finish executing or detect status change (for SIGTSTP)
			waitpid(rc, NULL, WUNTRACED); //normal wait() cannot handle/detect SIGTSTP. However, waitpid() detects even process changes and returns.
		else			//child process, wherein we are executing the actual command
		{	
			signal(SIGINT, SIG_DFL);	// Handling ^C signal in child process
			signal(SIGTSTP, SIG_DFL); //Here, we may handle normally BUT parent process detects this process change and finishes it.
			
			close(STDOUT_FILENO);
			open(tokens[op_file_i], O_CREAT | O_WRONLY | O_APPEND);

			if(execvp(tokens[0], tokens) == -1)	//returns -1 if something went wrong. or else executes safely
			{
				printf("Shell: Incorrect command\n");
				exit(1);
			}
		}
	}
}

int main()
{
    signal(SIGINT, SIG_IGN);	// Ignore ^C signal, as we should only exit upon "exit" command
    signal(SIGTSTP, SIG_IGN);   // Ignore ^Z signal, as we should only exit upon "exit" command

	// Initial declarations
    size_t MAXSIZE = 256;
    char command_type;
    char* command = (char*)malloc(sizeof(char)*MAXSIZE); //256 array size
    char** tokens;
    char pwd[MAXSIZE]; //In instructions, this is named as currentWorkingDirectory. However, to shorten it and make it similar to the command, I named as pwd
	int i, ntoks, token_index=0;

	while(1)
	{
		getcwd(pwd, sizeof(pwd));
		printf("%s$", pwd); // Print the prompt in format - currentWorkingDirectory$   
	
		getline(&command, &MAXSIZE, stdin); // accept input with 'getline()'
		command[strlen(command)-1]='\0';	//getline() retains \n at end of string. This doesn't work with strstep(), for which we need null-terminated string.

		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
		tokens = parseInput(command, &ntoks, &command_type);
		
		if(command_type=='i') //Incorrect command
        {
            printf("Shell: Incorrect command\n");
            continue;
        } 
        else if(command_type=='e') //Empty command
            continue;
		else if(strcmp(tokens[0],"exit")==0)	// When user uses exit command.
		{
			printf("Exiting shell...\n");
			break;
		}
		
		if(command_type=='p')
		{
			executeParallelCommands(tokens, ntoks);		// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
			wait(NULL); //for any process that still runs outside of the function. "Reduces" the issue of output of last command being printed after prompt for next command while still keeping the parallel nature 
		}
		else if(command_type=='s')
			executeSequentialCommands(tokens, ntoks);	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
		else if(command_type=='o')
			executeCommandRedirection(tokens, ntoks);	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
		else                                //remaining type not checked for is only 'c', the single command.
			executeCommand(tokens);		// This function is invoked when user wants to run a single commands

	}

    free(command);
    return 0;
}

