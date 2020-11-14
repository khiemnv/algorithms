//ringme.h
#include "vmem.h"
#include "MemIF.h"

#ifndef RING_MEM_H__
#define RING_MEM_H__

#ifdef RINGMEM_DLL
#ifdef RINGMEM_LIBRARY_EXPORT   // inside DLL
#   define RINGMEM_API   __declspec(dllexport)
#else // outside DLL
#   define RINGMEM_API   __declspec(dllimport)
#endif /*RINGMEM_LIBRARY_EXPORT*/
#else
#   define RINGMEM_API
#endif /*RINGMEM_DLL*/

#define RINGMEM_ALIGN_MODE 0  //=0: align by 16; =1: by system page

typedef char* PCHAR;
typedef void* HANDLE;

struct RingMemParams :public BufferParams
{
  int type;
  int mode;
  VMem *hVMem;
};

//RingMem
//CASE 1
//             |data          |
// Buffer |... |x| ...      |x|...         |
//              ^            ^
//              miFirst      miLast
//CASE 2
//        |data part2|          |data part1|
// Buffer |...     |x|...       |x|...     |
//                  ^            ^
//                  miLast       miFirst       
//
/*RINGMEM_API*/
class RingMem
  :public QueueIF
{
private:
  int mType;    //work as queue or stack
  int mMode;    //specify  Buffer is absolute addr(=0) or relative addr(=1)
  int miFirst;
  int miLast;
  int mCount;   //by byte
  int mSize;    //by byte
  int mAllocatedSize;   //size of recent allocated space
  union
  {
    int  mOffset;
    char *mBuffer;
    VMem *mVMem;
  };
  char* get(int index, int* count);
  char* get(char* outBuf, int index, int size);
  char* set(int index, char* inBuf, int size);
  int push(int index, char *outBuf, int size);
  int pop(char *outBuf, int index, int size);
  int writeZ(char *data, int size);
  int readZ(char *outBuff, int size, int *read);
  int peekZ(int index, char *outBuff, int nByte, int *pRead);
  int locateZ(int index, PCHAR *pHandle, int nByte, int *locatedSize);
  int allocateZ(PCHAR *pHandle, int size, int *allocatedSize);
#if (ENABLE_LOCK_FUNCTION)
  HANDLE mLockHandle;
#endif
public:
  enum RingMemAddrMode
  {
    RingMemAddrModeAbsolute = 0x00,
    RingMemAddrModeRelative = 0x01,
    RingMemVMemMode = 0x02,

    RingMemTypeQueue = 0x00,  //default mode
    RingMemTypeStack = 0x10,
  };
  RINGMEM_API RingMem& operator= (const RingMem & other);
  RINGMEM_API void Reset(char *buffer, int size, int mode = 0);
  RINGMEM_API static int AlignedModuleSize();
  RINGMEM_API static int SystemPageSize();

  //func Init, Final
public:
  RINGMEM_API RingMem(RingMemParams* p);
  RINGMEM_API int Init(RingMemParams *);
  RINGMEM_API int MapExistedContent(int size);
  RINGMEM_API int Final();

  //func RingMem
  //  init ring mem
  //PARAMS
  //  __in buffer: pointer to buffer that hold ring mem data
  //  __int size: size of buffer
  RINGMEM_API RingMem(char *buffer, int size);
  RINGMEM_API ~RingMem();
  //func Write
  //  Write data to ring mem
  //PARAMS
  //  __in data: poiter to buffer that hold data
  //  __in nByte: number of byte need be written
  //  __out pWrited: number of byte was written
  //NOTE
  //  + pointer auto seek after write (like file write)
  //  + if remain is not enough, overwrite the data in begin mem
  //    write data|data part2|...|data part1|
  //    ringmem   |over write|   |remain    |
  //  + nByte should > 0
  RINGMEM_API int Write(char *data, int nByte, int *pWrited);
  //func Read
  //  Read data from ring mem
  //PARAMS
  //  __out outBuff: pointer to buff that hold read data
  //  __in nByte: number of byte want to read
  //  __out pRead: number of byte was read
  //NOTE
  //  + pointer auto seek after read (like file read)
  //  + nByte should > 0
  RINGMEM_API int Read(char *outBuff, int nByte, int *pRead);
  //func Peek
  //  like Read() but DONT seek pointer
  //PARAMS
  //  __in index: start read byte (zero base)
  //  __out outBuff: pointer to buff that hold read data
  //  __in nByte: number of byte want to read
  //  __out pRead: number of byte was read
  RINGMEM_API int Peek(int index, char *outBuff, int nByte, int *pRead);
  //func Discard
  //  remove nByte from begin mem
  //NOTE
  //  like Read() but DONT write out data to outBuff
  RINGMEM_API int Discard(int nByte, int *pDiscarded);
  RINGMEM_API int IsEmpty();
  RINGMEM_API int IsFull();
  //return the remain of the ring mem in byte
  RINGMEM_API int Remain();
  //empty the ring mem
  RINGMEM_API int Empty();
  //func Allocate
  //  allocate a mem space in  RMem
  //NOTE
  //  can not allocate new space if pre allocated space not commit
  RINGMEM_API int Allocate(PCHAR *pHandle, int size, int *allocatedSize);
  //func Locate
  //  locate data at special index
  //NOTE
  //  like Peek() but not copy data to outBuff
  RINGMEM_API int Locate(int index, PCHAR *pHandle, int nByte, int *locatedSize);
  //func Commit
  //  commit recent allocated space
  RINGMEM_API int Commit(int nByte, int *commited);
  RINGMEM_API int Commit(int index);
  RINGMEM_API int BufferSize();
  RINGMEM_API int Count();
#if (ENABLE_LOCK_FUNCTION)
  RINGMEM_API int AcqLock();
  RINGMEM_API int RlsLock();
#endif
};
#endif //RING_MEM_H__