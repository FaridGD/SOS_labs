#include<stdio.h>
#include<stdbool.h>
#include<string.h>

int main(int argc, char* argv[])
{
	char* pattern;
	bool error = false;
	bool any = false;
	bool show = (argc - 2) > 1;

	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s PATTERN [FILE...]\n", argv[0]);
		return 2;
	}
	pattern = argv[1];

	if(argc > 2)
	{
		for(int i = 2; i < argc; i++)
		{
			FILE * file = fopen(argv[i], "r");
			if(file == NULL)
			{
				error = true;
				perror(argv[i]);
				continue;
			}

			char buffer[1024];
			while(fgets(buffer, sizeof(buffer), file) != NULL)
			{
				if(strstr(buffer, pattern) != NULL)
				{
					any = true;
					if(show)
					{
						fputs(argv[i], stdout);
						fputc(':', stdout);
					}
					fputs(buffer, stdout);
					
				}
			}
			fclose(file);
		}
	}
	else
	{
		char buffer[1024];
		while(fgets(buffer, sizeof(buffer), stdin) != NULL)
		{
			if(strstr(buffer, pattern) != NULL)
			{
				any = true;
				fputs(buffer, stdout);
			}
		}
	}
	if(error)
	{
		return 2;
	}
	return any ? 0 : 1;
}
