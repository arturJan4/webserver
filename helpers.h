// Artur Jankowski, 317928
#ifndef HELPERS_HEADER_FILE
#define HELPERS_HEADER_FILE

#include <stdbool.h>

// test directory path and checks if it's valid
bool directory_exists(char *str);

// test if file exists for full path
bool file_exists(char *path);

// check whether path to directory/host is a prefix of path
bool is_prefix_dir(char *dirhost, char *path);

#endif