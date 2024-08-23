#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define OUT_FILE "/var/tmp/aesdsocketdata"

int run = 1;
int working = 0;
int sockfd = -1;

void handle_signal(int signal) {
    syslog(LOG_DEBUG, "Caught signal. exiting");
    if (working) run = 0;
    else {
        if(sockfd != -1) close(sockfd);
        remove(OUT_FILE);
        closelog();
        exit(0);
    };
}

int main(int argc, char** argv) {
    openlog("aesdsocket", 0, LOG_USER);
    char server_ip[INET_ADDRSTRLEN];
    char client_ip[INET_ADDRSTRLEN];
    remove(OUT_FILE);
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    int deamon = 0;
    if (argc > 1 && !strcmp(argv[1], "-d")) { 
        deamon = 1;
    };

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;  // use IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo("NULL", "9000", &hints, &res);
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); 
    if (sockfd == -1) {
        freeaddrinfo(res);
        printf("Failed create socket: %s\n", strerror(errno));
        return -1;
    }
    inet_ntop(AF_INET, &(res->ai_addr), server_ip, INET_ADDRSTRLEN);
    
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        syslog(LOG_ERR, "Failed to set SO_REUSEADDR: %s", strerror(errno));
        freeaddrinfo(res);
        close(sockfd);
        return -1;
    }

    if(bind(sockfd, res->ai_addr, res->ai_addrlen)) {
        syslog(LOG_ERR, "Failed to bind socket to port 9000: %s", strerror(errno));
        printf("Failed to bind socket to port 9000: %s\n", strerror(errno));
        freeaddrinfo(res);
        close(sockfd);
        return -1;
    }

    freeaddrinfo(res);

    if(deamon) {
        printf("running as deamon...\n");
        if(fork()) exit(0);
    }

    uint8_t buffer[BUFSIZ];
    while (run) { 
        if (listen(sockfd, 10)) {
            syslog(LOG_ERR, "Failed to listen on %s:9000 : %s", server_ip, strerror(errno));
            return -1;
        }
        
        struct sockaddr client;
        socklen_t size = sizeof(client);
        working=0;
        int fd = accept(sockfd, (struct sockaddr *)&client, &size);
        working=1;

        if (fd == -1) {
            syslog(LOG_ERR, "Failed to connect to client: %s", strerror(errno));
            return -1;
        } else {
            struct sockaddr_in *client_in = (struct sockaddr_in *)&client;
            inet_ntop(AF_INET, &(client_in->sin_addr), client_ip, INET_ADDRSTRLEN);
            syslog(LOG_DEBUG, "Accepted connection from %s", client_ip);
        }
        
        FILE *outf = fopen(OUT_FILE, "ab");
        while (1)
        {
            ssize_t bytes = recv(fd, buffer, BUFSIZ, 0);
            if( bytes <= 0) { 
                if (bytes == 0) {
                    syslog(LOG_DEBUG, "Connection closed by client.");
                } else {
                    syslog(LOG_ERR, "Error receiving data: %s", strerror(errno));
                }
                break;  
            } else {
                int nl_found = 0;
                for (int i = 0; i< bytes; i++) {
                    if (buffer[i] == '\n') { bytes = i+1; nl_found = 1; break; }
                }
                
                fwrite(buffer, 1, bytes, outf);
                fflush(outf);
                if(nl_found) break;
            }
            
        }
        fclose(outf);

        outf = fopen(OUT_FILE, "rb");
        while (!feof(outf))
        {
            size_t bytes = fread(buffer, 1, BUFSIZ, outf); 
            ssize_t s = send(fd, (void*) buffer, bytes, 0);
        }
        fclose(outf);

        close(fd); 
        syslog(LOG_DEBUG, "Closed connection from %s", client_ip);
    }

    close(sockfd);
    remove(OUT_FILE);
    closelog();
    return 0;
}
