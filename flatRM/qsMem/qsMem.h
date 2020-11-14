#ifndef __QSMEM_H__
#define __QSMEM_H__

struct QSMemParams
{
  enum {
    modeQueue     = 0x01,
    modeStack     = 0x02,
    addrAbsolute  = 0x10,
    addrRelative  = 0x20,   //not recommend
    addrVMem      = 0x40,
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
{
  int miFirst;
  int mCount;   //by byte

  char* get(char* outBuf, int index, int size);
  char* set(int index, char* inBuf, int size);
public:
  int Init(QSMemParams*);
  int Final();

  int Read(char *outBuff, int nByte, int *pRead);
  int Write(char *data, int nByte, int *pWrite);
  int Peek(int index, char *outBuff, int nByte, int *pRead);
  int Discard(int nByte, int *pDiscard);

  int Remain();
  int Count();
};
#endif  //__QSMEM_H__
