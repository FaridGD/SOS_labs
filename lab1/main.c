#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>

typedef struct {
    bool n;
    bool E;
    bool b;
    bool s;
} Flags;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        perror("Please add a file name");
        return 1;
    }

    Flags flags = {false, false, false, false};
    int counter = 0;

    int opt;
    while ((opt = getopt(argc, argv, "Ebns")) != -1) {
        switch (opt) {
            case 'E':
                flags.E = true;
                break;
            case 'b':
                flags.b = true;
                break;
            case 'n': 
                flags.n = true;
                break;
            case 's':
                flags.s = true;
                break;
        }
    }
    for (int i = optind; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");
        if (file == NULL) {
            perror("No such file or error occured during opening file");
            return 1;
        }

        char buffer[1024];
        int empty_line_count = 0;
        bool skip_line = false;

        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            bool is_empty = (strlen(buffer) == 1 && buffer[0] == '\n') ||
                           (strlen(buffer) == 0);

            if (flags.s) {
                if (is_empty) {
                    empty_line_count++;
                    if (empty_line_count > 1) {
                        skip_line = true;
		    }
                } else {
                    empty_line_count = 0;
                }
            }
            if (skip_line) {
                skip_line = false;
                continue;
            }

            bool non_empty = !is_empty;
            if (flags.n || flags.b) {
                if (flags.b) {
                    if (non_empty) {
                        counter++;
                        printf("%6d\t", counter);
                    }
                } else {
                    counter++;
                    printf("%6d\t", counter);
                }
            }


	    if (flags.E) {
                char* p = strchr(buffer, '\n');
                if (p != NULL) {
                    *p = '\0';
                    printf("%s$\n", buffer);
                } else {
                    printf("%s", buffer);
                }
            } else {
                printf("%s", buffer);
            }
        }
        fclose(file);
    }
    return 0;
}
