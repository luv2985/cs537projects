#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256
#define PIPE_SIZE 100


int hist_flag = 0;

int shellvartot = 0;
int shellvarcnt = 0;
char* shellvars[100];
char* shellvals[100];

int hist_size = 5;
char** hist;

/*
 * replace the incoming string with its correct variable
 */
char* replace_var (char* rep_n)
{
	char* cp_rep_n = strdup(rep_n);
	int flag = 0;
	if (cp_rep_n[0] == '"')
	{
		flag = 1;
		cp_rep_n[strlen(cp_rep_n) - 1] = '\0';
		cp_rep_n = cp_rep_n + 1;
	}

	char* env_value = getenv(cp_rep_n + 1);
	if (env_value != NULL)
	{
		if (flag == 1)
		{
			return env_value;
		}
		else
		{
			char tempstr[256];
			sprintf(tempstr, "\"%s\"", env_value);
            return strdup(tempstr);
		}
	}

	for (int i = 0; i < shellvarcnt; i++)
	{
		if (strcmp(shellvars[i], cp_rep_n + 1) == 0)
		{
			if (flag == 0){
				return shellvals[i];
			}
			else
			{
				char tempstr[256];
				sprintf(tempstr, "\"%s\"", shellvals[i]);
				return strdup(tempstr);
				//printf("the command was: %s", rep_n);
			}
		}
	}
	
	return rep_n;
}

/*
 * storing an item into history
 */
void hist_store (char* input)
{
	if (hist != NULL && (hist[0] == NULL || strcmp(hist[0], input) != 0))
	{
		char* tempplace = strdup(input);
		char* temp = strdup(input);
		for (int i = 0; i < hist_size; i++)
		{
			if (hist[i] != NULL)
			{
				temp = strdup(hist[i]);
				hist[i] = strdup(tempplace);
				tempplace = strdup(temp);

			}
			else
			{
				hist[i] = strdup(tempplace);
				break;
			}
		}
	}
	//else { printf("that was ur last command, stupid\n"); }
}

/*
 * running or checking for builtins to run
 * return 0 if there are no builtins ran, return 1 builtins where ran
 */
int runbuiltins (int argc, char** argv)
{
	if (strcmp(argv[0], "cd") == 0) // cd builtin
	{
		if (chdir(argv[1]) < 0)
		{
			perror("change directory failed");
			return -1;
		}
		return 1; // successful cd
	}
	else if (strcmp(argv[0], "export") == 0) // export env variable builtin
	{
		char* var = strsep(&argv[1], "=");
		char* val = strsep(&argv[1], "=");
		if (setenv(var, val, 1) < 0)
		{
			perror("set environment variable failed");
			return -1;
		}
		return 1;
	}
	else if (strcmp(argv[0], "local") == 0) // create local variable
	{
		char* var = strsep(&argv[1], "=");
		char* val = strsep(&argv[1], "=");
		//printf("%s = %s", var, val);
		int mv_flag = 0;
		int totiter = shellvarcnt;
		for (int i = 0; i < totiter; i++)
		{
			if (mv_flag == 1)
			{
				shellvars[i - 1] = shellvars[i];
				shellvals[i - 1] = shellvals[i];
			}
			else if (strcmp(shellvars[i], var) == 0)
			{
				mv_flag = 1;
				shellvarcnt--;
			}
			
			
		}
		if (mv_flag == 0)
		{
			shellvars[shellvarcnt] = var;
			shellvals[shellvarcnt] = val;
			shellvarcnt++;
		}
		return 1;
	}
	else if (strcmp(argv[0], "vars") == 0)
	{
		for (int i = 0; i < shellvarcnt; i++)
		{
			printf("%s=%s\n", shellvars[i], shellvals[i]);
		}
		return 1;
	}
	/*else if (strcmp(argv[0], "echo") == 0)
	{
		char echo_out[BUFFER_SIZE] = "\0";
		strcat(echo_out, argv[1]);
		//built in echo command
		for (int i = 2; i < argc; i++)
		{
			strcat(echo_out, " ");
			strcat(echo_out, argv[i]);
		}
		printf("%s\n", echo_out);

		char store_to_hist[256] = "echo ";
		strcat(store_to_hist, echo_out);
		hist_store(strdup(store_to_hist));
		echo_out[0] = '\0';
		return 1;
	}*/
	
	return 0;
}


/*
 * exec single command
 */
int exec_arg (char** argv)
{
	pid_t pr = fork();
	
	if (pr == 0)
	{
		/*if (strcmp(argv[0], "bash") == 0 && strcmp(argv[1], "-c") == 0)
		{
			char* args[] = {"bash", "-c", "\"$myenvvar\""};
			if (execvp("bash", args) == -1)
			{
		    	printf("execvp: No such file or directory for bash\n");
		       	exit(-1);
			}
		}
		else*/ if (execvp(argv[0], argv) < 0)
		{
			printf("execvp: No such file or directory\n");
			exit(-1);
		}
		return 0;
	}
	else
	{
		int status;
		waitpid(pr, &status, 0);
	}
	return 0;
}

/*
 * exec a pipe
 */
int exec_args_pipe (int argc, char*** argv)
{
	// create array of pipes
	int pipefds[argc - 1][2];
	pid_t* prs = (pid_t*) malloc(argc * sizeof(pid_t));
	if (prs == NULL)
	{
		perror("malloc failed");
		exit(-1);
	}

	for (int i = 0; i < argc - 1; i++)
	{
		if (pipe(pipefds[i]) < 0)
		{
			perror("failed creating pipes");
			exit(-1);
		}
	}

	// fork child processes
	for (int cmdcnt = 0; cmdcnt < argc; cmdcnt++)
	{
		// FORK
		//printf("forking processes\n");
		prs[cmdcnt] = fork();

		if (prs[cmdcnt] < 0) // check forked correctly
		{
			perror("failed fork");
			exit(-1);
		}
		else if (prs[cmdcnt] == 0) // child process
		{
			
			if (cmdcnt > 0)
			{
				//close(pipefds[cmdcnt - 1][1]);
				// dup read port
				if (dup2(pipefds[cmdcnt - 1][0], 0) < 0)
				{
					perror("dup2 failed while not first command");
					exit(-1);
				}
				close(pipefds[cmdcnt - 1][0]);
			}

			// if not last command
			if (cmdcnt + 1 < argc)
			{
				//close(pipefds[cmdcnt][0]);
				// dup the write port
				if (dup2(pipefds[cmdcnt][1], 1) < 0)
				{
					perror("dup2 failed while not in last command");
					exit(-1);
				}
				close(pipefds[cmdcnt][1]);
			}

			for (int i = 0; i < argc - 1; i++)
			{
                close(pipefds[i][0]);
                close(pipefds[i][1]);
            }
			
			printf("executing system call on");
			// execute sys call
			if (execvp(argv[cmdcnt][0], argv[cmdcnt]) < 0)
			{
				printf("execvp: No such file or directory\n");
				exit(-1);
			}
		}
	}
	
	// close parent pipes
	for (int i = 0; i < argc - 1; i++) 
	{
		close(pipefds[i][0]); close(pipefds[i][1]);
		/*
		printf("closing parent pipes\n");
		if (i != 0) { close(pipefds[i][0]); close(pipefds[i][1]); } // close next pipe
		if (i + 1 != argc - 1) { close(pipefds[i - 1][0]); close(pipefds[i - 1][1]); } // close prev pipe
		*/
	}
	
	// wait children
	for (int i = 0; i < argc; i++)
	{
		//printf("waiting child process\n");
		int status;
		waitpid(prs[i], &status, 0);
		/*if ( WIFEXITED(status) )
		{
        int exit_status = WEXITSTATUS(status);        
        printf("Exit status of the child was %d\n", exit_status);
		} else { printf("abnormal exit"); }*/
	}
	return 0;	
}

/*
 * history builtin commands
 */
char* resolve_hist_cmd(char* cmd)
{
	// using token to find the rest of the command
	char* tok_argone = strsep(&cmd, " ");
	
	if (tok_argone == NULL) // print history
	{
		hist_flag = 1;

		if (hist != NULL)
		{
			for (int i = 0; i < hist_size; i++)
			{
				if (hist[i] != NULL) { printf("%d) %s\n", i + 1, hist[i]); }
			}
		}
		return NULL;
	}

	char* tok_argtwo = strsep(&cmd, " ");
	if (tok_argtwo == NULL) // run recalled command
	{
		int arg_one = atoi(tok_argone);
		if (arg_one < hist_size && arg_one > 0)
		{
			return hist[arg_one];
		}
	}
	else if (strcmp(tok_argone, "set") == 0) // resize history
	{
		hist_flag = 1;

		// realloc history
		int arg_two = atoi(tok_argtwo);
		int prevsize = hist_size;

		for (int i = arg_two; i < hist_size; i++)
		{
			hist[i] = NULL;
		}

		hist_size = arg_two;
		hist = (char**) realloc(hist, hist_size * sizeof(char*));
		
		for (int i = prevsize; i < hist_size; i++)
		{
			hist[i] = NULL;
		}
		/*if (hist == NULL)
		{
			printf("Memory reallocation failed\n");
			exit(-1);
		}*/
		return NULL;
	}
	return NULL;
}

/*
 * parses a single command string into constituant parts
 */
int parscmd (char* command, char** parsedcommand)
{
	if (command == NULL) {
        return -1;
    }

	// rid of initial space
	if (command[0] == ' ') { command = command + 1; }
	
    char* token = strsep(&command, " ");

	if (strcmp(token, "echo") == 0)
    {
        parsedcommand[0] = strdup("echo");
		if (command[0] != '$')
		{
			parsedcommand[1] = strdup(command);
		}
		else
		{
			parsedcommand[1] = strdup(replace_var(command));
		}
		parsedcommand[2] = NULL;

		return 2;
    }

    int i = 0;
    // Keep retrieving tokens until none are left
    while (token != NULL && token[0] != '\0') {
		if (token[0] == '$' || (token[0] == '\"' && token[1] == '$'))
		{
			token = strdup(replace_var(token));
		}

        // Store the token in the array
        parsedcommand[i] = token;
        i++;

        token = strsep(&command, " ");
    }
	parsedcommand[i] = NULL;

    return i;
}

/*
 * main shell loop
 */
int mainf (char** batchfile)
{
	int batchfile_index = 0;
	while(1)
	{
		hist_flag = 0;
		char* cmd_in = NULL;
		size_t bs = BUFFER_SIZE;
		ssize_t read;

		// GET NEXT COMMAND
		if (batchfile == NULL)
		{
			/* INTERACTIVE MODE */
			printf("wsh> ");
			if ((read = getline(&cmd_in, &bs, stdin)) < 0)
			{
				//perror("getline failed");
				//exit(-1);
			}
		}
		else
		{
			/* BATCH MODE */
			cmd_in = strdup(batchfile[batchfile_index]);
			read = strlen(cmd_in);
			batchfile_index++;
		}
		
		// get rid of \n
		if (cmd_in[read - 1] == '\n') { cmd_in[read - 1] = '\0'; }
		char* cmd_in_cp = strdup(cmd_in); // static var
		char* cmd_in_h = strdup(cmd_in);

		// builtin exit command
		if (strncmp(cmd_in, "exit", 4) == 0) { exit(0); }

		// PARSE HISTORY
		char* token_h = strsep(&cmd_in_h, " ");
		if (strcmp(token_h, "history") == 0) /* RESOLVE HISTORY COMMAND */
		{
			char* replace_cmd = resolve_hist_cmd(cmd_in_h);
			if (replace_cmd != NULL) { cmd_in = replace_cmd; } /* REPLACE IF WE NEED TO */
		}
		/*else if (strcmp(token_h, "echo") == 0)
		{
			char* echo_cmd[3];
			echo_cmd[0] = strdup("echo");
			echo_cmd[1] = strdup(cmd_in + 5);
			echo_cmd[2] = NULL;
			exec_arg(echo_cmd);
			hist_store(cmd_in_cp);
			hist_flag = 1;
		}*/


		// PARSE INPUT
		if (hist_flag == 0)
		{

		char* token = strsep(&cmd_in, "|");
		int tokencnt = 0;
		char** parsedcmd[20];
		// alloc space
		for (int i = 0; i < 20; i++) {
			parsedcmd[i] = (char **)malloc(sizeof(char *));
			if (parsedcmd[i] == NULL) {
				perror("Memory allocation failed");
				exit(EXIT_FAILURE);
			}
        }

		int firstlen = 0;
        while (token != NULL && token[0] != '\0') {
			// calls parse command
			int templen = parscmd(strdup(token), parsedcmd[tokencnt]);
			if (templen < 0)
			{
				perror("parse command failed");
				exit(-1);
			}

			if (tokencnt == 0) { firstlen = templen; }

			tokencnt++;
			token = strsep(&cmd_in, "|");
		}
		
		// EXECUTE CMD
		// if tokkencnt > 1, pipe, otherwise, check builtins, and then execvp
		if (tokencnt > 1)
		{
			exec_args_pipe(tokencnt, parsedcmd);
			hist_store(cmd_in_cp);
		}
		else if (runbuiltins(firstlen, parsedcmd[0]) == 0) // if builtins miss, then run execvp on single command
		{
			exec_arg(parsedcmd[0]);
			hist_store(cmd_in_cp);
		}
		
		// free parsed command	
		for (int i = 0; i < 20; i++)
		{
		//	free(parsedcmd[i]);
		}
		
		}
	}
}


/* 
 * main function dealing with initial inputs
 * 
 */
int main (int argc, char * argv[])
{
	// init history
	hist = (char**) malloc(hist_size * sizeof(char*));

	if (argc > 2)
	{
		printf("Usage: ./wsh or ./wsh INPUTFILE.sh");
		exit(-1);
	}
	else if (argc == 1)
	{
		mainf(NULL);
		// interactive cmd line
	}
	else
	{
		// open bash cmd file and feed commands into mainf
		FILE* file = fopen(argv[1], "r");
	    if (!file) {
			 perror("Could not open file\n");
		     exit(-1);
		}
		
		// Reset file pointer
		fseek(file, 0, SEEK_SET);

		// Allocate memory for an array of strings (char*)
		char** lines = (char**) malloc(100 * sizeof(char*));
		if (!lines) {
			perror("Memory allocation failed\n");
			exit(-1);
		}

		char lineBuffer[256];

		int lineIndex = 0;
		while (fgets(lineBuffer, 256, file) != NULL)
		{
			//printf("%s\n", lineBuffer);
			lines[lineIndex] = strdup(lineBuffer);
			lineIndex++;
		}
		//printf("%d\n", lineIndex);
		//printf("%s\n", lines[0]);

		mainf(lines);

		fclose(file);


	}
	free(hist);
}
