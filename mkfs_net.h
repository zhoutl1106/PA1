#ifndef MKFS_NET_H
#define MKFS_NET_H

#include <string>
#include <iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>//close()
#include<netinet/in.h>//struct sockaddr_in
#include<arpa/inet.h>//inet_ntoa
#include <netdb.h>
using namespace std;

class mkfs_net
{
public:
    mkfs_net();
    ~mkfs_net();
    void clientConnect(string server, int port);
    void clientSendCmdLine(string line);
    void clientWaitRespond();

    void serverStart(int port);
    string serverRead();
    void serverGrabCout();
    void serverReleaseCoutAndRespond();

private:
    string serverName;
    int m_port;
    int sock_fd;
    int conn_fd;
    int stdoutCopy;
    int pipedOutFd;
};

#endif // MKFS_NET_H
