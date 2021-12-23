#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

//based on https://github.com/softScheck/tplink-smartplug/blob/master/tplink_smartplug.py

const char* getEmeterCommandString = "{\"emeter\":{\"get_realtime\":{}}}";
int lasterrno = 0;

void PrintError(const char* message)
{
    if(errno == lasterrno)
    {
        printf("%s\n", message);

        return;
    }

    const char* errStr = strerror(errno);
    printf("%s: %s (%d)\n", message, errStr, errno);

    lasterrno = errno;
}

bool GetAddressFromArgs(const char* addrStr, struct sockaddr_in* addrResult)
{
    addrResult->sin_family = AF_INET;
    addrResult->sin_port = htons(9999);

    if(inet_pton(AF_INET, addrStr, &addrResult->sin_addr) != 1) {
        PrintError("Not a valid address!");

        return false;
    }

    return true;
}

int Connect(const struct sockaddr_in* address)
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == 0)
    {
        PrintError("Can not create socket");

        return 0;
    }

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    if (connect(sock, (struct sockaddr*)address, sizeof(struct sockaddr_in)) < 0)
    {
        PrintError("Can not connect");

        close(sock);

        return 0;
    }

    return sock;
}

void CloseConnection(int sock)
{
    shutdown(sock, SHUT_RDWR);

    close(sock);
}

void Encrypt(char* str, int len)
{
    int key = 171;
    for (size_t i = 0; i < len; i++)
    {
        int c = str[i];
        int a = key ^ c;
        key = a;
        str[i] = (char)a;
    }
}

void Decrypt(char* data, int len)
{
    int key = 171;
    for (size_t i = 0; i < len; i++)
    {
        char c = data[i];
        char a = key ^ c;
        key = c;
        data[i] = a;
    }
}

struct TPMessageBuffer
{
    int32_t length;
    char data[2048 - sizeof(int32_t)];
};

void GetResponse(int sock)
{
    TPMessageBuffer buffer;
    int len = strlen(getEmeterCommandString);

    memcpy(buffer.data, getEmeterCommandString, len + 1); //make sure to copy null byte
    Encrypt(buffer.data, len);
    buffer.length = htonl(len); //Length to network byte order
    int res = send(sock, &buffer, len + sizeof(buffer.length), 0);
    if(res == 0)
    {
        PrintError("Send error");

        return;
    }

    res = recv(sock, &buffer, sizeof(buffer), 0);
    if(res == 0)
    {
        PrintError("Recv error");

        return;
    }
    len = htonl(buffer.length); //length from network byte order
    Decrypt(buffer.data, len);
    buffer.data[len] = '\0';

    puts(buffer.data);
}

int main(int argc, char* argv[])
{
    struct sockaddr_in address;
    int sock;

    if(argc != 2) {
        PrintError("Invalid number of arguments!");

        return 1;
    }

    if(!GetAddressFromArgs(argv[1], &address))
        return 1;

    sock = Connect(&address);
    if(sock == 0)
        return 1;

    GetResponse(sock);

    CloseConnection(sock);

    return 0;
}
