// Tier 1 Server side C/C++ program to Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <mysql/mysql.h>
#include <netdb.h>
#define PORT 8080 //default port

int sqlfunction(char *);


int main(int argc, char const *argv[])
{
    int server_fd , new_socketfd, pid, portno, flag , server_length, client_length;
    struct sockaddr_in server_address, client_address ;
    //char buffer[1024] = {0};  used that in local buffer of communicate function
    int opt = 1;
    
    if (argc < 2)
    {	    fprintf(stderr, "ERROR, no port provided\n");
	    exit(1);  
    }
            
    // Creating socket file descriptor
    server_fd = socket(AF_INET , SOCK_STREAM, 0);
    
    if (server_fd < 0)
    {   perror("Socket Creation Failed");
        exit(EXIT_FAILURE);
    }
    
    // The bzero( str, n) function copies n bytes, each with a value of zero, into string s.
    bzero( (char *) &server_address, sizeof(server_address));
    
    //getting port number from argv[1] and initializing server's socketaddr_in struct (predefined) in socket.h
    portno = atoi(argv[1]);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons( portno );
    server_length = sizeof(server_address);
    /*// Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }*/
	// Tell me the need of forcefully attaching port to server
      
    // Binding socket
    if (bind(server_fd, (struct sockaddr *)&server_address, server_length)<0)
    {   perror("Bind() failed");
        exit(EXIT_FAILURE);
    }
    
    listen(server_fd , 5);
    client_length = sizeof(client_address);
    
    while(1)
    {
        new_socketfd = accept(server_fd, (struct sockaddr *)&client_address, &client_length); 
        if(new_socketfd < 0)
	{
        	perror("Accept() Error");
        	exit(EXIT_FAILURE);
	}
    
        pid = fork();
	if(pid < 0)
	{	perror("ERROR on fork");
	 	exit(EXIT_FAILURE);
	}
        
	if(pid == 0)
	{	close(server_fd);
	 	communicate(new_socketfd);
	 	exit(0);
	}
        
	else close(new_socketfd);
    }
    
	return 0;
}

void communicate (int sock)
{
    int comflag=0;
    char buffer[256];
    char msg[256];
    bzero(buffer, 256);
    
    comflag = read(sock, buffer , 255);
    printf("\n size of buffer recieved is %d",comflag);
    if(comflag<0)
    {
	    perror("ERROR reading from socket");
	    exit(EXIT_FAILURE);
    }
    printf("Message from client (ith) :%s\n" , buffer);
    comflag = sqlfunction(buffer);
    if(comflag<0)
    {
        perror("ERROR accesing DB ");
    
    }
    else if(comflag == 0)
    {
	sprintf(msg,"%s","FAILED");
	comflag=write(sock , msg ,strlen(msg) );
	printf("Waiting for new client");
    }
    else
    {
        printf("%s\t",buffer);
        comflag=write(sock , buffer ,strlen(buffer) );
	    call_back(buffer);
	    comflag=write(sock , buffer ,strlen(buffer) );
    }	
    
    if (comflag < 0)
    {
	    perror("ERROR writing to socket");
	    exit(EXIT_FAILURE);
    }
	
    return;
}

int sqlfunction(char * buffer)
{	char id[13];
    memset(id, '\0', sizeof(id));
	int count;
    MYSQL *con = mysql_init(NULL);
    // printf("TEST0");
	if (con == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(con));
		return -1;
	}
	if (mysql_real_connect(con, "localhost", "guptas", "cs744iitb", NULL, 0, NULL, 0) == NULL) 
	{
		fprintf(stderr, "%s\n", mysql_error(con));
		mysql_close(con);
		return -1;
	} 
	if (mysql_query(con, "USE testdb")) 
	{
		fprintf(stderr, "%s\n", mysql_error(con));
		mysql_close(con);
		return -1;
    }
    // printf("TEST1");
 char query[100] = "SELECT region FROM login WHERE id ='";
 strncpy(id,buffer,12);
 id[12] = '\0';
 strcat(query, id);
 strcat(query, "'");

// char query[] = "SELECT region FROM login WHERE id ='123456789123'";
//  printf("TEST2");
 	if(mysql_query(con, query)!=0)
	{	fprintf(stderr,"%s\n", mysql_error(con));
		mysql_close(con);
		return -1;
	}

	else
	{
        // printf("TEST3");
		MYSQL_RES *query_results = mysql_store_result(con);
		if (query_results)
		{
            MYSQL_ROW row;
            // printf("TEST4");
			if((row = mysql_fetch_row(query_results))!=0)
			{	
                printf("Client Authenticated\t");
                strcpy(buffer, row[0]);
                return 1;
            }
            // else{ printf("ERROR0");}
			// mysql_close(con);
			return 0;    			//No result found; 
		}
	}

}

void call_back(char *buffer)
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
//printf("New Socket to be created");    
sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(PORT);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
    {
         error("ERROR writing to socket");
         exit(1);
    }
    bzero(buffer,256);
    //printf("\nWaiting for back_End server\n");
    n = read(sockfd,buffer,sizeof(buffer));
    printf("\n size of buffer is : %d",n);
    if (n < 0) 
    {
        error("ERROR reading from socket");
        exit(1);
    }
    printf("\nRegion of Client : %s\n",buffer);
  	
}

