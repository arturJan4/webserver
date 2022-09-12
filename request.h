// Artur Jankowski, 317928
#ifndef REQUEST_HEADER_FILE
#define REQUEST_HEADER_FILE

#include <stdbool.h>

typedef struct {
  char *address;
  char *host;
  char *connection;
} HTTPrequest;

// initalize empty HTTPrequest struct
HTTPrequest *init_http_request();
void free_http_request(HTTPrequest *request);

// parse request getting the address, connection and host from headers
bool parse_request(char *buffer, HTTPrequest *request);

bool stay_alive(HTTPrequest *request);
#endif