#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>

#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_CYAN "\033[36m"

int compare_names(const void *a, const void *b)
{
    const char *nameA = *(const char **)a;
    const char *nameB = *(const char **)b;
    return strcasecmp(nameA, nameB);
}

const char* get_file_color(mode_t mode)
{
    if (S_ISDIR(mode)) return COLOR_BLUE;
    if (S_ISLNK(mode)) return COLOR_CYAN;
    if (mode & S_IXUSR) return COLOR_GREEN;
    return COLOR_RESET;
}

void print_colored_name(const char* name, mode_t mode)
{
    const char* color = get_file_color(mode);
    printf("%s%s%s", color, name, COLOR_RESET);
}

typedef struct
{
	bool a;
	bool l;
} Fl;

void print_permissions(mode_t mode)
{
    printf((S_ISDIR(mode)) ? "d" : "-");
    printf((mode & S_IRUSR) ? "r" : "-");
    printf((mode & S_IWUSR) ? "w" : "-");
    printf((mode & S_IXUSR) ? "x" : "-");
    printf((mode & S_IRGRP) ? "r" : "-");
    printf((mode & S_IWGRP) ? "w" : "-");
    printf((mode & S_IXGRP) ? "x" : "-");
    printf((mode & S_IROTH) ? "r" : "-");
    printf((mode & S_IWOTH) ? "w" : "-");
    printf((mode & S_IXOTH) ? "x" : "-");
}

void print_file_info(const char *filename, const char *display_name)
{
    struct stat file_stat;

    if (stat(filename, &file_stat) != 0)
    {
        perror("lstat");
        return;
    }

    print_permissions(file_stat.st_mode);

    printf(" %lu", file_stat.st_nlink);

    struct passwd *pw = getpwuid(file_stat.st_uid);
    struct group *gr = getgrgid(file_stat.st_gid);
    printf(" %s %s", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");

    printf(" %ld", file_stat.st_size);

    char time_buf[20];
    struct tm *timeinfo = localtime(&file_stat.st_mtime);
    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", timeinfo);
    printf(" %s", time_buf);
    printf(" ");
    print_colored_name(display_name, file_stat.st_mode);
    printf("\n");
}

bool is_directory(const char* path)
{
	struct stat st;
	if (stat(path, &st) != 0)
	{
		return false;
	}
	return S_ISDIR(st.st_mode);
}

char* my_strdup(const char* str)
{
    size_t len = strlen(str) + 1;
    char* new_str = malloc(len);
    if (new_str)
    {
        memcpy(new_str, str, len);
    }
    return new_str;
}

void hpath(const char* path, Fl flags)
{
	long total_blocks = 0;
	if (!is_directory(path))
	{
		fprintf(stderr, "Ошибка: директория не найдена\n");
		return;
	}
	
	DIR *dir = opendir(path);
	if (!dir)
	{
		fprintf(stderr, "Не удалось открыть директорию\n");
		return;
	}

	struct dirent* entry;
    
	char **filenames = NULL;
	int count = 0;
	int capacity = 100;
    
	filenames = malloc(capacity * sizeof(char*));
    
    	while ((entry = readdir(dir)) != NULL)
    	{
        	if (!flags.a && entry->d_name[0] == '.')
            		continue;

        	char full_path[1024];
        	snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        	struct stat file_stat;

        	if (stat(full_path, &file_stat) == 0)
        	{
            		total_blocks += file_stat.st_blocks;
        	}
        
        	if (count >= capacity)
		{
            		capacity *= 2;
            		filenames = realloc(filenames, capacity * sizeof(char*));
        	}
        
        	filenames[count] = my_strdup(entry->d_name);
        	count++;
	}
    
    	closedir(dir);
    
    	qsort(filenames, count, sizeof(char*), compare_names);
	
	if (flags.l)
	{
		printf("total %ld\n", total_blocks / 2);
	}
	
	for (int i = 0; i < count; i++)
	{
		char full_path[1024];
		snprintf(full_path, sizeof(full_path), "%s/%s", path, filenames[i]);
		
		struct stat file_stat;
		if (stat(full_path, &file_stat) != 0)
		{
			file_stat.st_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		}

		if (flags.l)
		{
			print_file_info(full_path, filenames[i]);
		}
		else
		{
			print_colored_name(filenames[i], file_stat.st_mode);
			printf("\n");
		}
		
		free(filenames[i]);
	}
	free(filenames);
}


int main(int argc, char* argv[])
{
	Fl flags = {false, false};

	int opt;
	while ((opt = getopt(argc, argv, "al")) != -1)
	{
		switch (opt)
		{
			case 'a':
				flags.a = true;
				break;
			case 'l':
				flags.l = true;
				break;
		}

	}
	if (optind == argc)
	{
        	hpath(".", flags);
	}
	else
	{
        	for (int i = optind; i < argc; i++)
		{
			hpath(argv[i], flags);
		}
        }

    return 0;
}
