//ringmem.cpp
#include "ringmem.h"

//#include <assert.h>
#include <crtdbg.h>
#define assert(x) _ASSERT(x)
#include <Windows.h>

#if (1)
#define MY_ASSERT(x)           assert((x))
#else
#define MY_ASSERT(x)
#endif
#define ALIGNBY8(size)        (((size) + 15) & (~15))
#define ALIGN_BY(size, block)  (((size) + (block) -1) & ~((block) - 1))

RingMem::RingMem(char *buffer, int size)
{
  //not need init iFirst, iLast
  //ref funcs writeZ, readZ case empty
  mMode = RingMemAddrModeAbsolute;
  mType = RingMemTypeQueue;
  mCount = 0;
  mSize = size;
  mBuffer = buffer;
  mAllocatedSize = 0;

  miFirst = 0;
  miLast = -1;
#if (ENABLE_LOCK_FUNCTION)
  mLockHandle = CreateMutex(
    NULL,              // default security attributes
    FALSE,             // initially owned
    NULL);             // unnamed mutex
#endif /*ENABLE_LOCK_FUNCTION*/
}

/* RINGMEM_API */
RingMem::~RingMem()
{
#if (ENABLE_LOCK_FUNCTION)
  if (mLockHandle) {
    CloseHandle(mLockHandle);
    mLockHandle = 0;
  }
#endif
}
/* RINGMEM_API */
int RingMem::IsEmpty() { return mCount == 0; };
/* RINGMEM_API */
int RingMem::IsFull() { return mCount == mSize; };
/* RINGMEM_API */
int RingMem::Remain() { return (mSize - mCount); };
/* RINGMEM_API */
int RingMem::Write(char *data, int nByte, int *pWriten)
{
  int part1, part2;
  if (mAllocatedSize == 0)
  {
    if (nByte > mSize)
    {
      MY_ASSERT(0);     //not recommend
      nByte = mSize;
    }
    MY_ASSERT(nByte <= mSize);
    *pWriten = nByte;

    //type stack
    //case 1
    //buffer  |   |data |<-content->|
    //        |<-space->|
    //case2
    //                   |<-content      ->|
    //buffer  |data part2|     | data part1|
    //        |<-space ->|     |over write |

    //type queue
    //case 1
    //buffer |<-content->.|data|...|
    //                    |remain  |
    //case 2
    //       |<-content   ->|
    //buffer |data part2|...|data part1|
    //       |over write|   |remain    |
    part1 = mSize - mCount;
    if (part1 >= nByte)
    {
      writeZ(data, nByte);
    }
    else
    {
      part2 = nByte - part1;
      MY_ASSERT(part2 > 0);
      if (mType == RingMemTypeStack)
      {
        writeZ(data + part2, part1);
        writeZ(data, part2);
      }
      else
      {
        MY_ASSERT(mType == RingMemTypeQueue);
        writeZ(data, part1);
        writeZ(data + part1, part2);
      }
    }
  }
  else
  {
    MY_ASSERT(0); //not recommend
    *pWriten = 0;
  }
  return 0;
}

/* RINGMEM_API */
int RingMem::Read(char *outBuff, int nByte, int *pRead)
{
  if (nByte <= mCount)
  {
    //buffer |data   ...|
    //       |outbuff|
    readZ(outBuff, nByte, pRead);
  }
  else
  {
    //buffer |data   ...|
    //       |outbuff......|
    MY_ASSERT(nByte > mCount);
    readZ(outBuff, mCount, pRead);
  }
  return 0;
}
/* RINGMEM_API */
int RingMem::Peek(int index, char *outBuff, int nByte, int *pRead)
{
  int adjust = mCount - index;
  if (adjust <= 0)
  {
    *pRead = 0;
  }
  else if (nByte <= adjust)
  {
    //buffer |data   ...|
    //       |outbuff|
    peekZ(index, outBuff, nByte, pRead);
  }
  else
  {
    //buffer |data   ...|
    //       |outbuff......|
    MY_ASSERT((nByte > adjust) && (adjust > 0));
    peekZ(index, outBuff, adjust, pRead);
  }
  return 0;
}
/* RINGMEM_API */
int RingMem::Discard(int nByte, int *pDiscarded)
{
  if (nByte > mCount) 
  {
    nByte = mCount;
  }

  {
    //buffer |data   ...|
    //       |discard|
    mCount -= nByte;
    miFirst += nByte;
    miFirst %= mSize;
    *pDiscarded = nByte;
  }
  //else
  //{
  //  //buffer |data   ...|
  //  //       |discard......|
  //  MY_ASSERT(nByte > mCount);
  //  *pDiscarded = mCount;
  //  mCount = 0;
  //}
  return 0;
}
/* RINGMEM_API */
int RingMem::Empty()
{
  mCount = 0;   //ref funcs writeZ, readZ case empty
  return 0;
}
/* RINGMEM_API */
int RingMem::Allocate(PCHAR *pHandle, int size, int *allocatedSize)
{
  if (mType == RingMemTypeQueue && mAllocatedSize == 0)
  {
    if (size > mSize)
    {
      size = mSize;
    }

    if (mCount == 0)
    {
      miFirst = 0;
      miLast = -1;
      mAllocatedSize = size;
      *pHandle = get(0, &mAllocatedSize);
      *allocatedSize = mAllocatedSize;
    }
    else
    {
      allocateZ(pHandle, size, allocatedSize);
    }
  }
  else
  {
    MY_ASSERT(0);   //not recommend
    *allocatedSize = 0;
  }
  return 0;
}
int RingMem::allocateZ(PCHAR *pHandle, int size, int *allocatedSize)
{
  int delta;
  MY_ASSERT(mType == RingMemTypeQueue);
  MY_ASSERT(mAllocatedSize == 0);
  //case 1
  //                  |allocate size|
  //  buffer |data ...|allocated    |         |
  //                 ^-iLast
  //         |<-- buffer size              -->|
  //case 2
  //                               |allocate size|
  //  buffer |data ...             |allocated |
  mAllocatedSize = mSize - ((miLast + 1) % mSize);
  if (mAllocatedSize > size)
  {
    mAllocatedSize = size;
  }
  *pHandle = get((miLast + 1) % mSize, &mAllocatedSize);
  *allocatedSize = mAllocatedSize;
  //adjust miFist like Write(mAllocatedSize...)

  //      |buffer                                  |
  //case1 |data                |                   |
  //       ^-iFirst           ^-iLast
  //                           |<-allocated->|
  //case2 |data2   |                 |data1        |
  //              ^-iLast             ^-iFirst
  //               |<-allocated->|
  //case3 |data2   |          |data1               |
  //              ^-iLast      ^-iFirst
  //               |<-allocated->|
  delta = mAllocatedSize - (mSize - mCount);
  if (delta > 0)
  {
    //case 3
    miFirst = (miFirst + delta) % mSize;
    mCount -= delta;
  }
  return 0;
}
/* RINGMEM_API */
int RingMem::Locate(int index, PCHAR *pHandle, int nByte, int *locatedSize)
{
  int adjust = mCount - index;
  if (adjust <= 0)
  {
    *locatedSize = 0;
  }
  else if (nByte <= adjust)
  {
    //buffer |data   ...    |
    //           |nByte|
    locateZ(index, pHandle, nByte, locatedSize);
  }
  else
  {
    //buffer |data   ...|
    //        |nByte......|
    MY_ASSERT((nByte > adjust) && (adjust > 0));
    locateZ(index, pHandle, adjust, locatedSize);
  }
  return 0;
}
int RingMem::locateZ(int index, PCHAR *pHandle, int nByte, int *locatedSize)
{
  int located, delta;
  //LOGICAL
  //                             |locate  |
  //RingMem |data                         |
  //         ^-iFirst             ^      ^-iLast
  //                              index
  //PHYSICAL
  //case 1
  //buffer  |data             |           |
  //              ^-delta    ^-iLast
  //case 2  |data 2        |     |data 1  |
  //                      ^-iLast    ^delta
  //case 3  |data 2        |     |data 1  |
  //              ^delta  ^-iLast
  delta = (miFirst + index) % mSize;
  if (delta < miLast)
  {
    //case 1 & 3
    located = miLast - delta + 1;
  }
  else
  {
    //case 2
    located = mSize - delta;
  }

  *pHandle = get(delta, &located);
  if (nByte <= located)
  {
    *locatedSize = nByte;
  }
  else
  {
    *locatedSize = located;
  }
  return 0;
}
/* RINGMEM_API */
int RingMem::Commit(int nByte, int *commited)
{
  if (mType == RingMemTypeQueue && mAllocatedSize != 0)
  {
    if (nByte > mAllocatedSize)
    {
      MY_ASSERT(0);     //not recommend
      nByte = mAllocatedSize;
    }

    mAllocatedSize = 0;
    *commited = nByte;
    ////adjust miLast like Write(nByte...)
    //if (mCount == 0)
    //{
    //  miLast = nByte - 1;
    //}
    //else
    {
      miLast = (miLast + nByte) % mSize;
    }
    mCount += nByte;
  }
  return 0;
}
int RingMem::Commit(int index)
{
  if (mMode == RingMemVMemMode) {
    if (index < mCount) {
      int rc = mVMem->Commit(index);
      MY_ASSERT(rc == 0);
    }
  }
  return 0;
}
int RingMem::writeZ(char *data, int nByte)
{
  //int remain = mSize - mCount;
  int delta;
  //MY_ASSERT(remain >= nByte || remain == 0);
  MY_ASSERT((mSize - mCount) >= nByte || (mSize - mCount) == 0);

  if (mCount == 0)
  {
    //case empty
    //+ type stack
    // buffer|   |data|
    //           |   ^-miLast
    //            ^-miFirst
    //+ type queue
    // buffer|data|...|
    //        ^  ^-miLast
    //        miFirst
    mCount += nByte;
    if (mType == RingMemTypeStack)
    {
      miLast = mSize - 1;
      miFirst = mSize - nByte;
      set(miFirst, data, nByte);
    }
    else
    {
      MY_ASSERT(mType == RingMemTypeQueue);
      miLast = nByte - 1;
      miFirst = 0;
      set(0, data, nByte);
    }
  }
  else if (mCount < mSize)
  {
    //case normal
    //+ type stack
    //       |<-space->|<-content->|
    // buffer|   |data |           |
    //                 |          ^-miLast
    //                  ^-miFirst
    //+ type queue
    //        |<-content->|<-space->|
    // buffer |           |data|... |
    //        |          ^-miLast
    //         ^-miFirst
    mCount += nByte;
    if (mType == RingMemTypeStack)
    {
      miFirst = (miFirst + mSize - nByte) % mSize;
      push(miFirst, data, nByte);
    }
    else
    {
      MY_ASSERT(mType == RingMemTypeQueue);
      delta = (miLast + 1) % mSize;
      push(delta, data, nByte);
      miLast += nByte;
      miLast %= mSize;
    }
  }
  else
  {
    MY_ASSERT(mCount == mSize);
    //case full
    //+ type stack
    //        |<-content        ->|
    // buffer |        |over write|
    //        |                  ^-miLast
    //         ^-miFirst
    //+ type queue
    //        |<-content       ->|
    // buffer |over write |      |
    //        |                 ^-miLast
    //         ^-miFirst
    if (mType == RingMemTypeStack)
    {
      miFirst = (miFirst + mSize - nByte) % mSize;
      miLast = (miLast + mSize - nByte) % mSize;
      push(miFirst, data, nByte);
    }
    else
    {
      MY_ASSERT(mType == RingMemTypeQueue);
      miFirst += nByte;
      miFirst %= mSize;
      delta = (miLast + 1) % mSize;
      push(delta, data, nByte);
      miLast += nByte;
      miLast %= mSize;
    }
  }
  return 0;
}

int RingMem::readZ(char *outBuff, int nByte, int *pRead)
{
  int delta;
  MY_ASSERT(mCount >= nByte);
  //case empty
  if (mCount == 0)
  {
    *pRead = 0;
    return 0;
  }
  else
  {
    //case normal
    mCount -= nByte;
    delta = miFirst;
    pop(outBuff, delta, nByte);
    miFirst += nByte;
    miFirst %= mSize;
    *pRead = nByte;
  }
  return 0;
}

int RingMem::peekZ(int index, char *outBuff, int nByte, int *pRead)
{
  MY_ASSERT(index >= 0);
  int delta;
  MY_ASSERT(mCount >= (nByte + index));
  //case empty
  if (mCount == 0)
  {
    *pRead = 0;
    return 0;
  }
  else
  {
    //case normal
    delta = (miFirst + index) % mSize;
    pop(outBuff, delta, nByte);
    *pRead = nByte;
  }
  return 0;
}

int RingMem::push(int index, char *srcBuf, int size)
{
  MY_ASSERT(index >= 0);
  int part1;
  //type queue similar as type stack
  //case 1
  // Buffer: |...|srcbuff|...|
  //              ^-index
  //case 2
  // Buffer: |srcbuff part2|...|srcbuff part1|
  //                            ^-index
  part1 = mSize - index;
  if (part1 >= size)
  {
    set(index, srcBuf, size);
  }
  else
  {
    MY_ASSERT(part1 < size);
    set(index, srcBuf, part1);
    set(0, srcBuf + part1, size - part1);
  }
  return 0;
}

int RingMem::pop(char *outBuf, int index, int size)
{
  MY_ASSERT(index >= 0);
  int part1;
  // Buffer: |...|outData|...|
  //              ^
  //              index
  part1 = mSize - index;
  if (part1 >= size)
  {
    get(outBuf, index, size);
  }
  else
  {
    // Buffer: |outData part2|...|outData part1|
    //                            ^
    //                            index
    MY_ASSERT(part1 < size);
    get(outBuf, index, part1);
    get(outBuf + part1, 0, size - part1);
  }
  return 0;
}
char* RingMem::get(int index, int* count)
{
  MY_ASSERT(index >= 0);
  if (mMode == RingMemVMemMode) {
    int rc;
    int n;
    char* data;
    rc = mVMem->Locate(index, &data, &n);
    MY_ASSERT(rc == 0);
    MY_ASSERT(n <= *count);
    if (n < *count) { *count = n; }
    return data;
  }
  else if (mMode == RingMemAddrModeAbsolute) {
    return mBuffer + index;
  }
  else {
    MY_ASSERT(mMode == RingMemAddrModeRelative);
    return (char*)this + mOffset + index;
  }
}
char* RingMem::get(char* outBuf, int index, int size)
{
  MY_ASSERT(index >= 0);
  if (mMode == RingMemVMemMode) {
    int n, rc;
    rc = mVMem->Read(index, outBuf, size, &n);
    MY_ASSERT(rc == 0);
    MY_ASSERT(n == size);
    return outBuf;
  }
  else if (mMode == RingMemAddrModeAbsolute) {   //absolute addr
    return (char*)memcpy(outBuf, mBuffer + index, size);
  }
  else {
    MY_ASSERT(mMode == RingMemAddrModeRelative);
    return (char*)memcpy(outBuf, (char*)this + mOffset + index, size);
  }
}
char* RingMem::set(int index, char* inBuf, int size)
{
  MY_ASSERT(index >= 0);
  if (mMode == RingMemVMemMode) {
    int n, rc;
    rc = mVMem->Write(index, inBuf, size, &n);
    MY_ASSERT(rc == 0);
    MY_ASSERT(n == size);
    return inBuf;
  }
  else if (mMode == RingMemAddrModeAbsolute) {
    return (char*)memcpy(mBuffer + index, inBuf, size);
  }
  else {
    MY_ASSERT(mMode == RingMemAddrModeRelative);
    return (char*)memcpy((char*)this + mOffset + index, inBuf, size);
  }
}

int RingMem::BufferSize(){ return mSize; };
int RingMem::Count(){ return mCount; };
#if (ENABLE_LOCK_FUNCTION)
/*RINGMEM_API*/
int RingMem::AcqLock()
{
  if (mLockHandle) {
    WaitForSingleObject(
      mLockHandle,    // handle to mutex
      INFINITE);      // no time-out interval;
  }
  return 0;
}
/*RINGMEM_API*/
int RingMem::RlsLock()
{
  if (mLockHandle) {
    ReleaseMutex(mLockHandle);
  }
  return 0;
}
#endif /*ENABLE_LOCK_FUNCTION*/
/*RINGMEM_API*/
RingMem & RingMem::operator= (const RingMem & other)
{
  MY_ASSERT(this != &other);
  if (mMode == other.mMode) {
    //not need init iFirst, iLast
    //ref funcs writeZ, readZ case empty
    miFirst = other.miFirst;
    miLast = other.miLast;
    mCount = other.mCount;
    mSize = other.mSize;
    mAllocatedSize = other.mAllocatedSize;
    mBuffer = other.mBuffer;
#if (ENABLE_LOCK_FUNCTION)
    if (mLockHandle) {
      CloseHandle(mLockHandle);
      mLockHandle = 0;
    }
    if (other.mLockHandle) {
      DuplicateHandle(GetCurrentProcess(),
        other.mLockHandle,
        GetCurrentProcess(),
        &mLockHandle,
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS);
    }
#endif /*ENABLE_LOCK_FUNCTION*/
  }
  else {
    MY_ASSERT(0);
  }
  return *this;
}
/*RINGMEM_API*/
void RingMem::Reset(char *buffer, int size, int mode)
{
  if (mode == RingMemAddrModeAbsolute) {
    MY_ASSERT(buffer != (char*)this);
    mBuffer = buffer;
  }
  else {
    MY_ASSERT(mode == RingMemAddrModeRelative);
    MY_ASSERT(buffer == (char*)this);

    int moduleSize = AlignedModuleSize();
    memset(buffer, 0, moduleSize);
    mOffset = moduleSize;
    size -= moduleSize;
  }

  mMode = mode;
  mCount = 0;
  MY_ASSERT(size > 0);
  mSize = size;
  mAllocatedSize = 0;
  miFirst = 0;
  miLast = -1;
}
/*RINGMEM_API*/
int RingMem::SystemPageSize()
{
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return si.dwPageSize;
}
int RingMem::AlignedModuleSize()
{
#if (RINGMEM_ALIGN_MODE == 1)
  return ALIGN_BY(sizeof(RingMem), SystemPageSize());
#else
  return ALIGNBY8(sizeof(RingMem));
#endif
}

//-----------------RMem & VMem-------------------
RingMem::RingMem(RingMemParams* p)
{
  Init(p);
}
int RingMem::Init(RingMemParams* p)
{
  mType = p->type;
  MY_ASSERT(mType == RingMem::RingMemTypeQueue
    || mType == RingMem::RingMemTypeStack);

  mMode = p->mode;
  if (p->mode == RingMemVMemMode) {
    mVMem = p->hVMem;
    mSize = mVMem->Size();
  }
  else if (p->mode == RingMemAddrModeAbsolute) {
    MY_ASSERT(p->buffer != (char*)this);
    mBuffer = p->buffer;
    mSize = p->bufferSize;
  }
  else {
    int moduleSize;
    MY_ASSERT(p->mode == RingMemAddrModeRelative);
    MY_ASSERT(p->buffer == (char*)this);
    moduleSize = AlignedModuleSize();
    memset(p->buffer, 0, moduleSize);
    mOffset = moduleSize;
    mSize = p->bufferSize - moduleSize;
  }

  mCount = 0;
  MY_ASSERT(mSize > 0);
  mAllocatedSize = 0;

  return 0;
}
int RingMem::MapExistedContent(int size)
{
  if (0 < size && size <= mSize) {
    mCount = size;
    miFirst = 0;
    miLast = size - 1;
  }
  return 0;
}
int RingMem::Final()
{
  return 0;
}
