/**
  * FileName: Network.h
  * Author: Created by tyreezhang
  * History:
  */

#ifndef __NETWORK_COMMON_OPER__H_
#define __NETWORK_COMMON_OPER__H_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/un.h>


#define SOCKET int
#define SOCKET_ERR  -1
#define ADDR_ERR    -2
#define MAX_LOOP_COUNT  3

namespace NetWork
{
   bool interrupted();
   bool wouldBlock();
   bool connectFailed();
   bool connectionRefused();
   bool connectInProgress();
   bool connectionLost();
   bool notConnected();
   

   int createSocket(bool);
   int closeSocket(SOCKET);
   int shutdownSocketWrite(SOCKET);
   int shutdownSocketRead(SOCKET);
   int shutdownSocketReadWrite(SOCKET);
   
   int setBlock(SOCKET, bool);
   int setReuseAddr(SOCKET);
   int setTcpNoDelay(SOCKET);
   int setLinger(SOCKET, int onff, int linger);
   int setKeepAlive(SOCKET);
   int setPktInfo(SOCKET);
   int setKeepAliveConf(SOCKET fd, int idletime, int interval, int cnt);
   int setSendBufferSize(SOCKET, int);
   int getSendBufferSize(SOCKET);
   int setRecvBufferSize(SOCKET, int);
   int getRecvBufferSize(SOCKET);
   int setRecvTimeOut(SOCKET, int);
   int getRecvTimeOut(SOCKET);
   int setSendTimeOut(SOCKET, int);
   int getSendTimeOut(SOCKET);


   int doBind(SOCKET, struct sockaddr_in&);
   int doBind(SOCKET, struct sockaddr_un &);
   int doListen(SOCKET, int);
   int doConnect(SOCKET, struct sockaddr_in&,int);
   SOCKET doAccept(SOCKET);
   
   int getAddress(const std::string&, int, struct sockaddr_in&, bool isServer = true);
   int getIfAddress(const std::string&, int , struct sockaddr_in&);
   int getHostAddress(const std::string&, int, std::vector<struct sockaddr_in>&);
   bool compareAddress(const struct sockaddr_in&, const struct sockaddr_in&);
   
   int createPipe(SOCKET fds[2], bool block = false);
   std::string errorToString(int);
   std::string fdToString(SOCKET);
   
   int fdToLocalAddress(SOCKET, struct sockaddr_in&);
   int fdToLocalAddress(SOCKET, unsigned int &, unsigned short &);
   int fdToRemoteAddress(SOCKET, struct sockaddr_in&);
   int fdToRemoteAddress(SOCKET, unsigned int &, unsigned short &);
   std::string addrToString(const struct sockaddr_in&);
   std::string addrToString(unsigned int&, unsigned short &);

   std::vector<std::string> getLocalHosts();
   std::string inetAddrToString(const struct in_addr&);
   struct in_addr stringToInetAddr(const std::string &strIP);
   unsigned int stringToIpAddr(const std::string &strIP);
}

#endif
