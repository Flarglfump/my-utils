#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
	if (argc == 2)
	{
		if (strcmp(argv[1], "home") == 0)
		{
			system("ssh gwitske@home.cs.siue.edu");
		}
		else if (strcmp(argv[1], "os") == 0)
		{
			system("ssh gwitske@os.cs.siue.edu");
		}
		else
		{
			printf("Error: unrecognized ssh destination\n");
		}
	}
	else if (argc == 1)
	{
		system("ssh gwitske@home.cs.siue.edu");
	}
	else
	{
		printf("Error: command only takes 2 arguments\n");
	}

	return 0;
}
