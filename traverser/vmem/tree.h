#ifndef __TREE_H__
#define __TREE_H__

#include <tchar.h>
#include <Windows.h>

#define TRUNCATE_SWAP_FILE  (0)
#define TRUNCATE_LOG_FILE   (1)
#define USE_QSMEM           (1)   //use simple queue/stack instead of RingMem
#define AUTO_ENQUEUE        (0)
#define AUTO_CLEAR          (1)
#define cLEAN_FIRST         (1)
#define REMOVE_DIR_LEVEL    (1)

#define TREE_MAX_DEPTH      16    //default value

typedef class MemMngIF MemMngIF;

//-----------------node--------------------------
#define _NODE_MNG_
typedef struct nodeHdr nodeHdr;
typedef struct node node;

class nodeMng
{
public:
  int nodeAlignSize(nodeHdr* pHdr);
  int nodePop(MemMngIF* pmm, node* pNode);
  int nodePush(MemMngIF* pmm, node* pNode);
  int nodeFormat(node* pNode, int nodeId, TCHAR *zName, int isDir);
  int nodePush(MemMngIF* pStack, int* in);
  int nodePop(MemMngIF* pStack, int* out);
};
//-----------------path--------------------------
#define _PATH_MNG_
class pathMng
{
public:
  int pathPush(TCHAR* buff, int count, TCHAR* zElement);
  int pathPop(TCHAR* buff);
};
//-----------------file--------------------------
#define _FILE_COPY_
typedef struct copyData copyData;
class fileCopy_IF {
public:
  virtual int copyDir(TCHAR* zFile) = 0;
  virtual int copyFirst(copyData*) = 0;
  virtual int copyNext(copyData*) = 0;
  virtual int copyClose(copyData*) = 0;
  virtual ~fileCopy_IF(){}
};
class fileCopy
	: public fileCopy_IF
{
public:
  int copyFile(TCHAR* srcFile, TCHAR* desFile);
  int copyFile(copyData*, TCHAR* srcFile, TCHAR* desFile);
  int truncate(TCHAR* zFile, long long size);
public:
  //copyFile_IF
  int copyDir(TCHAR* zFile);
  int copyFirst(copyData*);
  int copyNext(copyData*);
  int copyClose(copyData*);
  ~fileCopy();
};
//-----------------big file
class bigFile
{
private:
  //file info
  struct file_info {
    HANDLE hFile;
    long long curPos;
    long long offset;
    long long size;
    TCHAR zName[MAX_PATH];
  } mHostFile,
    mSubFile;
  //state flags
  enum {
    none        = 0,
    initialized = 0x01,

    exportMode  = 0x10,

    opening     = 0x20, //sub file is opening
  };
  int mFlags;
  //pg header
  struct file_hdr {
    long long offset;   //start of file data
    long long size;     //actual data size
    TCHAR zName[MAX_PATH];
  };
  enum {pgSize = 4096,};
  char mBuff[pgSize];

  int initHdr(BOOL isHost);
  int loadHdr(BOOL isHost);
  int saveHdr(BOOL isHost);
  int allocateSubFile();
  int commitSubFile();
  int locateSubFile();
  int discardSubFile();
  int setPos(long long pos);
  long long getPos();
  long long align(long long size);
  BOOL aligned(long long size);
public:
  bigFile(TCHAR* zName, BOOL isExport);
  ~bigFile();

  HANDLE CreateFile(TCHAR* zName);
  BOOL CloseFile(HANDLE);
  BOOL ReadFile(HANDLE, char* buff, int size, int* nRead);
  BOOL WriteFile(HANDLE, char* data, int size, int* nWrite);
  BOOL SeekFile(HANDLE, long long);
  BOOL TruncateFile(HANDLE, long long size);
};
//-----------------key mng
typedef int (*cryptCallback)(HCRYPTKEY, void*buff, DWORD* size);
int encrypt(HCRYPTKEY key, void* buff, DWORD* size);
int decrypt(HCRYPTKEY key, void*buff, DWORD* size);
int initKey(HCRYPTKEY *pKey, HCRYPTPROV* pProv);
int finalKey(HCRYPTKEY hKey, HCRYPTPROV hProv);
HCRYPTKEY createKey (
  HCRYPTPROV hProv,
 unsigned char* keyData,
 unsigned int keyDataSize,
 unsigned char* ivData,
 unsigned int ivDataSize,
 int encryptMode
);
//-----------------copy file db
class fileDbCopy
	: public fileCopy
{
private:
  cryptCallback mCryptCallback;
  HCRYPTPROV mProv;
  HCRYPTKEY mKey;
  long long mValidData;
public:
  fileDbCopy(int isExport);
  int copyFile(TCHAR* srcFile, TCHAR* desFile, bool encrypt);
  int copyFileDb(copyData*, TCHAR* srcFile, TCHAR* desFile);
public:
  int copyFirst(copyData*);
  int copyNext(copyData*);
  ~fileDbCopy();
};
//-----------------cleaner-----------------------
#define _CLEANER_
typedef struct walkData walkData;
class walkCallback
{
public:
  virtual int onMetFile(int level, TCHAR* path, TCHAR* zName) = 0;
  virtual int onMetDir(int level, TCHAR* path, TCHAR* zName) = 0;
};
class recursiveWalk
{
public:
  int cleanFiles(TCHAR* zDesPath, int excludeDirLevel);
  int listFiles(TCHAR* zDesPath, int* pDepth);
  int Walk(TCHAR* path);
  int walkPreOrder(walkData *pData, TCHAR* path, int curLevel);
};
//-----------------tree obj----------------------
#define _TREE_MNG_
class treeMng
  :public nodeMng
{
private:
  MemMngIF* mQueueHandle;
  int mQueueItemCount;
  int getId();
public:
  enum {RootId = -1};
  int Init(MemMngIF* pQueue);
  int Final();
  int EnQueue(int parentId, TCHAR* zName, BOOL isDir);
  int DeQueue(node* pNode);
};
//-----------------extractor---------------------
#define _EXTRACTOR_
typedef struct ExtractData ExtractData;
class extractor :
  public pathMng,
  public nodeMng
{
public:
  int extractFirst(ExtractData* pParams);
  int extractNext(ExtractData* pParams);
};
//-----------------traverse context--------------
#define _TRAVERSE_CONTEXT_
typedef struct traversalContext traversalContext;
class contextMng
{
public:
  int contextPush(MemMngIF *pStack,traversalContext* in);
  int contextPop(MemMngIF *pStack, traversalContext* out);
};
//-----------------traverser---------------------
#define _TRAVERSER_
typedef struct traverseData traverseData;
//struct TraversalCallback
//{
//  //callback methods
//  int onGoUpBegin(int , void*);
//  int onGoUpComplete(int , void*);
//  int onGoDownBegin(int , void*);
//  int onGoDownComplete(int , void*);
//  //params for callback
//  void *pParams;
//};
class traverser :
  public pathMng,
  public contextMng
{
public:
  int traverseFirst(traverseData* p);
  int traverseFilter(traverseData*p);
  int traverseNext(traverseData* p);
  int traverseCancel(traverseData* p);
};
//-----------------transportArg------------------
#define _TRANSPORT_ARG_
typedef struct transportArgs transportArgs;
class argsParser
{
public:
  transportArgs* InitArgs(TCHAR* zParams);
  void FinalArgs(transportArgs*);

  int FirstParams(transportArgs*);
  int NextParams(transportArgs*);
private:
  TCHAR* nextElement(TCHAR* zPath, TCHAR* seps, TCHAR** curPos);
  int formatParams(TCHAR* zPath);
};
//-----------------explorer----------------------
#define _EXPLORER_
typedef struct exploreData exploreData;
class explorer
  :public fileCopy,
  public nodeMng,
  public traverser
{
public:
  int getDataSize(int depth);
  int InitData(treeMng* treeHandle, char* buff, int size);
  exploreData* InitData(treeMng* treeHandle);
  void FinalData(exploreData*);

  int exploreBegin(exploreData* in_out, transportArgs*);
  int exploreBegin(exploreData* in_out, TCHAR* zAlias);
  int exploreFirst(exploreData* in_out, transportArgs*);
  int exploreFirst(exploreData* in_out, TCHAR* zSrcPath);
  int exploreNext(exploreData* in_out);
  int exploreCancel(exploreData* in_out);
  int explorePush(exploreData* in_out);
  int exploreEnd(exploreData* in);
  int getFileSize(exploreData* in, long long *pSize);
};
//-----------------exporter----------------------
#define _EXPORTER_
typedef struct XportData XportData;
class Xporter
  : public extractor
  , public fileCopy
  , public recursiveWalk
{
public:
  enum error_code {
    error_copyComplete  = 0,
    error_copyNext      = 1,
    error_copyFail      = 2,
  };
  int getDataSize();
  int InitData(treeMng* treeHandle, char* buff, int size);
  XportData* InitData(treeMng* treeHandle);
  void FinalData(XportData*);

  int extractFirst(XportData*);
  int clean(transportArgs*);
  //int xportNode(XportData*, transportArgs*);
  int xportFirst(XportData*, transportArgs*);
  int xportFirst(transportArgs*, exploreData*, XportData*);
  int xportNext(XportData*);
  int xportCancel(XportData*);
  int extractNext(XportData*);

  int getFileSize(XportData*, long long* pSize, long long *pCopied);
#if(1)
  //WORKING MODE 1 - 1 thread no file.bin
  int convertEDtoXD(exploreData*, XportData*);
  int xportBegin(transportArgs*, exploreData*, XportData*);
#endif
private:
  //int xportOne(copyData*, TCHAR* zName, TCHAR* zAlias, TCHAR* srcDir, TCHAR* desDir, int isDir);
};
//-----------------importer----------------------
#define _IMPORTER_
class importer
{
public:
};
//-----------------log---------------------------
#define _LOG_
#define USE_MY_LOG
int GetModuleDir(TCHAR *buff, int count);
enum {
  L4DEBUG = 0,
  L4ERROR = 1,
};
#ifdef USE_MY_LOG
#define LOG_DEBUG(f, ...) {TCHAR _buff_[MAX_PATH]; _stprintf_s(_buff_, MAX_PATH, f, __VA_ARGS__); gLog.LogWrite(L4DEBUG, _buff_);}
#define LOG_ERROR(f, ...) {TCHAR _buff_[MAX_PATH]; _stprintf_s(_buff_, MAX_PATH, f, __VA_ARGS__); gLog.LogWrite(L4ERROR, _buff_);}
//#define LOG_DEBUG(l, f, ...) {TCHAR buff[MAX_PATH]; _stprintf_s(buff, MAX_PATH, f, __VA_ARGS__); gLog.LogWrite(l, buff);}
//#define LOG_WRITE2(p, l, f, ...) {TCHAR buff[MAX_PATH]; _stprintf_s(buff, MAX_PATH, f, __VA_ARGS__); p->LogWrite(p->l, buff);}
#else
#define LOG_DEBUG(...)
//#define LOG_WRITE2(...)
#endif

class log
{
private:
  DWORD startTime;
  FILE *fLogError;
  FILE *fLogDebug;
  CRITICAL_SECTION lock4AllThreadWriteLock;
public:
  void logInit();
  void logFinal();
  //log() {logInit();}
  //~log() {logFinal();}

  void LogWrite(int l, TCHAR* text);
};

extern log gLog;
#endif //__TREE_H__
