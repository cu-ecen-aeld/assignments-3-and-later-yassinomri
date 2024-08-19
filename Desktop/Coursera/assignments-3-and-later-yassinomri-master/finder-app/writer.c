#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    openlog("writer", LOG_PERROR | LOG_PID, LOG_USER);

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file> <string>\n", argv[0]);
        syslog(LOG_ERR, "Invalid number of arguments: %d", argc);
        closelog();
        return 1;
    }

    const char *file_path = argv[1];
    const char *string_to_write = argv[2];

    FILE *file = fopen(file_path, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s: %s\n", file_path, strerror(errno));
        syslog(LOG_ERR, "Error opening file %s: %s", file_path, strerror(errno));
        closelog();
        return 1;
    }

    fprintf(file, "%s", string_to_write);
    fclose(file);

    syslog(LOG_DEBUG, "Writing %s to %s", string_to_write, file_path);
    closelog();
    return 0;
}

