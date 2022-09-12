// Artur Jankowski, 317928
#ifndef SERVER_HEADER_FILE
#define SERVER_HEADER_FILE

#include <arpa/inet.h>  // sockaddr
#include <stdbool.h>
#include <stdint.h>  // uint*
#include <stdio.h>   // FILE
#include <stdlib.h>

#define TIMEOUT 750000  // 0.75s

// prepare listening socket for server communication
int prepare_socket(struct sockaddr_in *server, uint16_t port);

// accept wrapper
int accept_connecting(int serverfd);
// close wrapper
void close_connecting(int connected_sockfd);

// select wrapper for nonblocking sleep for maximum TIMEOUT microseconds
int receive_packets(uint8_t *recv_buffer, int connected_sockfd);

// send wrapper, return 0 on failure
ssize_t send_buffer_response(int connected_sockfd, uint32_t len, char *buffer);

// send buffer in chunks smaller than IP_MAXPACKET
bool send_buffer_response_file(int connected_sockfd, FILE *f, uint32_t len,
                               char *buffer);

#endif