#ifndef __VMEM_H__
#define __VMEM_H__

#include <tchar.h>

#define ALIGN_BY_16(x) (((x) + 0x0f) & ~0x0f)
#define MY_MIN(x,y) ((x)<=(y)?(x):(y))

#define PG_SELECT_MODE (1)  //select page that has max addr to swap
                            //compatible with queue

//-----------------STRUCTRURE & INTERFACE------------------
typedef char* PCHAR;
//mem
class MemIF {
public:
  virtual int Read(int addr, char* buff, int size, int *count) = 0;
  virtual int Write(int addr, char* data, int size, int *count) = 0;
};

MemIF* CreateMem();
void DestroyMem(MemIF* target);

class FileIF{
public:
  virtual int Read(int addr, char* buff, int size, int *count) = 0;
  virtual int Write(int addr, char* buff, int size, int *count) = 0;
};

//-------structure-----------
struct FileSwapParams
{
  enum {
    ModeNormal,
    ModeNoBuffer,
  };
  TCHAR* zName;
  int mode;
};
struct BufferParams
{
  char* buffer;
  int bufferSize;
};
struct PageMngParams :public BufferParams
{
  int pgDataSize;
  FileIF* hFile;
};
struct VMemParams :public PageMngParams
{
  int vMemSize;
};
//-----------------CLASS & IMPLEMENT-----------------------
//file swap
class FileSwap : public FileIF
{
private:
  void* mFileHandle;
  int initialize(TCHAR* zFile, int mode);
public:
  static int Truncate(TCHAR* zFile, int size);
  FileSwap(TCHAR* zFile);
  FileSwap(FileSwapParams *p);
  ~FileSwap();
  int Read(int addr, char* buff, int size, int *count);
  int Write(int addr, char* buff, int size, int *count);
  int Extend(int size);
  int Truncate(int size);
  int Size();
};

//page mng
//id    [0]     [1]     [2]
//name  |page 1 |page 2 |page 3 |
enum {
  PageDataSize = 8,
  VMemSize = 32,
  //PageDataSize = 512,
  //VMemSize = 0x7FFFffff,
};
typedef struct page {
  int id;
  int size;   //page capacity (in byte)

  int nextId;
  int addr;

  int nRead;
  int nWrite;
  char data[1];
}Page, *PPAGE;
//pgRoot
//pgFree

//vMem
//addr  |pgNo
//0x0000|page 1|
//0x0100|page 3|
//0x0200|page 2|

//PageMng
class PageMng {
private:
  int pgRoot;
  int pgFree;
  int pgCount;
  int pgDataSize;
  FileIF* pgFile;

  //buffer
  int mBuffSize;
  char* mBuff;
  //locate
  int pgLocked;

  int pgReset(page* target);
  int pgSize();
  int pgHold(int addr, page* target);
  int pgIsEmpty(page* target);
  page* pgIndex(int i);
  page* pgFind(int addr);

  page* pgAllocate();
  int pgSwap(page* target);
  int pgLoad(int addr, page* target, int mode);
  int pgSelect();

  int pgDetach(int pgId, int* pgList);
  int pgAttach(int pgId, int* pgList);

  int pgListSize(int pgId);
  int pgCountUsed();

  int pgSetup();
  int pgSwapAll();
  int initialize(char* buffer, int bufferSize, int blockSize, FileIF* hFile);
public:
  static int CalcPgSize(int dataSize) {
    return ALIGN_BY_16(sizeof(page)) + dataSize;
  }
  int Init(char* buff, int size);
  int Init(PageMngParams*);
  int Final();
  int Reset(char* buff = 0, int size = 0);

  int Locate(int addr, PPAGE* out, int mode);
  int Push(int addr, char* data, int size, page* target, int* count);
  int Pop(int addr, char* buff, int size, page* target, int* count);
  int Size();
  int Lock(page* target);
  int Unlock(page* target);
};

//VMem
class VMem
{
private:
  PageMng mPageMng;
  int mSize;

  int setZ(int addr, char* data, int size);
  int getZ(int addr, char* buff, int size);
  int locateZ(int addr, PCHAR* pData, int *count);
  int commitZ(int addr);
public:
  int Init(char* buff, int size);
  int Init(VMemParams*);
  int Final();
  int Reset(char* buff = 0, int size = 0);
  int Size();

  int Read(int addr, char* buff, int size, int *count);
  int Write(int addr, char* data, int size, int *count);
  //note: can not locate if pgCount == 1
  int Locate(int addr, PCHAR* pData, int *count);
  int Commit(int addr);
};
#endif //__VMEM_H__
