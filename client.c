#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>


/*  Global constants  */

#define MAX_LINE           (1000)

ssize_t Readline(int, void *, size_t);
ssize_t Writeline(int, const void *, size_t);

/*  main()  */

int main(int argc, char *argv[]) {

    int			conn_s;                 /*  connection socket         */
	int 		tcp_conn_s;				/*  tcp connection socket     */
    int 		portno;                 /*  tcp listening port number */
    int         serverPort;				/*	server UDP port number	  */
    struct		sockaddr_in servaddr;   /*  socket address structure  */
	struct 		sockaddr_in tcp_addr;	/*	tcp socket address structure */
    char		buffer[MAX_LINE];       /*  character buffer          */
    char		msg[MAX_LINE + 7];		/*  for socket read and write */
	int 		f_size;					/* 	holds size of file from server */
	char		f_name[MAX_LINE];		/*	holds file name			*/
    struct		hostent	*server;		/*  holds remote IP address   */
    char		user_entry[10];			/*  for user entered command  */
	ssize_t		n;						/*  for reading from buffer   */
	ssize_t		rv;						/*	holds size returned by recvfrom() */
    char		c;						/*  for reading from buffer   */
	int 		i, s;					/*  for reading from buffer   */
	FILE 		*fp;					/*	for file writing		  */
	void 		*ptr;					/*	for file writing		  */
	size_t		received;				/*	size of file received	*/	
	int			slen;					/* 	stores size of servaddr 	*/
	ssize_t		buf_used;				/*	amount of data received from buffer */


    /*  Get command line arguments  */

    /*  eg. ./client 127.0.0.1 7000  */

    //check if command line arguments are valid
    if(argc < 4){
        printf("Invalid Arguments!");
	exit(1);
    }

    //get the remort port number and server address from command line arguments
    portno = atoi(argv[1]);
    server = gethostbyname(argv[2]);
    serverPort = atoi(argv[3]);

     if (server == NULL) {
             fprintf(stderr,"ERROR, no such host\n");
             exit(1);
     }
	

    /*  Create the listening socket  */

    if ( (conn_s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ) {
	fprintf(stderr, "ECHOCLNT: Error creating listening socket.\n");
	exit(EXIT_FAILURE);
    }


    /*  Set all bytes in socket address structure to
        zero, and fill in the relevant data members   */

    bzero((char *) &servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(serverPort);

    /*  Byte copy the address retrieved from server  into the
        server addr structure      */
     bcopy((char *)server->h_addr, (char *)&servaddr.sin_addr.s_addr, server->h_length);

	slen = sizeof(servaddr);
   
    do{
		memset(user_entry, 0, sizeof(user_entry));
		
		printf("Enter 's' for a string request: \n");
		printf("Enter 't' for a file request: \n");
		printf("Enter 'q' to quit.\n");
		scanf("%s", user_entry);
		//fflush(stdin);
		//printf("user_entry:  |%s|", user_entry);
		getchar();
		if(!(strcmp(user_entry, "s")) || !(strcmp(user_entry, "s\n")))
		{
			memset(buffer, 0, sizeof(buffer));
			memset(msg, 0, sizeof(msg));
			// Get string to echo from user  
			printf("Enter the string to echo: ");
			strncpy(msg, "CAP\n", 5);

		    fgets(buffer, MAX_LINE, stdin);
			strcat(msg, buffer);
			strcat(msg, "\n");
			//printf(msg);

		    // Send string to server
		    sendto(conn_s, msg, strlen(msg), 0, (struct sockaddr *) &servaddr, sizeof(servaddr));
			//Read first line from server
			memset(buffer, 0, sizeof(buffer));
		    //try to receive some data
			//TO DO: set flags to MSG_DONTWAIT
		    rv = recvfrom(conn_s, buffer, sizeof(buffer), 0, (struct sockaddr *)&servaddr, &slen);
			if (rv == 0) {
				fprintf(stderr, "Connection closed.\n");
				abort();
			}
		    if (rv < 0 && errno == EAGAIN) {
			    /* no data for now, call back when the socket is readable */
			    return;
		    }
		    if (rv < 0) {
			    perror("Connection error");
			    abort();
		    }
			buf_used += rv;
	        // Output echoed string
			printf("Server Response: %s\n", buffer);
			fflush(stdin);
		} 
		else if(!(strcmp(user_entry, "t")) || !(strcmp(user_entry, "t\n")))
		{
			memset(buffer, 0,  sizeof(buffer));
			memset(msg, 0, sizeof(msg));
			memset(f_name, 0, sizeof(f_name));
			printf("Enter the file name: ");
			strcpy(msg, "FILE\n");
		    
			fgets(buffer, MAX_LINE, stdin);
			strcpy(f_name, buffer);
			int nlen = strlen(f_name);
			//Remove '\n' char if any from name of file from fgets
			if (nlen > 0 && f_name[nlen-1] == '\n')
			{
				f_name[nlen-1] = '\0';
			}

			strcat(msg, buffer);
			strcat(msg, "\n");

			memset(buffer, 0, sizeof(buffer));

			//Put the tcp port of the client into the buffer char array
			sprintf(buffer, "%d", portno); 

			//Append the tcp port string to the message being sent
			strcat(msg, buffer);
			strcat(msg, "\n");
			//printf(msg);

		    // Send string to server
			sendto(conn_s, msg, strlen(msg), 0, (struct sockaddr *) &servaddr, sizeof(servaddr));
			
			memset(buffer, 0, sizeof(buffer));
			i = 0;

			rv = recvfrom(conn_s, buffer, sizeof(buffer), 0, (struct sockaddr *)&servaddr, &slen);
			if (rv == 0) {
				fprintf(stderr, "Connection closed.\n");
				abort();
			}
		    if (rv < 0 && errno == EAGAIN) {
			    /* no data for now, call back when the socket is readable */
			    return;
		    }
		    if (rv < 0) {
			    perror("Connection error");
			    abort();
		    }
			
			if (rv > 0) {
                buffer[rv] = 0;
                printf("received message: \"%s\"\n", buffer);
			}
	
			//Process received string
			memset(msg, 0, sizeof(msg));

			char* pch = strchr(buffer,'\n');
			strncpy(msg, buffer, pch-buffer+1);
			msg[MAX_LINE+6] = '\0';
			printf("File Status: %s\n", msg);
			//ptr = malloc(1);
			
			//Check if file is not found
			if(strcmp(msg, "NOT FOUND\n") == 0){
				printf("Specified file not found\n");
			} else if (strcmp(msg, "OK\n") == 0){
				//received = 9
				memset(msg, 0, sizeof(msg));
				strcpy(msg, &buffer[pch-buffer]);
				printf("Size: %s", msg);
				/*int len = strlen(msg);
				//Remove '\n' char if any from name of file from fgets
				if (len > 0 && msg[len-1] == '\n')
				{
					msg[len-1] = '\0';
				}*/

				 /*  Create the tcp listening socket  */

				if ( (tcp_conn_s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
					fprintf(stderr, "ECHOCLNT: Error creating listening socket.\n");
					exit(EXIT_FAILURE);
				}


				/*  Set all bytes in socket address structure to
					zero, and fill in the relevant data members   */

				bzero((char *) &tcp_addr, sizeof(tcp_addr));
				tcp_addr.sin_family      = AF_INET;
				tcp_addr.sin_port        = htons(portno);

				/*  Byte copy the address retrieved from server  into the
					server addr structure      */
				 bcopy((char *)server->h_addr, (char *)&tcp_addr.sin_addr.s_addr, server->h_length);
		
				/*  connect() to the remote echo server  */

				if ( connect(tcp_conn_s, (struct sockaddr *) &tcp_addr, sizeof(tcp_addr) ) < 0 ) {
					printf("ECHOCLNT: Error calling connect()\n");
					exit(EXIT_FAILURE);
				}


				//Open a new file for writing
				fp = fopen (f_name, "w+");
				memset(buffer, 0, sizeof(buffer));
				n = 0;
				received = 0;
				f_size = 0;
				f_size = atoi(msg);
				printf("File size is %d in int\n", f_size);

				//Read the file received from server into new file
				while (received <= f_size - 1)
				{	
					n = read(tcp_conn_s, &c, 1);
					if(n > 0)
				    {
						fwrite(&c, 1, 1, fp);
						//printf(&c);
						received += n;
						//printf("received %d so far \n", received);
					} 
				}
				
				fclose(fp);
				close(tcp_conn_s);
				printf("Received: %d\n", received);
			}
			//fflush(stdin);
			//free(ptr);
		} 
		else if (strcmp(user_entry, "q"))
		{
			//printf("Invalid Input!\n");
		}
	
    } 
	while(strcmp(user_entry, "q") );
    
	memset(msg, 0, sizeof(msg));
	/*strcpy(msg, "QUIT\n");
	strcat(msg, "exiting socket\n");
	sendto(conn_s, msg, strlen(msg), 0, (struct sockaddr *) &servaddr, sizeof(servaddr));*/

    return EXIT_SUCCESS;
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
