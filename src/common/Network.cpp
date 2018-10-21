#include <time.h>
#include <sys/time.h>
#include <poll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <string.h>
#include <string>
#include <netdb.h>
#include "Network.h"


bool NetWork::interrupted()
{
    return errno == EINTR;
}

bool NetWork::wouldBlock()
{
    return (errno == EAGAIN || errno == EWOULDBLOCK);
}

bool NetWork::connectFailed()
{
    return (errno == ECONNREFUSED || 
            errno == ETIMEDOUT ||
            errno == ENETUNREACH || 
            errno == EHOSTUNREACH || 
            errno == ECONNRESET || 
            errno == ESHUTDOWN || 
            errno == ECONNABORTED);
}

bool NetWork::connectionRefused()
{
    return errno == ECONNREFUSED;
}

bool NetWork::connectInProgress()
{
    return errno == EINPROGRESS;
}

bool NetWork::connectionLost()
{
    return (errno == ECONNRESET ||
            errno == ENOTCONN ||
            errno == ESHUTDOWN ||
            errno == ECONNABORTED ||
            errno == EPIPE);
}

bool NetWork::notConnected()
{   
    return errno == ENOTCONN;
}



int NetWork::createSocket(bool udp)
{
    SOCKET fd;
    
    if(udp)
    {
        fd = socket(AF_INET, SOCK_DGRAM, 0);
    }
    else
    {
        fd = socket(AF_INET, SOCK_STREAM, 0);
    }

    if(fd < 0)
    {
        return SOCKET_ERR;
    }
    
    if(!udp)
    {
        setTcpNoDelay(fd);
        setKeepAlive(fd);
    }

    return fd;
}


int NetWork::closeSocket(SOCKET fd)
{
    int err = errno;
    int flag = close(fd);
    errno = err;

    return flag;
}

int NetWork::shutdownSocketWrite(SOCKET fd)
{
    return shutdown(fd,SHUT_WR);
}

int NetWork::shutdownSocketRead(SOCKET fd)
{
    return shutdown(fd,SHUT_RD);
}

int NetWork::shutdownSocketReadWrite(SOCKET fd)
{
    return shutdown(fd,SHUT_RDWR);
}
 
int NetWork::setBlock(SOCKET fd, bool block)
{
    int flag = fcntl(fd,F_GETFL);
    if(block)
    {
        flag &= ~O_NONBLOCK; 
    }
    else 
    {
        flag |= O_NONBLOCK;
    }

    if(fcntl(fd,F_SETFL,flag))
    {
        return SOCKET_ERR;
    }

    return 0;
}

int NetWork::setTcpNoDelay(SOCKET fd)
{
    int flag = 1;
    if(setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char*)&flag,sizeof(int)))
    {
        return SOCKET_ERR;
    }

    return 0;
}

int NetWork::setLinger(SOCKET fd,int onff,int linger)
{
    struct linger stLinger;
    stLinger.l_onoff  = onff;
    stLinger.l_linger = linger;

    if(setsockopt(fd,SOL_SOCKET,SO_LINGER,&stLinger,sizeof(stLinger)))
    {
        return SOCKET_ERR;
    }

    return 0;
}


int NetWork::setKeepAlive(SOCKET fd)
{
    int flag = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&flag, sizeof(int)))
    {
        return SOCKET_ERR;
    }

    return 0;
}

int NetWork::setPktInfo(SOCKET fd)
{
    int opt = 1;
    if(setsockopt(fd, IPPROTO_IP, IP_PKTINFO, &opt, sizeof(int)))
    {
        return SOCKET_ERR;
    }

    return 0;
}


int NetWork::setKeepAliveConf(SOCKET fd,int idletime,int interval,int cnt)
{
    if(setsockopt(fd,SOL_TCP,TCP_KEEPIDLE,&idletime,sizeof(idletime)) == -1)
    {
        return SOCKET_ERR;
    }

    if(setsockopt(fd,SOL_TCP,TCP_KEEPINTVL,&interval,sizeof(interval)) == -1)
    {
        return SOCKET_ERR;
    }

    if(setsockopt(fd,SOL_TCP,TCP_KEEPCNT,&cnt,sizeof(cnt)) == -1)
    {
        return SOCKET_ERR;
    }

    return 0;
}

int NetWork::setSendBufferSize(SOCKET fd,int sz)
{
    if(setsockopt(fd,SOL_SOCKET,SO_SNDBUF,(char*)&sz,sizeof(int)))
    {
        return SOCKET_ERR;
    }

    return 0;
}

int NetWork::getSendBufferSize(SOCKET fd)
{
    int sz = -1;
    socklen_t len = sizeof(int);
    if(getsockopt(fd, SOL_SOCKET, SO_SNDBUF,(char*)&sz,&len) || len != sizeof(int))
    {
        return SOCKET_ERR;
    }

    return sz;
}

int NetWork::setRecvBufferSize(SOCKET fd,int sz)
{
    int recvSize;

    if((recvSize = getRecvBufferSize(fd)) < 0)
    {
        return SOCKET_ERR;
    }

    if(recvSize >= sz)
    {
        return 0;
    }
    
    if(setsockopt(fd, SOL_SOCKET, SO_RCVBUF,(char*)&sz,sizeof(int)))
    {
        return SOCKET_ERR;
    }

    return 0;
}

int NetWork::getRecvBufferSize(SOCKET fd)
{
    int rz = -1;
    socklen_t len = sizeof(int);
    if(getsockopt(fd, SOL_SOCKET, SO_RCVBUF,(char*)&rz,&len) || len != sizeof(int))
    {
        return SOCKET_ERR;
    }

    return rz;
}


int NetWork::setRecvTimeOut(SOCKET fd, int millsecond)
{
	struct timeval tv;
	
	tv.tv_sec = millsecond / 1000;
	tv.tv_usec = (millsecond % 1000) * 1000;
	
	if(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv)))
    {
		return SOCKET_ERR;
	}
	
	return 0;
}

int NetWork::getRecvTimeOut(SOCKET fd)
{
	struct timeval tv;
	socklen_t len = sizeof(struct timeval);
	
	if(getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, &len) || len != sizeof(struct timeval))
    {
		return SOCKET_ERR;
	}
	
	return tv.tv_sec * 1000 + tv.tv_usec;
}

int NetWork::setSendTimeOut(SOCKET fd, int millsecond)
{
	struct timeval tv;
	
	tv.tv_sec = millsecond / 1000;
	tv.tv_usec = (millsecond % 1000) * 1000;
	
	if(setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv)))
    {
		return SOCKET_ERR;
	}
	
	return 0;
}

int NetWork::getSendTimeOut(SOCKET fd)
{
	struct timeval tv;
	socklen_t len = sizeof(struct timeval);
	
	if(getsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, &len) || len != sizeof(struct timeval))
    {
		return SOCKET_ERR;
	}
	
	return tv.tv_sec * 1000 + tv.tv_usec;
}


int NetWork::setReuseAddr(SOCKET fd)
{
    int flag = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(int)))
    {
        return SOCKET_ERR;
    }

    return 0;
}

int NetWork::doBind(SOCKET fd,struct sockaddr_in &addr)
{
    if(bind(fd,(struct sockaddr*)&addr,sizeof(addr)))
    {
        return SOCKET_ERR;
    }

    return 0;
}

int NetWork::doBind(SOCKET fd, struct sockaddr_un &addr)
{
    if(bind(fd, (struct sockaddr*)&addr,sizeof(addr)))
    {
        return SOCKET_ERR;
    }

    return 0;
}

int NetWork::doListen(SOCKET fd,int blocklog)
{
    int tryCount = 0;
    while(++tryCount <= MAX_LOOP_COUNT)
    {
        if(listen(fd,blocklog))
        {
            if(interrupted())
            {
                continue;
            }
            
            return SOCKET_ERR;
        }

        break;
    }

    return 0;
}

int NetWork::doConnect(SOCKET fd,struct sockaddr_in &addr,int timeout)
{
repeatConnect:
    if(connect(fd,(struct sockaddr*)&addr,sizeof(addr)))
    {
        if(interrupted())
        {
            goto repeatConnect;
        }

        if(connectInProgress())
        {
            struct pollfd pollfd;
            pollfd.fd = fd;
            pollfd.events = POLLOUT;

        repeatPoll:
            int ret = poll(&pollfd,1,timeout);
            if(ret == 0)
            {
                return SOCKET_ERR;
            }
            else if(ret < 0)
            {
                if(interrupted())
                {
                    goto repeatPoll;
                }

                return SOCKET_ERR;
            }

            int value = 0;
            socklen_t len = sizeof(int);
            if(getsockopt(fd,SOL_SOCKET,SO_ERROR,(char*)&value,&len) || value > 0)
            {
                return SOCKET_ERR;
            }

            return 0;
        }

        
        return SOCKET_ERR;
    }

    return 0;
}

SOCKET NetWork::doAccept(SOCKET fd)
{
   int rcvfd = -1;
   int repeatCount = 20;

   while(repeatCount--)
   {
       if((rcvfd = ::accept(fd,NULL,NULL)) < 0)
       {
           if(interrupted())
           {
               continue;
           }

           return SOCKET_ERR;
       }

       break;
   }

   return rcvfd;
}


int NetWork::getIfAddress(const std::string &strIf, int port, struct sockaddr_in &addr)
{
    int retCode = 0;
    memset(&addr, 0, sizeof(addr));
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if(strIf == "*")
    {
        addr.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        int retCode  = -1;
        int tryCount = 3;
        struct ifaddrs* ifres;

        while(tryCount-- > 0)
        {
            if((retCode = getifaddrs(&ifres)) < 0)
            {
                if(interrupted())
                {
                    continue;
                }

                return SOCKET_ERR;
            }

            break;
        }

        if(ifres == NULL || retCode != 0)
        {
            return SOCKET_ERR;
        }

        retCode = SOCKET_ERR;
        for(struct ifaddrs* ifaddr = ifres; ifaddr; ifaddr = ifaddr->ifa_next)
        {
            if((ifaddr->ifa_addr->sa_family == AF_INET)&&(strIf == ifaddr->ifa_name))
            {
                retCode = 0;
                addr.sin_addr = ((struct sockaddr_in*)ifaddr->ifa_addr)->sin_addr;
                
                break;
            }
        }

        free(ifres);
    }

    return retCode;
}

int NetWork::getAddress(const std::string &host,int port,struct sockaddr_in &addr,bool isServer)
{
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if(host == "*")
    {
        addr.sin_addr.s_addr = INADDR_ANY; 
    }
    else 
    {
        addr.sin_addr.s_addr = inet_addr(host.c_str());
    }

    if(addr.sin_addr.s_addr == INADDR_NONE)
    {
        if(!isServer)
        {
            return SOCKET_ERR; 
        }

        int retry = 5;
        struct addrinfo hints = {0};
        struct addrinfo *res = NULL;

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;      // wild IP  
     
        int rs = 0;
        char buf[128] = {0};
        sprintf(buf,"%d",port);

        do
        {
           rs = getaddrinfo(NULL,buf,&hints,&res);
        }while(res == NULL && rs == EAI_AGAIN && --retry >=0);
        
        if(rs != 0 || res == NULL)
        {
            return SOCKET_ERR;
        }        

        struct addrinfo *cur = NULL;
        for(cur = res; cur != NULL;cur = cur->ai_next)
        {
            if(cur->ai_family != AF_INET || !cur->ai_addr)
            {
                continue;
            }

            addr = *((struct sockaddr_in*)cur->ai_addr);
            break;
        }

        if(res)
        {
            freeaddrinfo(res);
        }

        if(cur == NULL)
        {
            return SOCKET_ERR;
        }
    }

    return 0;
}

int NetWork::getHostAddress(const std::string& host,int port,std::vector<struct sockaddr_in>& addrs)
{
    int retry = 5;
    struct addrinfo hints = {0};
    struct addrinfo *res = NULL;

    hints.ai_family = AF_INET;    
    hints.ai_socktype = SOCK_STREAM;                                                                                           
    int rs = 0;
    char buf[128] = {0};
    sprintf(buf,"%d",port);

    do  
    {  
       rs = getaddrinfo(host.empty()? NULL : host.c_str(), buf, &hints, &res);
    }while(res == NULL && rs == EAI_AGAIN && --retry >= 0);
                                                                                                                              
    if(rs != 0 || res == NULL)
    {  
        return SOCKET_ERR;
    }                                                                                                                     

    struct sockaddr_in addr;
    struct addrinfo *cur = NULL;
    for(cur = res; cur != NULL;cur = cur->ai_next)
    {  

        if(cur->ai_family != AF_INET || !cur->ai_addr)
        {   
            continue;
        }   


        addr = *((struct sockaddr_in*)cur->ai_addr);
        addrs.push_back(addr);
    }   

    if(res)
    {   
        freeaddrinfo(res);
    }   

    return 0;
}


bool NetWork::compareAddress(const struct sockaddr_in&  addr1,const struct sockaddr_in& addr2)
{
    return (addr1.sin_family == addr2.sin_family) &&
           (addr1.sin_port == addr2.sin_port) &&
           (addr1.sin_addr.s_addr == addr2.sin_addr.s_addr);
}

int NetWork::createPipe(SOCKET fds[2],bool block)
{
    if(::pipe(fds))
    {
        return SOCKET_ERR;
    } 

    if(!block)
    {
        setBlock(fds[0],false);
        setBlock(fds[1],false);
    }

    return 0;
}

std::string NetWork::errorToString(int err)
{
    return strerror(err);
}

std::string NetWork::fdToString(SOCKET fd)
{
   if(fd < 0)
   {
       return "<closed>";
   }

   struct sockaddr_in localAddr;
   fdToLocalAddress(fd, localAddr);

   struct sockaddr_in remoteAddr;
   int ret = fdToRemoteAddress(fd, remoteAddr);

   int  iLen = 0;
   char szBuf[1024] = {0};

   iLen += snprintf(szBuf + iLen, sizeof(szBuf) - iLen, "local address = %s", addrToString(localAddr).c_str());
   if(ret)
   {
       iLen += snprintf(szBuf + iLen, sizeof(szBuf) - iLen, "\nremote address = <not connected>");
   }
   else
   {
       iLen += snprintf(szBuf + iLen, sizeof(szBuf) - iLen, "\nremote address = %s", addrToString(remoteAddr).c_str());
   }

   return szBuf;
}

int NetWork::fdToLocalAddress(SOCKET fd,struct sockaddr_in& addr)
{
    socklen_t len = sizeof(addr);
    return getsockname(fd,(struct sockaddr*)&addr,&len);
}

int NetWork::fdToLocalAddress(SOCKET fd, unsigned int &ip, unsigned short &port)
{
    int iRet;
    struct sockaddr_in addr;
    if((iRet = fdToLocalAddress(fd, addr)) != 0)
    {
        return iRet;
    }

    ip = addr.sin_addr.s_addr;
    port = addr.sin_port;
    
    return 0;   
}


int NetWork::fdToRemoteAddress(SOCKET fd,struct sockaddr_in& addr)
{
    socklen_t len = sizeof(addr);
    return getpeername(fd,(struct sockaddr*)&addr,&len);
}

int NetWork::fdToRemoteAddress(SOCKET fd, unsigned int &ip, unsigned short &port)
{
    int iRet;
    struct sockaddr_in addr;
    if((iRet = fdToRemoteAddress(fd, addr)) != 0)
    {
        return iRet;
    }

    ip = addr.sin_addr.s_addr;
    port = addr.sin_port;
    
    return 0;   
}

std::string NetWork::addrToString(const struct sockaddr_in& addr)
{
    int  iLen = 0;
    char buf[512] = {0};
    if(inet_ntop(AF_INET,(const void*)&addr.sin_addr, buf, sizeof(buf))!= NULL)
    {
        iLen = strlen(buf);
    }

    snprintf(buf + iLen, sizeof(buf) - iLen -1, ":%u", ntohs(addr.sin_port));

  
    return buf;
}

std::string NetWork::addrToString(unsigned int& ip, unsigned short &port)
{
    struct sockaddr_in addr;

    addr.sin_addr.s_addr = ip;
    addr.sin_port = port;

    return addrToString(addr);
}


std::vector<std::string> NetWork::getLocalHosts()
{
    std::vector<std::string>  ifaddrVec;
    struct ifaddrs *ifap = NULL;
    struct ifaddrs *curr = NULL;

    if(getifaddrs(&ifap) == 0) 
    {
        for(curr = ifap;curr != NULL; curr = curr->ifa_next)
        {
           if(curr->ifa_addr && curr->ifa_addr->sa_family == AF_INET)
           {
               struct sockaddr_in *inaddr = (struct sockaddr_in*)curr->ifa_addr;
               ifaddrVec.push_back(inetAddrToString(inaddr->sin_addr));
           }
        }

        if(ifap)
        {
            freeifaddrs(ifap);
        }
    } 

    return  ifaddrVec;
}

std::string NetWork::inetAddrToString(const struct in_addr& addr)
{
    char buf[512] = {0};
    if(inet_ntop(AF_INET,(const void*)&addr,buf,512))
    {
         return std::string(buf);
    }

    return "";
}

struct in_addr NetWork::stringToInetAddr(const std::string &strIP)
{
    struct in_addr iaddr;
    if(inet_pton(AF_INET, strIP.c_str(), &iaddr) != 1)
    {
        iaddr.s_addr = INADDR_ANY;
    }

    return iaddr;
}

unsigned int NetWork::stringToIpAddr(const std::string &strIP)
{
    struct in_addr iaddr = stringToInetAddr(strIP);

    return iaddr.s_addr;
}


