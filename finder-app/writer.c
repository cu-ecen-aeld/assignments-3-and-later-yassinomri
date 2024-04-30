#include <stdio.h>
#include <syslog.h>

int main(int argc, char const *argv[]) {
    openlog(argv[0], LOG_PID, LOG_USER); 

    // Check if the number of arguments is not equal to 3 (beside the program name itself)
    if (argc != 3) {
        syslog(LOG_ERR, "Usage: %s <file_path> <text_string>", argv[0]);
        closelog(); 
        return 1;
    }

    // Get the file path and text string from command line arguments
    const char *writefile = argv[1];
    const char *writestr = argv[2];

    // Write the text string to the file, overwriting any existing file
    FILE *fp;
    fp = fopen(writefile, "w");
    if (fp == NULL) {
        syslog(LOG_ERR, "Error: Could not create file %s", writefile);
        closelog(); 
        return 1;
    }
    fprintf(fp, "%s\n", writestr);
    fclose(fp);

    // Log the successful operation with syslog
    syslog(LOG_DEBUG, "Writing \"%s\" to %s", writestr, writefile);

    closelog(); 
    return 0;
}

