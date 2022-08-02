#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>

#define NUM_OF_ARGS 3

int main(int argc, char *argv[])
{
    openlog(NULL, 0, LOG_USER);

    if(argc != NUM_OF_ARGS)
    {
        syslog(LOG_ERR, "Invalid Number of Arguments: %d", (argc-1));
        printf("Invalid Number of Arguments: %d\n", (argc-1));
        exit(1);
    }

    char *writefile = argv[1];
    char *writestr = argv[2];
    FILE *fp = fopen(writefile, "w+");

    if(fp == NULL)
    {
        syslog(LOG_ERR, "Couldn't create a file: %s", writefile);       
        printf("Couldn't create a file: %s\n", writefile);       
        exit(1);
    }

    int n_written = fprintf(fp, "%s", writestr);
    if(n_written != strlen(writestr))
    {
        syslog(LOG_ERR, "Writing %s to %s was failed", writestr, writefile);
        printf("Writing %s to %s was failed\n", writestr, writefile);
        exit(1);
    }

    syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);
    printf("Writing %s to %s\n", writestr, writefile);

    fclose(fp);
    closelog();

    return 0;
}