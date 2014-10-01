//#include <assert.h>
#include <crtdbg.h>
#define assert(x) _ASSERT(x)
#include <Windows.h>

#include "qsMem.h"
#include "vmem.h"

//PARAM
//  __in_out count: locate size
char* QSMem::get(int index, int* count)
{
  assert(index >= 0);
  if (mFlags & addrVMem) {
    int rc;
    int n;
    char* data;
    rc = ((VMem*)mVMem)->Locate(index, &data, &n);
    assert(rc == 0);
    assert(n <= *count);
    if (n < *count) { *count = n; }
    return data;
  }
  else if (mFlags & addrAbsolute) {
    return mBuffer + index;
  }
  else {
    assert(mFlags & addrRelative);
    return (char*)this + mOffset + index;
  }
}
char* QSMem::get(char* outBuf, int index, int size)
{
  assert(index >= 0);
  if (mFlags & addrVMem) {
    int n, rc;
    rc = ((VMem*)mVMem)->Read(index, outBuf, size, &n);
    assert(rc == 0);
    assert(n == size);
    return outBuf;
  }
  else if (mFlags & addrAbsolute) {   //absolute addr
    return (char*)memcpy(outBuf, mBuffer + index, size);
  }
  else {
    assert(mFlags & addrRelative);
    return (char*)memcpy(outBuf, (char*)this + mOffset + index, size);
  }
}
char* QSMem::set(int index, char* inBuf, int size)
{
  assert(index >= 0);
  if (mFlags & addrVMem) {
    int n, rc;
    rc = ((VMem*)mVMem)->Write(index, inBuf, size, &n);
    assert(rc == 0);
    assert(n == size);
    return inBuf;
  }
  else if (mFlags & addrAbsolute) {
    return (char*)memcpy(mBuffer + index, inBuf, size);
  }
  else {
    assert(mFlags & addrRelative);
    return (char*)memcpy((char*)this + mOffset + index, inBuf, size);
  }
}

//-------public methods----------------
#define ALIGN_BY(size, block)  (((size) + (block) -1) & ~((block) - 1))

int QSMem::Init(QSMemParams*p)
{
  int rc = 0;
  //init all member
  miFirst = 0;
  mCount = 0;
  mAllocatedSize = 0;
  //+ init flags, buffer, buffer size
  mFlags = p->mFlags;
  if (mFlags & addrRelative) {
    //take care when use
    int moduleSize;
    assert((void*)this == (void*)p);
    moduleSize = ALIGN_BY(sizeof(QSMem), 0x10);
    mOffset = moduleSize;
    mBufferSize -= moduleSize;
  }
  else if (mFlags & addrVMem) {
    mVMem = p->mVMem;
    mBufferSize = ((VMem*)mVMem)->Size();
  }
  else {
    //default mode addr absolute
    assert(mFlags & addrAbsolute);
    mFlags |= addrAbsolute;
    mBuffer = p->mBuffer;
    mBufferSize = p->mBufferSize;
  }

  //set cur pointer
  if (mFlags & modeStack) {
    if (mFlags & mapExistingContent) {
      //not implement
      rc = -1;
      assert(0);
    }
    miFirst = mBufferSize;
  }
  else {
    //default mode queue
    assert(mFlags & modeQueue);
    mFlags |= modeQueue;
    miFirst = 0;
    if (mFlags & mapExistingContent) {
      mCount = mBufferSize;
    }
  }
  return rc;
}
int QSMem::Final()
{
  mFlags = 0;
  mBufferSize = 0;
  mBuffer = 0;
  mCount = 0;
  miFirst = 0;
  return 0;
}
int QSMem::Read(char *outBuff, int nByte, int *pRead)
{
  if (nByte > mCount) {
    nByte = mCount;
  }
  if (nByte > 0) {
    get(outBuff, miFirst, nByte);
    miFirst += nByte;
    mCount -= nByte;
    *pRead = nByte;
  }
  else {
    assert(nByte == 0);
    *pRead = 0;
  }
  return 0;
}
int QSMem::Write(char *data, int nByte, int *pWrite)
{
  int avaiable;
  if (mAllocatedSize == 0)
  {
    if (mFlags & modeStack) {
      avaiable = mBufferSize - mCount;
    }
    else {
      avaiable = mBufferSize - miFirst + mCount;
    }
    if (nByte > avaiable) {
      nByte = avaiable;
    }
    if (nByte > 0) {
      if (mFlags & modeStack) {
        set(miFirst - nByte, data, nByte);
        miFirst -= nByte;
        mCount += nByte;
        *pWrite = nByte;
      }
      else {
        assert(mFlags & modeQueue);
        set(miFirst + mCount, data, nByte);
        mCount += nByte;
        *pWrite = nByte;
      }
    }
    else {
      assert(nByte == 0);
      *pWrite = 0;
    }
  }
  else {
    //should Commit() Allocated space before Write()
    assert(nByte == 0);
    *pWrite = 0;
  }
  return 0;
}
int QSMem::Peek(int index, char *outBuff, int nByte, int *pRead)
{
  assert(index >= 0);
  int avaiable = mCount - index;
  if (nByte > avaiable) {
    nByte = avaiable;
  }
  if (nByte > 0) {
    get(outBuff, miFirst + index, nByte);
    *pRead = nByte;
  }
  else {
    assert(nByte == 0);
    *pRead = 0;
  }
  return 0;
}
int QSMem::Discard(int nByte, int *pDiscard)
{
  if (nByte > mCount)
    nByte = mCount;
  miFirst += nByte;
  mCount -= nByte;
  *pDiscard = nByte;
  return 0;
}
int QSMem::Allocate(char** pHandle, int size, int *allocatedSize)
{
  int avaiable;
  if (mFlags & modeQueue && mAllocatedSize == 0)
  {
    avaiable = mBufferSize - miFirst + mCount;
    if (size > avaiable) {
      size = avaiable;
    }

    mAllocatedSize = size;
    *pHandle = get(miFirst + mCount, &mAllocatedSize);
    *allocatedSize = mAllocatedSize;
  }
  else
  {
    assert(0);   //not recommend
    *allocatedSize = 0;
  }
  return 0;
}
int QSMem::Commit(int nByte, int *committed)
{
  if ((mFlags & modeQueue) && (mAllocatedSize != 0))
  {
    if (nByte > mAllocatedSize)
    {
      assert(0);     //not recommend
      nByte = mAllocatedSize;
    }
    //adjust like Write(nByte...)
    mCount += nByte;

    mAllocatedSize = 0;
    *committed = nByte;
  }
  return 0;
}
int QSMem::Remain()
{
  int remain;
  if (mFlags & modeStack){
    remain = mBufferSize - mCount;
  }
  else {
    assert(mFlags & modeQueue);
    remain = mBufferSize - miFirst - mCount;
  }
  return remain;
}
int QSMem::Count()
{
  return (mCount);
}
//-----------------lock obj----------------------
Lock::Lock(void* pLockData)
{
  mLockData = pLockData;
}
Lock::~Lock()
{
  mLockData = 0;
}
void Lock::enter()
{
  EnterCriticalSection((LPCRITICAL_SECTION)mLockData);
}
void Lock::leave()
{
  LeaveCriticalSection((LPCRITICAL_SECTION)mLockData);
}
//-----------------share queue-------------------
#define LOCK_EXECUTE(x) {int rc; mLock->enter(); rc = (x); mLock->leave(); return rc;}

ShareQueue::ShareQueue(MemMngIF* queueHandle, LockIF* lockHandle)
{
  mQueue = queueHandle;
  mLock = lockHandle;
}
ShareQueue::~ShareQueue()
{
  mQueue = 0;
  mLock = 0;
}

int ShareQueue::Read(char *outBuff, int nByte, int *pRead)
{
  LOCK_EXECUTE(mQueue->Read(outBuff, nByte, pRead));
}
int ShareQueue::Write(char *data, int nByte, int *pWrite)
{
  LOCK_EXECUTE(mQueue->Write(data, nByte, pWrite));
}
int ShareQueue::Peek(int index, char *outBuff, int nByte, int *pRead)
{
  LOCK_EXECUTE(mQueue->Peek(index, outBuff, nByte, pRead));
}
int ShareQueue::Discard(int nByte, int *pDiscard)
{
  LOCK_EXECUTE(mQueue->Discard(nByte, pDiscard));
}
int ShareQueue::Allocate(char** pHandle, int size, int *allocatedSize)
{
  LOCK_EXECUTE(mQueue->Allocate(pHandle, size, allocatedSize));
}
int ShareQueue::Commit(int nByte, int *committed)
{
  LOCK_EXECUTE(mQueue->Commit(nByte, committed));
}
int ShareQueue::Remain()
{
  LOCK_EXECUTE(mQueue->Remain());
}
int ShareQueue::Count()
{
  LOCK_EXECUTE(mQueue->Count());
}
