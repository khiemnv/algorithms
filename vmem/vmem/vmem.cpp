#include "vmem.h"

//#include <assert.h>
#include <crtdbg.h>
#define assert(x) _ASSERT(x)

#include <stdlib.h>
#include <string.h>
#include <Windows.h>

//MemIF
class VMemFull : public MemIF
{
private:
  char* mBuff;
  int mSize;
  VMem mVmem;
public:
  enum { BufferSize = 256, };
  VMemFull()
  {
    mBuff = new char[BufferSize];
    if (mBuff) {
      if (mVmem.Init(mBuff, BufferSize) == 0) {
        mSize = mVmem.Size();
        assert(mSize > 0);
      }
      else {
        delete mBuff;
        mBuff = 0;
        mSize = 0;
      }
    }
    else {
      mSize = 0;
    }
  }
  ~VMemFull() {
    if (mSize > 0) { mVmem.Final(); }
    if (mBuff) { delete mBuff; }
  }
  int Size() { return mSize; }

  int Read(int addr, char* buff, int size, int *count) {
    return mVmem.Read(addr, buff, size, count);
  }
  int Write(int addr, char* buff, int size, int *count) {
    return mVmem.Write(addr, buff, size, count);
  }
};
MemIF* CreateMem()
{
  VMemFull *p = new VMemFull();
  if (p->Size() == 0) {
    delete p;
    p = 0;
  }
  return p;
}
void DestroyMem(MemIF* target)
{
  VMemFull *p = (VMemFull*)target;
  delete p;
}

//-----------------FileSwap----------------------
int FileSwap::Truncate(TCHAR* zFile, int size)
{
  int rc;
  HANDLE hf = CreateFile(
    zFile,
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    0,
    OPEN_ALWAYS,
    FILE_ATTRIBUTE_NORMAL,
    0
    );
  if (hf != INVALID_HANDLE_VALUE) {
    rc = SetFilePointer(hf, size, 0, FILE_BEGIN);
    if (rc == size) {
      rc = SetEndOfFile(hf);
      assert(rc);
      rc = 0;
    }
    else {
      rc = GetLastError();
    }
  } else {
    rc = GetLastError();
  }
  return rc;
}
FileSwap::FileSwap(TCHAR* zFile)
{
  initialize(zFile, FileSwapParams::ModeNormal);
}
//if success func return 0
int FileSwap::initialize(TCHAR* zFile, int mode)
{
  int rc;
  DWORD flags;
  HANDLE hFile;

  mFileHandle = 0;
  mMapHandle = 0;
  mMapAddr = 0;

  flags = FILE_ATTRIBUTE_NORMAL;
  if (mode == FileSwapParams::ModeNoBuffer)
    flags |= FILE_FLAG_NO_BUFFERING;
  hFile = CreateFile(
    zFile,
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    0,
    OPEN_ALWAYS,
    flags,
    0
    );
  if (hFile != INVALID_HANDLE_VALUE) {
    mFileHandle = hFile;
    rc = 0;
  } else {
    rc = GetLastError();
  }

  return rc;
}
FileSwap::FileSwap(FileSwapParams *p)
{
  initialize(p->zName, p->mode);
}
FileSwap::~FileSwap()
{
  if (mMapAddr) {
    UnmapViewOfFile(mMapAddr);
    mMapAddr = 0;
  }
  if (mMapHandle) {
    CloseHandle(mMapHandle);
    mMapHandle = 0;
  }
  if (mFileHandle) {
    CloseHandle(mFileHandle);
    mFileHandle = 0;
  }
}
//if success func return 0
int FileSwap::Read(int addr, char* buff, int size, int *count)
{
  DWORD nRead = 0;
  int rc;
  rc = SetFilePointer(mFileHandle, addr, 0, FILE_BEGIN);
  if (rc == addr) {
    rc = ReadFile(
      mFileHandle,
      buff,
      size,
      &nRead,
      0);
    assert(rc);
    *count = nRead;
    rc = 0;
  }
  else {
    assert(0);
    *count = 0;
    rc = GetLastError();
  }
  return rc;
}
//if success func return 0
int FileSwap::Write(int addr, char* buff, int size, int *count)
{
  DWORD nWrite = 0;
  int rc;

  rc = SetFilePointer(mFileHandle, addr, 0, FILE_BEGIN);
  if (rc == addr) {
    rc = WriteFile(
      mFileHandle,
      buff,
      size,
      &nWrite,
      0);
    assert(rc);
    *count = nWrite;
    rc = 0;
  }
  else {
    assert(0);
    *count = 0;
    rc = GetLastError();
  }

  return rc;
}
int FileSwap::Extend(int size)
{
  int rc;
  LARGE_INTEGER mapSize;
  mapSize.HighPart = 0;
  mapSize.LowPart = size;
  HANDLE hMap = CreateFileMapping(
    mFileHandle,
    NULL,
    PAGE_READWRITE,
    mapSize.HighPart, mapSize.LowPart,
    NULL
    );
  if (hMap != INVALID_HANDLE_VALUE) {
    CloseHandle(hMap);
    assert(Size() == size);
    rc = 0;
  }
  else {
    rc = GetLastError();
  }
  return rc;
}
//if success function return 0
int FileSwap::Truncate(int size)
{
  int rc;
  rc = SetFilePointer(mFileHandle, size, 0, FILE_BEGIN);
  if (rc == size) {
    rc = SetEndOfFile(mFileHandle);
    assert(rc);
    rc = 0;
  }
  else {
    rc = GetLastError();
  }
  return rc;
}
//return file size
int FileSwap::Size()
{
  return GetFileSize(mFileHandle, 0);
}
//if success func return 0
int FileSwap::MapFile(void** pMadAddr)
{
  int rc;
  LARGE_INTEGER mapSize;
  HANDLE hMap;
  void* mapAddr;

  mapSize.HighPart = 0;
  mapSize.LowPart = Size();
  hMap = CreateFileMapping(
    mFileHandle,
    NULL,
    PAGE_READWRITE,
    mapSize.HighPart, mapSize.LowPart,
    NULL
    );
  if (hMap != INVALID_HANDLE_VALUE) {
    mapAddr = MapViewOfFile(
      hMap,
      FILE_MAP_READ,
      0,
      0,
      mapSize.LowPart
      );
    if (hMap) {
      mMapHandle = hMap;
      mMapAddr = mapAddr;
      *pMadAddr = mapAddr;
      rc = 0;
    } else {
      rc = GetLastError();
      CloseHandle(hMap);
    }
  }
  else {
    rc = GetLastError();
  }
  return rc;
}
int FileSwap::UnMapFile()
{
  if (mMapAddr) {
    UnmapViewOfFile(mMapAddr);
    mMapAddr = 0;
  }
  if (mMapHandle) {
    CloseHandle(mMapHandle);
    mMapHandle = 0;
  }
  return 0;
}
//-----------------PageMng-----------------------
int PageMng::pgReset(page* target)
{
  assert(target->id >= 0);
  assert(target->id < pgCount);
  target->nRead = 0;
  target->nWrite = 0;
  target->nextId = -1;
  return 0;
}
int PageMng::pgSize()
{
  return (ALIGN_BY_16(sizeof(page)) + pgDataSize);
}
//if pg hold addr func return 1
//else if addr out of page func return 0
int PageMng::pgHold(int addr, page* target)
{
  int d = addr - target->addr;
  if (d >= 0 && d < target->size)
    return 1;
  else
    return 0;
}
//if all data in page was popped out, func return 1
//else if remain data, func return 0
int PageMng::pgIsEmpty(page* target)
{
  if (target->nRead == target->nWrite)
    return 1;
  else
    return 0;
}
page* PageMng::pgIndex(int i)
{
  assert(i >= 0 && i < pgCount);
  return (page*)(mBuff + i*pgSize());
}

//if found, func return pointer to page that hold addr
//else func return 0
page* PageMng::pgFind(int addr)
{
  int rc;
  int i;
  page *pPage = 0;

  i = pgRoot;
  rc = -1;
  for (; i != -1; i = pPage->nextId) {
    pPage = pgIndex(i);
    //if found
    if (pgHold(addr, pPage)) {
      rc = 0;
      break;
    }
  }

  if (rc == 0)
    return pPage;
  else
    return 0;
}
//locate a free page
//NOTE: case Locate() >>Read()/Write() not swap locked page
page* PageMng::pgAllocate()
{
  assert(pgCount > 0);
  assert(pgCount == pgCountUsed());

  int rc;
  page* pPage;

  //check pgFree list
  if (pgFree != -1)
  {
    pPage = pgIndex(pgFree);
    pgFree = pPage->nextId;
  }
  //if no free page,
  //select a page,
  //swap this page,
  //detach a page from pgRoot list
  else
  {
    //select special page to swap
    int pgId = pgSelect();
    assert(pgId != -1 && pgId != pgLocked);
    pPage = pgIndex(pgId);
    rc = pgSwap(pPage);
    assert(rc == 0);
    rc = pgDetach(pPage->id, &pgRoot);
    assert(rc == 0);
  }

  //reset page
  assert(pPage->id != pgLocked);
  pgReset(pPage);
  pPage->addr = 0;

  return pPage;
}
int PageMng::pgSwap(page* target)
{
  int rc = 0;
  int n;
  //nerver swap a lockeed page
  assert(target->id != pgLocked);
  if (pgFile)
  {
    rc = pgFile->Write(target->addr, target->data, target->size, &n);
    assert(n == pgDataSize);
  }
  return rc;
}
int PageMng::pgLoad(int addr, page* target, int mode)
{
  int rc = 0;
  int n;
  //nerver reload a locked page
  assert(target->id != pgLocked);
  //data  |   |         |
  //      |   ^-addr
  //      ^-pg addr
  target->addr = addr - (addr % pgDataSize);
  if (mode && pgFile)
  {
    rc = pgFile->Read(target->addr, target->data, target->size, &n);
    assert(n == pgDataSize);
  }
  return 0;
}
#if (PG_SELECT_MODE == 1)
int PageMng::pgSelect()
{
  int i;
  page *pPage;

  //select last page in data list
  i = pgRoot;
  assert(i >= 0 && i < pgCount);
  pPage = pgIndex(i);

  //not select locked page to swap
  if (i == pgLocked)
  {
    i = pPage->nextId;
  }

  return i;
}
#else
int PageMng::pgSelect()
{
  int i;
  int prev;
  page *pPage;

  //select last page in data list
  prev = -1;
  i = pgRoot;
  assert(i >= 0 && i < pgCount);
  for (;;)
  {
    pPage = pgIndex(i);
    if (pPage->nextId == -1)
      break;
    prev = i;
    i = pPage->nextId;
  }

  //not select locked page to swap
  if (i == pgLocked)
    i = prev;

  return i;
}
#endif  //PG_SELECT_MODE

int PageMng::pgDetach(int pgId, int* pgList)
{
  int rc = 0;
  int i;
  int prev;
  page* pPage;

  //detach from list
  i = *pgList;
  prev = -1;
  for (;;) {
    pPage = pgIndex(i);

    //if found
    if (pPage->id == pgId)
      break;

    prev = i;
    i = pPage->nextId;

    //if not found
    if (i == -1)
    {
      assert(0);
      rc = -1;
      break;
    }
  }

  //if page != root page
  if (prev != -1)
  {
    pgIndex(prev)->nextId = pPage->nextId;
  }
  else
  {
    *pgList = pPage->nextId;
  }

  return rc;
}
#if (PG_SELECT_MODE == 1)
int PageMng::pgAttach(int pgId, int* pgList)
{
  page* pTarget;
  int i;
  int prev;
  page* pPage = 0;

  //find position to insert
  pTarget = pgIndex(pgId);
  for (i = pgRoot, prev = -1; i != -1;)
  {
    pPage = pgIndex(i);
    if (pTarget->addr > pPage->addr)
      break;

    prev = i;
    i = pPage->nextId;
  }
  assert((pgRoot == -1) || pPage);

  //insert
  if (prev == -1)
  {
    //begin
    assert((pgRoot == -1) || pTarget->addr > pPage->addr);
    assert((pgRoot == -1) || pPage->id == pgRoot);
    pTarget->nextId = pgRoot;
    pgRoot = pTarget->id;
  }
  else if (i == -1)
  {
    //end pos
    assert(pPage->addr > pTarget->addr);
    pPage->nextId = pTarget->id;
    assert(pTarget->id == -1);
  }
  else
  {
    //mid
    assert(pTarget->addr > pPage->addr);
    pTarget->nextId = pPage->id;
    pPage = pgIndex(prev);
    assert(pPage->addr > pTarget->addr);
    pPage->nextId = pTarget->id;
  }
  return 0;
}
#else
int PageMng::pgAttach(int pgId, int* pgList)
{
  page* pPage;
  //add to list
  pPage = pgIndex(pgId);
  pPage->nextId = *pgList;
  *pgList = pPage->id;
  return 0;
}
#endif  //PG_SELECT_MODE
int PageMng::pgListSize(int pgId)
{
  int n = 0;
  for (;;)
  {
    if (pgId == -1)
      break;
    n++;
    pgId = pgIndex(pgId)->nextId;
  }
  return n;
}
int PageMng::pgCountUsed()
{
  return (pgListSize(pgRoot) + pgListSize(pgFree));
}
int PageMng::pgSetup()
{
  page *pPage;

  //build page data list
  pgRoot = -1;

  //build page free list
  pgFree = -1;
  for (int i = pgCount - 1; i >= 0; i--)
  {
    pPage = pgIndex(i);

    pPage->id = i;
    pgReset(pPage);
    pPage->size = pgDataSize;
    pPage->nextId = pgFree;
    pPage->addr = 0;

    pgFree = i;
  }

  return 0;
}
int PageMng::pgSwapAll()
{
  int rc;
  page *pPage;
  int i;
  //swap all pages
  i = pgRoot;
  for (;;)
  {
    if (i == -1)
      break;
    pPage = pgIndex(i);
    rc = pgSwap(pPage);
    assert(rc == 0);
    i = pPage->nextId;
  }
  return 0;
}

int PageMng::initialize(char* buffer, int bufferSize, int blockSize, FileIF* hFile)
{
  int n;
  int rc;

  //init private members
  pgDataSize = blockSize;
  pgFile = hFile;

  n = bufferSize / pgSize();
  assert(n > 0);
  mBuff = buffer;
  mBuffSize = bufferSize;
  pgCount = n;

  //setup free list, data list
  if (n > 0)
    rc = pgSetup();
  else
    rc = -1;

  //init pgLocked
  pgLocked = -1;

  return rc;
}
int PageMng::Init(char* buff, int size)
{
  return initialize(buff, size, PageDataSize, 0);
}
int PageMng::Init(PageMngParams* p)
{
  return initialize(p->buffer, p->bufferSize, p->pgDataSize, p->hFile);
}
int PageMng::Final()
{
  int rc;
  //swap all data to file
  rc = pgSwapAll();
  assert(rc == 0);
  //reset buffer
  pgCount = 0;
  mBuff = 0;
  mBuffSize = 0;
  //reset page lists
  pgRoot = -1;
  pgFree = -1;
  return rc;
}
int PageMng::Reset(char* buff, int size)
{
  int n;
  int rc;

  //if not finalized, swap all pages
  if (pgCount > 0)
  {
    rc = pgSwapAll();
    assert(rc == 0);
  }

  //update buffer
  if (buff != 0)
  {
    n = size / pgSize();
    assert(n > 0);
    mBuff = buff;
    mBuffSize = size;
    pgCount = n;
  }

  //re build page lists
  if (n > 0)
    rc = pgSetup();
  else
    rc = -1;

  return rc;
}
//case Allocate() >>Read()/Write(): not swap locked page
int PageMng::Locate(int addr, PPAGE* out, int mode)
{
  assert(out);
  int rc = 0;
  page *pPage;

  //find page in buffer
  //if not found, get free page, load data to this page
  pPage = pgFind(addr);
  if (pPage == 0)
  {
    //get a free page
    pPage = pgAllocate();
    assert(pPage->id != pgLocked);
    assert(pPage != 0);
    //load data
    rc = pgLoad(addr, pPage, mode);
    assert(rc == 0);
    //attach to data list
    rc = pgAttach(pPage->id, &pgRoot);
    assert(rc == 0);
  }

  *out = pPage;
  return rc;
}
int PageMng::Push(int addr, char* data, int size, page* target, int* count)
{
  int remain;
  int d;

  //                        |<-remain ->|
  //          |<-d        ->|<-size->|  |
  //page data |             |           |
  //          |             ^-addr
  //          ^-pg addr
  d = addr - target->addr;
  remain = target->size - d;
  assert(size <= remain);

  if (size > remain)
  {
    size = remain;
    assert(0);
  }
  memcpy(target->data + d, data, size);

  *count = size;
  target->nWrite += size;

  return 0;
}
int PageMng::Pop(int addr, char* buff, int size, page* target, int* count)
{
  int rc = 0;
  int avaiable;
  int d;

  //          |   |<-size->|  |
  //page data |   |           |
  //          |   ^-addr
  //          ^-pg addr
  d = addr - target->addr;
  assert(d >= 0);
  avaiable = target->size - d;
  assert(avaiable >= size);
  if (size > avaiable)
  {
    size = avaiable;
    assert(0);
  }
  memcpy(buff, target->data + d, size);

  *count = size;
  target->nRead += size;

  return rc;
}
int PageMng::Size()
{
  return (pgCount * pgDataSize);
}
//not lock if buffer has one page
int PageMng::Lock(page* target)
{
  int rc = -1;
  if (pgCount > 1)
  {
    pgLocked = target->id;
    rc = 0;
  }
  return rc;
}
int PageMng::Unlock(page* target)
{
  int rc = -1;
  if (pgLocked != -1)
  {
    assert(pgLocked == target->id);
    pgLocked = -1;
    rc = 0;
  }
  return rc;
}
//VMem
int VMem::setZ(int addr, char* data, int size)
{
  int rc = 0;
  page* pPage;
  int space;
  int n;
  int d;
  //1. locate end page; check page space
  //2. if enough space
  //3. else locate next page, jump to (1)
  while (size > 0)
  {
    rc = mPageMng.Locate(addr, &pPage, 0);
    assert(rc == 0);
    d = addr - pPage->addr;
    space = pPage->size - d;
    assert(space > 0);
    n = MY_MIN(space, size);

    //        |<-space->|
    //buff  | |         |
    //      | ^-addr
    //      ^-pg addr
    rc = mPageMng.Push(addr, data, n, pPage, &n);
    assert(rc == 0);
    assert(n == size || n == space);

    addr += n;
    data += n;
    size -= n;
  }
  return rc;
}
int VMem::getZ(int addr, char* buff, int size)
{
  int rc;
  page* pPage;
  int avaiable;
  int n;
  int d;
  //1. locate start page; check page space
  //2. if enough data
  //3. else locate next page, free start page, jump to (1)
  while (size > 0)
  {
    rc = mPageMng.Locate(addr, &pPage, 1);
    assert(rc == 0);
    d = addr - pPage->addr;
    avaiable = pPage->size - d;
    n = MY_MIN(avaiable, size);

    //        |<-avaiable->|
    //data  | |            |
    //      | ^-addr
    //      ^-pg addr
    rc = mPageMng.Pop(addr, buff, n, pPage, &n);
    assert(n == size || n == avaiable);

    buff += n;
    addr += n; //addr of the next page
    size -= n;
  }
  return rc;
}
int VMem::locateZ(int addr, PCHAR* pData, int *count)
{
  int rc;
  page* pPage;
  int d;
  rc = mPageMng.Locate(addr, &pPage, 1);
  assert(rc == 0);
  d = addr - pPage->addr;
  *count = pPage->size - d;
  *pData = pPage->data + d;
  rc = mPageMng.Lock(pPage);
  assert(rc == 0);
  return rc;
}
int VMem::commitZ(int addr)
{
  int rc;
  page* pPage;
  rc = mPageMng.Locate(addr, &pPage, 0);
  assert(rc == 0);
  rc = mPageMng.Unlock(pPage);
  assert(rc == 0);
  return rc;
}
int VMem::Init(char* buff, int size)
{
  int rc;
  mSize = VMemSize;
  rc = mPageMng.Init(buff, size);
  return rc;
}
int VMem::Init(VMemParams* p)
{
  int rc;
  mSize = p->vMemSize;
  rc = mPageMng.Init(p);
  assert(rc == 0);
  return rc;
}
int VMem::Final()
{
  int rc;
  mSize = 0;
  rc = mPageMng.Final();
  return rc;
}
int VMem::Reset(char* buff, int size)
{
  int rc;
  rc = mPageMng.Reset(buff, size);
  return rc;
}
int VMem::Size()
{
  return mSize;
}
int VMem::Read(int addr, char* buff, int size, int *count)
{
  int rc = -1;
  int avaiable;

  if (0 <= addr && addr < mSize)
  {
    //      |<-data size    ->|
    //          |<-r size->|
    //data  |   |             |
    //          ^-addr
    avaiable = mSize - addr;
    if (size > avaiable)
      size = avaiable;
    rc = getZ(addr, buff, size);

    if (rc == 0)
      *count = size;
    else
      *count = 0;
  }

  return rc;
}
int VMem::Write(int addr, char* data, int size, int *count)
{
  int rc = -1;
  int avaiable;

  if (0 <= addr && addr < mSize)
  {
    //      |<-buff size    ->|
    //          |<-w size->|
    //buff  |   |             |
    //          ^-addr
    avaiable = mSize - addr;
    if (size > avaiable)
      size = avaiable;
    rc = setZ(addr, data, size);

    if (rc == 0)
      *count = size;
    else
      *count = 0;
  }

  return rc;
}

//locate data in page cache
//__in addr: zero base index (by byte)
//__out pData: point to data content
//__out count: hold the data size
//like read but not copy data to out buffer
int VMem::Locate(int addr, PCHAR* pData, int *count)
{
  int rc = -1;
  int n;

  if (0 <= addr && addr < mSize)
  {
    //      |<-data size    ->|
    //          |<-count    ->|
    //data  |   |             |
    //          ^-addr
    rc = locateZ(addr, pData, &n);
    if (rc == 0)
      *count = n;
    else
      *count = 0;
  }

  return rc;
}
//should call right after Locate()
int VMem::Commit(int addr)
{
  int rc = -1;
  if (0 <= addr && addr < mSize)
    rc = commitZ(addr);
  return rc;
}
