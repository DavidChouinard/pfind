/*
 * pfind.c
 *
 * Author: David Chouinard
 *   Date: Feb 28 2014
 *   Desc: pfind recursively traverses the passed directory and prints all
 *         files that fullfil the passed name pattern and file type conditions.
 *         It is meant as a drop-in replacement for the GNU find utility (but
 *         only provides a subset of functionality).
 *  Usage:
 *         pfind starting_dir [-name filename-or-pattern] [-type {f|d|b|c|p|l|s}]
 */

#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include  <dirent.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <unistd.h>
#include  <stdbool.h>
#include  <limits.h>
#include  <fnmatch.h>
#include  <libgen.h>
#include  <errno.h>


/* prototypes */
void print_usage(char *program);
void usage_error(char *message, char *argument);
bool validate_type(char* type);
void searchdir(char *dirname, char *findme, char type);
bool compare_type(mode_t mode, char type);


int main(int argc, char* argv[]) {
    char *dirname = NULL;
    char *findme = NULL;
    char type = '\0';

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {  // Skip argv[0] (program name)
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-name") == 0) {
                if (i + 1 <= argc - 1) {
                    i++;
                    findme = argv[i];
                } else usage_error("missing argument to `%s'", argv[i]);
            } else if (strcmp(argv[i], "-type") == 0) {
                if (i + 1 <= argc - 1) {
                    i++;
                    if (validate_type(argv[i]))
                        type = argv[i][0];
                    else
                        usage_error("invalid argument `%s' to `-type'", argv[i]);
                } else usage_error("missing argument to `%s'", argv[i]);
            } else usage_error("invalid predicate `%s'", argv[i]);
        } else {
            if (dirname)
                print_usage(argv[0]);
            else
                dirname = argv[i];
        }
    }

    if (!dirname) print_usage(argv[0]);

    // Remove trailing slash from passed directory, if any
    if (dirname[strlen(dirname) - 1] == '/')
        dirname[strlen(dirname) - 1] = '\0';

    searchdir(dirname, findme, type);
}

/*
 * Prints usage instructions to stderr and exits
 *
 * Arguments:
 *    char *program: calling program, ie. argv[0]
 */
void print_usage(char *program) {
    fprintf(stderr, "usage: %s starting_dir [-name filename-or-pattern] [-type {f|d|b|c|p|l|s}]\n", program);
    exit(1);
}

/*
 * Prints error message to stderr and exits
 *
 * Arguments:
 *    char *message: error message with a single %s token
 *    char *argument: faulty argument (will replace %s in message)
 */
void usage_error(char *message, char *argument) {
    fputs("pfind: ", stderr);
    fprintf(stderr, message, argument);
    fputs("\n", stderr);
    exit(1);
}

/*
 * Validates that the type argument is a valid value
 *
 * Arguments:
 *    char *type: type string to validate
 */
bool validate_type(char* type) {
    return strlen(type) == 1 && (type[0] == 'f' || type[0] == 'd' || type[0] == 'c'
            || type[0] == 'b' || type[0] == 'p' || type[0] == 'l' || type[0] == 's');
}

/*
 * Calls perror and prefixes the program name
 *
 * Arguments:
 *    char *error: error message to perror
 */
void perror_prefix(char *error) {
    char message[PATH_MAX + 7] = "pfind: ";
    perror(strcat(message, error));
}

/*
 * Recursively searches the passed directory for files matching both findme and
 * type and outpus the result to stdout
 *
 * Arguments:
 *    char *dirname: directory to search (with no trailing slashes)
 *    char *findme: file name shell pattern to search, NULL if none
 *    char type: restrict search to a file type, null character if not set
 */
void searchdir(char *dirname, char *findme, char type) {
    DIR *dir;  /* current working directory */
    struct dirent *direntp;  /* current working dir entry */
    struct stat statp;  /* current stat of file/directory */

    if (lstat(dirname, &statp) == -1) {
        perror_prefix(dirname);
        return;
    }

    if ((!findme || !fnmatch(findme, basename(dirname), 0)) && compare_type(statp.st_mode, type)) {
        puts(dirname); // TODO: realpath   // found one!
    }

    // Base condition of the recursion
    if (!S_ISDIR(statp.st_mode)) return;

    if ((dir = opendir(dirname)) == NULL) {
        perror_prefix(dirname);
        return;
    }

    while (true) {  // a bit clunky, but I don't see how it can be cleaner
        errno = 0;
        direntp = readdir(dir);

        if (direntp == NULL) {
            if (errno != 0)
                perror_prefix(dirname);
            break;
        }

        // recursively call ourself on each subdirectory
        if (strcmp(direntp->d_name, ".") != 0 && strcmp(direntp->d_name, "..") != 0) {
            // dynamically allocate string so that we don't exhaust space on the stack
            char *current_path = malloc(strlen(dirname) + strlen(direntp->d_name) + 2);
            sprintf(current_path, "%s/%s", dirname, direntp->d_name);
            searchdir(current_path, findme, type);
            free(current_path);
        }
    }

    closedir(dir);
}

/*
 * Helper function that compares the command line type option to the mode
 * returned by stat
 *
 * Arguments:
 *    mode_t mode: mode returned by stat
 *    char type: file type command line option
 */
bool compare_type(mode_t mode, char type) {
    switch (type) {
        case '\0':
            return true;
        case 'f':
            return S_ISREG(mode);
        case 'd':
            return S_ISDIR(mode);
        case 'c':
            return S_ISCHR(mode);
        case 'b':
            return S_ISBLK(mode);
        case 'p':
            return S_ISFIFO(mode);
        case 'l':
            return S_ISLNK(mode);
        case 's':
            return S_ISSOCK(mode);
        default:
            return false; // we should never get here
    }
}
