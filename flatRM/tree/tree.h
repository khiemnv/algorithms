#ifndef __TREE_H__
#define __TREE_H__

#include <tchar.h>

#define TRUNCATE_SWAP_FILE (0)

struct nodeHdr
{
  enum {
    isLeaf = 0x01,
    isNode = 0x02,
  };
  int id;
  int parentId;
  int flags;
  int size;
};

typedef struct treeNode :public nodeHdr
{
  enum {
    MAX_DATA_SIZE = 256 * 2,
  };
  TCHAR data[MAX_DATA_SIZE];
} node, FtNode, *PFTNODE;

struct FileTreeParams
{
  enum {
    queueModeVmem = 0x01,
    stackModeVmem = 0x02,

    importMode = 0x10,
    exportMode = 0x20,
  };
  int mode;
  //for queue rm
  //+ buffer
  char* queueBuffer;
  int queueBufferSize;
  //+ swap file
  int vMemSize;
  int pgSize;
  TCHAR* zSwapFile;

  //for stack rm
  char* stackBuffer;
  int stackBufferSize;

  //root dir
  TCHAR* zRoot;
  int maxDept;

  //debug
  TCHAR* zLogFile;
};

class FileTree
{
private:
  int mMode;
  //queue
  void* mQueueFS;
  void* mQueueVMem;
  void* mQueueRm;
  int mQueueItemCount;
  //stack
  void* mStackRm;
  //root path
  TCHAR* mzRootPath;
#if (TRUNCATE_SWAP_FILE)
  //file bin
  int mFileSwapSize;
  char* mzFileSwap;
#endif
  int maxDept;
  //[log]
  FILE* fLog;

  int getId();
  node* queueAllocate();
  int queueCommit(int size);
  int nodeAlignSize(nodeHdr* in);
  int nodeFormat(node* in, TCHAR *zName, int isDir);

  int pathPush(TCHAR* buff, int size, TCHAR* zElement);
  int pathPop(TCHAR* buff);

  //conflict & share area
  //+ share rm
  int push(void* prm, node* in);
  int pop(void* prm, node* out);
  int remain(void* prm);    //get remain size of rm
  //+ state share
  int setBuildComplete();
  int isBuildComplete();
public:
  FileTree(FileTreeParams*);
  ~FileTree();
  int Init(FileTreeParams*);
  int Final();

  enum {RootId = -1};
  int Build();
  int Extract();

  enum { timeOut = 100,};
  int FindFile(TCHAR* path, int parentId);

  int EnQueue(int parentId, TCHAR* zName, int isDir);
  int DeQueue(node* out);
  int Push(node *in);
  int Pop(node *out);
};
#endif //__TREE_H__
