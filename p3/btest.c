#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main ()
{
	char* argv[] = {"bash", "-c", "\"$myenvvar\"", NULL};
	if (execvp("bash", argv) < 0 )
	{
		printf("wrong");
		exit(-1);
	}
}
