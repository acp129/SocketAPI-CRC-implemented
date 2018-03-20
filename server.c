#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include<signal.h>


int socketVal;
int severSocketVal;

char *MakeCRC(char *BitString)
{
    static char Res[9];                                 // CRC Result
    char CRC[8];
    int  i;
    char DoInvert;

    for (i=0; i<8; ++i)  CRC[i] = 0;                    // Init before calculation

    for (i=0; i<strlen(BitString); ++i)
        {
        DoInvert = ('1'==BitString[i]) ^ CRC[7];         // XOR required?

        CRC[7] = CRC[6];
        CRC[6] = CRC[5];
        CRC[5] = CRC[4] ;
        CRC[4] = CRC[3] ;
        CRC[3] = CRC[2];
        CRC[2] = CRC[1] ^ DoInvert;
        CRC[1] = CRC[0] ^ DoInvert;
        CRC[0] = DoInvert;
        }

    for (i=0; i<8; ++i)  Res[7-i] = CRC[i] ? '1' : '0'; // Convert binary to ASCII
    Res[8] = 0;                                         // Set string terminator

    return(Res);
}



void handle_sigint(int sig)
{
    if(socketVal>0){
      printf("closing socket val :%d  \n",socketVal );
      close(socketVal);
    }
    sleep(1);
    //printf("closing server socket val :%d  \n",severSocketVal );
    //close(severSocketVal);
    exit(0);

}

char *probEr(char *sendingString,int n){
  char *data=sendingString;
  // printf("data:%s   n=%d\n",data,n );
  for (int i=0; i<strlen(data) ; i++){
    if(rand()%n==0){
      if(data[i]=='0'){
        data[i]='1';
      }
      else{
        data[i]='0';
      }
    }
  }
  printf("data Sent:%s   n=%d\n",data,n );
  return data;
}

int check(char *buffer){

    char *arr=MakeCRC(buffer);

    for (int i=0;i<8;i++){
        if(arr[i]=='1'){
          return 0;
        }
    }
    return 1;
}

int main(int argc, char const *argv[])
{
    int server_fd,conn_fd, new_socket, valread,i=2,checkFlag,n=100;
    struct sockaddr_in serv_address,cli_address;
    socklen_t clilen;
    int opt = 1;
    pid_t childpid;
    int addrlen = sizeof(serv_address);
    char buffer[20]={2} ;
    char *ack = "1" ,*nack="0";
    clock_t begin;
    int time_spent=0,PORT;

    if(argc==2){
      printf("%s\n",argv[1] );
      PORT=atoi(argv[1]);
    }
    else{
      printf("please enter:<executable code><Server Port number>\n");
      exit(0);
    }
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    severSocketVal=server_fd;
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    serv_address.sin_family = AF_INET;
    serv_address.sin_addr.s_addr = INADDR_ANY;
    serv_address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&serv_address,
                                 sizeof(serv_address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    // if ((new_socket = accept(server_fd, (struct sockaddr *)&serv_address,
    //                    (socklen_t*)&addrlen))<0)
    // {
    //     perror("accept");
    //     exit(EXIT_FAILURE);
    // }
    //checking for the Connection establishment
    // send(new_socket , hello , strlen(hello) , 0 );
    // valread = read( new_socket , buffer, 1024);

    //defining the signal to handle ctrl+c request and terminating the program.
    signal(SIGINT, handle_sigint);
    int x=0;
    while(1){
      //trying to accect a new connection request from the client
      //connn_fd is the connection file descriptor .
      clilen=sizeof(cli_address);
      printf("waiting for connection......\n" );
      conn_fd=accept(server_fd, (struct sockaddr *)&cli_address, (socklen_t*)&clilen);
      printf("client request recived .....\n" );

      //creating a child to handle that request.
      //parent proces creates a child to handle every client
      if( (childpid=fork()) == 0 ){
         printf ("%s\n","Child created for dealing with client requests");
         //close(server_fd);
         printf("%d\n",conn_fd );
         socketVal=conn_fd;
         while( (valread = read( conn_fd , buffer, 1024))>0 ){
            //printf("%d\n",valread );
            checkFlag=check(buffer);
            //printf("%d\n",checkFlag );
            if(!checkFlag){
              char sendingString[]="";
              strcat(sendingString,nack);
              strcat(sendingString,MakeCRC(nack));
              send(conn_fd , probEr(sendingString,n) , strlen(sendingString) , 0 );
            }
            else{
              char sendingString[]="";
              strcat(sendingString,ack);
              strcat(sendingString,MakeCRC(ack));
              send(conn_fd , probEr(sendingString,n) , strlen(sendingString) , 0 );

            }

            printf("\t\t-----------------------------------------\n");
            //printf("buffer :%s\n",buffer );
            //printf("msg  was sent\n" );
         }

      }
      socketVal=-1;
      close(conn_fd);
    }
    printf("lauda\n");
    return 0;
    exit(0);
}
