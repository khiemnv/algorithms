#ifndef __QSMEM_H__
#define __QSMEM_H__

#include "MemIF.h"

struct QSMemParams
{
  enum {
    modeQueue     = 0x01,
    modeStack     = 0x02,

    addrAbsolute  = 0x10,
    addrRelative  = 0x20,   //not recommend
    addrVMem      = 0x40,

    mapExistingContent = 0x0100,
  };
  int mFlags;
  int mBufferSize;
  union
  {
    int mOffset;
    char *mBuffer;
    void *mVMem;
  };
};

class QSMem :private QSMemParams
  ,public MemMngIF
{
  int miFirst;
  int mCount;   //by byte
  int mAllocatedSize;

  char* get(int index, int* count);
  char* get(char* outBuf, int index, int size);
  char* set(int index, char* inBuf, int size);
public:
  int Init(QSMemParams*);
  int Final();

  int Read(char *outBuff, int nByte, int *pRead);
  int Write(char *data, int nByte, int *pWrite);
  int Peek(int index, char *outBuff, int nByte, int *pRead);
  int Discard(int nByte, int *pDiscard);
  int Allocate(char** pHandle, int size, int *allocatedSize);
  int Commit(int nByte, int *commited);

  int Remain();
  int Count();
};

class ShareQueue
  :public MemMngIF
{
public:
  ShareQueue(QSMemParams*p);
  ~ShareQueue();

#define LOCK_EXEC(x)  {enterLock(); int rc = x; leaveLock(); return rc;}
  int Peek(int index, char *outBuff, int nByte, int *pRead);
  int Discard(int nByte, int *pDiscard);
  int Read(char *outBuff, int nByte, int *pRead);
  int Write(char *data, int nByte, int *pWrite);
  int Remain();
  int Count();
  int Allocate(char** pHandle, int size, int *allocatedSize);
  int Commit(int nByte, int *commited);

private:
  QSMem mQueue;
  void* mLockHandle;
  void enterLock();
  void leaveLock();
};
#endif  //__QSMEM_H__
