// Artur Jankowski, 317928
#ifndef RESPONSE_HEADER_FILE
#define RESPONSE_HEADER_FILE

#include <stdint.h>  // uint*

#include "request.h"

typedef enum {
  ISOK = 200,
  RESTRICTED = 403,
  NOT_FOUND = 404,
  ISDIR = 301,
  WRONG_HOST = 501,
} PathState_t;

// build's path and checks which response code should it return
PathState_t build_path(HTTPrequest *request, char *directory, char *path);

// initalize HTTP response
void init_respond();

// add a line ending with CRLF
void add_respond_line(char *line);

// add a header ending with CRLF
void add_respond_header(char *header_name, char *value);

// get Content-type header value from file path
char *get_content_type(char *path);

// try to send 501 response, return true on success
void send_501(int connected_sockfd);

// build's entire http response
bool build_http_response(HTTPrequest *request, char *directory,
                         int connected_sockfd);

#endif