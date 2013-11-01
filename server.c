#include "../nplib/np_header.h"
#include "../nplib/np_lib.h"

#define BACKLOG 10
#define SERV_PORT 9877

void str_echo(int);
void sig_child_handler( int sig );

int main(int argc, char** argv)
{
  int listenfd, connfd, on=1;
  pid_t childpid;
  socklen_t cliaddrsize;
  struct sockaddr_in cliaddr, servaddr;
  char clientip[ INET_ADDRSTRLEN ];
//initializing sockaddr_in structure
  bzero( &servaddr, sizeof(servaddr) );
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
  servaddr.sin_port = htons(SERV_PORT);
//setting signal handler
  struct sigaction newDisp;
  newDisp.sa_handler = sig_child_handler;
  newDisp.sa_flags = 0;
  sigemptyset( &newDisp.sa_mask );
  Sigaction( SIGCHLD, &newDisp, NULL );
//activating server: create socket, bind to address:port, set option, start listening
  listenfd = Socket( AF_INET, SOCK_STREAM, 0 );
  Bind( listenfd, (const SA*)&servaddr, sizeof(servaddr) );
  Setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, ( char* )&on, sizeof( on ) );
  Listen( listenfd, BACKLOG );

  while(1){
	cliaddrsize = sizeof(cliaddr);
	connfd = Accept( listenfd, (SA*)&cliaddr, &cliaddrsize );
    printf( "Accepting connetion from: %s : %d\n", 
             inet_ntop( AF_INET, (void*)&cliaddr.sin_addr, clientip, sizeof(clientip) ), ntohs( cliaddr.sin_port ) );

	if( (childpid = Fork()) == 0 ){
      Close(listenfd);
	  str_echo(connfd);
      printf( "Closing connetion from: %s : %d\n", 
               inet_ntop( AF_INET, (void*)&cliaddr.sin_addr, clientip, sizeof(clientip) ), ntohs( cliaddr.sin_port ) );
	  exit(0);
	}
	Close(connfd);
  }

  Close( listenfd );
  return 0;
}

void
str_echo(int fd )
{
  ssize_t number_read;
  char buf[MAXLINE];

again:
  while( (number_read = read( fd, buf, MAXLINE )) > 0 )
	Writen( fd, buf, number_read );

  if( number_read < 0 )
	if( errno == EINTR ) goto again;
	else err_sys( "str_echo: read error" );
}

void
sig_child_handler( int sig )
{
  int stat;
  pid_t childid;
  while( (childid = wait( &stat )) > 0 )
	printf( "child %d terminates\n", childid );
  return;
}

