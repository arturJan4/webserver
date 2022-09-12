// Artur Jankowski, 317928

#include "helpers.h"

#include <assert.h>    // assert.h
#include <dirent.h>    // opendir
#include <errno.h>     // errno
#include <stdio.h>     // printf, fpritnf
#include <stdlib.h>    // EXIT_FAILURE
#include <string.h>    // strerror
#include <sys/stat.h>  // stat

/* Uncomment this line for debugging output */
//#define DEBUG
#ifdef DEBUG
#define debug(fmt, ...) printf("%s: " fmt "\n", __func__, __VA_ARGS__)
#define msg(...) printf(__VA_ARGS__)
#else
#define debug(fmt, ...)
#define msg(...)
#endif

// test directory path and checks if it's valid
bool directory_exists(char *str) {
  assert(str != NULL);

  DIR *dir = opendir(str);
  if (dir) {  // should exist
    closedir(dir);
    return true;
  } else if (ENOENT == errno) {
    debug("directory (\"%s\") doesn't exists: %s\n", str, strerror(errno));
    return false;
  } else if (ENOTDIR == errno) {
    debug("(\"%s\") is not a directory: %s\n", str, strerror(errno));
    return false;
  } else {
    fprintf(stderr, "directory (\"%s\") error: %s\n", str, strerror(errno));
    exit(EXIT_FAILURE);
  }
}

// test if file exists for given path
bool file_exists(char *path) {
  assert(path != NULL);

  struct stat buffer;
  return (stat(path, &buffer) == 0);
}

// realpath wrapper for is_prefix_dir, return 0 on error
int Realpath(char *from, char *to) {
  if (realpath(from, to) == NULL) {
    // file doesn't exists or not enough permissions
    if (errno == ENOENT || errno == EACCES) {
      return 0;
    }

    // other errors
    fprintf(stderr, "realpath other for %s:%s error: %s\n", from, to,
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  return 1;
}

// check whether full path to directory/host is a prefix of path
// used to check if they work on same subtree of directories
bool is_prefix_dir(char *dirhost, char *path) {
  assert(dirhost != NULL);
  assert(path != NULL);

  char fullpath[PATH_MAX];
  char fulldirhost[PATH_MAX];

  if (!Realpath(dirhost, fulldirhost)) {
    return true;
  }
  if (!Realpath(path, fullpath)) {
    return true;
  }

  debug("comparing fulldirhost: %s, and fullpath: %s", fulldirhost, fullpath);
  return strncmp(fulldirhost, fullpath, strlen(fulldirhost)) == 0;
}