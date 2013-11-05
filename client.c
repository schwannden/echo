#include "../nplib/np_header.h"
#include "../nplib/np_lib.h"

#define SERV_PORT 9877
#define max(x, y) ((x>y)? (x) : (y))
#define min(x, y) ((x>y)? (y) : (x))

void str_cli( FILE*, int );

int main(int argc, char** argv)
{
  if(argc != 2)
    err_quit( "usae: client <IPaddress>" );

  int channelfd;
  struct sockaddr_in servaddr;

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

void str_cli( FILE* infile, int channelfd )
{
  char recvline[MAXLINE], sendline[MAXLINE];
  fd_set rset;
  int max_fd, stdineof = 0;
  FD_ZERO( &rset );

  while(1){
    if( stdineof == 0 )
      FD_SET( STDIN_FILENO, &rset );
    FD_SET( channelfd, &rset );
    max_fd = max( STDIN_FILENO, channelfd ) + 1;

    //blocking for either user or socket inputs
    Select( max_fd, &rset, NULL, NULL, NULL);

    //If user inputs
    if( FD_ISSET(STDIN_FILENO, &rset) )
      //if input ends with new line
      if( Fgets( sendline, MAXLINE, infile ) != NULL )
        Writen( channelfd, sendline, strlen(sendline) );
      //if input ends with EOF
      else{
        Shutdown( channelfd, SHUT_WR );
        stdineof = 1;
        FD_CLR( STDIN_FILENO, &rset );
      }
 
    //if socket inputs
    if( FD_ISSET(channelfd, &rset ) ) {
      //if nothing is read
      if( Readline( channelfd, recvline, MAXLINE ) == 0 )
        //normal termination
        if( stdineof == 1 )
          return;
        //abnormal termination
        else
          err_sys( "str_cli: server terminate prematurely" );
      //if something is read
      Fputs( recvline, stdout ); 
    }
  }
}
