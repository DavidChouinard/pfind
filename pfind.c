/*
 * aac.cpp
 *
 * Author: David Chouinard
 *   Date: Feb 9 2014
 *   Desc: aac parses login and logout records and reports on the total
 *         connection time for a given user. It is meant as a drop-in
 *         replacement for the GNU ac utility (but only provides a subset of
 *         functionality).
 *  Usage:
 *         aac username [-f wtmp]
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
void searchdir(char *dirname, char *findme, char type);
bool compare_type(mode_t mode, char type);
void error_prefix();
void missing_argument(char *argument);
void invalid_predicate(char *predicate);


int main(int argc, char* argv[]) {
  char *dirname = NULL;
  char *findme = NULL;
  char type = '\0';

  for (int i = 1; i < argc; i++) {  // Skip argv[0] (program name)
    if (argv[i][0] == '-') {
      if (strcmp(argv[i], "-name") == 0) {
        if (i + 1 <= argc - 1) {
          i++;
          findme = argv[i];
        } else missing_argument(argv[i]);
      } else if (strcmp(argv[i], "-type") == 0) {
        if (i + 1 <= argc - 1) {
          i++;
          type = argv[i][0];  //TODO validation
        } else missing_argument(argv[i]);
      } else invalid_predicate(argv[i]);
    } else {
      if (dirname)
        print_usage(argv[0]);
      else
        dirname = argv[i];
    }
  }

  if (!dirname) print_usage(argv[0]);

  searchdir(dirname, findme, type);
}

/*
 * Prints usage instructions to stderr
 *
 * Arguments:
 *    char *program: calling program, ie. argv[0]
 */
void print_usage(char *program) {
  fprintf(stderr, "usage: %s starting_dir [-name filename-or-pattern] [-type {f|d|b|c|p|l|s}]\n", program);
  exit(1);
}

void error_prefix() {
  fputs("pfind: ", stderr);
}

void perror_prefix(char *path) {
  char message[PATH_MAX + 7] = "pfind: ";
  perror(strcat(message, path));
}

void missing_argument(char *argument) {
  error_prefix();
  fprintf(stderr, "missing argument to `%s'\n", argument);
  exit(1);
}

void invalid_predicate(char *predicate) {
  error_prefix();
  fprintf(stderr, "invalid predicate `%s'\n", predicate);
  exit(1);
}

void searchdir(char *dirname, char *findme, char type) {
  DIR *dir;  /* current working directory */
  struct dirent *direntp;  /* current working dir entry */
  struct stat statp;  /* current stat of file/directory */

  if (lstat(dirname, &statp) == -1) {
    perror_prefix(dirname);
    return;
  }

  if ((!findme || !fnmatch(findme, basename(dirname), 0)) && compare_type(statp.st_mode, type)) {
    puts(dirname); // TODO: realpath
  }

  // Base condition of the recursion
  if (!S_ISDIR(statp.st_mode)) return;

  if ((dir = opendir(dirname)) == NULL) {
    perror_prefix(dirname);
    return;
  }

  char current_path[PATH_MAX];
  /*while ((direntp = readdir(dir)) != NULL) {*/
  while (true) {
    errno = 0;
    direntp = readdir(dir);

    if (direntp == NULL) {
      if (errno != 0)
        perror_prefix(dirname);
      break;
    }

    if (strcmp(direntp->d_name, ".") != 0 && strcmp(direntp->d_name, "..") != 0) {
      sprintf(current_path, "%s/%s", dirname, direntp->d_name);
      searchdir(current_path, findme, type);
    }
  }

  closedir(dir);
}

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
