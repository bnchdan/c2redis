#include <arpa/inet.h>  // inet_addr
#include <stdio.h>      // printf, fgets
#include <netinet/in.h> // sockaddr, sockaddr_in
#include <sys/socket.h> // socket, connect, AF_INET, SOCK_STREAM
#include <sys/types.h>  // htonl, htons
#include <unistd.h>     // read, write, close
#include <string.h>

#define REDIS_IP "127.0.0.1"
#define REDIS_PORT 6379

#define MAX_MESSAGE_SIZE 255
#define MAX_KEY_SIZE 64
#define MAX_VALUE_SIZE 64
#define MAX_TIME_SIZE 4

#define TIMEOUT 5000 //timeout[us] for redis client

//default ports if is not ussed
#define DEFAULT_PORT_MIN 35070
#define DEFAULT_PORT_MAX 35079


int procesResponse(char out[]);

int create_connection(int port){
    struct sockaddr_in my_addr, my_addr1;
    int client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0)
      return 0;
         
    struct timeval timeout;
    timeout.tv_sec  = 0; 
    timeout.tv_usec = TIMEOUT;

    setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(REDIS_PORT);
     
    // This ip address will change according to the machine
    my_addr.sin_addr.s_addr = inet_addr(REDIS_IP);
    if (port != 0 ){
      // binding client with default port
      my_addr1.sin_family = AF_INET;
      my_addr1.sin_addr.s_addr = INADDR_ANY;
      my_addr1.sin_port = htons(port);
      
      my_addr1.sin_addr.s_addr = inet_addr(REDIS_IP);

      int reuse = 1;
      if (setsockopt(client, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
          perror("setsockopt(SO_REUSEPORT) failed");

      int myBind = bind(client, (struct sockaddr*) &my_addr1, sizeof(struct sockaddr_in));
      if ( myBind != 0)
          {printf("Unable to bind %d\n", myBind); return 0;}
    }
    socklen_t addr_size = sizeof my_addr;
    int con=1;
    
    con = connect(client, (struct sockaddr*) &my_addr, sizeof my_addr);
    if (con == 0){  
        //printf("Client Connected, port: %d\n", port);
    }
    else{
        //printf("Error in Connection\n"); 
        return 0;
    }

    return client;
}

int connect2redis(){
  int out;

  if (DEFAULT_PORT_MIN > DEFAULT_PORT_MAX ){
    return 0;
  }

  //connect to a port from DEFAULT PORTS
  for (int port=DEFAULT_PORT_MIN;  port <= DEFAULT_PORT_MAX; port++){
      out = create_connection(port);
      if (out == 0 ){
        //port is not available
        continue;
      }
      return out;
  }
  //connect to a random port
  out = create_connection(0);  
  if (out == 0 ){
    printf("ERROR!!!");
    return 0;
  }
  return out;
}

/**
* EXAMPLE: 
*     *5\r\n
*     $3\r\n
*     GET\r\n
*     $3\r\n
*     foo\r\n
**/
int doGET(char key[]){
  char command[MAX_MESSAGE_SIZE];
  bzero(command, MAX_MESSAGE_SIZE);


  int key_length = strlen(key);

  if ( key_length > MAX_KEY_SIZE )
    return 0;
  
  //create redis command
  snprintf(
            command,
            MAX_MESSAGE_SIZE, 
            "*%d\r\n$%d\r\nGET\r\n$%d\r\n%s\r\n",
            2,3,
            strlen(key),key
          );

  //coonnect to redis
  int socket_file_descriptor = connect2redis();

  if (socket_file_descriptor == 0 ){
    return 0;
  }

  //send to redis
  write(socket_file_descriptor, command, sizeof(command));

  //read output from redis
  char out[MAX_MESSAGE_SIZE];
  int num_read = read(socket_file_descriptor, out, sizeof(out));
  
  printf("\nout %s \n", out);

  //close socket
  close(socket_file_descriptor);
  
  return 1;
}

/**
* EXAMPLE: 
*     *5\r\n
*     $3\r\n
*     SET\r\n
*     $3\r\n
*     foo\r\n
*     $4\r\n
*     test\r\n
*     $2\r\n
*     EX\r\n
*     $2\r\n
*     60\r\n
**/
int doSET(char key[],  char value[], char time_expire[]){

  char command[MAX_MESSAGE_SIZE];
  bzero(command, MAX_MESSAGE_SIZE);

  int key_length = strlen(key), value_length = strlen(value), time_expire_length = strlen(time_expire);

  if ( key_length > MAX_KEY_SIZE || value_length > MAX_VALUE_SIZE || time_expire_length > MAX_TIME_SIZE )
    return 0;
  
  //create redis command
  snprintf(
            command,
            MAX_MESSAGE_SIZE, 
            "*%d\r\n$%d\r\nSET\r\n$%d\r\n%s\r\n$%d\r\n%s\r\n$%d\r\n%s\r\n$%d\r\n%s\r\n",
            5,3,
            strlen(key),key, 
            strlen(value), value,
            2, "EX", 
            strlen(time_expire), time_expire
          );

  //coonnect to redis
  int socket_file_descriptor = connect2redis();

  if (socket_file_descriptor == 0 ){
    return 0;
  }

  //send to redis
  write(socket_file_descriptor, command, sizeof(command));

  //read output from redis
  char out[MAX_MESSAGE_SIZE];
  bzero(out, MAX_MESSAGE_SIZE);

  int num_read = read(socket_file_descriptor, out, sizeof(out));
  
  printf("\nout %s ", out);

  //close socket
  close(socket_file_descriptor);
  
  return 1;
}


int doPing(){
  char command[MAX_MESSAGE_SIZE];
  bzero(command, MAX_MESSAGE_SIZE);

  //create redis command
  snprintf(
            command,
            MAX_MESSAGE_SIZE, 
            "*%d\r\n$%d\r\nPING\r\n",
            1,4
          );

  //coonnect to redis
  int socket_file_descriptor = connect2redis();

  if (socket_file_descriptor == 0 ){
    return 0;
  }

  //send to redis
  write(socket_file_descriptor, command, sizeof(command));

  //read output from redis
  char out[MAX_MESSAGE_SIZE];
  bzero(out, MAX_MESSAGE_SIZE);

  int num_read = read(socket_file_descriptor, out, sizeof(out));
  
  printf("\nout %s \n", out);
  
  //close socket
  close(socket_file_descriptor);
  
  return 1;
}

void doCommand(char command[]){
  int socket_file_descriptor = connect2redis();

  if (socket_file_descriptor == 0 ){
    return;
  }
  char buffer[MAX_MESSAGE_SIZE];
  bzero(buffer, MAX_MESSAGE_SIZE);

  if (strlen(command) > MAX_MESSAGE_SIZE ){
	return;
  }
  strcpy(buffer, command);
  char out[MAX_MESSAGE_SIZE];
  bzero(out, MAX_MESSAGE_SIZE);

  write(socket_file_descriptor, buffer, strlen(buffer));
  
  int num_read = read(socket_file_descriptor, out, sizeof(out));
  printf("\nout %s\n", out);

  close(socket_file_descriptor);
  
}
