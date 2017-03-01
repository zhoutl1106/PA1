#ifndef ZFS_H
#define ZFS_H

#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>
using namespace std;

#define BLOCK_SIZE (4096)
#define TOTAL_SIZE (100*1024*1024)
#define BLOCK_NUM (TOTAL_SIZE/BLOCK_SIZE)

#define GET_BLOCK_INDEX(x)  ((x)/BLOCK_SIZE)
#define SET_NAME(x,y) (strcpy(x,y))
#define BUF2FCB(x) ((FCB*)(blockBuf[x]))

#define MAX_NAME 256

#define MAX_BLOCK_POINTER ((BLOCK_SIZE - 272)/4)
#define MAX_DIRECT_DATA (MAX_BLOCK_POINTER * BLOCK_SIZE)


enum blockType{T_DIR,T_FILE,T_IGN};

struct FCB
{
    blockType type;
    int block;
    int parent;
    char name[MAX_NAME];
    int size;           // number of files of DIR, len of file for FILE
    int direct_pointer[MAX_BLOCK_POINTER];
};

class ZFS
{
public:
    ZFS();
    ~ZFS();
    void mkfs();
    void pwd();
    void open(string name, string flag);
    string read(int fd, int size);
    void write(int fd, string dat);
    void seek(int fd, int offset);
    void close(int fd);
    void mkdir(string dir);
    void rmdir(string dir);
    void cd(string dir);
    void ls();
    void cat(string name);
    void tree();
    void import(string src, string dest);
    void exprt(string src, string dest);

private:
    FILE *fd;

    FCB* findPath(string path); // return parent FCB contents path, support absolute path and relative path.
    FCB* findFile(string path); // return file FCB support absolute path and relative path.

    void writeFCB(int block, FCB* p);
    void writeData(int block, void* p, int len, int blockOffset);
    string readData(int block, int len, int offset);
    void updatePageNum();

    void lsOfBlockFcb(int block);
    void insertFCBtoDirBlock(int block, FCB* p);

    int currentDirBlock;
    int nextNewInodeBlock;
    int nextNewDataBlock;

    FCB *currentWorkingVFile;
    int VFilePage;
    int VFilePageOffset;
    int VFileOffset;
};

#endif // ZFS_H
