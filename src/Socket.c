#include "Socket.h"

#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif

#ifdef _WIN32
#include <Ws2tcpip.h>
WSADATA wsaData;
int socket_init = 0;
//#define close(s) closesocket(s)
#endif

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static char str_err[1024];
static char _recv_buf[RECV_BUF_SIZE];

#define SYSTEM_SOCKET_ERROR 1
#define MYLIB_SOCKET_ERROR  2

/* 获取文件大小
 * filename: 文件名
 * 返回：文件大小，单位为字节，出错返回-1
 */
static long get_file_size(const char *filename)
{
    struct stat statbuf;
    if (stat(filename, &statbuf) == -1) {
        //print_error(filename, 0);
        return -1;
    }
    long size = statbuf.st_size;
    return size;
}

/* 打印文件大小
 * 打印格式：文件大小： xxx(xxx)\n
 */
static void print_file_size(long size)
{
    if (size < 1024)
        printf("文件大小: %ldB", size);
    else if (size < 1048756)
        printf("文件大小: %.2fK", size / 1024.);
    else if (size < 1073741824)
        printf("文件大小: %.2fM", size / 1024. / 1024);
    else
        printf("文件大小: %.2fG", size / 1024. / 1024 / 1024);
    printf("(%ld)\n", size);
}


/* 打印当前错误
 * errstr: 错误描述
 * is_exit: 是否在出错时退出程序
 */
static void print_error(const char *errstr, int err_type)
{
    int  err_code;    
    if (err_type == SYSTEM_SOCKET_ERROR) {
#if defined(_WIN32)
        err_code = WSAGetLastError();
        FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, err_code, 0, str_err, sizeof str_err, NULL);
#elif defined(Linux)
        err_code = errno;
        strcpy(str_err, strerror(errno));
#endif
    } else {
        err_code = -1;
        strcpy(str_err, "This Socket Object is not a ServerSocket");
    }
#if PRINT_ERR
    fprintf(stderr, "Error %d: %s: %s\n", err_code, errstr, str_err);
#endif
}

static int __socket_init(void)
{
#ifdef _WIN32
    if (!socket_init && WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        print_error("Windows WSAStartup Error", SYSTEM_SOCKET_ERROR);
        return -1;
    }
    socket_init = 1;
#endif 
    return 0;
}

/* 创建一个Socket服务器对象，创建socket并开始监听
 * type: socket类型(为SOCKET_TCP、SOCKET_UDP中的一个)
 * port: 端口号
 * listen_count: 监听的数量
 */
Socket Socket_CreateServer(SocketType type, unsigned int port, unsigned int listen_count)
{
    Socket _socket;
    int _type;

    /* TCP或者UDP */
    switch (type) {
    case SOCKET_TCP: _type = SOCK_STREAM; break;
    case SOCKET_UDP: _type = SOCK_DGRAM; break;
    default: return NULL;
    }

    __socket_init();

    /* 创建socket套接字 */
    _socket = (Socket)malloc(sizeof(struct _Socket));
    if (_socket) {
        _socket->socket = socket(AF_INET, _type, 0);
        if (_socket->socket == -1) {
            print_error("socket create error", SYSTEM_SOCKET_ERROR);
            free(_socket);
            return NULL;
        }
        /* 绑定 */
        memset(&_socket->_addr, 0, sizeof _socket->_addr);
        _socket->_addr.sin_family = AF_INET;
        _socket->_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        _socket->_addr.sin_port = htons(port);
        _socket->port = port;
        _socket->_sc = SOCKET_SERVER;
        inet_ntop(AF_INET, &_socket->_addr.sin_addr.s_addr, _socket->ipstr, sizeof(_socket->ipstr));

        if (bind(_socket->socket, (struct sockaddr *)&_socket->_addr, sizeof _socket->_addr) == -1) {
            print_error("Socket Bind Error", SYSTEM_SOCKET_ERROR);
            free(_socket);
            return NULL;
        }
        /* 监听 */
        if (listen(_socket->socket, listen_count) == -1) {
            print_error("Socket Listen Error", SYSTEM_SOCKET_ERROR);
            free(_socket);
            return NULL;
        }
    }

    return _socket;
}

/* 创建一个Socket客户端对象，创建socket并连接服务器
 * type: socket类型(为SOCKET_TCP、SOCKET_UDP中的一个)
 * port: 服务器端口号
 * ipstr: 服务器ip地址
 * 返回客户端对象，失败返回NULL
 */
Socket Socket_CreateClient(SocketType type, const char *ipstr, unsigned int port)
{
    Socket _socket;
    int _type;

    /* TCP或者UDP */
    switch (type) {
    case SOCKET_TCP: _type = SOCK_STREAM; break;
    case SOCKET_UDP: _type = SOCK_DGRAM; break;
    default: return NULL;
    }
    __socket_init();

    /* 创建socket套接字 */
    _socket = (Socket)malloc(sizeof(struct _Socket));
    if (_socket) {
        _socket->socket = socket(AF_INET, _type, 0);
        if (_socket->socket == -1) {
            print_error("Socket Create Error", SYSTEM_SOCKET_ERROR);
            free(_socket);
            return NULL;
        }
        /* 绑定 */
        memset(&(_socket->_addr), 0, sizeof(_socket->_addr));
        _socket->_addr.sin_family = AF_INET;
        if (inet_pton(AF_INET, ipstr, &_socket->_addr.sin_addr.s_addr) <= 0) {
            print_error("Socket Get IP Error", SYSTEM_SOCKET_ERROR);
            free(_socket);
            return NULL;
        }
        strcpy(_socket->ipstr, ipstr);
        _socket->_addr.sin_port = htons(port);
        _socket->port = port;
        _socket->_sc = SOCKET_CLIENT;
        inet_ntop(AF_INET, &_socket->_addr.sin_addr.s_addr, _socket->ipstr, sizeof _socket->ipstr);
        /* 连接 */
        if (connect(_socket->socket, (struct sockaddr *)&_socket->_addr, sizeof(_socket->_addr)) == -1) {
            print_error("Socket Connect Error", SYSTEM_SOCKET_ERROR);
            free(_socket);
            return NULL;
        }
    }

    return _socket;
}

/* 服务器Socket对象接受客户端的连接，连接成功后返回一个Server-Client Socket对象
 * _socket: 服务器Socket对象
 * 返回：连接到的客户端对象，如果失败返回NULL
 */
Socket Socket_ServerAccept(Socket _socket)
{
    Socket _client;
    int client_addr_len = sizeof(struct sockaddr_in);

    if (!_socket || _socket->_sc != SOCKET_SERVER) {
        print_error("Socket_ServerAccept Error", MYLIB_SOCKET_ERROR);
        return NULL;
    }

    _client = (Socket)malloc(sizeof(struct _Socket));
    if (_client) {
        _client->socket = accept(_socket->socket, (struct sockaddr *)&_client->_addr, &client_addr_len);
        if (_client->socket == -1) {
            print_error("Socket Accept Error", SYSTEM_SOCKET_ERROR);
            free(_client);
            return NULL;
        }

        _client->port = ntohs(_client->_addr.sin_port);
        _client->_sc = SOCKET_SERVER_CLIENT;
        inet_ntop(AF_INET, &_socket->_addr.sin_addr.s_addr, _client->ipstr, sizeof(_client->ipstr));
    }

    return _client;
}

int Socket_Send(Socket _socket, const char *buf, int len)
{
    int n = -1;
    if (!_socket) {
        print_error("Socket_Send Error", MYLIB_SOCKET_ERROR);
        return n;
    }
    if ((n = send(_socket->socket, buf, len, 0)) == -1) {
        print_error("Socket Send Error", SYSTEM_SOCKET_ERROR);
    }

    return n;
}

/* 发送字符串
 * _socket: Socket对象
 * str: 待发送的字符串
 * 返回发送的字符个数，失败返回-1
 */
int Socket_SendString(Socket _socket, const char *str)
{
    int n;

    if (!_socket) {
        print_error("Socket_ServerAccept Error", MYLIB_SOCKET_ERROR);
        return -1;
    }
    /* 发送字符串 */
    if ((n = send(_socket->socket, str, strlen(str), 0)) == -1) {
        print_error("Socket Send Error", SYSTEM_SOCKET_ERROR);
    }

    return n;
}

/* 发送文件
 * _socket: Socket对象
 * file_name: 待发送的文件名
 * 返回: 发送成功返回0，失败返回-1
 */
int Socket_SendFile(Socket _socket, const char *file_name, void(*fun)(int))
{
    FILE *fp;
    size_t n;
    long file_size, current_size = 0;
    char buf[RECV_BUF_SIZE / 2];
    int persent;

    if (!_socket) {
        print_error("Socket_SendFile Error", MYLIB_SOCKET_ERROR);
        return -1;
    }
    /* 获取文件大小 */
    if ((file_size = get_file_size(file_name)) == -1)
        return -1;
    printf("文件名: %s  ", file_name);
    //print_file_size(file_size);
    /* 打开待发送的文件 */
    if ((fp = fopen(file_name, "r")) == NULL) {
        print_error(file_name, SYSTEM_SOCKET_ERROR);
        return -1;
    }
    /* 先发送文件的大小 */
    if (send(_socket->socket, (char *)&file_size, sizeof file_size, 0) == -1) {
        print_error("Socket Send Error", SYSTEM_SOCKET_ERROR);
        fclose(fp);
        return -1;
    }
    Socket_SendDelay();
    /* 循环发送，直到文件发送完成 */
    while (!feof(fp)) {
        n = fread(buf, 1, RECV_BUF_SIZE / 2, fp);
        current_size += n;
        send(_socket->socket, buf, n, 0);
        persent = current_size * 100 / file_size;
        if (fun)
            fun(persent);
    }

    fclose(fp);
    return 0;
}

int Socket_Recv(Socket _socket, char * buf, int buf_size)
{
    int n = -1;
    if (!_socket) {
        print_error("Socket_Recv Error", MYLIB_SOCKET_ERROR);
        return n;
    }
    if ((n = recv(_socket->socket, buf, buf_size, 0)) == -1) {
        print_error("Socket Recv Error", SYSTEM_SOCKET_ERROR);
    }

    return n;
}

/* 接收字符串
 * _socket: Socket对象
 * buf: 接收字符串缓存区，如果为NULL，使用内置缓存区
 * 返回：接收字符串缓存区
 */
char *Socket_RecvString(Socket _socket, char *buf)
{
    int n;
    buf = buf ? buf : _recv_buf;

    if (!_socket) {
        print_error("Socket_RecvString Error", MYLIB_SOCKET_ERROR);
        return NULL;
    }
    /* 接收字符串 */
    if ((n = recv(_socket->socket, buf, RECV_BUF_SIZE, 0)) == -1) {
        print_error("Socket Recv Error", SYSTEM_SOCKET_ERROR);
        return NULL;
    }
    buf[n] = '\0';

    return buf;
}

/* 接收文件
 * _socke: Socket对象
 * file_name: 接收到的文件的文件名
 * 返回: 如果接收失败返回-1，成功返回0
 */
int Socket_RecvFile(Socket _socket, const char *file_name, void(*fun)(int))
{
    FILE *fp;
    int n;
    int persent = 0;
    char buf[RECV_BUF_SIZE];
    long file_size, currect_size = 0;

    if (!_socket) {
        print_error("Socket_RecvFile Error", MYLIB_SOCKET_ERROR);
        return -1;
    }
    /* 新建一个空文件，由于存储接收到的文件 */
    if ((fp = fopen(file_name, "w")) == NULL) {
        print_error(file_name, SYSTEM_SOCKET_ERROR);
        return -1;
    }
    /* 获取待接收文件的大小 */
    if (recv(_socket->socket, (char *)&file_size, sizeof file_size, 0) == -1) {
        print_error("Socket Recv Error", SYSTEM_SOCKET_ERROR);
        fclose(fp);
        return -1;
    }
    printf("文件名: %s  ", file_name);
    print_file_size(file_size);
    /* 循环接收文件内容，直到接收完成（接收的内容等于文件内容） */
    while (currect_size < file_size) {
        n = recv(_socket->socket, buf, RECV_BUF_SIZE, 0);
        fwrite(buf, 1, (size_t)n, fp);
        currect_size += n;
        persent = currect_size * 100 / file_size;
        if (fun)
            fun(persent);
    }

    fclose(fp);
    return 0;
}

/* 打印socket对象
 */
void Socket_Print(Socket _socket)
{
    char *typestr, *_scstr;

    if (_socket == NULL) {
        printf("Socket(NULL)\n");
        return;
    }
    typestr = _socket->type == SOCKET_TCP ? "TCP" : "UDP";
    _scstr = _socket->_sc == SOCKET_SERVER ? "Sever" : _socket->_sc == SOCKET_CLIENT ? "Client" : "ServerClient";
    printf("%sSocket(type: %s, ip: %s, port: %d)\n", _scstr, typestr, _socket->ipstr, _socket->port);
}

/* 清除socket对象
 */
void Socket_Delete(Socket _socket)
{
    if (_socket) {
#ifdef _WIN32
        closesocket(_socket->socket);
#else
        close(_socket->socket);
#endif
        free(_socket);
    }
}

/* 发送间隔延时 */
void Socket_SendDelay(void)
{
#ifdef _WIN32
    Sleep(SOCKET_USLEEP_TIME);
#elif defined(Linux)
    usleep(SOCKET_USLEEP_TIME);
#endif
}

const char * Socket_GetLastError(void)
{
    return str_err;
}

/* ---------------------------------------------------------------------------- */

#define HTTP_POST "POST /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n" \
    "Content-Type:application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s"
#define HTTP_GET "GET /%s HTTP/1.1\r\nHOST: %s\r\nConnection: close\r\nAccept: */*\r\n\r\n"


static char *get_host(char *host_name)
{
    int n = 0;
    if (!strncmp(host_name, "http://", 7))
        n = 7;
    if (!strncmp(host_name, "https://", 8))
        n = 8;
    if (n)
        strcpy(host_name, host_name + n--);
    while (host_name[++n] && host_name[n] != '/');
    host_name[n] = 0;

    return host_name;
}

static char *get_url(char *host_name)
{
    int n = 0;
    if (!strncmp(host_name, "http://", 7))
        n = 6;
    if (!strncmp(host_name, "https://", 8))
        n = 7;

    while (host_name[++n] && host_name[n] != '/');
    if (!n)
        return NULL;
    if (host_name[n] == '/')
        n++;
    strcpy(host_name, host_name + n); 

    return host_name;
}

HttpClient Socket_HttpGet(const char *host, HttpProessFunc func)
{
    struct hostent *_host;
    char host_buf[4096];
    int i, n, ln = 0;
    char *tph = NULL;
    HttpClient client = (HttpClient)malloc(sizeof(struct _HttpClient));
    if (client == NULL)
        return NULL;

    __socket_init();
    strcpy(host_buf, host);
    strcpy(client->url, get_url(host_buf));
    strcpy(host_buf, host);
    strcpy(client->host_name, get_host(host_buf));
    strcpy(client->whole_url, host);
    //printf("host_name: %s\n", client->host_name);
    if ((_host = gethostbyname(client->host_name)) == NULL) {
        print_error(host_buf, SYSTEM_SOCKET_ERROR);
        return NULL;
        free(client);
    }

    inet_ntop(AF_INET, _host->h_addr, host_buf, sizeof host_buf);
    //printf("IP: %s\n", host_buf);
    Socket _socket = Socket_CreateClient(SOCKET_TCP, host_buf, 80);

    if (_socket == NULL) {
        free(client);
        return NULL;
    }

    sprintf(host_buf, HTTP_GET, client->url, client->host_name);
    //printf("\n\nheader:\n%s\n\n", host_buf);
    if (Socket_SendString(_socket, host_buf) < 0) {
        free(client);
        return NULL;
    }

    client->status = -1;
    client->body_len = 0;
    n = recv(_socket->socket, host_buf, (sizeof host_buf) - 1, 0);
    if (n > 0) {
        client->status = (host_buf[9] - '0') * 100 + (host_buf[10] - '0') * 10 + host_buf[11] - '0';
        tph = strstr(host_buf, "Location: ");
        if (tph) {
            HttpClient _client;
            for (i = 0; tph[i] != '\r'; i++);
            tph[i] = 0;
            _client = Socket_HttpGet(tph + strlen("Location: "), func);
            free(client);
            return _client;
        }

        tph = strstr(host_buf, "Content-Length: ");
        if (tph) {
            tph += strlen("Content-Length: ");
            while (isdigit(*tph))
                client->body_len = client->body_len * 10 + *tph++ - '0';
        }
        tph = strstr(host_buf, "\r\n\r\n");
        if (tph) {
            *tph = 0;
            ln = strlen(host_buf);
            client->header = (char *)malloc(ln);
            if (client->header)
                strcpy(client->header, host_buf);
        }
    } else {
        free(client->header);
        free(client);
        Socket_Delete(_socket);
        return NULL;
    }

    if (client->body_len) {
        ln += 4;
        client->body = (char *)malloc(client->body_len + 10);
        if (client->body) {
            memset(client->body, 0, client->body_len + 10);
            memcpy(client->body, tph + 4, n - ln);
        } else {
            Socket_Delete(_socket);
            return client;
        }
        ln = n - ln;
    } else {
        client->body = NULL;
        Socket_Delete(_socket);
        return client;
    }

    while ((n = recv(_socket->socket, host_buf, (sizeof host_buf) - 1, 0)) > 0) {
        memcpy(client->body + ln, host_buf, n);
        ln += n;
        if (func)
            func((int)(100. * ln / client->body_len));
    }

    Socket_Delete(_socket);
    return client;
}

void Socket_FreeHttpClient(HttpClient client)
{
    if (client) {
        free(client->body);
        free(client->header);
        free(client);
    }
}

char *Socket_URLEncode(char *result, const char *str)
{
    int i;
    int j = 0;//for result index
    char ch;
    int strSize = strlen(str);

    if ((str == NULL) || (result == NULL)) 
        return 0;

    for (i = 0; i < strSize; ++i) {
        ch = str[i];
        if (((ch >= 'A') && (ch < 'Z')) ||
            ((ch >= 'a') && (ch < 'z')) ||
            ((ch >= '0') && (ch < '9'))) {
            result[j++] = ch;
        } else if (ch == ' ') {
            result[j++] = '+';
        } else if (ch == '.' || ch == '-' || ch == '_' || ch == '*') {
            result[j++] = ch;
        } else {
            sprintf(result + j, "%%%02X", (unsigned char)ch);
            j += 3;
        }
    }

    result[j] = '\0';
    return result;
}
