#include <stdio.h>

//Input Buffer
static char input[2048];

int main(int argc, char** argv)
{

	//Version and Exit info
	puts("Lispy Version 0.0.0.1");
	puts("Press Ctrl+z to Exit");

	//It's the loop that never ends 
	while (1) 
	{
		fputs("lispy> ", stdout);

		fgets(input, 2048, stdin);

		printf("No you're a %s", input);
	}

	return 0;
}