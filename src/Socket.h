#ifndef __SOCKET_H__
#define __SOCKET_H__

#if defined(_WIN32)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include <WinSock2.h>
#elif defined(Linux)
#include <sys/socket.h>
#endif

/* socket的类型(TCP / UDP) */
typedef enum _SocketType {
    SOCKET_TCP, SOCKET_UDP,
} SocketType;

#define PRINT_ERR    1

/* Socket对象的类型标记 */
#define SOCKET_SERVER        0
#define SOCKET_CLIENT        1
#define SOCKET_SERVER_CLIENT 2

#define EXIT_IF_SOCKET_ERROR 0
#define RECV_BUF_SIZE   2096

#define SOCKET_USLEEP_TIME 30000

/* Socket对象的封装 */
typedef struct _Socket {
    int socket;                  /* socket文件描述符 */
    SocketType type;             /* socket类型 */
    int port;                    /* 端口号 */
    int _sc;
    char ipstr[20];              /* IP地址 */
    struct sockaddr_in _addr;
} *Socket;

Socket  Socket_CreateServer(SocketType type, unsigned int port, unsigned int listen_count);
Socket  Socket_CreateClient(SocketType type, const char *ipstr, unsigned int port);
Socket  Socket_ServerAccept(Socket _socket);
int     Socket_Send(Socket _socket, const char *buf, int len);
int     Socket_Recv(Socket _socket, char *buf, int buf_size);
int     Socket_SendString(Socket _socket, const char *str);
char *Socket_RecvString(Socket _socket, char *buf);
int   Socket_SendFile(Socket _socket, const char *file_name, void(*fun)(int));
int   Socket_RecvFile(Socket _socket, const char *file_name, void(*fun)(int));
void  Socket_Print(Socket _socket);
void  Socket_Delete(Socket _socket);
void  Socket_SendDelay(void);
const char *Socket_GetLastError(void);
char *Socket_URLEncode(char *result, const char *str);

typedef struct _HttpHeaderDict{
    char *key;
    char *value;
} HttpHeaderDict;

typedef struct _HttpClient {
    //Socket client;
    char whole_url[1024];
    char url[512];
    char host_name[512];
    int body_len;
    int status;
    //HttpHeaderDict *head;
    char *header;
    char *body;
} *HttpClient;

typedef void(*HttpProessFunc)(int proess);
HttpClient Socket_HttpGet(const char *host, HttpProessFunc func);
void Socket_FreeHttpClient(HttpClient client);

#endif
