// Artur Jankowski, 317928
#include "request.h"

#include <assert.h>  // assert
#include <stdio.h>   // stderr
#include <stdlib.h>  // NULL
#include <string.h>  // strchr

/* Uncomment this line for debugging output */
#define DEBUG
#ifdef DEBUG
#define debug(fmt, ...) printf("%s: " fmt "\n", __func__, __VA_ARGS__)
#define msg(...) printf(__VA_ARGS__)
#else
#define debug(fmt, ...)
#define msg(...)
#endif

// initialize empty HTTPRequest
HTTPrequest *init_http_request() {
  HTTPrequest *temp = malloc(sizeof(HTTPrequest));
  temp->address = NULL;
  temp->host = NULL;
  temp->connection = strdup("close");

  return temp;
}

void free_http_request(HTTPrequest *request) {
  free(request->address);
  free(request->host);
  free(request->connection);
  free(request);
}

// fills request for header with data from the string between value_start and
// value_end pointers
void fill_request(HTTPrequest *request, char *header, char *value_start,
                  char *value_end) {
  // interesting headers
  const char host_str[] = "Host";
  const char conn_str[] = "Connection";

  // if header matches -> check value and add to HTTP_Request
  if (strncmp(host_str, header, strlen(host_str)) == 0) {
    char value[value_end - value_start + 1];
    strncpy(value, value_start, value_end - value_start);
    value[sizeof(value) - 1] = '\0';

    char *port = strchr(value, ':');
    if (port != NULL) {
      *port = '\0';  // trim port
    }
    request->host = strdup(value);
  } else if (strncmp(conn_str, header, strlen(conn_str)) == 0) {
    char value[value_end - value_start + 1];
    strncpy(value, value_start, value_end - value_start);
    value[sizeof(value) - 1] = '\0';

    request->connection = strdup(value);
  }
}

// parse request getting the address, connection and host from headers
bool parse_request(char *buffer, HTTPrequest *request) {
  assert(buffer != NULL);

  // filter out rubbish data
  if (strncmp(buffer, "GET", 3) != 0) {
    debug("%s", "not a GET request");
    return false;
  }

  char *path_start = strchr(buffer, ' ') + 1;
  if (path_start == NULL) return false;
  char *path_end = strchr(path_start, ' ');
  if (path_end == NULL) return false;
  char *line_end = strstr(path_end, "\n") + 1;  // line after GET request
  if (line_end == NULL) return false;

  // path after GET
  char path[path_end - path_start + 1];
  strncpy(path, path_start, path_end - path_start);
  path[sizeof(path) - 1] = '\0';

  request->address = strdup(path);  // malloc + copy
  if (request->address == NULL) {
    fprintf(stderr, "error while saving request address\n");
    return false;
  }

  // split lines of request by \n\r
  char *delim = "\n\r";
  char *token;
  token = strtok(line_end, delim);

  // start reading headers
  while (token != NULL) {
    // separate into <header_name: value>
    char *header_end = strchr(token, ':');
    if (header_end == NULL) return false;
    char *value_start = strchr(header_end, ' ') + 1;
    char *value_end = token + strlen(token);

    // only header_name
    char header[header_end - token + 1];
    strncpy(header, token, header_end - token);
    header[sizeof(header) - 1] = '\0';

    // fill value for useful header_names
    fill_request(request, header, value_start, value_end);

    token = strtok(NULL, delim);  // next token
  }

  // didn't find required headers
  if (request->address == NULL || request->host == NULL) {
    return false;
  }

  return true;
}

bool stay_alive(HTTPrequest *request) {
  const char alive_str[] = "keep-alive";
  const char close_str[] = "close";

  if (strncmp(alive_str, request->connection, strlen(alive_str)) == 0) {
    return true;
  } else if (strncmp(close_str, request->connection, strlen(close_str)) == 0) {
    return false;
  }

  fprintf(stderr, "unknown connection type (%s) for request",
          request->connection);
  exit(EXIT_FAILURE);
}