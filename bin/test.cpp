#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(5575);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        printf("create socket success.\r\n");
        return 0;
    }

    if(connect(sockfd,(struct sockaddr*)&addr, sizeof(struct sockaddr)) < 0)
    {
        printf("Connect failed.\r\n");
        return 0;
    }
    
    while(true)
    {
    char buf[1024] = {0};
    gets(buf);
    int iSize = write(sockfd, buf, strlen(buf));
    printf("Write Size:%d\r\n", iSize);

    char szBuf[1024] = {0};
    iSize = read(sockfd, szBuf, sizeof(szBuf));
    printf("recv[%d] %s\r\n", iSize, szBuf);
    }

    close(sockfd);

    return 0;
}


