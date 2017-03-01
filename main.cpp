#include <iostream>
#include <sstream>
#include "zfs.h"
#include <stdlib.h>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[])
{
    string line;
    string cmd,arg0,arg1;
    int op0,op1;

    ZFS zfs;
    while(getline(cin,line))
    {
        cmd = "";
        arg0 = arg1 = "";
        op0 = op1 = -1;
        istringstream iss(line);
        iss>>cmd;

        //        cout<<cmd<<","<<iss.str()<<endl;
        if(cmd == "mkfs")
            zfs.mkfs();
        else if(cmd == "pwd")
            zfs.pwd();
        else if(cmd == "open")
        {
            iss>>arg0>>arg1;
            zfs.open(arg0,arg1);
        }
        else if(cmd == "read")
        {
            iss>>op0>>op1;
            zfs.read(op0,op1);
        }
        else if(cmd == "write")
        {
            iss>>op0;
            getline(iss,arg0);
            zfs.write(op0,arg0);
        }
        else if(cmd == "seek")
        {
            iss>>op0>>op1;
            zfs.seek(op0,op1);
        }
        else if(cmd == "close")
        {
            iss>>op0;
            zfs.close(op0);
        }
        else if(cmd == "mkdir")
        {
            iss>>arg0;
            zfs.mkdir(arg0);
        }
        else if(cmd == "rmdir")
        {
            iss>>arg0;
            zfs.rmdir(arg0);
        }
        else if(cmd == "cd")
        {
            iss>>arg0;
            zfs.cd(arg0);
        }
        else if(cmd == "ls")
            zfs.ls();
        else if(cmd == "cat")
        {
            iss>>arg0;
            zfs.cat(arg0);
        }
        else if(cmd == "tree")
            zfs.tree();
        else if(cmd == "import")
        {
            iss>>arg0>>arg1;
            zfs.import(arg0,arg1);
        }
        else if(cmd == "export")
        {
            iss>>arg0>>arg1;
            zfs.exprt(arg0,arg1);
        }


    }

    return 0;
}
