#include "zfs.h"

char blockBuf[BLOCK_NUM][BLOCK_SIZE];

char* buf;

ZFS::ZFS()
{
    //    cout<<MAX_BLOCKINDEX_PER_BLOCK<<","<<MAX_DIR_ENTRIES_PER_BLOCK<<","<<MAX_DIR_DIRECT_ENTRIES<<","<<MAX_DIR_FIRST_ENTRYIES<<endl;
//    cout<<sizeof(FCB)<<endl;
}

ZFS::~ZFS()
{

}

void ZFS::updatePageNum()
{
    VFilePage = VFileOffset / BLOCK_SIZE;
    VFilePageOffset = VFileOffset % BLOCK_SIZE;
}

void ZFS::mkfs()
{
    std::ofstream ofs("output.img", std::ios::binary | std::ios::out);
    ofs.seekp((100<<20) - 1);
    ofs.write("", 1);
    ofs.close();

    currentDirBlock = 0;
    nextNewInodeBlock = 1;
    nextNewDataBlock = BLOCK_NUM - 1;

    FCB root;
    root.block = 0;
    root.parent = 0;
    root.type = T_DIR;
    SET_NAME(root.name,"/");
    root.size = 0;
    writeFCB(0, &root);
}

void ZFS::writeFCB(int block, FCB *p)
{
    cout<<"  # write FCB to Block "<<block<<endl;
    memcpy(blockBuf[block],p,BLOCK_SIZE);
    int offset = block * BLOCK_SIZE;
    fd = fopen("output.img","r+");
    fseek(fd,offset,SEEK_SET);
    fwrite(p,sizeof(FCB),1,fd);
    fclose(fd);
}

void ZFS::writeData(int block, void *p, int len, int blockOffset)
{
//    cout<<"  # write data to Block "<<block <<", len = "<<len<<", offset = "<<blockOffset<<endl;
//    printf("%s, %c, %X, %X %X\n",buf, ((char*)p)[0],((char*)p)[0],blockBuf[block][0],blockBuf[block][1]);
    memcpy(blockBuf[block] + blockOffset, p, len);
//    printf("%X %X\n", blockBuf[block][0],blockBuf[block][1]);
    int offset = block * BLOCK_SIZE;
    fd = fopen("output.img","r+");
    fseek(fd,offset+blockOffset,SEEK_SET);
    fwrite(p,len,1,fd);
    fclose(fd);
}

void ZFS::pwd()
{
    FCB* fcb = BUF2FCB(currentDirBlock);
    cout<<fcb->name<<endl;
}

void ZFS::lsOfBlockFcb(int block)
{
    FCB *fcb = BUF2FCB(block);
    int len = fcb->size;

    cout<<". -> "<<fcb->block<<endl;
    cout<<".. -> "<<fcb->parent<<endl;

    for(int j = 0;j<len;j++)
    {
        FCB *child = BUF2FCB(fcb->direct_pointer[j]);
        if(child->type != T_IGN)
            cout<<child->name<<" -> "<<child->block<<","<<child->parent<<endl;
    }
}

FCB* ZFS::findPath(string dir)
{
    int block = currentDirBlock;
    if(dir == ".") return BUF2FCB(currentDirBlock);
    if(dir == "..")
    {
        return BUF2FCB(BUF2FCB(currentDirBlock)->parent);
    }
    while(dir.find("/") != string::npos)
    {
        if(dir.find_first_of("/") == 0)
        {
            block = 0;
            dir = dir.substr(1);
            continue;
        }
        string sub = dir.substr(0,dir.find_first_of("/"));
        dir = dir.substr(dir.find_first_of("/")+1);
        cout<<sub<<"--"<<dir<<endl;
        FCB* path = (FCB*)(blockBuf[block]);
        if(sub == ".") continue;
        if(sub == "..")
        {
            block = path->parent;
            continue;
        }
        int i;
        for(i = 0;i<path->size;i++)
        {
            FCB* child = BUF2FCB(path->direct_pointer[i]);
            if(strcmp(child->name,sub.c_str()) == 0)
            {
                //                cout<<"find path got "<<child->name<<" @ idx "<<i<<endl;
                break;
            }
        }
        block = (BUF2FCB(path->direct_pointer[i]))->block;
        //        cout<<"find path enter block"<<block<<endl;
    }
    return BUF2FCB(block);
}

FCB* ZFS::findFile(string path)
{
    FCB* parent = findPath(path);
    string name = path.substr(path.find_last_of("/") + 1);
    int i;
    for(i = 0;i<parent->size;i++)
    {
        FCB* child = BUF2FCB(parent->direct_pointer[i]);
        if(strcmp(child->name,name.c_str()) == 0)
        {
            //            cout<<"find file block "<<child->block<<endl;
            return BUF2FCB(child->block);
        }
    }
    return NULL;
}

void ZFS::open(string name, string flag)
{
    if(flag == "r")
    {
        currentWorkingVFile = findFile(name);
        cout<<"SUCCESS, fd = "<<currentWorkingVFile->block<<endl;

        VFileOffset = 0;
        updatePageNum();
    }
    else
    {
        currentWorkingVFile = findFile(name);
        if(currentWorkingVFile != NULL)
        {
//            cout<<"open exist file to write"<<endl;
            VFileOffset = currentWorkingVFile->size;
            updatePageNum();
        }
        else
        {
            FCB fcb;
            fcb.type = T_FILE;
            fcb.parent = currentDirBlock;
            fcb.block = nextNewInodeBlock;
            nextNewInodeBlock ++;
            SET_NAME(fcb.name,name.c_str());
            fcb.size = 0;
            fcb.direct_pointer[0] = nextNewDataBlock;
            nextNewDataBlock --;
            writeFCB(fcb.block,&fcb);
            insertFCBtoDirBlock(currentDirBlock, &fcb);
            currentWorkingVFile = BUF2FCB(fcb.block);
            cout<<"SUCCESS, fd = "<<fcb.block<<endl;
            VFileOffset = 0;
            updatePageNum();
        }
    }
}

string ZFS::readData(int block, int len, int offset)
{
    string ret(blockBuf[block] + offset, len);
//    cout<<"read Data got "<<ret << " of len " << len<< ", offset "<< offset<<", block "<<block<<endl;
    return string(blockBuf[block] + offset, len);
}

string ZFS::read(int fd, int size)
{
    string ret;
    currentWorkingVFile = BUF2FCB(fd);

    if(VFileOffset + size > currentWorkingVFile->size)
    {
        size = currentWorkingVFile->size - VFileOffset;
//        cout<<"truncate read to "<<size<<endl;
    }

    // try read up current page.
    int pageremain = BLOCK_SIZE - VFilePageOffset;
    int readSize = size > pageremain? pageremain:size;

    ret += readData(currentWorkingVFile->direct_pointer[VFilePage],readSize,VFilePageOffset);

    VFileOffset += readSize;
    updatePageNum();

    size -= readSize;

    while(size > 0)
    {
        readSize = size > BLOCK_SIZE? BLOCK_SIZE:size;
        ret += readData(currentWorkingVFile->direct_pointer[VFilePage],readSize,VFilePageOffset);
        size -= readSize;
        VFileOffset += readSize;
        updatePageNum();
    }
    cout << ret << endl;
    return ret;
}

void ZFS::write(int fd, string dat)
{
    currentWorkingVFile = BUF2FCB(fd);
    int p0;
    for(p0 = 0;p0<dat.size();p0++)
        if(dat.c_str()[p0] != ' ')
            break;

    dat = dat.substr(p0);
    int len = dat.length();
//    cout<<"write "<<dat<<", "<<len<<endl;
    if(currentWorkingVFile->size + len > MAX_DIRECT_DATA)
    {
        cout<<"Write Error: File Size Limition Reached."<<endl;
    }

    buf = new char[len];
    strcpy(buf,dat.c_str());
    // try fill current page;
    if(VFilePageOffset == 0 && len != 0)
    {
//        cout << "allocate new data block "<<nextNewDataBlock<<endl;
        currentWorkingVFile->direct_pointer[VFilePage] = nextNewDataBlock;
        nextNewDataBlock --;
    }
    char *p = buf;
    int pageremain = BLOCK_SIZE - VFilePageOffset;
    int writeSize = len > pageremain? pageremain:len;
    writeData(currentWorkingVFile->direct_pointer[VFilePage],p,writeSize,VFilePageOffset);
    VFileOffset += writeSize;
    updatePageNum();
    p += writeSize;
    len -= writeSize;
    currentWorkingVFile->size += writeSize;

    while(len > 0)
    {
//        cout << "allocate new data block "<<nextNewDataBlock<<endl;
        currentWorkingVFile->direct_pointer[VFilePage] = nextNewDataBlock;
        nextNewDataBlock --;
        writeSize = len > BLOCK_SIZE? BLOCK_SIZE:len;
        writeData(currentWorkingVFile->direct_pointer[VFilePage],p,writeSize,0);
        currentWorkingVFile->size += writeSize;
        len -= writeSize;
        VFileOffset += writeSize;
        updatePageNum();
    }
    delete [] buf;
}

void ZFS::seek(int fd, int offset)
{
//    cout<<"seek "<<fd<<","<<offset<<endl;
    currentWorkingVFile = BUF2FCB(fd);
    if(offset <= currentWorkingVFile->size)
    {
        VFileOffset = offset;
        updatePageNum();
    }
}

void ZFS::close(int fd)
{
    FCB* path = (FCB*)(blockBuf[currentDirBlock]);
    int i;
    for(i = 0;i<path->size;i++)
    {
        FCB* child = (FCB*)(blockBuf[path->direct_pointer[i]]);
        if(child->block == fd)
        {
            cout<<"close, fd = "<<child->block<<endl;
            break;
        }
    }
}

void ZFS::insertFCBtoDirBlock(int block, FCB *p)
{
    FCB* dir = (FCB*)(blockBuf[block]);

//    cout<<"  # insert inode " << p->block<<","<<p->name << " to "<< block<<","<<dir->name<<endl;
    dir->size += 1;
    if(dir->size >= MAX_BLOCK_POINTER)
    {
        cout<<"Directory Full, Maximum Entries is 186"<<endl;
        return;
    }

    dir->direct_pointer[dir->size - 1] = p->block;
    writeFCB(block, dir);
}

void ZFS::mkdir(string dir)
{
    if(dir == "") return;

    FCB* parent = findPath(dir);

    FCB root;
    root.block = nextNewInodeBlock;
    root.parent = parent->block;
    root.type = T_DIR;
    SET_NAME(root.name,dir.substr(dir.find_last_of("/") + 1).c_str());
    root.size = 0;
    writeFCB(root.block, &root);
    nextNewInodeBlock ++;

    insertFCBtoDirBlock(parent->block, &root);
}

void ZFS::rmdir(string dir)
{
    if(dir == "") return;
    FCB* fcb = findFile(dir);
    //    cout<<"rmdir set block "<<fcb->block;
    fcb->type = T_IGN;
    writeFCB(fcb->block,fcb);
}

void ZFS::cd(string dir)
{
    if(dir == ".")
        return;
    else if(dir == "..")
        currentDirBlock = BUF2FCB(currentDirBlock)->parent;
    else
    {
        FCB* fcb = findFile(dir);
        currentDirBlock = fcb->block;
    }
}

void ZFS::ls()
{
    lsOfBlockFcb(currentDirBlock);
}

void ZFS::cat(string name)
{
    cout<<"cat "<<name<<endl;
    open(name,"r");
    read(currentWorkingVFile->block,currentWorkingVFile->size);
}

void ZFS::tree()
{
    cout<<"tree"<<endl;
}

void ZFS::import(string src, string dest)
{
//    cout<<"import "<<src<<","<<dest<<endl;
    open(dest,"w");
    FILE *fd = fopen(src.c_str(), "r");
    if(fd == NULL)
        perror("fopen");
    char buf[1024];
    int size = 0;
    while(!feof(fd))
    {
        size = fread(buf,1,1024,fd);
        cout<<"import size "<<size<<endl;
        write(currentWorkingVFile->block, string(buf,size));
    }
    fclose(fd);
}

void ZFS::exprt(string src, string dest)
{
//    cout<<"exprt "<<src<<","<<dest<<endl;
    open(src,"r");
    FILE *fd = fopen(dest.c_str(), "w");
    string ret = read(currentWorkingVFile->block, currentWorkingVFile->size);
    fwrite(ret.c_str(),currentWorkingVFile->size,1,fd);
    fclose(fd);
}
