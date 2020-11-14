#include <Windows.h>
#include <stdio.h>

#include "moveDataMng.h"

#define TRANSPORT_PARAMS_ZSEPS  TEXT("|<>")

typedef class Traverser Traverser;
typedef class FileSwap FileSwap;
typedef class VMem VMem;
typedef class MemMngIF MemMngIF;
typedef class tree tree;
typedef class exporter exporter;

//-----------------transport arg-----------------
struct TransportArgs {
public:
  TCHAR* srcpath;
  TCHAR* alias;
  TCHAR* despath;

  int nRead;
  int count;
  TCHAR* curPos;
  int iSeps;
  int iParams;
  TCHAR data[1];

  int Init(char* buff, int size, TCHAR* zSeps, TCHAR* zParams, TransportArgs**);
  int FirstParams(TransportArgs*);
  int NextParams(TransportArgs*);
private:
  int formatPrams(TCHAR* zPath);
  TCHAR* zParams();
  TCHAR* zSeps();
};

//-----------------transport params
struct TransporterParams {
  enum {
    queueModeVmem = 0x01,
    stackModeVmem = 0x02,

    importMode = 0x10,
    exportMode = 0x20,
  };
  int mode;

  //queue
  //+ buffer
  char* queueBuffer;
  int queueBufferSize;
  //+ swap file
  int vMemSize;
  int pgSize;
  TCHAR* zSwapFile;

  //stack
  char* stackBuffer;
  int stackBufferSize;

  //zParams
  TCHAR *zParams;

  //debug
  TCHAR* zLogFile;
};

//-----------------Thread Prog callback--------------------
DWORD WINAPI TraverseThreadMain_callback(LPVOID lpParam);
DWORD WINAPI ExportThreadMain_callback(LPVOID lpParam);
//-----------------transporter-------------------
class Transporter
  :public Transport_IF
  ,public TransportArgs
{
public:
  int Init(TransportParams*);
  int Start();
  int GetStatus(TransportStatus*);
  int Cancel();
  int Final();
  //---------------share objects---------------------------
private:
  struct {
    HANDLE traverserHandle;
    DWORD  traverserId;
    HANDLE exporterHandle;
    DWORD  exporterId;
    HANDLE importerHandle;
    DWORD  importerId;
  } mThreadMng;
  //(1) interrupt
private:
  struct {
    int isInterrupted;
    CRITICAL_SECTION lock;
  } mInterruptObj;
public:
  BOOL IsInterruptTransfer();
  BOOL InterruptTransfer(BOOL val);
private:
  //(2) node queue
  struct {
    FileSwap* fileSwapHandle;
    VMem*     vMemHandle;
    MemMngIF* qsMemHandle;
    tree*     treeHandle;
    CRITICAL_SECTION lock;
  } mNodeQueue;
  //(3) traverser
  struct {
    char*       stackBuffer;
    MemMngIF*   stackHandle;
    Traverser*  traverserHandle;
    long long   traversedSize;
    int         isTraveseCompleted;
    CRITICAL_SECTION lock;
  } mTraverseInfo;
  //(4) exporter
  struct {
    char*     stackBuffer;
    MemMngIF* stackHandle;
    exporter* exporterHandle;
    long long exportedSize;
    int       isExportCompleted;
    CRITICAL_SECTION lock;
  } mExportInfo;
  //(5) srcpaths, despaths, alias
  TCHAR* mzParams;
  //lock
  void enterLock(CRITICAL_SECTION*);
  void leaveLock(CRITICAL_SECTION*);
  //---------------command---------------------------------
public:
  //++++++structure declare++++++++++++++++++
  //take care when use
  typedef int (Transporter::*cmdCallback)(void*, void*, void* );

  typedef struct route_tbl_node {
    byte iCmd;	//command index in array commands
    byte iLeft;	//left node
    byte iRight;	//right node
    byte iCur;	//not use
  } RTBnode, *PRTBNODE;

  typedef struct cmd__ {
    cmdCallback pCallback;
    void* p1;
    void* p2;
    void* p3;
    //int p4;
  }command, *PCOMMAND;

  typedef struct route_tbl {
    int nNode;
    int prevNode;
    RTBnode arrNodes[1];
  } routeTbl, PROUTETBL;

  typedef struct logical_prog__ {
    routeTbl *pRouteTbl;
    int nCmd;
    command arrCmds[1];
  } LProgram, *PPROG;

#define calcRouteTbleSize(n) (n*sizeof(RTBnode) + sizeof(route_tbl))
#define calcProgSize(n) (n*sizeof(command) + sizeof(LProgram))
#define setNode(n,c,l,r)      {n.iCmd=c;n.iLeft=l;n.iRight=r;}
#define setCmd(cmd,cb,v1,v2,v3) {cmd.pCallback=cb;cmd.p1=v1;cmd.p2=v2;cmd.p3=v3;}

	enum cmd_return_code {
		cmdSuccess = 0,
		cmdFalse   = 1,
		cmdFail    = -1,
	};

	//++++++cmd declare++++++++++++++++++++++++
	//prog: traverse CMDs
  int cmdChkInterrupt(void*, void*, void*);
	int cmdGetFirstParams(void* , void* , void*);
	int cmdGetNextParams(void* , void* , void* );
	int cmdTraverseFirst(void* , void* , void*);
  int cmdEnqueueNode(void*, void*, void*);
	int cmdTraverseNext(void* , void* , void* );
  //prog: export/import
	int cmdExtractFirst(void* , void* , void*);
	int cmdExtractNext(void* , void* , void* );
	int cmdExportOne(void*, void*, void*);
	int cmdImportOne(void*, void*, void*);

	//execute logical program
	int execProg(RTBnode *arrNodes, int nNode, command* arrCmds, int nCmd, int *pPrevNode);
	int execProg(LProgram* pProg);
  //---------------thread manager--------------------------
private:
  HANDLE m_CreateThread(void* pcallback, void* param, DWORD *pId);
  int m_TerminateThread(HANDLE hThread, int timeOut = 100);
  int m_WaitThreadComplete(HANDLE hThread, int timeOut);
public:
  int TraverseThreadMain();
  int ExportThreadMain();
  int ImportThreadMain();
  //-----------------traverse--------------------------------

  //-----------------debugger--------------------------------
private:
	FILE *fLog4TransferThread;
	FILE *fLog4ExplorerThread;
	FILE *fLog4MainThread;
	FILE *fLog4AllThread;
	CRITICAL_SECTION lock4AllThreadWriteLock;
	void logInit();
	void logFinal();
public:
	enum {
		L4ALL = 0,
		L4TT = 1,
		L4ET = 2,
		L4MT = 4,
	};
#ifdef USE_MY_LOG
#define LOG_WRITE(l, f, ...) {TCHAR buff[MAX_PATH]; _stprintf_s(buff, MAX_PATH, f, __VA_ARGS__); LogWrite(l, buff);}
#define LOG_WRITE2(p, l, f, ...) {TCHAR buff[MAX_PATH]; _stprintf_s(buff, MAX_PATH, f, __VA_ARGS__); p->LogWrite(p->l, buff);}
#else
#define LOG_WRITE(...)
#define LOG_WRITE2(...)
#endif
	void LogWrite(int l, TCHAR* text);
};
