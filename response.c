// Artur Jankowski, 317928

#include "response.h"

#include <assert.h>      // assert
#include <limits.h>      // PATH_MAX
#include <netinet/ip.h>  // IP_MAXPACKET
#include <stdbool.h>     // bool
#include <stdio.h>       // FILE
#include <stdlib.h>
#include <string.h>  // strchr

#include "helpers.h"
#include "server.h"

/* Uncomment this line for debugging output */
//#define DEBUG
#ifdef DEBUG
#define debug(fmt, ...) printf("%s: " fmt "\n", __func__, __VA_ARGS__)
#define msg(...) printf(__VA_ARGS__)
#else
#define debug(fmt, ...)
#define msg(...)
#endif

char respond_buffer[IP_MAXPACKET];
uint32_t response_size = 0;

PathState_t build_path(HTTPrequest *request, char *directory, char *path) {
  assert(request != NULL);
  assert(directory != NULL);

  // e.g. directory/host/page.html -> OK
  // directory/host/../otherhost/page.html -> FORBIDDEN
  // directory/host/dir/ -> MOVE to directory/host/dir/index.html
  // directory/host/page404.html -> NOT FOUND
  // other -> 501 Not Implemented
  // directory/host
  char dirhost[PATH_MAX];
  sprintf(dirhost, "%s/%s", directory, request->host);

  // check if directory/host is a directory
  bool ret = directory_exists(dirhost);
  if (!ret) {
    debug("%s no such directory (domain)", dirhost);
    return WRONG_HOST;
  }

  // check if not using '..' or other tricks to escape domain
  sprintf(path, "%s%s", dirhost, request->address);
  ret = is_prefix_dir(dirhost, path);
  if (!ret) {
    debug("%s is accessing forbidden path", path);
    return RESTRICTED;
  }

  // path is directory instead of a file -> 301 move to index.html
  ret = directory_exists(path);
  if (ret) {
    return ISDIR;
  }

  // file doesn't exist -> 404
  if (!file_exists(path)) {
    return NOT_FOUND;
  }

  return ISOK;
}

void init_respond() {
  response_size = 0;
  respond_buffer[0] = '\0';
}

void add_respond_line(char *line) {
  assert(line != NULL);

  int wrote = sprintf(respond_buffer + response_size, "%s\r\n", line);
  response_size += wrote;
  debug("added '%s\\r\\n'", line);
}

void add_respond_header(char *header_name, char *value) {
  assert(header_name != NULL);
  assert(value != NULL);

  int wrote = sprintf(respond_buffer + response_size, "%s: %s \r\n",
                      header_name, value);

  response_size += wrote;
  debug("added '%s: %s \\r\\n'", header_name, value);
}

char *get_content_type(char *path) {
  assert(path != NULL);

  char *ext;

  ext = strrchr(path, '.');  // last occurence of a dot
  if (!ext) {
    return "application/octet-stream";  // no extension found
  }
  ext++;

  // "switch"
  if (strcmp(ext, "txt") == 0) {
    return "text/plain; charset=utf-8";
  } else if (strcmp(ext, "html") == 0) {
    return "text/html; charset=utf-8";
  } else if (strcmp(ext, "css") == 0) {
    return "text/css; charset=utf-8";
  } else if (strcmp(ext, "jpg") == 0) {
    return "image/jpg";
  } else if (strcmp(ext, "jpeg") == 0) {
    return "image/jpeg";
  } else if (strcmp(ext, "png") == 0) {
    return "image/png";
  } else if (strcmp(ext, "pdf") == 0) {
    return "application/pdf";
  }
  // default
  return "application/octet-stream";
}

void send_501(int connected_sockfd) {
  init_respond();

  add_respond_line("HTTP/1.1 501 Not Implemented");

  add_respond_header("Connection", "close");

  char length_str[128];
  sprintf(length_str, "%ld", strlen("\r\n501 Not Implemented\r\n"));
  add_respond_header("Content-Length", length_str);
  // body
  add_respond_line("");
  add_respond_line("501 Not Implemented");
  // CRLF
  add_respond_line("");

  printf("%s\n", "sending buffer...");
  send_buffer_response(connected_sockfd, response_size, respond_buffer);
}

bool build_http_response(HTTPrequest *request, char *directory,
                         int connected_sockfd) {
  char path[PATH_MAX];
  init_respond();

  // build and check if path is correct
  PathState_t code = build_path(request, directory, path);

  debug("built path: '%s' with code %d", path, code);

  // HTTP + status code
  switch (code) {
    case ISOK:
      add_respond_line("HTTP/1.1 200 OK");
      break;
    case ISDIR:
      add_respond_line("HTTP/1.1 301 Moved Permanently");
      break;
    case RESTRICTED:
      add_respond_line("HTTP/1.1 403 Forbidden");
      break;
    case NOT_FOUND:
      add_respond_line("HTTP/1.1 404 Not Found");
      break;
    case WRONG_HOST:
      add_respond_line("HTTP/1.1 501 Not Implemented");
      break;
  }

  if (code == ISDIR) {
    add_respond_header("Location", "/index.html");
  }

  // Content-type
  if (code == ISOK) {
    char *content_type = get_content_type(path);
    add_respond_header("Content-Type", content_type);
  }

  FILE *f = NULL;
  long file_size = 0;

  if (code == ISOK) {
    f = fopen(path, "rb");
    if (f == NULL) {
      return false;
    }

    debug("succesful open of file: %s", path);
    fseek(f, 0, SEEK_END);  // seek to end
    file_size = ftell(f);

    if (file_size < 0) {
      file_size = 0;
    }

    fseek(f, 0, SEEK_SET);  // seek back to beginning
  }

  add_respond_header("Connection", request->connection);

  // Content length
  char length_str[128];
  printf("response status code: %d\n", code);
  switch (code) {
    case ISOK:
      sprintf(length_str, "%ld", file_size);
      break;
    case RESTRICTED:
      sprintf(length_str, "%ld", strlen("\r\n403 Forbidden\r\n"));
      break;
    case NOT_FOUND:
      sprintf(length_str, "%ld", strlen("\r\n404 Not Found\r\n"));
      break;
    case WRONG_HOST:
      sprintf(length_str, "%ld", strlen("\r\n501 Not Implemented\r\n"));
      break;
    default:
      sprintf(length_str, "%ld", file_size);
      break;
  }
  add_respond_header("Content-Length", length_str);

  // body
  switch (code) {
    case RESTRICTED:
      add_respond_line("");
      add_respond_line("403 Forbidden");
      break;
    case NOT_FOUND:
      add_respond_line("");
      add_respond_line("404 Not Found");
      break;
    case WRONG_HOST:
      add_respond_line("");
      add_respond_line("501 Not Implemented");
      break;
    default:
      break;
  }
  // ending CRLF
  add_respond_line("");

  printf("%s\n", "sending buffer...");
  bool ret =
      send_buffer_response(connected_sockfd, response_size, respond_buffer);
  if (!ret) {
    return false;
  }

  if (f != NULL) {
    printf("%s\n", "sending file...");
    ret = send_buffer_response_file(connected_sockfd, f, file_size,
                                    respond_buffer);
    if (!ret) {
      return false;
    }
  }

  return true;
}