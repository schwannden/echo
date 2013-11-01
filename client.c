#include "../nplib/np_header.h"
#include "../nplib/np_lib.h"

#define SERV_PORT 9877

void str_cli( FILE*, int );

int main(int argc, char** argv)
{
  int channelfd;
  struct sockaddr_in servaddr;

  if(argc != 2)
	err_quit( "usae: client <IPaddress>" );

  bzero( &servaddr, sizeof(servaddr) );
  servaddr.sin_family = AF_INET;
  Inet_pton( AF_INET, argv[1], &servaddr.sin_addr );
  servaddr.sin_port = htons(SERV_PORT);

  channelfd = Socket( AF_INET, SOCK_STREAM, 0 );
  Connect( channelfd, (const SA*)&servaddr, sizeof(servaddr) );

  str_cli( stdin, channelfd );

  Close( channelfd );

  return 0;
}

void str_cli( FILE* infile, int sockfd )
{
  char recvline[MAXLINE], sendline[MAXLINE];
  while( Fgets( sendline, MAXLINE, infile ) != 0 ){
    Writen( sockfd, sendline, strlen(sendline) );
	sleep(10);
	if( Readline( sockfd, recvline, MAXLINE ) == 0 )
      err_sys( "str_cli: server terminate prematurely" );
	Fputs( recvline, stdout );
  }
}
