// Artur Jankowski, 317928

#include "server.h"

#include <arpa/inet.h>   // pton, ntop et.c
#include <assert.h>      // assert.h
#include <errno.h>       // errno
#include <netinet/ip.h>  // IP_MAXPACKET
#include <stdbool.h>     // bool
#include <stdio.h>       // printf, fpritnf
#include <string.h>      // memset, strerror
#include <unistd.h>      // close

/* Uncomment this line for debugging output */
//#define DEBUG
#ifdef DEBUG
#define debug(fmt, ...) printf("%s: " fmt "\n", __func__, __VA_ARGS__)
#define msg(...) printf(__VA_ARGS__)
#else
#define debug(fmt, ...)
#define msg(...)
#endif

// prepare listening socket for server communication
int prepare_socket(struct sockaddr_in *server, uint16_t port) {
  memset(server, 0, sizeof(*server));

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  const int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    fprintf(stderr, "setsockopt(SO_REUSEADDR) failed: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (sockfd < 0) {
    fprintf(stderr, "socket creation error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  server->sin_family = AF_INET;
  server->sin_port = htons(port);
  server->sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sockfd, (struct sockaddr *)server, sizeof(*server)) < 0) {
    fprintf(stderr, "socket bind error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (listen(sockfd, 64) < 0) {
    fprintf(stderr, "socket listen error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  return sockfd;
}

// accept wrapper
int accept_connecting(int serverfd) {
  int connected_sockfd = accept(serverfd, NULL, NULL);
  if (connected_sockfd < 0) {
    fprintf(stderr, "socket accept error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  return connected_sockfd;
}

// close wrapper
void close_connecting(int connected_sockfd) {
  if (close(connected_sockfd) < 0) {
    fprintf(stderr, "conn_socket close error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
}

// recv wrapper, pass allocated buffer
void receive_packet(uint8_t *recv_buffer, int connected_sockfd) {
  ssize_t bytes_read = recv(connected_sockfd, recv_buffer, IP_MAXPACKET, 0);
  if (bytes_read < 0) {
    fprintf(stderr, "recv error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  recv_buffer[bytes_read] = '\0';
}

// select wrapper for nonblocking sleep for maximum TIMEOUT microseconds
int receive_packets(uint8_t *recv_buffer, int connected_sockfd) {
  struct timeval tv_to;
  tv_to.tv_sec = 0;
  tv_to.tv_usec = TIMEOUT;

  fd_set descriptors;
  FD_ZERO(&descriptors);
  FD_SET(connected_sockfd, &descriptors);

  int ready;
  while ((ready = select(connected_sockfd + 1, &descriptors, NULL, NULL,
                         &tv_to)) > 0) {
    if (ready > 0) {
      receive_packet(recv_buffer, connected_sockfd);
      break;
    }

    FD_ZERO(&descriptors);
    FD_SET(connected_sockfd, &descriptors);
  }

  return ready;
}

// send wrapper, return 0 on failure
ssize_t send_buffer_response(int connected_sockfd, uint32_t len, char *buffer) {
  assert(buffer != NULL);

  ssize_t sent = send(connected_sockfd, buffer, len, 0);
  if (sent == -1) {
    fprintf(stderr, "error sending to socket %d, len: %d: %s\n",
            connected_sockfd, len, strerror(errno));
    return 0;
  }

  return sent;
}

// send buffer in chunks smaller than IP_MAXPACKET
bool send_buffer_response_file(int connected_sockfd, FILE *f, uint32_t len,
                               char *buffer) {
  assert(buffer != NULL);
  assert(f != NULL);

  int64_t to_send = len;    // how many bytes left to send
  char *buff_ptr = buffer;  // current position in the buffer

  while (to_send > 0) {
    int64_t sending = to_send;
    if (sending > IP_MAXPACKET)
      sending = IP_MAXPACKET;  // still more than max needed

    int readn = fread(buff_ptr, sizeof(char), sending, f);
    if (readn <= 0) {  // error reading file
      return false;
    }
    // send one chunk of data
    ssize_t ret = send_buffer_response(connected_sockfd, sending, buff_ptr);
    if (ret == 0) {  // error sending data
      return false;
    }

    to_send -= ret;
    debug("read: %d, sent %lu, left: %ld", readn, ret, to_send);
  }

  debug("%s", "file sent properly");
  fclose(f);
  return true;
}