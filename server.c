#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */
#include <dirent.h>	      /*  file dir functions	    */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <ctype.h>


/*  Global constants  */

#define ECHO_PORT          (2002)
#define MAX_LINE           (1000)
#define LISTENQ            (1024)   /*  Backlog for listen()   */

/*Function declarartions*/
ssize_t Readline(int, void *, size_t);
ssize_t Writeline(int, const void *, size_t);

int main(int argc, char *argv[]) {
    int       list_s;                /*  listening socket          */
    int       conn_s;                /*  connection socket         */
    short int port;                  /*  port number               */
    struct    sockaddr_in servaddr;  /*  socket address structure  */
    char      buffer[MAX_LINE];      /*  character buffer          */
	char 	  msg[MAX_LINE + 10];    /*  character buffer 		   */
    char      *endptr;               /*  for strtol()              */
    ssize_t   n;		     		 /*  for reading from buffer   */
    char      c;		     		 /*  for reading from buffer   */
	int 	  i;					 /*  for reading from buffer   */
    FILE      *fp;		     		 /*  for file reading          */
    int		  s;	     			 /*  for file reading	   	   */
    int	      f_len;		         /*  to store file length 	   */
	void 	  *ptr; 				 /*	for file reading */


    /*  Get port number from the command line, and
        set to default port if no arguments were supplied  */

    if ( argc == 2 ) {
        port = atoi(argv[1]);
    }
    else if ( argc < 2 ) {
	port = ECHO_PORT;
    }
    else {
	fprintf(stderr, "ECHOSERV: Invalid arguments.\n");
	exit(EXIT_FAILURE);
    }

	
    /*  Create the listening socket  */

    if ( (list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
	fprintf(stderr, "ECHOSERV: Error creating listening socket.\n");
	exit(EXIT_FAILURE);
    }


    /*  Set all bytes in socket address structure to
        zero, and fill in the relevant data members   */

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(port);


    /*  Bind our socket addresss to the 
	listening socket, and call listen()  */

    if ( bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
	fprintf(stderr, "ECHOSERV: Error calling bind()\n");
	exit(EXIT_FAILURE);
    }

    if ( listen(list_s, LISTENQ) < 0 ) {
	fprintf(stderr, "ECHOSERV: Error calling listen()\n");
	exit(EXIT_FAILURE);
    }

    
    /*  Enter an infinite loop to respond
        to client requests and echo input  */

    while ( 1 ) {

		/*  Wait for a connection, then accept() it  */

		if ( (conn_s = accept(list_s, NULL, NULL) ) < 0 ) {
			fprintf(stderr, "ECHOSERV: Error calling accept()\n");
			exit(EXIT_FAILURE);
		} else {
			while(1){
				i = 0;
				// Retrieve first line from the connected socket
				memset(buffer,0, sizeof(buffer));
				while ( (n = read(conn_s, &c, 1)) > 0 ) {
					buffer[i] = c;
					//strcat(buffer, c);
					i++;
					if ( (c == '\n')){
						break;
					}    
				}

				i = 0;
				printf(buffer);
				// Check what type of request
				if((strcmp(buffer, "CAP\n") == 0) || (strcmp(buffer, "CAP") == 0))
				{ //String request
					memset(buffer, 0, sizeof(buffer));
					memset(msg, 0, sizeof(msg));
					//Read next line
					while ( (n = read(conn_s, &c, 1)) > 0 ) {
						buffer[i] = c;
						i++;
						if ( (c == '\n')){
							break;
						}    
					}
					printf(buffer);
					sprintf(msg, "%d", strlen(buffer));
					strcat(msg, "\n");
			
					i = 0;
					while( buffer[i]){
						buffer[i] = toupper(buffer[i]);
						i++;
					}
					strcat(msg, buffer);

					// Write back the CAP string to the same socket.
					Writeline(conn_s, msg, strlen(msg));
					//bzero(buffer, MAX_LINE);
					//bzero(msg, MAX_LINE);    
				} 
				else if((strcmp(buffer, "FILE\n") == 0) || (strcmp(buffer, "FILE") == 0))
				{ //File request
					memset(buffer, 0, sizeof(buffer));
					//Read next line
					i = 0;
					while ( (n = read(conn_s, &c, 1)) > 0 ) 
					{
						if ( (c == '\n')){
							break;
						} 
						buffer[i] = c;
						i++;   
					} 	 
					printf(buffer);
					fp = fopen(buffer,"r");
					if(fp != NULL){
						fseek(fp, 0L, SEEK_END);
						f_len = ftell(fp);
						rewind(fp);
			
						bzero(buffer, MAX_LINE);
						bzero(msg, MAX_LINE);
						sprintf(msg, "%d", f_len);
						strcat(msg, "\n");
						printf("Size: %d", f_len);
						ptr = malloc(1);
						while(1)
						{
							if( feof(fp) ){ 
								break ;
							}							
							fread(ptr, 1, 1, fp);
							strcat(msg, ptr);
							printf(ptr);
						}
						fclose(fp);
						printf("Closed File");
					} 
					else 
					{//File not found
						printf("Setting msg not found");						
						memset(msg, 0, sizeof(msg));
						strcpy(msg, "9");
						strcat(msg, "\n");
						strcat(msg, "NOT FOUND");
					}

			   		Writeline(conn_s, msg, sizeof(msg));
				}
				else if(strcmp(buffer, "QUIT") == 0)
				{//Close connection request
					memset(buffer, 0, sizeof(buffer));	    
					//  Close the connected socket 

					if ( close(conn_s) < 0 ) {
						fprintf(stderr, "ECHOSERV: Error calling close()\n");
						exit(EXIT_FAILURE);
					}
				} 
				else 
				{
					//Writeline(conn_s, "Oops, An error occured", 23);
				}
			}
		}
	
    }

}


/*  Read a line from a socket  */

ssize_t Readline(int sockd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char    c, *buffer;

    buffer = vptr;

    for ( n = 1; n < maxlen; n++ ) {
	
	if ( (rc = read(sockd, &c, 1)) == 1 ) {
	    *buffer++ = c;
	    if ( c == '\n' )
		break;
	}
	else if ( rc == 0 ) {
	    if ( n == 1 )
		return 0;
	    else
		break;
	}
	else {
	    if ( errno == EINTR )
		continue;
	    return -1;
	}
    }

    *buffer = 0;
    return n;
}


/*  Write a line to a socket  */

ssize_t Writeline(int sockd, const void *vptr, size_t n) {
    size_t      nleft;
    ssize_t     nwritten;
    const char *buffer;

    buffer = vptr;
    nleft  = n;

    while ( nleft > 0 ) {
	if ( (nwritten = write(sockd, buffer, nleft)) <= 0 ) {
	    if ( errno == EINTR )
		nwritten = 0;
	    else
		return -1;
	}
	nleft  -= nwritten;
	buffer += nwritten;
    }

    return n;
}
