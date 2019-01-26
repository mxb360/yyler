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

/* ��ȡ�ļ���С
 * filename: �ļ���
 * ���أ��ļ���С����λΪ�ֽڣ�������-1
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

/* ��ӡ�ļ���С
 * ��ӡ��ʽ���ļ���С�� xxx(xxx)\n
 */
static void print_file_size(long size)
{
    if (size < 1024)
        printf("�ļ���С: %ldB", size);
    else if (size < 1048756)
        printf("�ļ���С: %.2fK", size / 1024.);
    else if (size < 1073741824)
        printf("�ļ���С: %.2fM", size / 1024. / 1024);
    else
        printf("�ļ���С: %.2fG", size / 1024. / 1024 / 1024);
    printf("(%ld)\n", size);
}


/* ��ӡ��ǰ����
 * errstr: ��������
 * is_exit: �Ƿ��ڳ���ʱ�˳�����
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

/* ����һ��Socket���������󣬴���socket����ʼ����
 * type: socket����(ΪSOCKET_TCP��SOCKET_UDP�е�һ��)
 * port: �˿ں�
 * listen_count: ����������
 */
Socket Socket_CreateServer(SocketType type, unsigned int port, unsigned int listen_count)
{
    Socket _socket;
    int _type;

    /* TCP����UDP */
    switch (type) {
    case SOCKET_TCP: _type = SOCK_STREAM; break;
    case SOCKET_UDP: _type = SOCK_DGRAM; break;
    default: return NULL;
    }

    __socket_init();

    /* ����socket�׽��� */
    _socket = (Socket)malloc(sizeof(struct _Socket));
    if (_socket) {
        _socket->socket = socket(AF_INET, _type, 0);
        if (_socket->socket == -1) {
            print_error("socket create error", SYSTEM_SOCKET_ERROR);
            free(_socket);
            return NULL;
        }
        /* �� */
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
        /* ���� */
        if (listen(_socket->socket, listen_count) == -1) {
            print_error("Socket Listen Error", SYSTEM_SOCKET_ERROR);
            free(_socket);
            return NULL;
        }
    }

    return _socket;
}

/* ����һ��Socket�ͻ��˶��󣬴���socket�����ӷ�����
 * type: socket����(ΪSOCKET_TCP��SOCKET_UDP�е�һ��)
 * port: �������˿ں�
 * ipstr: ������ip��ַ
 * ���ؿͻ��˶���ʧ�ܷ���NULL
 */
Socket Socket_CreateClient(SocketType type, const char *ipstr, unsigned int port)
{
    Socket _socket;
    int _type;

    /* TCP����UDP */
    switch (type) {
    case SOCKET_TCP: _type = SOCK_STREAM; break;
    case SOCKET_UDP: _type = SOCK_DGRAM; break;
    default: return NULL;
    }
    __socket_init();

    /* ����socket�׽��� */
    _socket = (Socket)malloc(sizeof(struct _Socket));
    if (_socket) {
        _socket->socket = socket(AF_INET, _type, 0);
        if (_socket->socket == -1) {
            print_error("Socket Create Error", SYSTEM_SOCKET_ERROR);
            free(_socket);
            return NULL;
        }
        /* �� */
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
        /* ���� */
        if (connect(_socket->socket, (struct sockaddr *)&_socket->_addr, sizeof(_socket->_addr)) == -1) {
            print_error("Socket Connect Error", SYSTEM_SOCKET_ERROR);
            free(_socket);
            return NULL;
        }
    }

    return _socket;
}

/* ������Socket������ܿͻ��˵����ӣ����ӳɹ��󷵻�һ��Server-Client Socket����
 * _socket: ������Socket����
 * ���أ����ӵ��Ŀͻ��˶������ʧ�ܷ���NULL
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

/* �����ַ���
 * _socket: Socket����
 * str: �����͵��ַ���
 * ���ط��͵��ַ�������ʧ�ܷ���-1
 */
int Socket_SendString(Socket _socket, const char *str)
{
    int n;

    if (!_socket) {
        print_error("Socket_ServerAccept Error", MYLIB_SOCKET_ERROR);
        return -1;
    }
    /* �����ַ��� */
    if ((n = send(_socket->socket, str, strlen(str), 0)) == -1) {
        print_error("Socket Send Error", SYSTEM_SOCKET_ERROR);
    }

    return n;
}

/* �����ļ�
 * _socket: Socket����
 * file_name: �����͵��ļ���
 * ����: ���ͳɹ�����0��ʧ�ܷ���-1
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
    /* ��ȡ�ļ���С */
    if ((file_size = get_file_size(file_name)) == -1)
        return -1;
    printf("�ļ���: %s  ", file_name);
    //print_file_size(file_size);
    /* �򿪴����͵��ļ� */
    if ((fp = fopen(file_name, "r")) == NULL) {
        print_error(file_name, SYSTEM_SOCKET_ERROR);
        return -1;
    }
    /* �ȷ����ļ��Ĵ�С */
    if (send(_socket->socket, (char *)&file_size, sizeof file_size, 0) == -1) {
        print_error("Socket Send Error", SYSTEM_SOCKET_ERROR);
        fclose(fp);
        return -1;
    }
    Socket_SendDelay();
    /* ѭ�����ͣ�ֱ���ļ�������� */
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

/* �����ַ���
 * _socket: Socket����
 * buf: �����ַ��������������ΪNULL��ʹ�����û�����
 * ���أ������ַ���������
 */
char *Socket_RecvString(Socket _socket, char *buf)
{
    int n;
    buf = buf ? buf : _recv_buf;

    if (!_socket) {
        print_error("Socket_RecvString Error", MYLIB_SOCKET_ERROR);
        return NULL;
    }
    /* �����ַ��� */
    if ((n = recv(_socket->socket, buf, RECV_BUF_SIZE, 0)) == -1) {
        print_error("Socket Recv Error", SYSTEM_SOCKET_ERROR);
        return NULL;
    }
    buf[n] = '\0';

    return buf;
}

/* �����ļ�
 * _socke: Socket����
 * file_name: ���յ����ļ����ļ���
 * ����: �������ʧ�ܷ���-1���ɹ�����0
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
    /* �½�һ�����ļ������ڴ洢���յ����ļ� */
    if ((fp = fopen(file_name, "w")) == NULL) {
        print_error(file_name, SYSTEM_SOCKET_ERROR);
        return -1;
    }
    /* ��ȡ�������ļ��Ĵ�С */
    if (recv(_socket->socket, (char *)&file_size, sizeof file_size, 0) == -1) {
        print_error("Socket Recv Error", SYSTEM_SOCKET_ERROR);
        fclose(fp);
        return -1;
    }
    printf("�ļ���: %s  ", file_name);
    print_file_size(file_size);
    /* ѭ�������ļ����ݣ�ֱ��������ɣ����յ����ݵ����ļ����ݣ� */
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

/* ��ӡsocket����
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

/* ���socket����
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

/* ���ͼ����ʱ */
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
