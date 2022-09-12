// Artur Jankowski, 317928
#include <netinet/ip.h>  // IP_MAXPACKET
#include <stdio.h>       // printf, fpritnf
#include <unistd.h>      // close

#include "input.h"
#include "response.h"
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

// main webserver loop
void webserver_loop(uint16_t port, char *directory) {
  struct sockaddr_in server;  // server address with port
  int sockfd = prepare_socket(&server, port);

  // buffer for received packet data
  uint8_t recv_buffer[IP_MAXPACKET + 2];

  // repeat for each client (one at a time)
  for (;;) {
    // connect with client (accept wrapper)
    int connected_sockfd = accept_connecting(sockfd);
    debug("connecting with socket: %d", connected_sockfd);
    printf("new connection\n");

    for (;;) {
      // keep connection for 1s (select)
      int ready = receive_packets(recv_buffer, connected_sockfd);
      if (ready <= 0)  // timeout or error
        break;

      // parse received request
      debug("<<%s>>", recv_buffer);
      HTTPrequest *request = init_http_request();  // allocated data
      bool valid_request = parse_request((char *)recv_buffer, request);
      if (!valid_request) {
        fprintf(stderr, "invalid request\n");
        send_501(connected_sockfd);
        break;
      }

      // retrieve 3 header's values we need
      printf("received GET %s, for host: %s, connection: %s\n",
             request->address, request->host, request->connection);

      // build and sends HTTP response based on request and directory
      if (!build_http_response(request, directory, connected_sockfd)) {
        break;
      }

      // if Connection: keep-alive -> don't break connetion
      if (!stay_alive(request)) {
        break;
      }

      free_http_request(request);
    }

    printf("%s", "closing connection\n");
    close_connecting(connected_sockfd);
  }

  close(sockfd);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage:     %s [port] [directory]\n", argv[0]);
    printf("port       - server's listening port \n");
    printf(
        "directory  - holds server's pages inside domainname directories \n");
    printf("Example:   \"%s 8888 webpages\" \n", argv[0]);
    return EXIT_FAILURE;
  }

  uint16_t port;
  char *directory = argv[2];
  parse_input(argv, &port, directory);

  printf("starting webserver with port: %u, directory: %s\n", port, directory);

  webserver_loop(port, directory);
  return 0;
}
