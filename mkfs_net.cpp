#include "mkfs_net.h"
#include <fcntl.h>

mkfs_net::mkfs_net()
{
    conn_fd = sock_fd = 0;
}

mkfs_net::~mkfs_net()
{
    if(conn_fd != 0) close(conn_fd);
    if(sock_fd != 0) close(sock_fd);
}

void mkfs_net::clientConnect(string server, int port)
{
    struct sockaddr_in clientaddr;
    bzero(&clientaddr,sizeof(clientaddr));

    clientaddr.sin_family=AF_INET;
    clientaddr.sin_addr.s_addr=htons(INADDR_ANY);
    clientaddr.sin_port=htons(0);

    sock_fd=socket(AF_INET,SOCK_STREAM,0);

    if(sock_fd<0)
    {
        perror("socket");
        exit(1);
    }
    else
        cout << "socket success"<<endl;

    if(::bind(sock_fd,(struct sockaddr*)&clientaddr,sizeof(clientaddr))<0)
    {
        perror("bind");
        exit(1);
    }
    else
        cout << "bind success"<<endl;

    struct sockaddr_in serv_addr;
    bzero(&serv_addr,sizeof(serv_addr));
    struct hostent *serv = gethostbyname(server.c_str());
    if (serv == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)serv->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          serv->h_length);
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(port);

    socklen_t svraddrlen=sizeof(serv_addr);
    cout<<"connecting to server" <<endl;
    if(connect(sock_fd,(struct sockaddr*)&serv_addr,svraddrlen)<0)
    {
        perror("connect");
        exit(1);
    }
    cout<<"Client Connected to Server"<<endl;
}

void mkfs_net::clientSendCmdLine(string line)
{
    cout<<"Read to send "<<line<<endl;
    int length=line.size();
    int ret = send(sock_fd, &length, 4, 0);
//    cout<<"send len " << ret << endl;
    ret = send(sock_fd, line.c_str(),length,0);
//    cout<<", send msg "<< ret<<endl;
}

void mkfs_net::clientWaitRespond()
{
    char plen[4];
    int size = 0;
    size = ::recv(sock_fd, plen,4,0);
    int len = *((int*)plen);
    cout<<"res len "<<len<<endl;
    char buf[32];
    string ret;
    while(len > 0)
    {
        int cnt = recv(sock_fd, buf, sizeof(buf)-1,0);
        string tmp(buf, cnt);
        ret.append(tmp);
        len -= cnt;
    }
    cout<<"remote : " <<ret<<endl;
}

void mkfs_net::serverStart(int port)
{
    struct sockaddr_in addr_serv,addr_client;
    sock_fd = socket(AF_INET,SOCK_STREAM,0);
    if(sock_fd < 0){
        perror("socket");
        exit(1);
    } else {
        printf("sock sucessful\n");
    }
    memset(&addr_serv,0,sizeof(addr_serv));
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_port = htons(port);
    addr_serv.sin_addr.s_addr =INADDR_ANY;
    memset(&addr_client,0,sizeof(addr_client));
    socklen_t client_len = sizeof(addr_client);
    int ret = ::bind(sock_fd,(struct sockaddr *)&addr_serv,sizeof(struct sockaddr_in));
    if(ret < 0)
    {
        perror("bind");
        exit(1);
    } else {
        printf("bind sucess\n");
    }
    if (listen(sock_fd,1) < 0){
        perror("listen");
        exit(1);
    } else {
        printf("listen sucessful\n");
    }

    printf("begin accept:\n");
    conn_fd = ::accept(sock_fd,(struct sockaddr *)&addr_client,&client_len);
    if(conn_fd < 0){
        perror("accept");
        exit(1);
    }
    printf("accept a new client,ip:%s\n",inet_ntoa(addr_client.sin_addr));
}

string mkfs_net::serverRead()
{
    char plen[4];
    int size = 0;
    size = ::recv(conn_fd, plen,4,0);
    int len = *((int*)plen);
    char buf[32];
    string ret;
    while(len > 0)
    {
        int cnt = recv(conn_fd, buf, sizeof(buf)-1,0);
        string tmp(buf, cnt);
        ret.append(tmp);
        len -= cnt;
    }
    cout<<"remote : "<<ret<<endl;
    return ret;
}

void mkfs_net::serverGrabCout()
{
    stdoutCopy = dup(1);

    system("rm out.txt");
    pipedOutFd = open("out.txt", O_WRONLY | O_CREAT, 0666);

    if(dup2(pipedOutFd,1) == -1){
        perror("dup2.1");
        exit(-1);
    }
}


void mkfs_net::serverReleaseCoutAndRespond()
{
    close(pipedOutFd);
    if(dup2(stdoutCopy,1) < 0){
        perror("dup2.1 back");
        exit(-1);
    }
    FILE *fd = fopen("out.txt","r");
    string ret;

    char buf[32];
    fseek(fd,0,SEEK_END);
    int len = ftell(fd);
    fseek(fd,0,SEEK_SET);
//    cout<<"file len = "<<len<<endl;
    send(conn_fd,&len,4,0);
    while(len > 0)
    {
        int size = fread(buf,1,32,fd);
        len -= size;
        ret += string(buf,size);
    }
    send(conn_fd,ret.c_str(),ret.size(),0);

    fclose(fd);
}
