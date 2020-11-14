#include <crtdbg.h>
#define assert(x) _ASSERT(x)

#include "tree.h"
#include "ringmem.h"

#include <stdio.h>
#include <Windows.h>

#define ALIGNBY4(size)        (((size) + 3) & (~3))
#define zEndNode              TEXT(".")

//[LOG]
#define LOG_Write(...) _ftprintf(fLog, __VA_ARGS__);


FileTree::FileTree(FileTreeParams* p)
{
  mMode = 0;
  mQueueFS = 0;
  mQueueVMem = 0;
  mQueueRm = 0;
  mQueueItemCount = 0;

  mStackRm = 0;
  mzRootPath = 0;
#if (TRUNCATE_SWAP_FILE)
  mzFileSwap = 0;
  mFileSwapSize = 0;
#endif
  //[log]
  fLog = 0;

  Init(p);
}
FileTree::~FileTree()
{
  Final();
}

int FileTree::getId()
{
  return mQueueItemCount++;
}
node* FileTree::queueAllocate()
{
  RingMem &rm = *(RingMem*)mQueueRm;
  int n = rm.Remain();
  char* pNode = 0;

  if (n >= sizeof(node)) {
    rm.Allocate(&pNode, sizeof(node), &n);
    assert(n == sizeof(node));
  }
  return (node*)pNode;
}
int FileTree::queueCommit(int size)
{
  RingMem &rm = *(RingMem*)mQueueRm;
  int n;
  rm.Commit(size, &n);
  assert(n == size);
  return 0;
}
int FileTree::nodeFormat(node* pNode, TCHAR *zName, int isDir)
{
  pNode->id = getId();

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
  n*=sizeof(TCHAR);
  pNode->size = n;

  //align size
  n = ALIGNBY4(n + sizeof(nodeHdr));
  assert(n == nodeAlignSize(pNode));
  return n;
}

int FileTree::Init(FileTreeParams* p)
{
  mMode = p->mode;
  //(1)init queue
  if (p->mode & p->queueModeVmem) {
#if (TRUNCATE_SWAP_FILE)
    //(.0)int mBinFile
    mzFileSwap = new char[MAX_PATH];
    strcpy_s(mzFileSwap, MAX_PATH, p->zSwapFile);
#endif
    //(.1)init file swap
    FileSwapParams fsp;
    fsp.zName = p->zSwapFile;      //"d:\\tmp\\fs.txt
    fsp.mode = fsp.ModeNoBuffer;
    FileSwap *fs = new FileSwap(&fsp);
    if (p->mode & p->exportMode) {
      fs->Truncate(0);
    }
    mQueueFS = fs;
    //(.2)init vmem for queue
    VMemParams vmp;
    vmp.vMemSize = p->vMemSize;   //1<<30
    vmp.buffer = p->queueBuffer;
    vmp.bufferSize = p->queueBufferSize;
    vmp.pgDataSize = p->pgSize;   //512
    vmp.hFile = fs;
    VMem *vmem = new VMem();
    vmem->Init(&vmp);
    mQueueVMem = vmem;
    //(.3)
    RingMemParams rmp;
    rmp.hVMem = vmem;
    rmp.mode = RingMem::RingMemVMemMode;
    rmp.type = RingMem::RingMemTypeQueue;
    RingMem* queueRM = new RingMem(&rmp);
    if (p->mode & p->importMode) {
      queueRM->MapExistedContent(fs->Size());
    }
    mQueueRm = queueRM;
  }
  else {
    RingMemParams rmp;
    rmp.buffer = p->queueBuffer;
    rmp.bufferSize = p->queueBufferSize;
    rmp.mode = RingMem::RingMemAddrModeAbsolute;
    rmp.type = RingMem::RingMemTypeQueue;
    RingMem* queueRM = new RingMem(&rmp);
    mQueueRm = queueRM;
  }
  //(2) init stack
  {
    RingMemParams rmp;
    rmp.buffer = p->stackBuffer;
    rmp.bufferSize = p->stackBufferSize;
    rmp.mode = RingMem::RingMemAddrModeAbsolute;
    rmp.type = RingMem::RingMemTypeStack;
    RingMem* stackRM = new RingMem(&rmp);
    mStackRm = stackRM;
  }

  //(3) init path
  {
    mzRootPath = new TCHAR[MAX_PATH];
    TCHAR *path = (TCHAR*)p->zRoot;
    int len = _tcslen(path);
    assert(mzRootPath);
    if (mzRootPath) {
      _tcsncpy_s(mzRootPath, MAX_PATH, path, len);
    }
    for(len--; mzRootPath[len] == TEXT('\\'); len--) {
      mzRootPath[len] = 0;
    }
  }

  maxDept = p->maxDept;

  //[log]
  _tfopen_s(&fLog, p->zLogFile, TEXT("w+"));

  return 0;
}

int FileTree::Final()
{
  if (mMode & FileTreeParams::queueModeVmem)
  {
    if (mQueueVMem) {
      VMem* vm = (VMem*)mQueueVMem;
      vm->Final();
      delete vm;
      mQueueVMem = 0;
    }
    if (mQueueFS) {
      delete (FileSwap*)mQueueFS;
      mQueueFS = 0;
    }
#if (TRUNCATE_SWAP_FILE)
    if (mMode & FileTreeParams::exportMode) {
      int rc;
      rc = FileSwap::Truncate(mzFileSwap, mFileSwapSize);
      assert(rc == 0);
      LOG_Write(TEXT("truncate size %d\n"), mFileSwapSize);
    }
    if (mzFileSwap) {
      delete (char*)mzFileSwap;
      mzFileSwap = 0;
    }
#endif
  }
  if (mQueueRm) {
    RingMem &queueRm = *(RingMem*)mQueueRm;
    delete (RingMem*)mQueueRm;
    mQueueRm = 0;
  }
  if (mStackRm) {
    delete (RingMem*)mStackRm;
    mStackRm = 0;
  }
  if (mzRootPath) {
    delete (TCHAR*)mzRootPath;
    mzRootPath = 0;
  }
  //[log]
  if (fLog) {
    fclose(fLog);
    fLog = 0;
  }
  return 0;
}
//return node id
//if queue full, wait infinite
int FileTree::EnQueue(int parentId, TCHAR* zName, BOOL isDir)
{
  int rc = 0;
  node* pNode;
  if (mMode & FileTreeParams::queueModeVmem) {
    node tmpNode;
    rc = nodeFormat(&tmpNode, zName, isDir);
    assert(remain(mQueueRm) >= rc);
    tmpNode.parentId = parentId;
    rc = push(mQueueRm, &tmpNode);
    assert(rc == nodeAlignSize(&tmpNode));
#if (TRUNCATE_SWAP_FILE)
    mFileSwapSize += rc;
#endif
    rc = tmpNode.id;
  }
  else {
    for (pNode = 0; pNode == 0; Sleep(timeOut)) {
      pNode = queueAllocate();
    }
    int size = nodeFormat(pNode, zName, isDir);
    pNode->parentId = parentId;
    queueCommit(size);
#if (TRUNCATE_SWAP_FILE)
    mFileSwapSize += size;
#endif
    rc = pNode->id;
  }
  return rc;
}
//if success func return 0
//else if error func return error code
int FileTree::DeQueue(node* pNode)
{
  int size = pop(mQueueRm, pNode);
  if (size)
    return 0;
  else
    return -1;
}
int FileTree::nodeAlignSize(nodeHdr* pHdr)
{
  return ALIGNBY4(pHdr->size + sizeof(nodeHdr));
}
//if success func return size of node
//else func return 0
int FileTree::pop(void* prm, node* pNode)
{
  int n = 0;
  RingMem &rm = *(RingMem*)prm;
  nodeHdr hdr;
  for(; rm.IsEmpty(); Sleep(timeOut)) {
    if (isBuildComplete()) break;
  }
  rm.Peek(0, (char*)&hdr, sizeof(nodeHdr), &n);
  if (n == 0) {
    //do nothing
  } else {
    assert(n == sizeof(nodeHdr));
    n += hdr.size;
    if (pNode) {
      rm.Peek(0, (char*)pNode, n, &n);
    }
    n = ALIGNBY4(n);
    assert(n == nodeAlignSize(&hdr));
    rm.Discard(n, &n);
  }
  return n;
}
//get remain size of rm
int FileTree::remain(void* prm)
{
  RingMem &rm = *(RingMem*)prm;
  return rm.Remain();
}
//+ state share
int FileTree::setBuildComplete()
{
  return 0;
}
int FileTree::isBuildComplete()
{
  return 1;
}


//if success func return 0
int FileTree::Push(node* pNode)
{
  int size = push(mStackRm, pNode);
  if (size)
    return 0;
  else
    return -1;
}
//if success func return size of node
//else if error func return 0
int FileTree::push(void* prm, node* pNode)
{
  RingMem &rm = *(RingMem*)prm;
  assert(!rm.IsFull());
  //int n = ALIGNBY4(pNode->size + sizeof(nodeHdr));
  int n = nodeAlignSize(pNode);
  rm.Write((char*)pNode, n, &n);
  return n;
}
//if success func return 0
//else if error func return error code
int FileTree::Pop(node* pNode)
{
  int size = pop(mStackRm, pNode);
  if (size)
    return 0;
  else
    return -1;
}
//return total file count in dir
int FileTree::FindFile(TCHAR* path, int parentId)
{
  WIN32_FIND_DATA fd;
  HANDLE hFind;
  TCHAR fileName[MAX_PATH];
  int rc;
  int count = 0;

  _stprintf_s(fileName, MAX_PATH, TEXT("%s\\*.*"), path);
  hFind = FindFirstFile(fileName, &fd);
  if (hFind != INVALID_HANDLE_VALUE) {
    for (;;)
    {
      if (_tcscmp(fd.cFileName, TEXT(".")) == 0
        || _tcscmp(fd.cFileName, TEXT("..")) == 0)
      {
        //do nothing
      }
      else if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        //is directory, enqueue + recursive find
        int id = EnQueue(parentId, fd.cFileName, TRUE);
        count++;

        TCHAR newPath[MAX_PATH];
        _stprintf_s(newPath, MAX_PATH, TEXT("%s\\%s"), path, fd.cFileName);
        count += FindFile(newPath, id);
      }
      else
      {
        //is file, push to queue
        rc = EnQueue(parentId, fd.cFileName, FALSE);
        count++;
      }

      //find next
      rc = FindNextFile(hFind, &fd);
      if (rc) {
        //continue;
      }
      else
      {
        FindClose(hFind);
        break;
      }
    }
  }
  return count;
}

int FileTree::Build()
{
  int rc;
  int id;

  //[log]
  int elapsed = GetTickCount();

  //(1) push folder to queue, find file in this folder
  //(2) if is folder, push to stack, goto (1)
  //(3) if is file push to queue
  id = EnQueue(RootId, mzRootPath, TRUE);
  rc = FindFile((TCHAR*)mzRootPath, id);

  //push end node
  EnQueue(RootId, zEndNode, TRUE);

  //when build complete
  setBuildComplete();

  //[log]
  elapsed = GetTickCount() - elapsed;
  LOG_Write(TEXT("build root %s elapsed %d\n"), mzRootPath, elapsed);

  return rc;
}
int FileTree::pathPush(TCHAR* buff, int size, TCHAR* zElement)
{
  int len = _tcslen(buff);
  if (len > 0) {
    buff[len] = TEXT('\\');
    len++;
  }
  len += _stprintf_s(buff + len, size - len, TEXT("%s"), zElement);
  return 0;
}
int FileTree::pathPop(TCHAR* buff)
{
  int len = _tcslen(buff);
  //buff = "C:\\folder"
  //           ^-len
  while(buff[len] != TEXT('\\')) {
    len--;
  }
  assert(len >= 0);
  buff[len] = 0;
  return 0;
}

int FileTree::Extract()
{
  int rc = 0;
  int topId = RootId;
  TCHAR curPath[MAX_PATH];
  node nd;

  //[log]
  int elapsed = GetTickCount();

  curPath[0] = 0;
  //(1) get node from queue
  //(2) if node is folder, push parent folder to stack
  //(3) if node is file, check is there any file
  //    if no more file, pop out parent folder, goto (1)

  //init
  rc = DeQueue(&nd);
  //begin loop
  for(; rc!= -1;)
  {
    assert(rc == 0);
    for(; nd.parentId != topId;) {
      node tempNode;
      rc = Pop(&tempNode);
      pathPop(curPath);
      topId = tempNode.parentId;
    }
    if(nd.flags & node::isNode)
    {
      if (nd.id == 0) {
        //root folder
        LOG_Write(TEXT("%s\n"), nd.data);
      }
      else {
        //create folder
        LOG_Write(TEXT("%s\\%s\n"), curPath, nd.data);
      }
      //push folder
      topId = nd.id;
      pathPush(curPath, MAX_PATH, nd.data);
      rc = Push(&nd);
      assert(rc == 0);
    }
    else
    {
      assert(nd.flags & node::isLeaf);
      //create file
      LOG_Write(TEXT("%s\\%s\n"), curPath, nd.data);
    }
    rc = DeQueue(&nd);

    //if end node
    if (nd.parentId == RootId) {
      assert(_tcscmp(nd.data, zEndNode) == 0);
      break;
    }
  }

  //[log]
  elapsed = GetTickCount() - elapsed;
  LOG_Write(TEXT("extract count %d elapsed %d\n"), mQueueItemCount, elapsed);

  return rc;
}
