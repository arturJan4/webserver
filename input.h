// Artur Jankowski, 317928
#ifndef INPUT_HEADER_FILE
#define INPUT_HEADER_FILE

#include <stdint.h>  // uint16_t

// parse input from user into port and directory (remember to allocate memory)
void parse_input(char **argv, uint16_t *port, char *directory);

#endif