#include <crtdbg.h>
#define assert(x) _ASSERT(x)

#include "tree.h"
#include "transporter.h"
#include "vmem.h"
#include "qsMem.h"

//-----------------transporter-------------------
Transport_IF* transport_load_transporter() {
  return new Transporter();
}

//+++++++structure
struct TransportData {
  int mode;
  //queue
  //+ buffer
  char* queueBuffer;
  int queueBufferSize;
  //+ swap file
  int vMemSize;
  int pgSize;
  TCHAR* zSwapFile;

  TCHAR* zParams;
};

TransportParams* Transporter::TransportDataInit(
  int    isExportMode,
  char*  queueBuffer,
  int    queueBufferSize,
  TCHAR* zSwapFile,
  TCHAR* zParams
  )
{
  enum {
    vMemSize = 1<<30,
    pgSize   = 4096,
    //zSwapFileLen = MAX_PATH,
  };

  char* buff;
  int reqSize = sizeof(TransportParams) + sizeof(TransportData)/* + zSwapFileLen * sizeof(TCHAR)*/;
  buff = new char[reqSize];

  TransportData &td = *(TransportData*)(buff + sizeof(TransportParams));
  td.mode = queueModeVmem;
  if (isExportMode) {
    td.mode |= exportMode;
  } else {
    td.mode |= importMode;
  }
  td.pgSize = pgSize;
  td.vMemSize = vMemSize;
  td.queueBuffer = queueBuffer;
  td.queueBufferSize = queueBufferSize;
  //td.zSwapFile = (TCHAR*)(buff + sizeof(TransportParams) + sizeof(TransportData));
  //_tcscpy_s(td.zSwapFile, zSwapFileLen, zSwapFile);
  td.zSwapFile = zSwapFile;
  //debug
  td.zParams = zParams;

  TransportParams &tp = *(TransportParams*)buff;
  tp.modType = tp.singleTransporter;
  tp.exParams = &td;

  return (TransportParams*)buff;
}
void Transporter::TransportDataFinal(TransportParams* p)
{
  if (p) delete (char*)p;
}
//+++++++methods
//if success return 0
//else return error code != 0
int Transporter::initThreads()
{
  int rc = 4; //4 steps
  argsParser ap;
  Xporter xpot;
  explorer expl;

  enum {
    initExploreData   = 0x01,
    initXportData     = 0x02,
    crtExploreThread  = 0x04,
    crtXportThread    = 0x08,
  };
  int flags = crtXportThread | initXportData;

#if (WORKING_MODE == 1)
  flags |= initExploreData;
#else
  if (mData.mode & exportMode) flags |= initExploreData | crtExploreThread;
#endif

  do {
    //(1) create explore thread 
    if (flags & crtExploreThread) {
      mThreadMng.explorerHandle = m_CreateThread(
        ExploreThreadMain_callback,
        this,
        &mThreadMng.explorerId
        );
      if (mThreadMng.explorerHandle == INVALID_HANDLE_VALUE) break;
    }
    rc--;
    //(2) create export/import thread
    mThreadMng.xporterHandle = m_CreateThread(
      XportThreadMain_callback,
      this,
      &mThreadMng.xporterId
      );
    if (mThreadMng.xporterHandle == INVALID_HANDLE_VALUE) break;
    rc--;
    LOG_DEBUG(TEXT("init thread data start"));
    //init threads data
    //(3) init explore data
    if (flags & initExploreData) {
      mExploringInfo.exploreArgs = ap.InitArgs(mData.zParams);
      if (mExploringInfo.exploreArgs == 0) break;
      mExploringInfo.exploreData = expl.InitData(mNodeQueue.treeHandle);
      if(mExploringInfo.exploreData == 0) {
        ap.FinalArgs(mExploringInfo.exploreArgs);
        break;
      }
    }
    LOG_DEBUG(TEXT("init expl data done"));
    rc--;
    //(4) init import/export data
    mXportingInfo.xportArgs = ap.InitArgs(mData.zParams);
    if(mXportingInfo.xportArgs == 0) break;
    mXportingInfo.xportData = xpot.InitData(mNodeQueue.treeHandle);
    if(mXportingInfo.xportData == 0) {
      ap.FinalArgs(mXportingInfo.xportArgs);
      break;
    }
    LOG_DEBUG(TEXT("init xpot data done"));
    rc--;
  } while(FALSE);

  switch (rc) {
    case 1: //(4) init import/export data fail  - rollback step 3
      if (flags & initExploreData) {
        assert(mExploringInfo.exploreData);
        expl.FinalData(mExploringInfo.exploreData);
        assert(mExploringInfo.exploreArgs);
        ap.FinalArgs(mExploringInfo.exploreArgs);
      }
      //fall through
    case 2: //(3) init explore data fail        - rollback step 2
      assert(mThreadMng.xporterHandle != INVALID_HANDLE_VALUE);
      m_TerminateThread(mThreadMng.xporterHandle);
      //fall through
    case 3: //(2) create export/import thread fail - rollback step 1
      if (flags & crtExploreThread) {
        assert(mThreadMng.explorerHandle != INVALID_HANDLE_VALUE);
        m_TerminateThread(mThreadMng.explorerHandle);
      }
      //fall through
    default:
      //all done or nothing done - do nothing
      break;
  }

  return rc;
}
int Transporter::initShareQueue()
{
  int rc = 6; //6 steps
  do {
    //(.1)init file swap
    FileSwapParams fsp;
    fsp.zName = mData.zSwapFile;      //"d:\\tmp\\fs.txt
    fsp.mode = fsp.ModeNoBuffer;
    FileSwap *fs = new FileSwap;
    if(!fs) break;
    if (fs->Init(&fsp)) {
      //if init fail
      delete fs;
      break;
    }
    if (mData.mode & exportMode) fs->Truncate(0);
    mNodeQueue.fileSwapHandle = fs;
    rc--;

    //(.2)init vmem for queue
    VMemParams vmp;
    vmp.vMemSize = mData.vMemSize;   //1<<30
    vmp.buffer = mData.queueBuffer;
    vmp.bufferSize = mData.queueBufferSize;
    vmp.pgDataSize = mData.pgSize;   //512
    vmp.hFile = fs;
    VMem *vmem = new VMem();
    if(!vmem) break;
    if(vmem->Init(&vmp)) {
      //if init fail
      delete vmem;
      break;
    }
    mNodeQueue.vMemHandle = vmem;
    rc--;

    //(.3) init queue
    QSMemParams qsp;
    qsp.mVMem = vmem;
    qsp.mBufferSize = vmem->Size();
    qsp.mFlags = qsp.addrVMem | qsp.modeQueue;
    if (mData.mode & importMode) qsp.mFlags |= qsp.mapExistingContent;
    QSMem* qsm = new QSMem();
    mNodeQueue.qsMemHandle = qsm;
    if (!qsm) break;
    qsm->Init(&qsp);
    rc--;

    //(.4) init lock for share queue
    InitializeCriticalSection(&mNodeQueue.lockData);
    mNodeQueue.lockHandle = new Lock(&mNodeQueue.lockData);
    if(!mNodeQueue.lockHandle) break;
    rc--;
    //(.5) init share queue
    mNodeQueue.shareQueue = new ShareQueue(mNodeQueue.qsMemHandle, mNodeQueue.lockHandle);
    if(!mNodeQueue.shareQueue) break;
    rc--;
    //(.6) init tree
    treeMng* tree = new treeMng;
    if (!tree) break;
    tree->Init(mNodeQueue.shareQueue);
    mNodeQueue.treeHandle = tree;
    rc--;
  } while (FALSE);

  switch (rc)
  {
  case 1: //(.6) init tree fail   - rollback all
    delete mNodeQueue.shareQueue;
    //fall through
  case 2: //(.5) init share queue - rollback step 4
    delete mNodeQueue.lockHandle;
    DeleteCriticalSection(&mNodeQueue.lockData);
    //fall through
  case 3: //(.4) init lock fail   - rollback step 3
    delete (QSMem*)mNodeQueue.qsMemHandle;
    //fall through
  case 4: //(.3) init queue fail  - rollback step 2
    delete mNodeQueue.vMemHandle;
    //fall through
  case 5: //(.2) init vmem fail   - rollback step 1
    delete mNodeQueue.fileSwapHandle;
    //fall through
  default:
    //all done or nothing done - do nothing
    break;
  }
  return rc;
}
int Transporter::finalShareQueue()
{
  assert(mNodeQueue.treeHandle);
  mNodeQueue.treeHandle->Final();
  delete mNodeQueue.treeHandle;       //.7
  assert(mNodeQueue.lockHandle);
  delete (Lock*)mNodeQueue.lockHandle;          //.6
  DeleteCriticalSection(&mNodeQueue.lockData);  //.5
  assert(mNodeQueue.shareQueue);
  delete (ShareQueue*)mNodeQueue.shareQueue;    //.4
  assert(mNodeQueue.qsMemHandle);
  ((QSMem*)mNodeQueue.qsMemHandle)->Final();
  delete (QSMem*)mNodeQueue.qsMemHandle;        //.3
  assert(mNodeQueue.vMemHandle);
  mNodeQueue.vMemHandle->Final();
  delete mNodeQueue.vMemHandle;          //.2
  assert(mNodeQueue.fileSwapHandle);
  mNodeQueue.fileSwapHandle->Final();
  delete mNodeQueue.fileSwapHandle;  //.1
  return 0;
}
int Transporter::Init(TransportParams* params)
{
  int rc;
  int nDone = 0;
  do {
    gLog.logInit();

    TransportData &tp = *(params->exParams);
    //init state, save mode
    //(0)
    mStateFlags = 0;
    mData.mode            = tp.mode;
    mData.queueBuffer     = tp.queueBuffer;
    mData.queueBufferSize = tp.queueBufferSize;
    mData.vMemSize        = tp.queueBufferSize;
    mData.pgSize          = tp.pgSize;
    mData.zSwapFile       = tp.zSwapFile;
    mData.zParams         = tp.zParams;

    //init share objects
    //(1)
    mInterruptObj.isInterrupted = FALSE;
    InitializeCriticalSection(&mInterruptObj.lock);
    nDone = 1;
#if (WORKING_MODE == 1)
#else
    //(2) share node queue
    rc = initShareQueue();
    if (rc) {break;}
#endif //WORKING_MODE 1
    nDone = 2;

    //(3)
    mExploringInfo.exploredSize = 0;
    mExploringInfo.isCompleted = 0;
    InitializeCriticalSection(&mExploringInfo.lock);
    //(3)
    mXportingInfo.xportedSize = 0;
    mXportingInfo.isCompleted = 0;
    InitializeCriticalSection(&mXportingInfo.lock);
    nDone = 3;

    //(4) create threads
    {
    }
    nDone = 4;

    //(5)
    rc = initThreads();
    if (rc) {break;}
    nDone = 5;
  } while(FALSE);

  //rollback if init fail
  switch (nDone) {
    case 5: //all init success
      setFlags(TransportStatus::initialized);
      rc = 0;
      break;
    case 4: //(5) init threads fail     - rollback step 4
      if (mThreadMng.xporterHandle != INVALID_HANDLE_VALUE)
        m_TerminateThread(mThreadMng.xporterHandle);
      if (mThreadMng.explorerHandle != INVALID_HANDLE_VALUE)
        m_TerminateThread(mThreadMng.explorerHandle);
      //fall through
    case 3: //(4) create threads fail   - rollback step 3
      DeleteCriticalSection(&mXportingInfo.lock);
      DeleteCriticalSection(&mExploringInfo.lock);
      //fall through
    case 2: //                          - rollback step 2
#if (WORKING_MODE == 1)
#else //WORKING_MODE 1
      finalShareQueue();
#endif //WORKING_MODE
      //fall through
    case 1: //(2) init share queue fail - rollback step 1
      DeleteCriticalSection(&mInterruptObj.lock);
      assert(rc);
      break;
  }
  return rc;
}
int Transporter::Start()
{
  assert(getState() == TransportStatus::initialized);
#if (WORKING_MODE == 1)
#else //WORKING_MODE 1
  if (mData.mode & exportMode) {
    //start traverse
    ResumeThread(mThreadMng.explorerHandle);
    setFlags(TransportStatus::exploring);
  }
#endif //WORKING_MODE 1
  //start export/import
  ResumeThread(mThreadMng.xporterHandle);
  setFlags(TransportStatus::xporting);

  return 0;
}
int Transporter::GetStatus(TransportStatus* pStatus)
{
  //+ check explore status
  enterLock(&mExploringInfo.lock);
  if (mStateFlags & TransportStatus::exploring) {
    pStatus->exploredSize = mExploringInfo.exploredSize;
    if (mExploringInfo.isCompleted) {
      resetFlags(TransportStatus::exploring);
      setFlags(TransportStatus::exploringComplete);
    }
  }
  leaveLock(&mExploringInfo.lock);
  //+ check export/import status
  enterLock(&mXportingInfo.lock);
  if (mStateFlags & TransportStatus::xporting) {
    pStatus->xportedSize = mXportingInfo.xportedSize + mXportingInfo.xportingSize;
    if (mXportingInfo.isCompleted) {
      resetFlags(TransportStatus::xporting);
      setFlags(TransportStatus::xportingComplete);
    }
  }
  leaveLock(&mXportingInfo.lock);
  //+ return status
  pStatus->stateFlags = mStateFlags;
  pStatus->exploredSize = mExploringInfo.exploredSize;
  pStatus->xportedSize = mXportingInfo.xportedSize;

  return 0;
}
int Transporter::Cancel()
{
  LOG_DEBUG(TEXT("[cancel] start"));
  Interrupt(TRUE);
  return 0;
}
int Transporter::finalThreads()
{
  enum {
    needInterupt = 0x01,
    terminateExplorer = 0x40,
    terminateXporter  = 0x80,
    cleanExploreData = 0x0100,
    cleanXportData   = 0x0200,

    waitTimeout = 500,
  };
  int flags = cleanXportData | terminateXporter;

  argsParser ap;
  Xporter xpot;
  explorer expl;

#if (WORKING_MODE == 1)
  flags |= cleanExploreData;
#else
  if (mData.mode & exportMode) flags |= cleanExploreData | terminateExplorer;
#endif

  int curState = getState();

  //+ check cur status
  if (flags & terminateExplorer) {
    if (curState & TransportStatus::exploring) {
      flags |= needInterupt;
    }
  }
  if (curState & TransportStatus::xporting) {
    flags |= needInterupt;
  }
  //+ interrupt
  if (flags & needInterupt) {
    Interrupt(TRUE);
  }
  //+ terminate threads - wait thread complete and terminate
  if (flags & terminateExplorer) {
    m_TerminateThread(mThreadMng.explorerHandle, waitTimeout);
  }
  if (flags & terminateXporter) {
    m_TerminateThread(mThreadMng.xporterHandle, waitTimeout);
  }
  //+ clean thread data after terminate
  if (flags & cleanExploreData) {
    ap.FinalArgs(mExploringInfo.exploreArgs);
    expl.FinalData(mExploringInfo.exploreData);
  }
  if (flags & cleanXportData) {
    ap.FinalArgs(mXportingInfo.xportArgs);
    xpot.FinalData(mXportingInfo.xportData);
  }

  return 0;
}
int Transporter::Final() {
  //(5) terminate threads + clear thread data
  finalThreads();
  //(3)
  DeleteCriticalSection(&mXportingInfo.lock);
  //(3)
  DeleteCriticalSection(&mExploringInfo.lock);
  //(2) share queue
#if (WORKING_MODE == 1)
#else //WORKING_MODE 1
  finalShareQueue();
#endif //WORKING_MODE 1
  //(1)
  DeleteCriticalSection(&mInterruptObj.lock);

  gLog.logFinal();

  assert(TransportStatus::initialized);
  resetFlags(TransportStatus::initialized);
  setFlags(TransportStatus::finalized);

  return 0;
}

//-------Private area--------
//-------thread mng----------
HANDLE Transporter::m_CreateThread(void* pcallback, void* param, DWORD *pId)
{
  DWORD tempId;
  if (pId == 0) {
    pId = &tempId;
  }
  HANDLE hThread = CreateThread( 
    NULL,                  // default security attributes
    0,                     // use default stack size  
    (LPTHREAD_START_ROUTINE)pcallback,              // thread function name
    param,                 // argument to thread function 
    CREATE_SUSPENDED,      // suspend
    pId);                  // returns the thread identifier 
  LOG_DEBUG(TEXT("m_CreateThread hThread %x id %x"), hThread, *pId);
  return hThread;
}

int Transporter::m_TerminateThread(HANDLE hThread, int timeOut)
{
  int rc = WaitForSingleObject((HANDLE)hThread, timeOut);
  if (rc == WAIT_TIMEOUT) {
    if (TerminateThread((HANDLE)hThread, 1)) {
      //terminate success
    } else {
      assert(0);
    }
  } else {
    assert(rc == WAIT_OBJECT_0);
  }

  rc = CloseHandle((HANDLE)hThread);
  assert(rc);

  return 0;
}

int Transporter::ExploreThreadMain()
{
  enum {
    iNodeGetFirstParams = 0,
    iNodeChkInterrupt1,
    iNodeBeginTree,
    iNodeTraverseFirst,
    iNodeChkInterrupt2,
    iNodeEnqueueNode,
    iNodeTraverseNext,
    iNodeEndTree,
    iNodeGetNextParams,

    endNode,
    nNode = endNode,
  };

  enum {
    icmdGetFirstParams = 0,
    icmdBeginTree,
    icmdTraverseFirst,
    icmdEnqueueNode,
    icmdTraverseNext,
    icmdEndTree,
    icmdGetNextParams,
    icmdChkInterrupt,

    nCmd,
  };

  char buff1[calcRouteTbleSize(nNode)];
  routeTbl* prtbl = (routeTbl*)buff1;
  char buff2[calcProgSize(nCmd)];
  LProgram* pprog = (LProgram*)buff2;

  prtbl->nNode = nNode;
  pprog->nCmd = nCmd;
  pprog->pRouteTbl = prtbl;

  RTBnode *arrNodes = prtbl->arrNodes;
  command *arrCmds = pprog->arrCmds;

  //traverse all file/folder and push to queue
  //    GetFirstParams,
  //+-->ChkInterrupt1,
  //|   BeginTree,
  //|   TraverseFirst,
  //| +-->ChkInterrupt2,
  //| |   EnqueueNode,
  //| +---TraverseNext, --+
  //|     EndTree,      <-+
  //+---GetNextParams,

  setNode(arrNodes[iNodeGetFirstParams], icmdGetFirstParams, iNodeChkInterrupt1, endNode);
  setNode(arrNodes[iNodeChkInterrupt1] , icmdChkInterrupt  , iNodeBeginTree    , endNode);
  setNode(arrNodes[iNodeBeginTree]     , icmdBeginTree     , iNodeTraverseFirst, endNode);
  setNode(arrNodes[iNodeTraverseFirst] , icmdTraverseFirst , iNodeChkInterrupt2, endNode);
  setNode(arrNodes[iNodeChkInterrupt2] , icmdChkInterrupt  , iNodeEnqueueNode  , endNode);
  setNode(arrNodes[iNodeEnqueueNode]   , icmdEnqueueNode   , iNodeTraverseNext , endNode);
  setNode(arrNodes[iNodeTraverseNext]  , icmdTraverseNext  , iNodeChkInterrupt2, iNodeEndTree);
  setNode(arrNodes[iNodeEndTree]       , icmdEndTree       , iNodeGetNextParams, endNode);
  setNode(arrNodes[iNodeGetNextParams] , icmdGetNextParams , iNodeChkInterrupt1, endNode);

  //init arguments
#if(0)
  argsParser ap;
  //TCHAR* zParams = TEXT("<X:\\|C|F:\\C\\><Z:\\|D|F:\\D\\><Y:\\|E|F:\\E\\>");
  //TCHAR* zParams = TEXT("<Y:\\|D|F:\\D\\><Z:\\|E|F:\\E\\>");
  TCHAR* zParams = mXportParams;
  transportArgs *pArgs = ap.InitArgs(zParams);
  explorer expl;
  exploreData *pED = expl.InitData(mNodeQueue.treeHandle);
#else
  transportArgs *pArgs = mExploringInfo.exploreArgs;
  exploreData *pED = mExploringInfo.exploreData;
#endif

  setCmd(arrCmds[icmdGetFirstParams], &Transporter::cmdGetFirstParams , pArgs  , 0   , 0);
  setCmd(arrCmds[icmdBeginTree]     , &Transporter::cmdBeginTree      , pArgs  , pED , 0);
  setCmd(arrCmds[icmdTraverseFirst] , &Transporter::cmdTraverseFirst  , pArgs  , pED , 0);
  setCmd(arrCmds[icmdEnqueueNode]   , &Transporter::cmdEnqueueNode    , pED    , 0   , 0);
  setCmd(arrCmds[icmdTraverseNext]  , &Transporter::cmdTraverseNext   , pED    , 0   , 0);
  setCmd(arrCmds[icmdEndTree]       , &Transporter::cmdEndTree        , pED    , 0   , 0);
  setCmd(arrCmds[icmdGetNextParams] , &Transporter::cmdGetNextParams  , pArgs  , 0   , 0);
  setCmd(arrCmds[icmdChkInterrupt]  , &Transporter::cmdChkInterrupt   , 0      , 0   , 0);

  LOG_DEBUG(TEXT("[expl] start exploring"));

  //exec prog
  int rc;
  rc = execProg(pprog);

  switch(pprog->pRouteTbl->prevNode) {
    case iNodeChkInterrupt1:
      break;
    case iNodeChkInterrupt2:  //while traversal
      cmdTraverseCancel(pED, 0, 0);
      break;
    default:
      break;
  }

  //final
#if(0)
  ap.FinalArgs(pArgs);
  expl.FinalData(pED);
#endif

  //update xporting info
  enterLock(&mExploringInfo.lock);
  mExploringInfo.isCompleted = TRUE;
  leaveLock(&mExploringInfo.lock);

  LOG_DEBUG(TEXT("[expl] end"));

  return 0;
}

int Transporter::XportThreadMain()
{
  enum {
    iNodeGetFirstParams = 0,
    iNodeChkInterrupt1,
    iNodeExtractFirst,
    iNodeClean,
    iNodeChkInterrupt2,
    //iNodeXportNode,
    iNodeXportFirst2,
    iNodeChkInterrupt3,
    iNodeXportNext,
    iNodeExtractNext,
    iNodeGetNextParams,

    endNode,
    nNode = endNode,
  };

  enum {
    icmdGetFirstParams = 0,
    icmdExtractFirst,
    icmdClean,
    //icmdXportNode,
    icmdXportFirst2,
    icmdXportNext,
    icmdExtractNext,
    icmdGetNextParams,
    icmdChkInterrupt,

    nCmd,
  };

  char buff1[calcRouteTbleSize(nNode)];
  routeTbl* prtbl = (routeTbl*)buff1;
  char buff2[calcProgSize(nCmd)];
  LProgram* pprog = (LProgram*)buff2;

  prtbl->nNode = nNode;
  pprog->nCmd = nCmd;
  pprog->pRouteTbl = prtbl;

  RTBnode *arrNodes = prtbl->arrNodes;
  command *arrCmds = pprog->arrCmds;

  //    GetFirstParams,
  //+-->ChkInterrupt1,
  //|     ExtractFirst,
  //|     Clean,
  //| +-->ChkInterrupt2,
  //| |     XportFirst,     --+
  //| | +-->ChkInterrupt3,    |
  //| | +---XportNext,  --+   |
  //| +---ExtractNext,  <-+ <-+
  //+---GetNextParams,

  setNode(arrNodes[iNodeGetFirstParams], icmdGetFirstParams, iNodeChkInterrupt1, endNode);
  setNode(arrNodes[iNodeChkInterrupt1] , icmdChkInterrupt  , iNodeExtractFirst , endNode);
  setNode(arrNodes[iNodeExtractFirst]  , icmdExtractFirst  , iNodeClean        , endNode);
  setNode(arrNodes[iNodeClean]         , icmdClean         , iNodeChkInterrupt2, endNode);
  setNode(arrNodes[iNodeChkInterrupt2] , icmdChkInterrupt  , iNodeXportFirst2  , endNode);
  setNode(arrNodes[iNodeXportFirst2]   , icmdXportFirst2   , iNodeChkInterrupt3, iNodeExtractNext);
  setNode(arrNodes[iNodeChkInterrupt3] , icmdChkInterrupt  , iNodeXportNext    , endNode);
  setNode(arrNodes[iNodeXportNext]     , icmdXportNext     , iNodeChkInterrupt3, iNodeExtractNext);
  setNode(arrNodes[iNodeExtractNext]   , icmdExtractNext   , iNodeChkInterrupt2, iNodeGetNextParams);
  setNode(arrNodes[iNodeGetNextParams] , icmdGetNextParams , iNodeChkInterrupt1, endNode);

  //init arguments
#if(0)
  argsParser ap;
  //TCHAR* zParams = TEXT("<X:\\|C|F:\\C\\><Z:\\|D|F:\\D\\><Y:\\|E|F:\\E\\>");
  //TCHAR* zParams = TEXT("<Y:\\|D|F:\\D\\><Z:\\|E|F:\\E\\>");
  TCHAR* zParams = mXportParams;
  transportArgs *pArgs = ap.InitArgs(zParams);
  Xporter xpot;
  XportData *pXD = xpot.InitData(mNodeQueue.treeHandle);
#else
  transportArgs *pArgs = mXportingInfo.xportArgs;
  XportData *pXD = mXportingInfo.xportData;
#endif
  setCmd(arrCmds[icmdGetFirstParams], &Transporter::cmdGetFirstParams, pArgs  , 0   , 0);
  setCmd(arrCmds[icmdExtractFirst]  , &Transporter::cmdExtractFirst  , pXD    , 0   , 0);
  setCmd(arrCmds[icmdClean]         , &Transporter::cmdClean         , pArgs  , 0   , 0);
  setCmd(arrCmds[icmdXportFirst2]   , &Transporter::cmdXportFirst2   , pXD    , pArgs, 0);
  setCmd(arrCmds[icmdXportNext]     , &Transporter::cmdXportNext     , pXD    , 0   , 0);
  setCmd(arrCmds[icmdExtractNext]   , &Transporter::cmdExtractNext   , pXD    , 0   , 0);
  setCmd(arrCmds[icmdGetNextParams] , &Transporter::cmdGetNextParams , pArgs  , 0   , 0);
  setCmd(arrCmds[icmdChkInterrupt]  , &Transporter::cmdChkInterrupt  , 0      , 0   , 0);

  LOG_DEBUG(TEXT("[xport] start"));

  //exec prog
  int rc;
  rc = execProg(pprog);

  switch (pprog->pRouteTbl->prevNode) {
    case iNodeChkInterrupt1:
      break;
    case iNodeChkInterrupt2:
      break;
    case iNodeChkInterrupt3:  //while copying
      cmdXportCancel(pXD, 0, 0);
      break;
    default:
      break;
  }

  //final args
#if(0)
  ap.FinalArgs(pArgs);
  xpot.FinalData(pXD);
#endif

  //update xporting info
  enterLock(&mXportingInfo.lock);
  mXportingInfo.isCompleted = TRUE;
  leaveLock(&mXportingInfo.lock);

  LOG_DEBUG(TEXT("[xport] end"));

  return 0;
}
#if (WORKING_MODE == 1)
int Transporter::XportThreadMain_1()
{
  enum {
    iNodeGetFirstParams = 0,
    iNodeChkInterrupt1,
    iNodeTraverseFirst,
    iNodeXportBegin,
    iNodeChkInterrupt2,
    iNodeXportFirst3,
    iNodeChkInterrupt3,
    iNodeXportNext,
    iNodeTraverseNext,
    iNodeGetNextParams,

    endNode,
    nNode = endNode,
  };

  enum {
    icmdGetFirstParams = 0,
    icmdTraverseFirst,
    icmdXportBegin,
    icmdXportFirst3,
    icmdXportNext,
    icmdTraverseNext,
    icmdGetNextParams,
    icmdChkInterrupt,

    nCmd,
  };

  char buff1[calcRouteTbleSize(nNode)];
  routeTbl* prtbl = (routeTbl*)buff1;
  char buff2[calcProgSize(nCmd)];
  LProgram* pprog = (LProgram*)buff2;

  prtbl->nNode = nNode;
  pprog->nCmd = nCmd;
  pprog->pRouteTbl = prtbl;

  RTBnode *arrNodes = prtbl->arrNodes;
  command *arrCmds = pprog->arrCmds;

  //    GetFirstParams,
  //+-->ChkInterrupt1,
  //|     TraverseFirst,
  //|     XportBegin,
  //| +-->ChkInterrupt2,
  //| |     XportFirst,     ----+
  //| | +-->ChkInterrupt3,      |
  //| | +---XportNext,  --+     |
  //| +---TraverseNext, <-+   <-+
  //+---GetNextParams,

  setNode(arrNodes[iNodeGetFirstParams], icmdGetFirstParams, iNodeChkInterrupt1, endNode);
  setNode(arrNodes[iNodeChkInterrupt1] , icmdChkInterrupt  , iNodeTraverseFirst, endNode);
  setNode(arrNodes[iNodeTraverseFirst] , icmdTraverseFirst , iNodeXportBegin   , endNode);
  setNode(arrNodes[iNodeXportBegin]    , icmdXportBegin    , iNodeChkInterrupt2, endNode);
  setNode(arrNodes[iNodeChkInterrupt2] , icmdChkInterrupt  , iNodeXportFirst3  , endNode);
  setNode(arrNodes[iNodeXportFirst3]   , icmdXportFirst3   , iNodeChkInterrupt3, iNodeTraverseNext);
  setNode(arrNodes[iNodeChkInterrupt3] , icmdChkInterrupt  , iNodeXportNext    , endNode);
  setNode(arrNodes[iNodeXportNext]     , icmdXportNext     , iNodeChkInterrupt3, iNodeTraverseNext);
  setNode(arrNodes[iNodeTraverseNext]  , icmdTraverseNext  , iNodeChkInterrupt2, iNodeGetNextParams);
  setNode(arrNodes[iNodeGetNextParams] , icmdGetNextParams , iNodeChkInterrupt1, endNode);

  //init arguments

  transportArgs *pArgs = mXportingInfo.xportArgs;
  XportData *pXD = mXportingInfo.xportData;
  exploreData *pED = mExploringInfo.exploreData;

  setCmd(arrCmds[icmdGetFirstParams], &Transporter::cmdGetFirstParams, pArgs  , 0   , 0);
  setCmd(arrCmds[icmdTraverseFirst] , &Transporter::cmdTraverseFirst , pArgs  , pED , 0);
  setCmd(arrCmds[icmdXportBegin]    , &Transporter::cmdXportBegin    , pArgs  , pED , pXD);
  setCmd(arrCmds[icmdXportFirst3]   , &Transporter::cmdXportFirst3   , pArgs  , pED , pXD);
  setCmd(arrCmds[icmdXportNext]     , &Transporter::cmdXportNext     , pXD    , 0   , 0);
  setCmd(arrCmds[icmdTraverseNext]  , &Transporter::cmdTraverseNext  , pED    , 0   , 0);
  setCmd(arrCmds[icmdGetNextParams] , &Transporter::cmdGetNextParams , pArgs  , 0   , 0);
  setCmd(arrCmds[icmdChkInterrupt]  , &Transporter::cmdChkInterrupt  , 0      , 0   , 0);

  LOG_DEBUG(TEXT("[xport] start"));

  //exec prog
  int rc;
  rc = execProg(pprog);

  switch (pprog->pRouteTbl->prevNode) {
    case iNodeChkInterrupt1:
      break;
    case iNodeChkInterrupt2:  //while traversing
      cmdTraverseCancel(pED, 0, 0);
      break;
    case iNodeChkInterrupt3:  //while copying
      cmdXportCancel(pXD, 0, 0);
      cmdTraverseCancel(pED, 0, 0);
      break;
    default:
      break;
  }

  //update xporting info
  enterLock(&mXportingInfo.lock);
  mXportingInfo.isCompleted = TRUE;
  leaveLock(&mXportingInfo.lock);

  LOG_DEBUG(TEXT("[xport] end"));

  return 0;
}
#endif //WORKING_MODE 1
//-------Thread Proc callback--------------------
DWORD WINAPI ExploreThreadMain_callback(LPVOID lpParam)
{
  Transporter &t = *(Transporter*)lpParam;
  int rc = t.ExploreThreadMain();
  return rc;
}
DWORD WINAPI XportThreadMain_callback(LPVOID lpParam)
{
  Transporter &t= *(Transporter*)lpParam;
  int rc;
#if (WORKING_MODE == 1)
  rc = t.XportThreadMain_1();
#else //WORKING_MODE 1
  rc = t.XportThreadMain();
#endif //WORKING_MODE 1
  return rc;
}
//-----------------share objects-----------------
void Transporter::enterLock(CRITICAL_SECTION *pLock)
{
	LOG_DEBUG(TEXT("enterLock()"));
	EnterCriticalSection(pLock);
}
void Transporter::leaveLock(CRITICAL_SECTION *pLock)
{
	LOG_DEBUG(TEXT("leaveLock()"));
	LeaveCriticalSection(pLock);
}
BOOL Transporter::IsInterrupted()
{
	int rc;
	//enter lock
	enterLock(&mInterruptObj.lock);
	rc = mInterruptObj.isInterrupted;
	leaveLock(&mInterruptObj.lock);
	//leave lock
	return rc;
}
//set interruptTransfer(TRUE/FALSE)
//return value was set
BOOL Transporter::Interrupt(BOOL val)
{
	//enter lock
	enterLock(&mInterruptObj.lock);
	mInterruptObj.isInterrupted = val;
	leaveLock(&mInterruptObj.lock);
	//leave lock
	return val;
}
//---------------command---------------------------------
//if interrupted func return cmdFalse
int Transporter::cmdChkInterrupt(void*, void*, void*) {
  int rc = IsInterrupted();
  if (rc) {
    rc = cmdFalse;
  } else {
    rc =cmdSuccess;
  }
  LOG_DEBUG(TEXT("cmdCheckInterruptTransfer() rc %d"), rc);
  return rc;
}
int Transporter::cmdGetFirstParams(void* pTransportArgs, void*, void*) {
  transportArgs &ta = *(transportArgs*)pTransportArgs;
  argsParser ap;
  int rc = ap.FirstParams(&ta);
  if (rc == 0) {
    rc = cmdSuccess;
  }
  else {
    rc = cmdFalse;
  }
  return rc;
}
int Transporter::cmdGetNextParams(void* pTransportArgs, void*, void*) {
  transportArgs &ta = *(transportArgs*)pTransportArgs;
  argsParser ap;
  int rc = ap.NextParams(&ta);
  if (rc == 0) {
    rc = cmdSuccess;
  }
  else {
    rc = cmdFalse;
  }
  return rc;
}
//+++++++explorer prog
//push root Node to share tree
int Transporter::cmdBeginTree(void* pTransportArgs, void* pExploreData, void*)
{
  int rc;
  explorer expl;
  transportArgs &arg = *(transportArgs*)pTransportArgs;
  exploreData &ed = *(exploreData*)pExploreData;

  //prepare before start
  rc = expl.exploreBegin(&ed, &arg);
  assert(rc == 0);

  if (rc == 0) {
    rc = cmdSuccess;
  }
  else {
    rc = cmdFalse;
  }

  return rc;
}
int Transporter::cmdTraverseFirst(void* pTransportArgs, void* pExploreData, void*) {
  int rc;
  explorer expl;
  transportArgs &arg = *(transportArgs*)pTransportArgs;
  exploreData &ed = *(exploreData*)pExploreData;

  //start
  rc = expl.exploreFirst(&ed, &arg);

  if (rc == 0) {
    rc = cmdSuccess;
  }
  else {
    rc = cmdFalse;
  }

  return rc;
}
int Transporter::cmdEnqueueNode(void* pExploreData, void*, void*) {
  int rc;
  explorer expl;
  exploreData &ed = *(exploreData*)pExploreData;
  long long fileSize;

  rc = expl.explorePush(&ed);

  //update explore size
  expl.getFileSize(&ed, &fileSize);
  enterLock(&mExploringInfo.lock);
  mExploringInfo.exploredSize += fileSize;
  leaveLock(&mExploringInfo.lock);

  if (rc == 0) {
    rc = cmdSuccess;
  }
  else {
    rc = cmdFalse;
  }
  return rc;
}
int Transporter::cmdTraverseNext(void* pExploreData, void* , void* ) {
  int rc;
  explorer expl;
  exploreData &ed = *(exploreData*)pExploreData;
  rc = expl.exploreNext(&ed);
  if (rc == 0) {
    rc = cmdSuccess;
  }
  else {
    rc = cmdFalse;
  }
  return rc;
}
//push endNodeto share tree
int Transporter::cmdEndTree(void* pExploreData, void*, void*)
{
  int rc;
  explorer expl;
  exploreData &ed = *(exploreData*)pExploreData;
  rc = expl.exploreEnd(&ed);
  assert(rc == 0);

  if (rc == 0) {
    rc = cmdSuccess;
  }
  else {
    rc = cmdFalse;
  }

  return rc;
}
int Transporter::cmdTraverseCancel(void* pExploreData, void*, void*)
{
  explorer expl;
  exploreData &ed = *(exploreData*)pExploreData;
  expl.exploreCancel(&ed);
  return 0;
}
//+++++++exporter prog
int Transporter::cmdExtractFirst(void* pXportData, void* , void*)
{
  int rc;
  Xporter xpot;
  XportData &ed = *(XportData*)pXportData;
  rc = xpot.extractFirst(&ed);
  if (rc == 0)
    rc = cmdSuccess;
  else
    rc = cmdFalse;
  return rc;
}
//clean destination directory before import/export
int Transporter::cmdClean(void* pTransportArgs, void*, void*)
{
  int rc;
  Xporter xpot;
  transportArgs &args = *(transportArgs*)pTransportArgs;
  rc = xpot.clean(&args);
  if (rc == 0)
    rc = cmdSuccess;
  else
    rc = cmdFalse;
  return rc;
}
int Transporter::cmdExtractNext(void* pXportData, void* , void* )
{
  int rc;
  Xporter xpot;
  XportData &ed = *(XportData*)pXportData;
  rc = xpot.extractNext(&ed);
  if (rc == 0)
    rc = cmdSuccess;
  else
    rc = cmdFalse;
  return rc;
}
//export/import node with 2 params
int Transporter::cmdXportFirst2(void* pXportData, void* pTransportArgs, void*)
{
  int rc;
  Xporter xpot;
  XportData &xd = *(XportData*)pXportData;
  transportArgs &args = *(transportArgs*)pTransportArgs;

  rc = xpot.xportFirst(&xd, &args);
  switch(rc) {
    case xpot.error_copyComplete:
      rc = cmdFalse;              //copy next file/folder
      break;
    case xpot.error_copyNext:
      rc = cmdSuccess;            //copy first block
      break;
    default:
      assert(0);
      rc = cmdFail;
      break;
  }
  return rc;
}
int Transporter::cmdXportNext(void* pXportData, void*, void*)
{
  int rc;
  Xporter xpot;
  XportData &xd = *(XportData*)pXportData;
  long long fileSize, copied;

  rc = xpot.xportNext(&xd);
  switch(rc) {
    case xpot.error_copyComplete:
      //update size
      xpot.getFileSize(&xd, &fileSize, &copied);
      enterLock(&mXportingInfo.lock);
      mXportingInfo.xportingSize = 0;
      mXportingInfo.xportedSize += copied;
      leaveLock(&mXportingInfo.lock);
      //stop copy
      rc = cmdFalse;                //copy next file/folder
      break;
    case xpot.error_copyNext:
      //update size
      xpot.getFileSize(&xd, &fileSize, &copied);
      enterLock(&mXportingInfo.lock);
      mXportingInfo.xportingSize = copied;
      leaveLock(&mXportingInfo.lock);
      //continue copy
      rc = cmdSuccess;              //copy next block
      break;
    default:
      assert(0);
      rc = cmdFail;
      break;
  }
  return rc;
}
int Transporter::cmdXportCancel(void* pXportData, void* pTransportArgs, void*)
{
  Xporter xpot;
  XportData &xd = *(XportData*)pXportData;
  xpot.xportCancel(&xd);
  return 0;
}
#if (WORKING_MODE == 1)
//+++++++no file.bin 1 threads
//clean desPath, modify traversal data callback
int Transporter::cmdXportBegin(void* pTransportArgs, void* pExploreData, void* pXportData)
{
  int rc;
  Xporter xpot;
  transportArgs &args = *(transportArgs*)pTransportArgs;
  exploreData &ed = *(exploreData*)pExploreData;
  XportData &xd = *(XportData*)pXportData;
  //+ clean
  rc = xpot.clean(&args);
  assert(rc == 0);
  //+ modify traversal data callback
  xpot.xportBegin(&args, &ed, &xd);
  return cmdSuccess;
}
//export/import file/folder with 3 params
int Transporter::cmdXportFirst3(void* pTransportArgs, void* pExploreData, void* pXportData)
{
  int rc;
  Xporter xpot;
  XportData &xd = *(XportData*)pXportData;
  exploreData &ed = *(exploreData*)pExploreData;
  transportArgs &args = *(transportArgs*)pTransportArgs;

  rc = xpot.xportFirst(&args, &ed, &xd);
  switch(rc) {
    case xpot.error_copyComplete:
      rc = cmdFalse;              //copy next file/folder
      break;
    case xpot.error_copyNext:
      rc = cmdSuccess;            //copy first block
      break;
    default:
      assert(0);
      rc = cmdFail;
      break;
  }
  return rc;
}
#endif //WORKING_MODE 1
//+++++++
int Transporter::execProg(LProgram* pProg)
{
	return execProg(
		pProg->pRouteTbl->arrNodes,
		pProg->pRouteTbl->nNode,
		pProg->arrCmds, pProg->nCmd,
		&(pProg->pRouteTbl->prevNode)
		);
}
int Transporter::execProg(RTBnode *arrNodes, int nNode, command* arrCmds, int nCmd, int *pPrevNode)
{
	int prevNode = 0;
	int iNode = 0;
	int rc;
	int iCmd;
	enum {abnormalExit = 1, normalExit = 0,};

	for (;;) {
		//get node cmd
		iCmd = arrNodes[iNode].iCmd;
		if (iCmd >= nCmd) {
			assert(0);
			rc = abnormalExit;
			break;	//abnormal terminate
		}

		//exec cmd
		rc = (this->*(arrCmds[iCmd].pCallback))(arrCmds[iCmd].p1, arrCmds[iCmd].p2, arrCmds[iCmd].p3);

		LOG_DEBUG(TEXT("execProg() arrNodes %x iNode %d iCmd %d rc %d"), arrNodes, iNode, iCmd, rc);

		//save recent executed node
		prevNode = iNode;

		//branch to next node
		if (rc == cmdSuccess) {
			iNode = arrNodes[iNode].iLeft;
		} else if (rc == cmdFalse) {
			iNode = arrNodes[iNode].iRight;
		} else {
			LOG_DEBUG(TEXT("abnormal exit prevnode %d nNode %d"), prevNode, nNode);
			rc = abnormalExit;
			break;	//abnormal terminate
		}

		//end 
		if (iNode >= nNode) {
			rc = normalExit;	//normal terminate
			break;
		}
	}

	*pPrevNode = prevNode;
	return rc;
}
