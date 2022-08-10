#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOCKET_DATA_FILEPATH "/var/tmp/aesdsocketdata"
#define PORT "9000"
#define MAX_PACKET_SIZE 1024

volatile int sig_received = 0;

void sig_handler(int signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        syslog(LOG_DEBUG, "Caught signal, exiting: %d", signo);
        printf("Caught signal, exiting: %d\n", signo);
	sig_received = 1;
    }
}


int main(int argc, char* argv[])
{
    printf("------------- Program start --------------\n");
    openlog(NULL, 0, LOG_USER);    

    // daemonize 
    if (argc == 2 && !(strcmp(argv[1], "-d")))
    {
	int pid = fork();
	if (pid == 0) 
	{
	    syslog(LOG_DEBUG, "Start as a daemon");
	    setsid();
	    chdir("/");
        }
	else if (pid > 0)
	{
            return 0;		
	}
	else 
	{
            syslog(LOG_ERR, "Failed to create a daemon");
	    perror("daemon");
            exit(EXIT_FAILURE);
	}
    }

    // To accept new client connection requests until SIGINT or SIGTERM is received.
    struct sigaction act;

    act.sa_handler = sig_handler;
    sigemptyset (&act.sa_mask);
    act.sa_flags = 0;


    if (sigaction(SIGINT, &act, NULL) == -1 || sigaction(SIGTERM, &act, NULL) == -1)
    {
        syslog(LOG_ERR, "Failed to execute sigaction()");
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    
    // opens a stream socket bound
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int ssfd, s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;/* TCP */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, PORT, &hints, &result);
    if (s != 0) 
    {
        syslog(LOG_ERR, "Failed to execute getaddrinfo()");
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    // failing and returning -1 if any of the socket connection steps fail.
    for (rp = result; rp != NULL; rp = rp->ai_next) 
    {

        ssfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (ssfd == -1) 
	{
	    continue;
	} 
	else if (setsockopt(ssfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) 
	{
            syslog(LOG_ERR, "Failed to run setsockopt()");
	    perror("setsockopt");
	    continue;
	}

        if (bind(ssfd, rp->ai_addr, rp->ai_addrlen) == 0) break; /* Success */
        
	close(ssfd);
    }

    if (rp == NULL) /* No address succeeded */
    {
        syslog(LOG_ERR, "Failed to bind");
        perror("bind");
	return -1;
    }

    freeaddrinfo(result);           /* No longer needed */

    // Listens and accepts a connection.
    if (listen(ssfd, 10) == -1)
    {
        syslog(LOG_ERR, "Failed to listen");
        printf("Failed to listen");
        perror("listen");
        close(ssfd);	
	return -1;
    }

    // creating this file if it doesn’t exist.
    int fd = open(SOCKET_DATA_FILEPATH, O_RDWR|O_CREAT|O_TRUNC|O_APPEND, 0644);
    if (fd == -1) 
    {
        syslog(LOG_ERR, "Cannot open output file");
        perror("open");
	close(ssfd);
	return -1;
    }

    int csfd;
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = (socklen_t)sizeof(client_addr);
    ssize_t bytes_read;
    char buf[MAX_PACKET_SIZE];

    while(!sig_received) 
    {
        csfd = accept(ssfd, (struct sockaddr*)&client_addr, &client_addrlen);
        if (csfd < 0 && errno == EINTR) break;
	else if (csfd < 0)
	{
            syslog(LOG_ERR, "Failed to executre accept()");
            perror("accept");
	    goto EXIT;
	}

        syslog(LOG_DEBUG, "Accepted connection from %s", inet_ntoa(client_addr.sin_addr));

        // Receives data over the connection and appends to file.
        do	
	{
            if ((bytes_read = read(csfd, buf, MAX_PACKET_SIZE)) == -1) 
                goto EXIT;

            if (write(fd, buf, bytes_read) < bytes_read) 
	    {
                syslog(LOG_ERR, "Failed to write data");
                perror("write");
	        break;	
	    }
        // use a newline to separate data packets received.
	} while(buf[bytes_read - 1] != '\n');

        // Returns the content to the client as soon as the received data packet completes.
        lseek(fd, 0, SEEK_SET);
	do {
            bytes_read = read(fd, buf, MAX_PACKET_SIZE);
            write(csfd, buf, bytes_read);
	} while(bytes_read);
//        lseek(fd, 0, SEEK_END);

        syslog(LOG_DEBUG, "Closed connection from %s", inet_ntoa(client_addr.sin_addr));
        close(csfd);
    }

EXIT:   

    printf("------------- Program end --------------\n");
    remove(SOCKET_DATA_FILEPATH);
    closelog();
    close(csfd);
    close(ssfd);
    close(fd);
    return -1;
}

