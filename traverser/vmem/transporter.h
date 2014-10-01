#include <Windows.h>
#include <stdio.h>

#include "moveDataMng.h"

#define WORKING_MODE  (2)       //1: 1 threads no file.bin
                                //2: 2 threads with file.bin
typedef class FileSwap FileSwap;
typedef class VMem VMem;
typedef class MemMngIF MemMngIF;
typedef class ShareQueue ShareQueue;
typedef class treeMng treeMng;
typedef class LockIF LockIF;

typedef struct transportArgs transportArgs;
typedef struct exploreData exploreData;
typedef struct XportData XportData;
typedef struct TransportParams TransportParams;
//-----------------Thread Prog callback--------------------
DWORD WINAPI ExploreThreadMain_callback(LPVOID lpParam);
DWORD WINAPI XportThreadMain_callback(LPVOID lpParam);
//-----------------transporter-------------------
class Transporter
  :public Transport_IF
{
private:
  enum {
    queueModeVmem = 0x01,
    stackModeVmem = 0x02,

    importMode = 0x10,
    exportMode = 0x20,
  };
  struct {
    int mode;
    //queue
    //+ buffer
    char* queueBuffer;
    int queueBufferSize;
    //+ swap file
    int vMemSize;
    int pgSize;
    TCHAR* zSwapFile;
    //+ export/import params
    TCHAR* zParams;
  } mData;

  int mStateFlags;
  void setFlags(int flag) { mStateFlags |= flag;}
  void resetFlags(int flag) {mStateFlags &= ~flag;}
  int getState() { return mStateFlags;}
public:
  TransportParams* TransportDataInit(
    int    isExportMode,
    char*  queueBuffer,
    int    queueBufferSize,
    TCHAR* zSwapFile,
    TCHAR* zParams
    );
  void TransportDataFinal(TransportParams*);

  int Init(TransportParams*);
  int Start();
  int GetStatus(TransportStatus*);
  int Cancel();
  int Final();
  //---------------share objects---------------------------
  //(1)
private:
  struct {
    int isInterrupted;
    CRITICAL_SECTION lock;
  } mInterruptObj;
public:
  BOOL IsInterrupted();
  BOOL Interrupt(BOOL val);
private:
  //(2)
  struct {
    FileSwap* fileSwapHandle;     //.1
    VMem*     vMemHandle;         //.2
    MemMngIF* qsMemHandle;        //.3
    ShareQueue*       shareQueue; //.4
    CRITICAL_SECTION  lockData;   //.5
    LockIF*   lockHandle;         //.6
    treeMng*  treeHandle;         //.7
  } mNodeQueue;
  //(3)
  struct {
    long long exploredSize;
    int isCompleted;
    CRITICAL_SECTION lock;
    //exploreData
    transportArgs* exploreArgs;
    exploreData* exploreData;
  } mExploringInfo;
  //(4)
  struct {
    long long xportingSize; //current copy size (one file)
    long long xportedSize;  //total copied size (all files)
    int isCompleted;
    CRITICAL_SECTION lock;
    //XportData
    transportArgs* xportArgs;
    XportData* xportData;
  } mXportingInfo;
  //lock
  void enterLock(CRITICAL_SECTION*);
  void leaveLock(CRITICAL_SECTION*);
  //(5)
private:
  struct {
    HANDLE explorerHandle;
    DWORD  explorerId;
    HANDLE xporterHandle;
    DWORD  xporterId;
  } mThreadMng;
  int initShareQueue();
  int finalShareQueue();
  int initThreads();
  int finalThreads();
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
  int cmdBeginTree(void*, void*, void*);
  int cmdTraverseFirst(void* , void* , void*);
  int cmdEnqueueNode(void*, void*, void*);
  int cmdTraverseNext(void* , void* , void* );
  int cmdEndTree(void*, void*, void*);
  int cmdTraverseCancel(void*, void*, void*);
  //prog: export/import
  int cmdExtractFirst(void* , void* , void*);
  int cmdClean(void*, void*, void*);
  int cmdExtractNext(void* , void* , void* );
  int cmdXportFirst2(void*, void*, void*);
  int cmdXportNext(void*, void*, void*);
  int cmdXportCancel(void*, void*, void*);
#if (WORKING_MODE == 1)
  int cmdXportBegin(void*, void*, void*);
  int cmdXportFirst3(void*, void*, void*);
#endif //WORKING_MODE 1
  //execute logical program
  int execProg(RTBnode *arrNodes, int nNode, command* arrCmds, int nCmd, int *pPrevNode);
  int execProg(LProgram* pProg);
  //---------------thread manager--------------------------
private:
  HANDLE m_CreateThread(void* pcallback, void* param, DWORD *pId);
  int m_TerminateThread(HANDLE hThread, int timeOut = 100);
  int m_WaitThreadComplete(HANDLE hThread, int timeOut);
public:
  int ExploreThreadMain();
  //Ex-port/Im-port
  int XportThreadMain();
#if (WORKING_MODE == 1)
  int XportThreadMain_1();
#endif //WORKING_MODE 1
};