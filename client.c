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
    int 		portno;                 /*  tcp listening port number */
    int         serverPort;				/*	server UDP port number	  */
    struct		sockaddr_in servaddr;   /*  socket address structure  */
    char		buffer[MAX_LINE];       /*  character buffer          */
    char		msg[MAX_LINE + 7];		/*  for socket read and write */
	char 		f_size[MAX_LINE];		/* 	holds size of file from server */
	char		f_name[MAX_LINE];		/*	holds file name			*/
    struct		hostent	*server;		/*  holds remote IP address   */
    char		user_entry[10];			/*  for user entered command  */
	ssize_t		n;						/*  for reading from buffer   */
    char		c;						/*  for reading from buffer   */
	int 		i, s;					/*  for reading from buffer   */
	FILE 		*fp;					/*	for file writing		  */
	void 		*ptr;					/*	for file writing		  */
	size_t		received;				/*	size of file received	*/	
	int			slen;


    /*  Get command line arguments  */

    /*  eg. ./client 127.0.0.1 7000  */

    //check if command line arguments are valid
    if(argc < 4){
        printf("Invalid Arguments!");
	exit(1);
    }

    //get the remort port number and server address from command line arguments
    portno = atoi(argv[2]);
    server = gethostbyname(argv[1]);
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
		printf("user_entry:  |%s|", user_entry);
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

		    // Send string to echo server, and retrieve response 
		    sendto(conn_s, msg, strlen(msg), 0, (struct sockaddr *) &servaddr, sizeof(servaddr));
			//Read first line from server
			memset(buffer, 0, sizeof(buffer));
		     //try to receive some data, this is a blocking call
		    if (recvfrom(s, buffer, sizeof(buffer), 0, (struct sockaddr *) &servaddr, &slen) == -1)
		    {
		        exit(EXIT_FAILURE);
		    }
	        // Output echoed string
			printf("String Size: %s\n", buffer); 
	        printf("Server response: %s\n", msg);
			fflush(stdin);
		} 
		else if(!(strcmp(user_entry, "t")) || !(strcmp(user_entry, "t\n")))
		{
			memset(buffer, 0,  sizeof(buffer));
			memset(msg, 0, sizeof(msg));
			printf("Enter the file name: ");
			strncpy(msg, "FILE\n", 6);
		    
			fgets(buffer, MAX_LINE, stdin);
			strcat(msg, buffer);
			strcat(msg, "\n");
			//printf(msg);

		    // Send string to echo server, and retrieve response 
		    Writeline(conn_s, msg, strlen(msg));
			
			memset(f_size, 0, sizeof(f_size));
			i = 0;

			//Read the size of the file to be received
		    while ( (n = read(conn_s, &c, 1)) > 0 ) 
			{
		        if ( (c == '\n')){
			    	break;
		        } 
				f_size[i] = c;
				i++;   
		    }
			
			i = 0;
			//ptr = malloc(1);
			memset(msg, 0, sizeof(msg));
			printf("Size: %d\n", atoi(f_size));
			//Check the first 9 bytes received to see if the file was not found
			while ( ((n = read(conn_s, &c, 1)) > 0))
			{
				if (i <= 9) 
				{
					msg[i] = c;
					i++; 
				} 
				else 
				{
					break;
				}
		    }
			received = i;

			//Check if file is not found
			if(strcmp(msg, "NOT FOUND") == 0){
				printf("Specified file not found\n");
			} else {
				//received = 9
				int len = strlen(buffer);
				//Remove '\n' char if any from name of file from fgets
				if (len > 0 && buffer[len-1] == '\n')
				{
					buffer[len-1] = '\0';
				}
				//Open a new file for writing
				fp = fopen (buffer, "w+");
				memset(buffer, 0, sizeof(buffer));
				//Write the first 9 bytes to the file
				printf(msg);
				fwrite(msg , 1 , strlen(msg) , fp );
				//Read the rest of file received from server into new file
				while ( ((n = read(conn_s, &c, 1)) > 0))
				{
					if(received < atoi(f_size))
				    {
						fwrite(&c, 1, 1, fp);
						printf(&c);
						received+= n;
						//printf("received so far: %d", received);
					} else {
						break;
					}
				}
				
				fclose(fp);
				printf("Received: %d", received);
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
    
	Writeline(conn_s, "QUIT", 5);

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
