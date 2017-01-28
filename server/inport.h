#ifdef WIN32 /* si vous �tes sous Windows */

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#elif defined (linux) /* si vous �tes sous Linux */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#else /* sinon vous �tes sur une plateforme non support�e */

#error not defined for this platform

#endif

void send_client(const char * buffer, SOCKET csock);
int recv_client(char * buffer, SOCKET csock);
#define SIZE 255
