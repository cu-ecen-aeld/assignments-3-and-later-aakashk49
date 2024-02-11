#define _XOPEN_SOURCE 600

#include <arpa/inet.h>
#include <fcntl.h>
#include <features.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#define PORT "9000"
#define BACKLOG 10
int sockfd;
char *fn = "/var/tmp/aesdsocketdata";

void handle_signal(int signal) {
  printf("\nReceived signal %d. Closing the server.\n", signal);
  syslog(LOG_INFO, "Caught signal, exiting\n");
  close(sockfd);
  // Delete the file
  if (unlink(fn) == -1) {
    perror("Error deleting file");
  }
  exit(EXIT_SUCCESS);
}

void handle_client(int new_fd, int fd_write) {
  char byte;
  // Receive data byte by byte and process
  ssize_t bytes_received;
  printf("going to rec data\n");
  while ((bytes_received = recv(new_fd, &byte, sizeof(byte), 0)) > 0) {
    // Process the received byte (you can replace this part with your own
    // logic)
    // printf("Received byte from server: %c\n", byte);

    int wr = write(fd_write, &byte, 1);
    if (wr < 0) {
      printf("Write failed\n");
    }

    if (byte == '\n') {
      fsync(fd_write);
      int fileDescriptor = fd_write;
      int fileSize = lseek(fd_write, 0, SEEK_END);
      if (lseek(fd_write, 0, SEEK_SET) == -1) {
        perror("Error seeking to beginning of file");
        close(fd_write);
        return; // Return an error code
      }
      char *buffer = (char *)malloc(fileSize);
      if (buffer == NULL) {
        perror("Error allocating buffer");
        close(fd_write);
        return; // Return an error code
      }
      ssize_t bytesRead = read(fileDescriptor, buffer, fileSize);
      if (bytesRead == -1) {
        perror("Error reading file");
        free(buffer);
        close(fileDescriptor);
        return; // Return an error code
      }
      // send
      ssize_t bytes_sent = send(new_fd, buffer, fileSize, 0);
      if (bytes_sent <= 0) {
        perror("Error sending data");
        break;
      }

      free(buffer);

      break;
    }
  }

  printf("closing the new_fd\n");
  // Close the new socket (we're done with this client)
  fsync(new_fd);

  close(new_fd);
}

int main(int argc, char *argv[]) {
  bool dmode = false;
  if (argc > 1)
    dmode = true;
  printf("Hello from Socket\n");
  // Open syslog with a custom identifier and options
  openlog("my_program", LOG_PID | LOG_CONS, LOG_USER);
  int sockfd, new_fd;
  struct addrinfo hints, *res; //, *p;
  // Create socket
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Error creating socket");
    exit(EXIT_FAILURE);
  }
  // Set up hints to getaddrinfo
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
  hints.ai_flags = AI_PASSIVE;     // use my IP

  // Get address information
  if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
    perror("getaddrinfo");
    exit(EXIT_FAILURE);
  }

  if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
    close(sockfd);
    perror("server: bind");
  }

  freeaddrinfo(res);

  // Start listening
  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  printf("Server is listening on port %s...\n", PORT);
  // Log a message with priority LOG_INFO
  syslog(LOG_INFO, "Server is listening on port %s...\n", PORT);
  // Accept connections and handle data
  if (dmode) {
    // Daemonize the server
    if (daemon(0, 0) == -1) {
      perror("Error daemonizing the server");
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    printf("Server daemonized\n");
  }
  int fd_write = open(fn, O_RDWR | O_CREAT | O_TRUNC,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

  if (fd_write == -1) {
    printf("Could not open file %s\n", fn);
    return 1;
  }
  // Set up signal handling for SIGINT and SIGTERM
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof client_addr;
  char client_ip[INET6_ADDRSTRLEN];
  while (1) {
    new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
    if (new_fd == -1) {
      perror("accept");
      // continue;
    }
    // Convert client's IP address to a string
    inet_ntop(client_addr.ss_family,
              &(((struct sockaddr_in *)&client_addr)->sin_addr), client_ip,
              sizeof client_ip);
    printf("Server: got connection from %s\n", client_ip);
    if(!dmode)
        handle_client(new_fd, fd_write);
    else{
         pid_t child_pid = fork();
         if (child_pid == -1) {
            perror("Error forking process");
            close(new_fd);
            continue;
        } else if (child_pid == 0) {
            // This is the child process
            close(sockfd); // Close the server socket in the child process

            handle_client(new_fd, fd_write);

            // The child process will exit after handling the client
        } else {
            // This is the parent process
            close(new_fd); // Close the client socket in the parent process

        }
    }
    printf("Closed connection from %s\n", client_ip);
    syslog(LOG_INFO, "Closed connection from %s\n", client_ip);
  }
  close(fd_write);
  close(sockfd);
  closelog();
  return 0;
}