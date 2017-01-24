//needed for out networking functionality
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

const int LISTEN_PORT_NUMER = 7000;
const int MAX_STRING_SIZE = 255;

int main()
{
     //sockets for the created socket and connections
     int sockfd, newsockfd;
     //length of the client addr
     socklen_t clilen;
     //structures for various information about the client and server
          struct sockaddr_in serv_addr, cli_addr;
     //the message storing strings
     char incoming_message[MAX_STRING_SIZE], temp_string[MAX_STRING_SIZE];

     //Create Socket, if fail, exit from program
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
     {
          printf("Error building Socket FD");
          exit(1);
     }
     
     //Set up our server information
          serv_addr.sin_family = AF_INET;
          serv_addr.sin_addr.s_addr = INADDR_ANY;
     
          //Assign the port number from assigned port above
          serv_addr.sin_port = htons(LISTEN_PORT_NUMER);
     
     //bind socket 
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
     {
          printf("Error in Binding");
                   exit(1);
     }
     
     //Set up the socket to listen
        if (listen(sockfd, 2) < 0)
     {
          printf("Error listening");
          exit (1);
     };
     
     //infinite loop
     while(1)
     {
          //set the clilen variable prior to passing to accept.
          //per the accept man page (explining why I am doing this)
          //it should initially contain the size of the structure pointed 
          //to by addr; on return it will contain the actual length (in bytes) 
          //of the address returned. This would fail if clilen was null
          clilen = sizeof(cli_addr);
          printf("Ready to accept connections\n");
          newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen );

          if (newsockfd < 0)
          {
               printf("Error accepting connection");
                       exit(1);
          }

          //read incoming message
          read(newsockfd, incoming_message, sizeof(incoming_message));

          //create the message to return
          sprintf(temp_string, "Hello %s, you said %s", inet_ntoa(cli_addr.sin_addr), incoming_message);
                    
          //write the return message
          if (write(newsockfd, temp_string, sizeof(incoming_message)) < 0)
          {
               printf("Error writing out");
               exit(1);
          }
          
          //close the connection
          close (newsockfd);
     }

     //This never executes
     close(sockfd);
     
     return 0;
}