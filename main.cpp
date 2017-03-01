#include <iostream>
#include <sstream>
#include "zfs.h"
#include <stdlib.h>
#include <unistd.h>
#include "mkfs_net.h"

using namespace std;

int main(int argc, char *argv[])
{

    if(argc == 4 && strcmp(argv[1],"-c") == 0)
    {
        mkfs_net net;
        int port = atoi(argv[3]);
        net.clientConnect(string(argv[2]),port);
        string line;
        while(getline(cin,line))
        {
            net.clientSendCmdLine(line);
            net.clientWaitRespond();
        }
    }
    else if(argc == 3 && strcmp(argv[1],"-s") == 0)
    {
        int port = atoi(argv[2]);
        ZFS zfs;
        zfs.startServer(port);
    }
    else
    {
        ZFS zfs;
        zfs.run();
    }




    return 0;
}
