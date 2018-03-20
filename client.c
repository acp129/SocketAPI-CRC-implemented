#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>



char *MakeCRC(char *BitString)
{
    static char Res[8];                                 // CRC Result
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
    Res[8] =0;                                         // Set string terminator

    return(Res);
}

char *getCRCstring(char *sendingString){

    char *res=sendingString;
    printf("%s\n",sendingString );
    strcat(res,MakeCRC(sendingString));
    printf("res :%s\n",res );
    return (res);
}

int check(char *buffer){
    // printf("buffer1:%s\n", buffer);
    char *arr=MakeCRC(buffer);
    int flag=0;
    printf("buffer3:%s\n",buffer );
    if(buffer[0]=='1'){
      flag=1;
    }
    // printf("arr    :%s\n",arr );
    // printf("buffer2:%s\n",buffer );
    for (int i=0;i<8;i++){
        if(arr[i]=='1'){
          return 0;
        }
    }

    if(flag==1)
      return 1;
    else
      return 0;
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
  printf("datta:%s   n=%d\n",data,n );
  return data;
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread,i=2,j=0;
    struct sockaddr_in serv_addr;
    char sendingString[] = "100000111",*finalCRCstring,*actualString,*hello="dharmesh";
    char buffer[20]={2};
    struct timeval tv={5, 0};
    //timeout set to be equal to 5 sec
    int PORT,n=100;

    if(argc==3){
      printf("host:%s\tPort:%s\n",argv[1],argv[2] );
      PORT=atoi(argv[2]);
    }
    else{
      printf("please enter:<executable code><Server IP Address><Server Port number>\n");
      exit(0);
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    setsockopt(sock, SOL_SOCKET,  SO_RCVTIMEO,(const char*)&tv, sizeof tv);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);


    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    // //checking for the Connection establishment
    // send(sock , hello , strlen(hello) , 0 );
    // printf("Hello message sent\n");
    // valread = read( sock , buffer, 1024);

    while(1){
      valread=-1;
      char sendingString[] = "100000111";
      sleep(1);
      strcat(sendingString,MakeCRC(sendingString));
      finalCRCstring=sendingString;
      printf("\t\t-----------------------------------------\n" );
      send(sock , probEr(finalCRCstring,n) ,strlen(finalCRCstring)  , 0 );
      //printf("Message:%s\n",finalCRCstring);
      while((valread=read(sock , buffer, 1024))<0 || (valread)>=0){
          int CheckFlag=check(buffer);
          char sendingString[] = "100000111";
          strcat(sendingString,MakeCRC(sendingString));
          finalCRCstring=sendingString;
          printf("flag:%d\n",CheckFlag );
          if (CheckFlag){
            break;
          }
          else{
            sleep(1);
            printf("\t\t-----------------------------------------\n" );
            send(sock , probEr(finalCRCstring,n)  ,strlen(finalCRCstring)  , 0 );
            printf("Resending Message:%s\n",finalCRCstring);

          }
      }




    }
    // return 0;
}
