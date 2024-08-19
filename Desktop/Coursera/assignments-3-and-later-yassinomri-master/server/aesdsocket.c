#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <signal.h>
#include <fcntl.h>

#define PORT 9000
#define BACKLOG 5
#define BUFFER_SIZE 1024
#define DATA_FILE "/var/tmp/aesdsocketdata"

int sockfd = -1;
int daemon_mode = 0;

void cleanup() {
    if (sockfd != -1) {
        close(sockfd);
    }
    remove(DATA_FILE);
}

void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        syslog(LOG_INFO, "Caught signal, exiting");
        cleanup();
        exit(EXIT_SUCCESS);
    }
}

void daemonize() {
    pid_t pid, sid;

    pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        // Parent process
        exit(EXIT_SUCCESS);
    }

    // Child process continues
    umask(0);

    sid = setsid();
    if (sid < 0) {
        perror("setsid failed");
        exit(EXIT_FAILURE);
    }

    if (chdir("/") < 0) {
        perror("chdir failed");
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main(int argc, char *argv[]) {
    int opt;
    int new_socket;
    struct sockaddr_storage client_addr;
    socklen_t addr_len;
    char buffer[BUFFER_SIZE];
    FILE *fp;
    ssize_t bytes_received;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
            case 'd':
                daemon_mode = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-d]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (daemon_mode) {
        daemonize();
        openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_DAEMON);
    } else {
        openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        syslog(LOG_ERR, "Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        syslog(LOG_ERR, "Bind failed");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(sockfd, BACKLOG) < 0) {
        syslog(LOG_ERR, "Listen failed");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Signal handling
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Main loop
    while (1) {
        addr_len = sizeof(client_addr);
        new_socket = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
        if (new_socket < 0) {
            syslog(LOG_ERR, "Accept failed");
            continue;
        }

        syslog(LOG_INFO, "Accepted connection");

        fp = fopen(DATA_FILE, "a+");
        if (fp == NULL) {
            syslog(LOG_ERR, "Failed to open data file");
            close(new_socket);
            continue;
        }

        while ((bytes_received = recv(new_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[bytes_received] = '\0';
            fputs(buffer, fp);
            fflush(fp);

            // Send the file contents back to the client
            rewind(fp);
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                send(new_socket, buffer, strlen(buffer), 0);
            }

            // Clear buffer and break if newline is received
            if (strchr(buffer, '\n') != NULL) {
                break;
            }
        }

        fclose(fp);
        close(new_socket);

        syslog(LOG_INFO, "Closed connection from %d.%d.%d.%d",
               (client_addr.ss_family == AF_INET ? 
               ((struct sockaddr_in*)&client_addr)->sin_addr.s_addr & 0xFF) :
               0,
               (client_addr.ss_family == AF_INET ? 
               (((struct sockaddr_in*)&client_addr)->sin_addr.s_addr >> 8) & 0xFF) :
               0,
               (client_addr.ss_family == AF_INET ? 
               (((struct sockaddr_in*)&client_addr)->sin_addr.s_addr >> 16) & 0xFF) :
               0,
               (client_addr.ss_family == AF_INET ? 
               (((struct sockaddr_in*)&client_addr)->sin_addr.s_addr >> 24) & 0xFF) :
               0);
    }

    cleanup();
    closelog();
    return 0;
}

