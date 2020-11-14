#include <crtdbg.h>
#define assert(x) _ASSERT(x)

#include "transporter.h"
#include "traverser.h"
#include "vmem.h"
#include "qsMem.h"

//-----------------transporter-------------------
Transport_IF* transport_load_transporter() {
  return new Transporter();
}
//if success return 0
//else return error code != 0
int Transporter::Init(TransportParams* params)
{
  TransporterParams &tp = *(params->exParams);
  //init share objects
  //(1)
  mInterruptObj.isInterrupted = 0;
  InitializeCriticalSection(&mInterruptObj.lock);
  //(2) share node queue
  {
    //(.1)init file swap
    FileSwapParams fsp;
    fsp.zName = tp.zSwapFile;      //"d:\\tmp\\fs.txt
    fsp.mode = fsp.ModeNoBuffer;
    FileSwap *fs = new FileSwap(&fsp);
    if (tp.mode & tp.exportMode) {
      fs->Truncate(0);
    }
    mNodeQueue.fileSwapHandle = fs;
    //(.2)init vmem for queue
    VMemParams vmp;
    vmp.vMemSize = tp.vMemSize;   //1<<30
    vmp.buffer = tp.queueBuffer;
    vmp.bufferSize = tp.queueBufferSize;
    vmp.pgDataSize = tp.pgSize;   //512
    vmp.hFile = fs;
    VMem *vmem = new VMem();
    vmem->Init(&vmp);
    mNodeQueue.vMemHandle = vmem;
    //(.3)
    QSMemParams qsp;
    qsp.mVMem = vmem;
    qsp.mBufferSize = vmem->Size();
    qsp.mFlags = qsp.addrVMem | qsp.modeQueue;
    if (tp.mode & tp.importMode)
      qsp.mFlags |= qsp.mapExistingContent;
    QSMem* qsm = new QSMem();
    qsm->Init(&qsp);
    mNodeQueue.qsMemHandle = qsm;
    //(.4)
    mNodeQueue.treeHandle = new tree;
    InitializeCriticalSection(&mNodeQueue.lock);
  }
  //(3)
  {
    Traverser* pTrav = new Traverser;
    mTraverseInfo.traverserHandle = pTrav;

    int bufferSize = pTrav->GetStackSize();
    char* buffer = new char[bufferSize];
    mTraverseInfo.stackBuffer = buffer;

    QSMemParams qsp;
    qsp.mBufferSize = bufferSize;
    qsp.mBuffer = buffer;
    qsp.mFlags = qsp.addrAbsolute | qsp.modeStack;
    QSMem* pStack = new QSMem;
    pStack->Init(&qsp);
    mTraverseInfo.stackHandle = pStack;

    mTraverseInfo.traversedSize = 0;
    mTraverseInfo.isTraveseCompleted = 0;
    InitializeCriticalSection(&mTraverseInfo.lock);
  }
  //(4)
  mExportInfo.exportedSize = 0;
  mExportInfo.isExportCompleted = 0;
  InitializeCriticalSection(&mExportInfo.lock);

  //(5) src paths, despaths, alias
  {
    int len = _tcslen(tp.zParams);
    mzParams = new TCHAR[len+1];
    _tcsncpy_s(mzParams, len + 1, tp.zParams, len);
  }

  //init threads
  mThreadMng.traverserHandle = m_CreateThread(
    TraverseThreadMain_callback,
    this,
    &mThreadMng.traverserId
    );
  mThreadMng.exporterHandle = m_CreateThread(
    TraverseThreadMain_callback,
    this,
    &mThreadMng.exporterId
    );

  return 0;
}
int Transporter::Start()
{
  //start traverse
  ResumeThread(mThreadMng.traverserHandle);
  //start export
  ResumeThread(mThreadMng.exporterHandle);
  return 0;
}
int Transporter::GetStatus(TransportStatus*) {return 0;}
int Transporter::Cancel() {return 0;}
int Transporter::Final()
{
  //(1)
  DeleteCriticalSection(&mInterruptObj.lock);
  //(2)
  {
    if (mNodeQueue.vMemHandle) {
      mNodeQueue.vMemHandle->Final();
      delete mNodeQueue.vMemHandle;
    }
    if (mNodeQueue.fileSwapHandle) delete mNodeQueue.fileSwapHandle;
    if (mNodeQueue.qsMemHandle) delete mNodeQueue.qsMemHandle;
    if (mNodeQueue.treeHandle) delete mNodeQueue.treeHandle;
    DeleteCriticalSection(&mNodeQueue.lock);
  }
  //(3)
  {
    if (mTraverseInfo.stackBuffer) delete mTraverseInfo.stackBuffer;
    if (mTraverseInfo.stackHandle) delete (QSMem*)mTraverseInfo.stackHandle;
    if (mTraverseInfo.traverserHandle) delete mTraverseInfo.traverserHandle;
  }
  //(4)
  {
    DeleteCriticalSection(&mExportInfo.lock);
  }
  //(5)
  if (mzParams) delete mzParams;

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
  LOG_WRITE(L4ALL, TEXT("m_CreateThread hThread %x id %x"), hThread, *pId);
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

int Transporter::TraverseThreadMain()
{
  enum {
    iNodeGetFirstParams = 0,
    iNodeChkInterrupt1,
    iNodeTraverseFirst,
    iNodeChkInterrupt2,
    iNodeEnqueueNode,
    iNodeChkInterrupt3,
    iNodeTraverseNext,
    iNodeGetNextParams,

    endNode,
    nNode = endNode,
  };

  enum {
    icmdGetFirstParams = 0,
    icmdTraverseFirst,
    icmdEnqueueNode,
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

  //traverse all file/folder and push to queue
  //(1)first params
  //traverse init
  //(2)traverse first
  //  if error goto (5)
  //(3)push node to queue
  //(4)traverse next
  //  if has node go to (3)
  //traverse final
  //(5)next params
  //  if has params go to (2)

  setNode(arrNodes[iNodeGetFirstParams], icmdGetFirstParams, iNodeChkInterrupt1, iNodeGetNextParams);
  setNode(arrNodes[iNodeChkInterrupt1] , icmdChkInterrupt  , iNodeTraverseFirst, endNode);
  setNode(arrNodes[iNodeTraverseFirst] , icmdTraverseFirst , iNodeChkInterrupt2, endNode);
  setNode(arrNodes[iNodeChkInterrupt2] , icmdChkInterrupt  , iNodeEnqueueNode  , endNode);
  setNode(arrNodes[iNodeEnqueueNode]   , icmdEnqueueNode   , iNodeChkInterrupt3, endNode);
  setNode(arrNodes[iNodeChkInterrupt3] , icmdChkInterrupt  , iNodeTraverseNext , endNode);
  setNode(arrNodes[iNodeTraverseNext]  , icmdTraverseNext  , iNodeChkInterrupt2, iNodeGetNextParams);
  setNode(arrNodes[iNodeGetNextParams] , icmdGetNextParams , iNodeChkInterrupt1, endNode);

  char taBuff[MAX_PATH];
  TransportArgs* pTA;
  TCHAR *zSeps = TRANSPORT_PARAMS_ZSEPS;
  TraverseData* pTD;
  Traverser &trav = *mTraverseInfo.traverserHandle;
  pTA->Init(taBuff, MAX_PATH, zSeps, mzParams, &pTA);
  trav.Init(mNodeQueue.treeHandle, mTraverseInfo.stackHandle, &pTD);

  setCmd(arrCmds[icmdGetFirstParams], &Transporter::cmdGetFirstParams , pTA    , 0   , 0);
  setCmd(arrCmds[icmdTraverseFirst] , &Transporter::cmdTraverseFirst  , pTA    , pTD, 0);
  setCmd(arrCmds[icmdEnqueueNode]   , &Transporter::cmdEnqueueNode    , pTD    , 0   , 0);
  setCmd(arrCmds[icmdTraverseNext]  , &Transporter::cmdTraverseNext   , pTD    , 0   , 0);
  setCmd(arrCmds[icmdGetNextParams] , &Transporter::cmdGetNextParams  , pTA   , 0   , 0);
  setCmd(arrCmds[icmdChkInterrupt]  , &Transporter::cmdChkInterrupt   , 0      , 0   , 0);

  //exec prog
  int rc;
  rc = execProg(pprog);

  trav.Final();
  return 0;
}

int Transporter::ExportThreadMain()
{
  enum {
    iNodeGetFirstParams = 0,
    iNodeChkInterrupt1,
    iNodeExtractFirst,
    iNodeChkInterrupt2,
    iNodeExportNode,
    iNodeChkInterrupt3,
    iNodeExtractNext,
    iNodeGetNextParams,

    endNode,
    nNode = endNode,
  };

  enum {
    icmdGetFirstParams = 0,
    icmdExtractFirst,
    icmdEnqueueNode,
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

  //(1)first params
  //(2)extract first node
  //  export node to USB
  //(3)extract next node
  //  export all file/folder
  //(4)next params
  //  if has params go to (2)

  setNode(arrNodes[iNodeGetFirstParams], icmdGetFirstParams, iNodeChkInterrupt1, iNodeGetNextParams);
  setNode(arrNodes[iNodeChkInterrupt1] , icmdChkInterrupt, iNodeExtractFirst, endNode);
  setNode(arrNodes[iNodeExtractFirst]  , icmdExtractFirst, iNodeChkInterrupt2, endNode);
  setNode(arrNodes[iNodeChkInterrupt2] , icmdChkInterrupt, iNodeExportNode, endNode);
  setNode(arrNodes[iNodeExportNode]    , icmdEnqueueNode, iNodeChkInterrupt3, endNode);
  setNode(arrNodes[iNodeChkInterrupt3] , icmdChkInterrupt, iNodeExtractNext, endNode);
  setNode(arrNodes[iNodeExtractNext]   , icmdExtractNext, iNodeChkInterrupt2, iNodeGetNextParams);
  setNode(arrNodes[iNodeGetNextParams] , icmdGetNextParams, iNodeChkInterrupt1, endNode);

  char taBuff[MAX_PATH];
  TransportArgs* pTA;
  TCHAR *zSeps = TRANSPORT_PARAMS_ZSEPS;
  ExtractData* pED;
  exporter &exp = *(mExportInfo.exporterHandle);
  pTA->Init(taBuff, MAX_PATH, zSeps, mzParams, &pTA);

  return 0;
}

//-------Thread Proc callback--------------------
DWORD WINAPI TraverseThreadMain_callback(LPVOID lpParam)
{
  Transporter &t = *(Transporter*)lpParam;
  int rc = t.TraverseThreadMain();
  return rc;
}

DWORD WINAPI ExportThreadMain_callback(LPVOID lpParam)
{
  Transporter &t= *(Transporter*)lpParam;
  int rc = t.ExportThreadMain();
  return rc;
}
//-----------------debugger--------------------------------
void Transporter::logInit()
{
#ifdef USE_MY_LOG
  TCHAR zDir[MAX_PATH];
  GetModuleDir(zDir, MAX_PATH);

  TCHAR buff[MAX_PATH];
  enum {nFile = 4,};
  struct {
    TCHAR *zFile;
    FILE **pHandle;
  } params[nFile] = {
    {TEXT("l4tt.log"), &fLog4TransferThread},
    {TEXT("l4et.log"), &fLog4ExplorerThread},
    {TEXT("l4mt.log"), &fLog4MainThread},
    {TEXT("l4all.log"), &fLog4AllThread},
  };
  for (int i = 0; i < nFile; i++) {
    _stprintf_s(buff, MAX_PATH, TEXT("%s\\%s"), zDir, params[i].zFile);
    int rc = _tfopen_s(params[i].pHandle, buff, TEXT("a+"));
    if (rc == 0) {
      //success
    } else {
      assert(0);
      rc = GetLastError();
    }
  }
  InitializeCriticalSection(&lock4AllThreadWriteLock);
#endif
}
//NOTE: logInit() should be called before this call
void Transporter::logFinal()
{
#ifdef USE_MY_LOG
  fclose(fLog4TransferThread);
  fclose(fLog4ExplorerThread);
  fclose(fLog4MainThread);
  fclose(fLog4AllThread);
  DeleteCriticalSection(&lock4AllThreadWriteLock);
#endif
}

void Transporter::LogWrite(int log, TCHAR* text)
{
  FILE *fout;
  DWORD temp = 0;
  DWORD curTID = GetCurrentThreadId();
  DWORD curTime = GetTickCount();
  int rqLck = 0;
  switch(log) {
    case L4TT:
      fout = fLog4TransferThread;
      //temp = transferThreadId;
      assert(temp == curTID);
      break;
    case L4ET:
      fout = fLog4ExplorerThread;
      //temp = explorerThreadId;
      assert(temp == curTID);
      break;
    case L4MT:
      temp = 1;
      fout = fLog4MainThread;
      break;
    default:
      assert(log == L4ALL);
      fout = fLog4AllThread;
      rqLck = 1;
      break;
  }

  if (rqLck) {
    EnterCriticalSection(&lock4AllThreadWriteLock);
  }

  _ftprintf_s(fout, TEXT("[%08x][%08x][%08x] %s \n"), curTime, curTID, temp, text);

  fflush(fout);

  if (rqLck) {
    LeaveCriticalSection(&lock4AllThreadWriteLock);
  }
}
//-----------------share objects-----------------
void Transporter::enterLock(CRITICAL_SECTION *pLock)
{
	LOG_WRITE(L4ALL, TEXT("enterLock()"));
	EnterCriticalSection(pLock);
}
void Transporter::leaveLock(CRITICAL_SECTION *pLock)
{
	LOG_WRITE(L4ALL, TEXT("leaveLock()"));
	LeaveCriticalSection(pLock);
}
BOOL Transporter::IsInterruptTransfer()
{
	int rc;
	//enter lock
	enterLock(&mInterruptObj.lock);
	rc = (mInterruptObj.isInterrupted != 0);
	leaveLock(&mInterruptObj.lock);
	//leave lock
	return rc;
}
//set interruptTransfer(TRUE/FALSE)
//return value was set
BOOL Transporter::InterruptTransfer(BOOL val)
{
	//enter lock
	enterLock(&mInterruptObj.lock);
	mInterruptObj.isInterrupted = val;
	leaveLock(&mInterruptObj.lock);
	//leave lock
	return val;
}
//-----------------traverse--------------------------------
////if ok return 0
//int Transporter::TransportArgs::formatPrams(TCHAR* zPath)
//{
//  int rc = 0;
//  int len = _tcslen(zPath);
//  for(len--;len > 0;len--) {
//    if (zPath[len] == TEXT('\\'))
//      zPath[len] = 0;
//    else
//      break;
//  }
//  rc = zPath[len - 1] == TEXT('\\');
//  return rc;
//}
//int Transporter::TransportArgs::FirstParams(TransportArgs* p)
//{
//  int rc = 0;
//  p->srcpath = _tcstok_s(p->zParams, p->zSeps, &(p->curPos));
//  formatPrams(p->srcpath);
//  p->alias = _tcstok_s(NULL, p->zSeps, &(p->curPos));
//  formatPrams(p->alias);
//  p->despath = _tcstok_s(NULL, p->zSeps, &(p->curPos));
//  formatPrams(p->despath);
//  p->nRead = 1;
//  return rc;
//}
//int Transporter::TransportArgs::NextParams(TransportArgs* p)
//{
//  int rc = 0;
//  if (p->nRead < p->count) {
//    p->nRead++;
//    p->srcpath = _tcstok_s(NULL, p->zSeps, &(p->curPos));
//    formatPrams(p->srcpath);
//    p->alias = _tcstok_s(NULL, p->zSeps, &(p->curPos));
//    formatPrams(p->alias);
//    p->despath = _tcstok_s(NULL, p->zSeps, &(p->curPos));
//    formatPrams(p->despath);
//  } else {
//    rc = -1;
//  }
//  return rc;
//}
//
////+++++++pathMng
////PARAMS
////  __in count: size of buff in TCHAR
//int Transporter::pathMng::pathPush(TCHAR* buff, int count, TCHAR* zElement)
//{
//  int len = _tcslen(buff);
//  if (len > 0) {
//    buff[len] = TEXT('\\');
//    len++;
//  }
//  len += _stprintf_s(buff + len, count - len, TEXT("%s"), zElement);
//  return 0;
//}
//int Transporter::pathMng::pathPop(TCHAR* buff)
//{
//  int len = _tcslen(buff);
//  //buff = "C:\\folder"
//  //           ^-len
//  while(buff[len] != TEXT('\\')) {
//    len--;
//  }
//  assert(len >= 0);
//  buff[len] = 0;
//  return 0;
//}
////+++++++fileMng
//int Transporter::cleanFiles(TCHAR* zDesPath)
//{
//  int rc;
//  TraverseData td;
//  _tcscpy_s(td.context.findPath, MAX_PATH, zDesPath);
//  td.stack = mTraverseData.stack;
//  TCHAR buff[MAX_PATH];
//  for (rc = td.TraverseFirst(&td); rc == 0;) {
//    if (td.TraverseFilter(&td))
//    {
//      //soft link or system file
//      //do nothing
//    }
//    //if folder
//    else if (td.context.fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
//      //do nothing
//    }
//    //if file
//    else {
//      //delete file
//      buff[0] = 0;
//      mPathMng.pathPush(buff, MAX_PATH, td.context.findPath);
//      mPathMng.pathPop(buff);
//      mPathMng.pathPush(buff, MAX_PATH, td.context.fd.cFileName);
//      rc = DeleteFile(buff);
//      if (!rc) {
//        rc = GetLastError();
//        assert(0);
//      }
//    }
//
//#if (REMOVE_DIR_LEVEL)
//    for (rc = TraverseNext(&td); rc != 0;) {
//      if (rc == td.error_completed_all) {
//        break;
//      }
//      else if (rc == td.error_completed_one_dir) {
//        //remove dir
//        pathPop(td.lastCompletedPath);
//        if (td.context.curLevel >= REMOVE_DIR_LEVEL) {
//          rc =  RemoveDirectory(td.lastCompletedPath);
//          if (!rc) {
//            rc = GetLastError();
//            assert(0);
//          }
//        }
//      }
//      //find next
//      rc = TraverseNext(&td);
//    }
//#else
//    rc = td.TraverseNext(&td);
//#endif
//  }
//  return rc;
//}
//---------------command---------------------------------
//if interrupted func return cmdFalse
int Transporter::cmdChkInterrupt(void*, void*, void*) {
  int rc = IsInterruptTransfer();
  if (rc) {
    rc = cmdFalse;
  } else {
    rc =cmdSuccess;
  }
  LOG_WRITE(L4TT, TEXT("cmdCheckInterruptTransfer() rc %d"), rc);
  return rc;
}
int Transporter::cmdGetFirstParams(void* pTransportArgs, void* , void*) {
  int rc = FirstParams((TransportArgs*)pTransportArgs);
  if (rc == 0) {
    rc = cmdSuccess;
  }
  else {
    rc = cmdFalse;
  }
  return rc;
}
int Transporter::cmdGetNextParams(void* pTransportArgs, void* , void* ) {
  int rc = NextParams((TransportArgs*)pTransportArgs);
  if (rc == 0) {
    rc = cmdSuccess;
  }
  else {
    rc = cmdFalse;
  }
  return rc;
}
//req init 
int Transporter::cmdTraverseFirst(void* pTransportArgs, void* pTraverseData, void* ) {
  TransportArgs &ta = *(TransportArgs*)pTransportArgs;
  TraverseData &td = *(TraverseData*)pTraverseData;
  Traverser &trav = *mTraverseInfo.traverserHandle;
  int rc = trav.TraverseBegin(&td, ta.alias);
  assert(rc == 0);
  rc = trav.TraverseFirst(&td, ta.srcpath);
  if (rc == 0)
    rc = cmdSuccess;
  else
    rc = cmdFalse;
  return rc;
}
int Transporter::cmdEnqueueNode(void* pTraverseData, void*, void*) {
  Traverser &trav = *mTraverseInfo.traverserHandle;
  TraverseData &td = *(TraverseData*)pTraverseData;
  int rc = trav.TraverseFilter(&td);
  if (rc == 0) {
    rc = trav.TraversePush(&td);
  }
  else {
    rc = 0;
  }

  if (rc == 0)
    rc = cmdSuccess;
  else
    rc = cmdFalse;
  return rc;
}
int Transporter::cmdTraverseNext(void* pTraverseData, void*, void*) {
  TraverseData &td = *(TraverseData*)pTraverseData;
  Traverser &trav = *mTraverseInfo.traverserHandle;
  int rc = trav.TraverseNext(&td);
  if (rc == 0)
    rc = cmdSuccess;
  else {
    trav.TraverseEnd();
    rc = cmdFalse;
  }
  return rc;
}

int Transporter::cmdExtractFirst(void* pPatient, void* pInfo, void*) {
  return 0;
}
int Transporter::cmdExtractNext(void*, void*, void*){ return 0; }
int Transporter::cmdExportOne(void*, void*, void*) { return 0; }
int Transporter::cmdImportOne(void*, void*, void*) { return 0; }

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

		LOG_WRITE(L4ALL, TEXT("execProg() arrNodes %x iNode %d iCmd %d rc %d"), arrNodes, iNode, iCmd, rc);

		//save recent executed node
		prevNode = iNode;

		//branch to next node
		if (rc == cmdSuccess) {
			iNode = arrNodes[iNode].iLeft;
		} else if (rc == cmdFalse) {
			iNode = arrNodes[iNode].iRight;
		} else {
			LOG_WRITE(L4ALL, TEXT("abnormal exit prevnode %d nNode %d"), prevNode, nNode);
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

//-----------------transport arg-----------------
//if ok return 0
int TransportArgs::formatPrams(TCHAR* zPath) {
  int rc = 0;
  int len = _tcslen(zPath);
  for (len--; len > 0; len--) {
    if (zPath[len] == TEXT('\\'))
      zPath[len] = 0;
    else
      break;
  }
  rc = zPath[len - 1] == TEXT('\\');
  return rc;
}
TCHAR* TransportArgs::zParams() {
  return data + iParams;
}
TCHAR* TransportArgs::zSeps() {
  return data + iSeps;
}
//if success return 0
//else if error return req buffer size need to store TransportArgs obj
int TransportArgs::Init(char* buff, int size, TCHAR* zSeps, TCHAR* zParams, TransportArgs** out)
{
  int lenSeps = _tcslen(zSeps);
  int lenParams = _tcslen(zParams);
  int reqSize = sizeof(TransportArgs) + (lenSeps + lenParams + 2) * sizeof(TCHAR);
  TransportArgs* &pta = *out;
  if (size >= reqSize) {
    pta = (TransportArgs*)buff;
    pta->iSeps = 0;
    _stprintf_s(pta->data, lenSeps + 1, TEXT("%s"), zSeps);
    pta->iParams = lenSeps + 1;
    _stprintf_s(pta->data + lenSeps + 1, lenParams + 1, TEXT("%s"), zParams);
  }
  else {
    assert(0);
    return reqSize;
  }
  return 0;
}
int TransportArgs::FirstParams(TransportArgs* p)
{
  int rc = 0;
  TransportArgs &ta = *p;
  ta.srcpath = _tcstok_s(ta.zParams(), ta.zSeps(), &(ta.curPos));
  formatPrams(ta.srcpath);
  ta.alias = _tcstok_s(NULL, ta.zSeps(), &(ta.curPos));
  formatPrams(ta.alias);
  ta.despath = _tcstok_s(NULL, ta.zSeps(), &(ta.curPos));
  formatPrams(ta.despath);
  ta.nRead = 1;
  return rc;
}
int TransportArgs::NextParams(TransportArgs* p)
{
  int rc = 0;
  TransportArgs &ta = *p;
  if (ta.nRead < ta.count) {
    ta.nRead++;
    ta.srcpath = _tcstok_s(NULL, ta.zSeps(), &(ta.curPos));
    formatPrams(ta.srcpath);
    ta.alias = _tcstok_s(NULL, ta.zSeps(), &(ta.curPos));
    formatPrams(ta.alias);
    ta.despath = _tcstok_s(NULL, ta.zSeps(), &(ta.curPos));
    formatPrams(ta.despath);
  }
  else {
    rc = -1;
  }
  return rc;
}