/* Cliser.h*/
/* Automatically generated by cliser on Wed Oct 25 17:33:56 PDT 2000 */
#define  LTRACE 0
#ifndef CLISER_DV_DEF
#define CLISER_DV_DEF

#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <values.h>
#include <sys/types.h>
#include <malloc.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <termio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <time.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* #include <arpa/inet.h> */
#include <netdb.h>
#define                        ON          1
#define                        OFF         0
#define                        BUFLEN      1024

typedef  int                   (* PFI_II_L) (int,int);

typedef enum {
  COMM_CONNECT,
  COMM_PING
} Comm_mode;
extern   int                   ser0_start_DV(int port);
extern   int                   ser0_serve_block_DV(int ss,PFI_II_L service);
extern   int                   ser0_stop_DV(int ss);
extern   int                   ser0_comm_DV(char *sername,int port,
                                         Comm_mode mode);
extern   int                   ser0_ping_DV(char *sername,int port);
extern   int                   ser0_connect_DV(char *sername,int port);

extern   void                  dump_hostent_DV(char *s, struct hostent *hp);

#endif
