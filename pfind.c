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
#include  <stdbool.h>

/* prototypes */
void print_usage(char *program);
void searchdir(char *dirname, char *findme, char type);
void get_file_stat(char *filename);
bool compare_type(unsigned char d_type, char type);


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
        } else print_usage(argv[0]);
      } else if (strcmp(argv[i], "-type") == 0) {
        if (i + 1 <= argc - 1) {
          i++;
          type = argv[i][0];  //TODO validation
        } else print_usage(argv[0]);
      } else print_usage(argv[0]);
    } else {
      if (dirname)
        print_usage(argv[0]);
      else
        dirname = argv[i];
    }
  }

  if (!dirname) dirname = ".";  // search current directory if none provided

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

void searchdir(char *dirname, char *findme, char type) {
  DIR *dir;  /* current working directory */
  struct dirent *direntp;  /* current working dir entry */

  if ((dir = opendir(dirname)) == NULL)
    perror("Cannot open directory");

  while ((direntp = readdir(dir)) != NULL) {
    /*puts(direntp->d_type);*/
    if (strcmp(direntp->d_name, findme) == 0 && compare_type(direntp->d_type, type)) {
      char *path = strcat(strcat(dirname, "/"), direntp->d_name);
      puts(path); // TODO: realpath
    }
  }

  closedir(dir);
}

bool compare_type(unsigned char d_type, char type) {
  return true;
}


void get_file_stat(char *filename) {
}
