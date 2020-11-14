//#include <assert.h>
#include <crtdbg.h>
#define assert(x) _ASSERT(x)

#include "qsMem.h"
#include "vmem.h"

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
  mCount = 0;
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

  if (mFlags & modeStack) {
    miFirst = mBufferSize;
  }
  else {
    //default mode queue
    assert(mFlags & modeQueue);
    mFlags |= modeQueue;
    miFirst = 0;
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
  return 0;
}
int QSMem::Peek(int index, char *outBuff, int nByte, int *pRead)
{
  assert(index >= 0);
  int avaiable = mCount - miFirst - index;
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
