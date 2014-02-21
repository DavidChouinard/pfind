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
#include  <sys/types.h>
#include  <dirent.h>
#include  <stdbool.h>
#include  <limits.h>
#include  <fnmatch.h>

/* prototypes */
void print_usage(char *program);
void searchdir(char *dirname, char *findme, char type);
bool compare_type(unsigned char d_type, char type);
void error_prefix();
void missing_argument(char *argument);
void invalid_predicate(char *predicate);


int main(int argc, char* argv[]) {
  char *dirname = NULL;
  char *findme = NULL; // TODO: default
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

  // Base condition of the recursion
  if ((dir = opendir(dirname)) == NULL) {
    error_prefix();
    perror(dirname);
    return;
  }

  char current_path[PATH_MAX];
  while ((direntp = readdir(dir)) != NULL) {
    if (strcmp(direntp->d_name, ".") != 0 && strcmp(direntp->d_name, "..") != 0) {
      sprintf(current_path, "%s/%s", dirname, direntp->d_name);
      if ((!findme || !fnmatch(findme, direntp->d_name, 0)) && compare_type(direntp->d_type, type)) {
        puts(current_path); // TODO: realpath
      }

      if (direntp->d_type == DT_DIR)
        searchdir(current_path, findme, type);
    }
  }

  closedir(dir);
}

bool compare_type(unsigned char d_type, char type) {
  switch (type) {
    case '\0':
      return true;
    case 'f':
      return d_type == DT_REG;
    case 'd':
      return d_type == DT_DIR;
    case 'b':
      return d_type == DT_BLK;
    case 'c':
      return d_type == DT_CHR;
    case 'p':
      return d_type == DT_FIFO;
    case 'l':
      return d_type == DT_LNK;
    case 's':
      return d_type == DT_SOCK;
    default:
      return false; // we should never get here
  }
}

