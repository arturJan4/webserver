// Artur Jankowski, 317928

#include "input.h"

#include <assert.h>   // assert
#include <errno.h>    // errno
#include <stdbool.h>  // bool
#include <stdio.h>    // fprintf
#include <stdlib.h>   // EXIT_FAILURE
#include <string.h>   // strlen

#include "helpers.h"

// converts string to number with error checking
long convert_to_number(char *number_str) {
  char *end;
  long number = strtol(number_str, &end, 0);
  if (*end != '\0') {
    fprintf(stderr, "%s is not a number!\n", number_str);
    exit(EXIT_FAILURE);
  }

  return number;
}

// gets port from string and checks if it is valid
uint16_t get_port(char *number_str) {
  long port = convert_to_number(number_str);

  if (port < 0 || port > 65535) {
    fprintf(stderr, "%s is not a valid port number ([0, 65535])!\n",
            number_str);
    exit(EXIT_FAILURE);
  }

  return (uint16_t)port;
}

void parse_input(char **argv, uint16_t *port, char *directory) {
  assert(port != NULL);
  assert(directory != NULL);

  *port = get_port(argv[1]);
  bool ret = directory_exists(directory);
  if (!ret)  // directory doesn't exist
    exit(EXIT_FAILURE);

  size_t directory_len = strlen(directory);
  // remove redundant '/' symbol
  // meaning we represnt directories as 'dirname' not 'dirname/'
  if (directory[directory_len - 1] == '/') directory[directory_len - 1] = '\0';
}