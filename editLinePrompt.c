#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>


int main(int argc, char** argv)
{
	puts("Lispy Version 0.0.0.2");
	puts("Now with editline");

	while (1)
	{
		//Output prompt to the user and get input
		char* input = readline("lispy> ");

		//Add input to history
		add_history(input);

		printf("No you're a %s\n", input);

		//Free retrieved input
		free(input);
	}
	return 0;
}