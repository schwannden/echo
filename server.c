#include "../nplib/np_header.h"
#include "../nplib/np_lib.h"

#define BACKLOG 1
#define SERV_PORT 9877

void sig_child_handler( int sig );

int main(int argc, char** argv)
{
  int i, listenfd, connfd, maxfd, on=1;
  int client_record[FD_SETSIZE];
  size_t client_max = 0, client_ready, bytes_read;
  char buf[MAXLINE];
  fd_set rset, rset_new;
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
//initializing fd_set structure, client_record, maxfd
  for( i = 0 ; i < FD_SETSIZE; i++ )
    client_record[i] = -1;
  bzero( &rset, sizeof(rset) );
  bzero( &rset_new, sizeof(rset_new) );
  FD_SET( listenfd, &rset_new );
  maxfd = listenfd +1;
//spining for incomming message or connection
  while(1){
    rset = rset_new;
    client_ready = select( maxfd, &rset, NULL, NULL, NULL );
    //new connection request
    if( FD_ISSET( listenfd, &rset ) ){
      //verbosely output accepted connection
      cliaddrsize = sizeof(cliaddr);
      connfd = Accept( listenfd, (SA*)&cliaddr, &cliaddrsize );
      printf( "Accepting connetion from: %s : %d, with file descriptor %d\n", 
               inet_ntop( AF_INET, (void*)&cliaddr.sin_addr, clientip, sizeof(clientip) ), ntohs( cliaddr.sin_port ), connfd );
      //find spot in client_record for the new connfd
      i = 0;
      while( i<FD_SETSIZE && client_record[i]>=0 )
        i++;
      //if client_record is full, quit
      if( i == FD_SETSIZE )
        err_sys( "too many client" );
      client_record[i] = connfd;
      FD_SET( connfd, &rset_new );
      //adjust upper bound for client_record
      if( i == client_max )
        client_max++;
      //adjust upper bound for rset_new
      if( connfd >= maxfd )
        maxfd = connfd + 1;
    }

    printf( "now maximun client fie descriptor is %d\n", maxfd-1);
    printf( "client_record is: [" );
    for( i = 0 ; i < client_max ; i++ )
      printf( "%d,", client_record[i]);
    printf( "%d]\n\n", client_record[maxfd]);

    for( i = 0 ; i < client_max ; i++ ){
      if( (connfd = client_record[i]) >= 0 )
        if( FD_ISSET( connfd, &rset ) ){
again:
          if( (bytes_read = read( connfd, buf, MAXLINE )) > 0 )
            Writen( connfd, buf, bytes_read );
          else if( bytes_read < 0 )
            if( errno == EINTR ) goto again;
            else err_sys( "str_echo: read error" );
          else if( bytes_read == 0 ){
            printf( "closing connection %d\n", connfd );
            close( connfd );
            client_record[i] = -1;
            FD_CLR( connfd, &rset_new );
            printf( "client_record is: [" );
            for( i = 0 ; i < client_max ; i++ )
              printf( "%d,", client_record[i]);
            printf( "%d]\n\n", client_record[maxfd]);
          }
          if( --client_ready <= 0 )
            break;
        }
    }
  }
  return 0;
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

