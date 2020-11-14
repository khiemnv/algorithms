#include <crtdbg.h>
#define assert(x) _ASSERT(x)
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>

#include "MemIF.h"
#include "qsMem.h"
#include "traverser.h"

#define ALIGNBY4(size)        (((size) + 3) & (~3))


//-----------------extract-------------
struct ExtractData
{
  enum {
    curPathSize = MAX_PATH,   //in TCHAR
  };
  int   topId;
  TCHAR curPath[curPathSize];
  node  nd;
  MemMngIF* pStack;

  int Init(MemMngIF *pStack);
};
int ExtractData::Init(MemMngIF *pStack)
{
  this->pStack = pStack;
  return 0;
}

int extractor::Init(tree* treeHandle, MemMngIF* stackHandle, ExtractData** out)
{
  int rc = 0;
  ExtractData* &pED = *out;
  pED = new ExtractData;
  if (pED) {
    mStackHandle = 0;
    pED->Init(stackHandle);
    mExtractData = pED;
    mTreeHandle = treeHandle;
  }
  else {
    assert(0);
    rc = -1;
  }
  return rc;
}
int extractor::Init(tree* treeHandle, char* buff, int size, ExtractData** out)
{
  int rc = 0;
  ExtractData* &pED = *out;
  QSMemParams qsp;
  QSMem* pQS;
  int stackSize = GetStackSize();
  if (size >= stackSize) {
    //(0) crt stack
    qsp.mBuffer = buff;
    qsp.mBufferSize = size;
    qsp.mFlags = qsp.modeStack | qsp.addrAbsolute;
    pQS = new QSMem;
    pQS->Init(&qsp);
    mStackHandle = pQS;
    //(1) crt extract data
    mExtractData = new ExtractData;
    mExtractData->Init(mStackHandle);
    pED = mExtractData;
    //(2) save tree handle
    mTreeHandle = treeHandle;
  }
  else {
    assert(0);
    rc = stackSize;
  }
  return rc;
}
int extractor::ExtractBegin(ExtractData*)
{
  return 0;
}
int extractor::ExtractFirst(ExtractData* pParams)
{
  int rc = 0;
  ExtractData &ep = *pParams;
  tree &tree = *mTreeHandle;
  //init extract params
  ep.topId = tree.RootId;
  ep.curPath[0] = 0;
  //init
  rc = tree.DeQueue(&ep.nd);
  return rc;
}

int extractor::ExtractNext(ExtractData* pParams)
{
  int rc = 0;
  ExtractData &ep = *pParams;
  tree &tree = *mTreeHandle;
  node tempNode;
  //begin loop
  //for(; rc!= -1;)
  {
    if (ep.nd.flags & node::isNode)
    {
      //if (ep.nd.id == 0) {
      //  //root folder
      //  LOG_Write(TEXT("%s\n"), ep.nd.data);
      //}
      //else {
      //  //create folder
      //  LOG_Write(TEXT("%s\\%s\n"), ep.curPath, ep.nd.data);
      //}
      //push folder
      ep.topId = ep.nd.id;
      assert(ep.topId != -1);
      pathPush(ep.curPath, ep.curPathSize, ep.nd.data);
      rc = Push(ep.pStack, &ep.nd);
      assert(rc == 0);
    }
    else
    {
      assert(ep.nd.flags & node::isLeaf);
      ////create file
      //LOG_Write(TEXT("%s\\%s\n"), ep.curPath, ep.nd.data);
    }
    rc = tree.DeQueue(&ep.nd);

    //if end node
    if (ep.nd.parentId == tree.RootId) {
      assert(_tcscmp(ep.nd.data, zEndNode) == 0);
      //break;
      rc = -1;
    }
    else {
      assert(rc == 0);
      for (; ep.nd.parentId != ep.topId;) {
        rc = Pop(ep.pStack, &tempNode);
        pathPop(ep.curPath);
        ep.topId = tempNode.parentId;
        assert(ep.topId != -1);
      }
    }
  }

  return rc;
}
int extractor::ExtractEnd() { return 0; }
int extractor::Final()
{
  if (mExtractData)
    delete mExtractData;
  if (mStackHandle)
    delete ((QSMem*)mStackHandle);
  return 0;
}
int extractor::GetStackSize()
{
  return sizeof(node)* treeMaxDepth;
}
//-----------------nodeMng-------------
//return node id
//if queue full, return -1
int nodeMng::EnQueue(MemMngIF* pQueue, int nodeId, int parentId, TCHAR* zName, int isDir)
{
  int rc = 0;
  node tmpNode;
  rc = nodeFormat(&tmpNode, nodeId, zName, isDir);
  if (remain(pQueue) >= rc) {
    tmpNode.parentId = parentId;
    rc = push(pQueue, &tmpNode);
    assert(rc == nodeAlignSize(&tmpNode));
    rc = tmpNode.id;
  }
  else {
    assert(0);
    rc = -1;
  }
  return rc;
}
//if success func return 0
//else if error func return error code
int nodeMng::DeQueue(MemMngIF* pQueue, node* pNode)
{
  int size = pop(pQueue, pNode);
  if (size)
    return 0;
  else
    return -1;
}
//if success func return 0
//else if error func return error code
int nodeMng::Pop(MemMngIF* pStack, node* pNode)
{
  int size = pop(pStack, pNode);
  if (size)
    return 0;
  else
    return -1;
}
int nodeMng::Push(MemMngIF* pStack, node* pNode)
{
  int size = push(pStack, pNode);
  if (size)
    return 0;
  else
    return -1;
}
//if success func return size of node
//else func return 0
int nodeMng::nodeAlignSize(nodeHdr* pHdr)
{
  return ALIGNBY4(pHdr->size + sizeof(nodeHdr));
}
int nodeMng::nodeFormat(node* pNode, int id, TCHAR *zName, int isDir)
{
  pNode->id = id;

  //set flags
  if (isDir)
    pNode->flags = node::isNode;
  else
    pNode->flags = node::isLeaf;

  //set data
  int n;
  for (n = 0; zName[n] != TEXT('\0'); n++)
  {
    pNode->data[n] = zName[n];
  }
  //data contain '\0' terminate
  pNode->data[n] = 0;
  n++;

  //set data size in byte
  n *= sizeof(TCHAR);
  pNode->size = n;

  //align size
  n = ALIGNBY4(n + sizeof(nodeHdr));
  assert(n == nodeAlignSize(pNode));
  return n;
}
int nodeMng::pop(MemMngIF* pmm, node* pNode)
{
  int n = 0;
  MemMngIF &mm = *(MemMngIF*)pmm;
  nodeHdr hdr;
  mm.Peek(0, (char*)&hdr, sizeof(nodeHdr), &n);
  if (n == 0) {
    //do nothing
  }
  else {
    assert(n == sizeof(nodeHdr));
    n += hdr.size;
    if (pNode) {
      mm.Peek(0, (char*)pNode, n, &n);
    }
    n = ALIGNBY4(n);
    assert(n == nodeAlignSize(&hdr));
    mm.Discard(n, &n);
  }
  return n;
}
//if success func return size of node
//else if error func return 0
int nodeMng::push(MemMngIF* pmm, node* pNode)
{
  MemMngIF &mm = *(MemMngIF*)pmm;
  int n = nodeAlignSize(pNode);
  if (mm.Remain() >= n)
    mm.Write((char*)pNode, n, &n);
  else
    n = 0;
  return n;
}
//get remain size of rm
int nodeMng::remain(MemMngIF* pmm)
{
  MemMngIF &mm = *(MemMngIF*)pmm;
  return mm.Remain();
}
//-----------------tree----------------
int tree::getId()
{
  return mNodeCount++;
}

int tree::EnQueue(int parentId, TCHAR* zName, int isDir)
{
  return nodeMng::EnQueue(mQueueHandle, getId(), parentId, zName, isDir);
}
int tree::DeQueue(node* pNode)
{
  return nodeMng::DeQueue(mQueueHandle, pNode);
}
//-----------------file----------------
int FileMng::ImportOne(TCHAR* zName, TCHAR* zAlias, TCHAR* srcDir, TCHAR* desDir, int isDir)
{
  int rc = 0;
  TCHAR srcPath[MAX_PATH];
  TCHAR desPath[MAX_PATH];
  int len = _tcslen(zAlias);
  //zFile = <srcDir>\\<path>
  //                ^-len
  assert(_tcsncmp(zName, zAlias, len) == 0);
  _stprintf_s(desPath, MAX_PATH, TEXT("%s\\%s"), desDir, zName + len + 1);
  _stprintf_s(srcPath, MAX_PATH, TEXT("%s\\%s"), srcDir, zName + len + 1);
  if (isDir) {
    exportDir(desPath);
  }
  else {
    exportFile(srcPath, desPath);
  }
  return rc;
}
int FileMng::ExportOne(TCHAR* zFile, TCHAR* srcDir, TCHAR* desDir, int isDir)
{
  int rc = 0;
  TCHAR buff[MAX_PATH];
  int len = _tcslen(srcDir);
  //zFile = <srcDir>\\<path>
  //                ^-len
  assert(_tcsncmp(zFile, srcDir, len) == 0);
  _stprintf_s(buff, MAX_PATH, TEXT("%s\\%s"), desDir, zFile + len + 1);
  if (isDir) {
    exportDir(buff);
  }
  else {
    exportFile(zFile, buff);
  }
  return rc;
}
int FileMng::exportDir(TCHAR* zFile)
{
  int rc = 0;
  rc = CreateDirectory(zFile, 0);
  if (rc) {
    rc = 0;
  }
  else {
    rc = GetLastError();
    if (rc == ERROR_ALREADY_EXISTS) {
#if (AUTO_CLEAR)
      //clear all file in foder
#endif
    }
  }
  return rc;
}
int FileMng::copyFirst(CopyData*p)
{
  int rc = 0;
  p->flags = p->initialized;
  p->fin = CreateFile(
    p->zFin,
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    0,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
    0
    );
  if (p->fin != INVALID_HANDLE_VALUE) {
    //get size of fin
    p->size_low = GetFileSize(p->fin, &(p->size_high));
    //create fout
    p->fout = CreateFile(
      p->zFout,
      GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      0,
      CREATE_NEW,
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
      0
      );
    if (p->fout == INVALID_HANDLE_VALUE) {
      rc = GetLastError();
      CloseHandle(p->fin);
    }
  }
  else {
    rc = GetLastError();
  }
  return rc;
}
//if success return 0
int FileMng::copyNext(CopyData*p)
{
  DWORD n = 0;
  int rc;
  for (;;) {
    DWORD temp;
    rc = ReadFile(p->fin, p->buff, p->pgSize, &temp, 0);
    if (rc) {
      n += temp;
      if (temp == 0 || n == p->blockSize) {
        break;
      }
    }
    else {
      break;
    }
  }
  if (rc) {
    if (n == 0) {
      copyClose(p);
      rc = -1;
    }
    else if (n == p->blockSize) {
      rc = WriteFile(p->fout, p->buff, n, &n, 0);
      if (rc) {
        p->copied += n;
        rc = 0;
      }
      else {
        rc = GetLastError();
      }
    }
    else if (n > 0) {
      //copy last path
      LARGE_INTEGER mapSize;
      LARGE_INTEGER mapBegin;
      HANDLE hMap;
      void* mapAddr;
      rc = GetFileSizeEx(p->fin, &mapSize);
      mapBegin.QuadPart = p->copied;
      hMap = CreateFileMapping(
        p->fout,
        NULL,
        PAGE_READWRITE,
        mapSize.HighPart, mapSize.LowPart,
        NULL
        );
      if (hMap != INVALID_HANDLE_VALUE) {
        mapAddr = MapViewOfFile(
          hMap,
          FILE_MAP_READ | FILE_MAP_WRITE,
          mapBegin.HighPart,
          mapBegin.LowPart,
          n
          );
        if (mapAddr) {
          memcpy(mapAddr, p->buff, n);
          p->copied += n;
          UnmapViewOfFile(mapAddr);
          rc = 0;
        }
        else {
          rc = GetLastError();
        }
        CloseHandle(hMap);
      }
      else {
        rc = GetLastError();
        CloseHandle(hMap);
      }
    }
  }
  else {
    rc = GetLastError();
  }
  return rc;
}
int FileMng::copyClose(CopyData*p)
{
  CloseHandle(p->fin);
  CloseHandle(p->fout);
  return 0;
}
//return 0
int FileMng::exportFile(TCHAR* srcFile, TCHAR* desFile)
{
  CopyData cd = { 0 };
  cd.zFin = srcFile;
  cd.zFout = desFile;
#if (ONE_COPY_DATA_BUFF)
  cd.buff = mCopyDataBuffer;
#endif
  DWORD elapsed = GetTickCount();
  for (int rc = copyFirst(&cd); rc == 0;) {
    //do something
    rc = copyNext(&cd);
  }
  elapsed = GetTickCount() - elapsed;
  return 0;
}
//-----------------path----------------
//PARAMS
//  __in count: size of buff in TCHAR
int PathMng::pathPush(TCHAR* buff, int count, TCHAR* zElement)
{
  int len = _tcslen(buff);
  if (len > 0) {
    buff[len] = TEXT('\\');
    len++;
  }
  len += _stprintf_s(buff + len, count - len, TEXT("%s"), zElement);
  return 0;
}
int PathMng::pathPop(TCHAR* buff)
{
  int len = _tcslen(buff);
  //buff = "C:\\folder"
  //           ^-len
  while (buff[len] != TEXT('\\')) {
    len--;
  }
  assert(len >= 0);
  buff[len] = 0;
  return 0;
}
//-----------------context-------------
struct TraversalContext {
  TCHAR findPath[MAX_PATH];
  WIN32_FIND_DATA fd;
  HANDLE hFind;
  int curId;
  int curLevel;
};

int context::contextPush(void *pStack, TraversalContext* in)
{
  MemMngIF &mm = *(MemMngIF*)pStack;
  int n = 0;
  mm.Write((char*)in, sizeof(TraversalContext), &n);
  assert(n == sizeof(TraversalContext));
  return n;
}
int context::contextPop(void *pStack, TraversalContext* out)
{
  MemMngIF &mm = *(MemMngIF*)pStack;
  int n = 0;
  if (mm.Count() > 0) {
    mm.Read((char*)out, sizeof(TraversalContext), &n);
    assert(n == sizeof(TraversalContext));
  }
  return n;
}
//-----------------traverser---------------------

struct TraverseData {
  typedef int(*pCallback)(int, void*);
  enum {
    initialized = 1,
    traversing,
    completed,

    maxDepth = 16,
#if (REMOVE_DIR_LEVEL)
    error_unknown = -1,
    error_completed_all = 1,
    error_completed_one_dir = 2,
#endif
  };
  int flags;
  int lastId;
#if (REMOVE_DIR_LEVEL)
  pCallback rmDir;
  TCHAR lastCompletedPath[MAX_PATH];
  int lastError;
#endif
  TraversalContext context;
  MemMngIF* stack;

  int Init(MemMngIF *pStack);
};

int TraverseData::Init(MemMngIF* pStack) {
  stack = pStack;
  return 0;
}

//if success func return 0
//else if buffer not enough, the func return error code
int Traverser::Init(tree* treeHandle, MemMngIF *stackHandle, TraverseData** out)
{
  int rc = 0;
  TraverseData* &pTD = *out;
  pTD = new TraverseData;
  if (pTD) {
    mStackHandle = 0;
    pTD->Init(stackHandle);
    mTraverseData = pTD;
    mTreeHandle = treeHandle;
  }
  else {
    assert(0);
    rc = -1;
  }
  return rc;
}
int Traverser::Init(tree* treeHandle, char* buff, int size, TraverseData** out)
{
  TraverseData* &pTD = *out;
  QSMemParams qsp;
  QSMem* pQS;
  int stackSize = GetStackSize();
  if (size >= stackSize) {
    //(0) crt stack
    qsp.mBuffer = buff;
    qsp.mBufferSize = size;
    qsp.mFlags = qsp.modeStack | qsp.addrAbsolute;
    pQS = new QSMem;
    pQS->Init(&qsp);
    mStackHandle = pQS;
    //(1) crt traverse data
    mTraverseData = new TraverseData;
    mTraverseData->Init(mStackHandle);
    pTD = mTraverseData;
    //(2) save tree handle
    mTreeHandle = treeHandle;
  }
  else {
    return stackSize;
  }
  return 0;
}
int Traverser::TraverseBegin(TraverseData* in_out, TCHAR *zAlias)
{
  TraverseData &td = *(in_out);
  tree &tree = *mTreeHandle;
  int nodeId;
  //prepare before start
  nodeId = tree.EnQueue(tree.RootId, zAlias, TRUE);
  //init traverseData.nodeId
  td.context.curId = nodeId;
  return 0;
}
int Traverser::TraverseEnd()
{
  tree &tree = *mTreeHandle;
  tree.EnQueue(tree.RootId, zEndNode, TRUE);
  return 0;
}
int Traverser::Final()
{
  if (mTraverseData)
    delete mTraverseData;
  if (mStackHandle)
    delete ((QSMem*)mStackHandle);
  return 0;
}
int Traverser::GetStackSize()
{
  return sizeof(TraversalContext)* (mTraverseData->maxDepth);
}
//get first file/folder info
//if success return 0
int Traverser::TraverseFirst(TraverseData* p, TCHAR* zSrcPath)
{
  int rc;
  TraverseData &td = *(p);
  tree &tree = *mTreeHandle;
  //init level and findpath
  td.context.curLevel = 0;
  td.context.findPath[0] = 0;
  assert(td.stack);
  pathPush(td.context.findPath, MAX_PATH, zSrcPath);
  //start traversal
  TraversalContext &context = p->context;
  pathPush(context.findPath, MAX_PATH, TEXT("*.*"));
  context.hFind = FindFirstFile(context.findPath, &context.fd);
  if (context.hFind != INVALID_HANDLE_VALUE) {
    //return rc = 0
    rc = 0;
  }
  else {
    rc = GetLastError();
  }
#if (REMOVE_DIR_LEVEL)
  p->lastError = rc ? p->error_unknown : 0;
#endif
  return rc;
}
//filter available files
//return 0 if success
int Traverser::TraverseFilter(TraverseData*p)
{
  int rc = 0;
  DWORD attrib = p->context.fd.dwFileAttributes;
  TCHAR* zName = p->context.fd.cFileName;
  if (attrib & (FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_SYSTEM))
    rc = attrib;
  else if (_tcscmp(zName, TEXT(".")) == 0 || _tcscmp(zName, TEXT("..")) == 0)
    rc = -1;
  return rc;
}
//push file/folder to queue node
//if success func return 0
//else if error fuc return -1
int Traverser::TraversePush(TraverseData *p)
{
  tree &tree = *mTreeHandle;
  assert(TraverseFilter(p) == 0);
  TraversalContext &context = p->context;
  int rc = tree.EnQueue(context.curId,
    context.fd.cFileName,
    context.fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
  if (rc != -1) {
    p->lastId = rc;
    rc = 0;
  }
  return rc;
}
//push prev result to queue
// and get next file/folder
//if success return 0
int Traverser::TraverseNext(TraverseData* p)
{
  int rc = 0;
  int level;
  enum {
    goDown = 0x01,
    goUp = 0x02,
    getSibling = 0x10,
  };
  int flags = 0;
  TraversalContext &context = p->context;

#if (REMOVE_DIR_LEVEL)
  assert(p->lastError != p->error_unknown);
  if (p->lastError == p->error_completed_one_dir) {
    flags |= getSibling;
  }
  else
#endif
    //enqueue prev result
  {
    if (TraverseFilter(p))
    {
      //do nothing
      flags |= getSibling;
    }
    //if folder
    else if (context.fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
#if (AUTO_ENQUEUE)
      //enqueue
      p->lastId = EnQueue(context.curId, context.fd.cFileName, TRUE);
#endif
      flags |= goDown;
    }
    //if file
    else{
#if (AUTO_ENQUEUE)
      //enqueue
      p->lastId = EnQueue(context.curId, context.fd.cFileName, FALSE);
#endif
      flags |= getSibling;
    }
  }

  //go deeper
  if (flags & goDown) {
#if (REMOVE_DIR_LEVEL)
    p->lastCompletedPath[0] = 0;
#endif
    level = context.curLevel + 1;
    //save parent context before go to child
    rc = contextPush(p->stack, &context);
    assert(rc == sizeof(TraversalContext));
    //prepare before go to child
    //+ make findPath
    //  + remove *.*
    //  + append new element
    pathPop(context.findPath);
    pathPush(context.findPath, MAX_PATH, context.fd.cFileName);
    //+ update curContext
    context.curId = p->lastId;
    context.curLevel = level;
    //find first file/folder in child
    pathPush(context.findPath, MAX_PATH, TEXT("*.*"));
    context.hFind = FindFirstFile(context.findPath, &context.fd);
    if (context.hFind != INVALID_HANDLE_VALUE) {
      rc = 0;
    }
    else {
      assert(0);
      rc = GetLastError();
    }
  }

  for (;;)
  {
    //get sibling
    if (flags & getSibling) {
      rc = FindNextFile(context.hFind, &context.fd);
      if (rc) {
        //success, return sibling
        rc = 0;
      }
      else {
        //if no more file
        rc = GetLastError();
        assert(rc == ERROR_NO_MORE_FILES);
        FindClose(context.hFind);
      }
    }

    //if success return got sibling
    //else if (error or no more file) goUp
    if (rc == 0) {
      break;
    }
    else {
      assert(flags & getSibling);
      flags |= goUp;
    }

    //goUp
    if (flags & goUp) {
#if (REMOVE_DIR_LEVEL)
      _tcscpy_s(p->lastCompletedPath, MAX_PATH, context.findPath);
#endif
      //pop out context
      //if (is root) stop traversal
      //else get parent's sibling
      rc = contextPop(p->stack, &context);
#if (REMOVE_DIR_LEVEL)
      if (rc == 0) {
        rc = p->error_completed_all;
      }
      else {
        rc = p->error_completed_one_dir;
      }
      break;
#endif
      if (rc == 0) {
        rc = -1;
        break;
      }
    }
  }
#if (REMOVE_DIR_LEVEL)
  p->lastError = rc;
#endif

  return rc;
}

//-----------------exporter------------
