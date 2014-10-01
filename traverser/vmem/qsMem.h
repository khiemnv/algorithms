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
private:
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
  int Commit(int nByte, int *committed);

  int Remain();
  int Count();
};

class Lock
  : public LockIF
{
private:
  void* mLockData;
public:
  Lock(void* pLockData);
  ~Lock();

  void enter();
  void leave();
};

class ShareQueue
	: public MemMngIF
{
private:
  MemMngIF* mQueue;
  LockIF* mLock;
public:
  ShareQueue(MemMngIF*, LockIF*);
  ~ShareQueue();

  int Read(char *outBuff, int nByte, int *pRead);
  int Write(char *data, int nByte, int *pWrite);
  int Peek(int index, char *outBuff, int nByte, int *pRead);
  int Discard(int nByte, int *pDiscard);
  int Allocate(char** pHandle, int size, int *allocatedSize);
  int Commit(int nByte, int *commited);
  int Remain();
  int Count();
};
#endif  //__QSMEM_H__
