
//-----------------tree node-----------
#define treeMaxDepth  16
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
    DATA_COUNT = 256,
  };
  TCHAR data[DATA_COUNT];
  
} node, FtNode, *PFTNODE;

typedef class MemMngIF MemMngIF;
//typedef struct node node;
//typedef struct nodeHdr nodeHdr;
typedef struct ExtractData ExtractData;
//typedef struct TCHAR TCHAR;
typedef struct TraverseData TraverseData;
typedef struct TraversalContext TraversalContext;

//-----------------path----------------
class PathMng {
public:
  int pathPop(TCHAR* buff);
  int pathPush(TCHAR* buff, int count, TCHAR* zElement);
};
//-----------------tree----------------
class nodeMng {
public:
  int EnQueue(MemMngIF* pQueue, int nodeId, int parentId, TCHAR* zName, int isDir);
  int DeQueue(MemMngIF* pQueue, node* pNode);
  int Pop(MemMngIF* pStack, node* pNode);
  int Push(MemMngIF* pStack, node* pNode);
private:
  int nodeAlignSize(nodeHdr* pHdr);
  int nodeFormat(node* pNode, int id, TCHAR* zName, int isDir);
  int pop(MemMngIF* pmm, node* pNode);
  int push(MemMngIF* pmm, node* pNode);
  int remain(MemMngIF* pmm);
};

class tree
  : public PathMng
  , public nodeMng
{
public:
#define zEndNode    TEXT(".")
#define zRootNode   TEXT("Root")
  const int RootId = -1;

  int EnQueue(int parentId, TCHAR* zName, int isDir);
  int DeQueue(node* pNode);

  int ExtractFirst(ExtractData* pParams);
  int ExtractNext(ExtractData* pParams);
private:
  MemMngIF* mQueueHandle;
  int mNodeCount;

  int getId();
};

//-----------------file----------------
class FileMng {
private:
public:
  struct CopyData {
    enum {
      initialized = 1,
      copying,
      completed,

      pgSize = 4 * 1024,
      blockSize = 1024 * 1024,
    };
    int flags;
    TCHAR *zFin;
    HANDLE fin;
    union {
      struct {
        DWORD size_low;
        DWORD size_high;
      };
      long long size;
    };
    TCHAR* zFout;
    HANDLE fout;
    long long copied;
#if (ONE_COPY_DATA_BUFF)
    char* buff;
#else
    char buff[blockSize];
#endif
    int count;
  };

  int ExportOne(TCHAR* zFile, TCHAR* srcDir, TCHAR* desDir, int isDir);
  int exportDir(TCHAR* zFile);
  int exportFile(TCHAR* srcFile, TCHAR* desFile);
  int copyFirst(CopyData*);
  int copyNext(CopyData*);
  int copyClose(CopyData*);
  int ImportOne(TCHAR* zFile, TCHAR* zAlias, TCHAR* srcDir, TCHAR* desDir, int isDir);
  int CleanFiles(TCHAR* zPath);
};
//-----------------context-------------
class context {
public:
  int contextPush(void *pStack, TraversalContext* in);
  int contextPop(void *pStack, TraversalContext* out);
};
//-----------------traverser-----------
class Traverser :
  public context,
  public PathMng
{
public:
  int Init(tree* treeHandle, char* buff, int size, TraverseData**);
  int Init(tree* treeHandle, MemMngIF *stackHandle, TraverseData**);
  int TraverseBegin(TraverseData*, TCHAR *zAlias);
  int TraverseFirst(TraverseData* in_out, TCHAR* zSrcPath);
  int TraverseFilter(TraverseData* in);
  int TraversePush(TraverseData* in);
  int TraverseNext(TraverseData* in_out);
  int TraverseEnd();
  int Final();

  int GetStackSize();
private:
  tree* mTreeHandle;
  MemMngIF* mStackHandle;
  TraverseData *mTraverseData;
};

//-----------------extractor-----------
class extractor :
  public PathMng,
  public nodeMng
{
public:
  int Init(tree* treeHandle, char* buff, int size, ExtractData**);
  int Init(tree* treeHandle, MemMngIF* stackHandle, ExtractData**);
  int ExtractBegin(ExtractData*);
  int ExtractFirst(ExtractData*);
  int ExtractNext(ExtractData*);
  int ExtractEnd();
  int Final();

  int GetStackSize();
private:
  tree* mTreeHandle;
  MemMngIF* mStackHandle;
  ExtractData* mExtractData;
};
//-----------------exporter/importer
class exporter
  : public extractor
{
public:
  int Export(TCHAR* zFile, TCHAR* srcDir, TCHAR* desDir, int isDir);
};
class importer
  : public extractor
{
public:
  int Import(TCHAR* zFile, TCHAR* srcDir, TCHAR* desDir, int isDir);
};