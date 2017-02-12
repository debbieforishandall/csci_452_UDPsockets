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
	int 	  tcp_list_s;			 /*  tcp listening socket 	   */
    int       conn_s;                /*  connection socket         */
    short int port;                  /*  port number               */
    struct    sockaddr_in servaddr;  /*  socket address structure  */
    char      buffer[MAX_LINE];      /*  character buffer          */
	char 	  msg_type[MAX_LINE + 1];	 /*	 character buffer			*/
	char 	  tcp_buff[MAX_LINE];	 /*	 tcp port string buffer		*/
	char 	  msg[MAX_LINE + 10];    /*  character buffer 		   	*/
	short int tcp_port;				 /* tcp port number 			*/
    char      *endptr;               /*  for strtol()              	*/
    ssize_t   n;		     		 /*  for reading from buffer   	*/
    char      c;		     		 /*  for reading from buffer   	*/
	int 	  i;					 /*  for reading from buffer   	*/
    FILE      *fp;		     		 /*  for file reading          	*/
    int		  sent;	     			 /*  for file sending	   	   	*/
    int	      f_len;		         /*  to store file length 	   	*/
	void 	  *ptr; 				 /*	for file reading 			*/
	struct 	  sockaddr_in  si_other;
	struct 	  sockaddr_in si_tcp;	/* for tcp file sending */
	int 	  s_len;


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

	s_len = sizeof(si_other);
    /*  Create the listening socket  */

    if ( (list_s = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
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

    if (  bind(list_s , (struct sockaddr*)&servaddr, sizeof(servaddr) ) == -1){
		fprintf(stderr, "ECHOSERV: Error calling bind()\n");
		exit(EXIT_FAILURE);
    }

    /*  Enter an infinite loop to respond
        to client requests and echo input  */

    while ( 1 ) {

		i = 0;
		// Retrieve first line from the connected socket
		memset(buffer,0, sizeof(buffer));
		memset(msg_type, 0, sizeof(msg_type));
		memset(msg, 0, sizeof(msg));
		printf("Waiting for data...");
        fflush(stdout);
		printf("waiting on port %d\n", port);
        n = recvfrom(list_s, buffer, MAX_LINE, 0, (struct sockaddr *) &si_other, &s_len);
        printf("received %d bytes\n", n);
        if (n > 0) {
                buffer[n] = 0;
                printf("received message: \"%s\"\n", buffer);
        }
		
		printf("made it \n");
		printf("Before spliting: %s", buffer);
		
		//Get the request type
		char* pch = strchr(buffer,'\n');
		strncpy(msg_type, buffer, pch-buffer+1);
		msg_type[MAX_LINE] = '\0';
	    
		/* copy unprocessed data to new buffer*/
		strcpy(msg, &buffer[pch-buffer+1]);

		printf("After spliting: %s", buffer);
		printf("Message Type: %s\n", msg_type);
		// Check what type of request
		if((strcmp(msg_type, "CAP\n") == 0) || (strcmp(msg_type, "CAP") == 0))
		{ //String request
			memset(buffer, 0, sizeof(buffer));

			fflush(stdout);
			printf("In CAP block: %s", msg);
			//sprintf(buffer, "%d", strlen(msg));
			//strcat(buffer, "\n");
	
			i = 0;
			while( msg[i]){
				msg[i] = toupper(msg[i]);
				i++;
			}
			strcat(buffer, msg);

			// Write back the CAP string to the same socket.
			if (sendto(list_s, buffer, strlen(buffer), 0, (struct sockaddr*) &si_other, sizeof(si_other)) == -1)
		    {
		        exit(EXIT_FAILURE);
		    }
			//bzero(buffer, MAX_LINE);
			//bzero(msg, MAX_LINE);    
		} 
		else if((strcmp(msg_type, "FILE\n") == 0) || (strcmp(msg_type, "FILE") == 0))
		{ //File request
			memset(buffer, 0, sizeof(buffer));
			memset(tcp_buff, 0, sizeof(tcp_buff));

			//Get the file name
			char* ptr = strchr(msg,'\n');
			strncpy(buffer, msg, ptr-msg);
			buffer[MAX_LINE] = '\0';
		
			// copy tcp port to new buffer
			strcpy(tcp_buff, &msg[ptr-msg+1]);	
			tcp_port = atoi(tcp_buff);
			 
			printf("File name: %s", buffer);
			printf("Tcp Port: %d\n", tcp_port);

			
			//Open file
			fp = fopen(buffer,"r");

			if(fp == NULL){
				//File not found
				printf("File not foune\n");
				memset(msg, 0, sizeof(msg));
				strcpy(msg, "NOT FOUND");
				strcat(msg, "\n");
				if (sendto(list_s, msg, strlen(msg), 0, (struct sockaddr*) &si_other, sizeof(si_other)) == -1)
				{
				    exit(EXIT_FAILURE);
				}
			}
			else{
				//Send OK\n###\n to client

				if (sendto(list_s, buffer, strlen(buffer), 0, (struct sockaddr*) &si_other, sizeof(si_other)) == -1)
				{
					exit(EXIT_FAILURE);
				}
				/*  Set all bytes in socket address structure to
				zero, and fill in the relevant data members   */

				memset(&si_tcp, 0, sizeof(si_tcp));
				si_tcp.sin_family      = AF_INET;
				si_tcp.sin_addr.s_addr = htonl(INADDR_ANY);
				si_tcp.sin_port        = htons(tcp_port);

				/*  Bind our socket addresss to the 
					listening socket, and call listen()  */

				int yes=1;

				if (setsockopt(tcp_list_s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
					perror("setsockopt");
					exit(1);
				}

				if ( bind(tcp_list_s, (struct sockaddr *) &si_tcp, sizeof(si_tcp)) < 0 ) {
					fprintf(stderr, "ECHOSERV: Error calling bind()\n");
					exit(EXIT_FAILURE);
				}

				if ( listen(tcp_list_s, LISTENQ) < 0 ) {
					fprintf(stderr, "ECHOSERV: Error calling listen()\n");
					exit(EXIT_FAILURE);
				}

				if ( (conn_s = accept(tcp_list_s, NULL, NULL) ) < 0 ) {
					fprintf(stderr, "ECHOSERV: Error calling accept()\n");
					exit(EXIT_FAILURE);
				} else {		
	
					fseek(fp, 0L, SEEK_END);
					f_len = ftell(fp);
					rewind(fp);
	
					bzero(buffer, MAX_LINE);
					bzero(msg, MAX_LINE);

					sprintf(msg, "%d", f_len);
					strcpy(buffer, "OK\n");
					strcat(buffer, msg);
					strcat(buffer, "\n");
				
					printf("Size: %d", f_len);

					ptr = malloc(1);
					n = 0;
					

					//write the file to socket
					while ( sent <= f_len)
					{
						fread(ptr, 1, 1, fp);
						if( feof(fp) ){ 
							break ;
						}
						if((n = write(conn_s, ptr, 1)) != -1){
							printf(ptr);
							sent+= n;
						}
					}

					printf("Closed File");
					//  Close the connected socket 

					if ( close(conn_s) < 0 ) {
						fprintf(stderr, "ECHOSERV: Error calling close()\n");
						exit(EXIT_FAILURE);
					}
				}
			} 
		}
		else if(strcmp(buffer, "QUIT") == 0)
		{
			memset(buffer, 0, sizeof(buffer));
		} 
		else 
		{
			//Writeline(conn_s, "Oops, An error occured", 23);
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
