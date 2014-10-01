#include <Windows.h>
#include "ringmem.h"
//-----------------file--------------------------
#define _FILE_MNG_
struct copyData {
  enum {
    initialized = 1,
    copying,
    completed,

    pgSize    = 4*1024,
    blockSize = 512*1024,
  };
  int flags;
  TCHAR zFin[MAX_PATH];
  HANDLE fin;
  union {
    struct {
      DWORD size_low;
      DWORD size_high;
    };
    long long size;
  };
  TCHAR zFout[MAX_PATH];
  HANDLE fout;
  long long copied;
  char buff[blockSize];
  int count;
};
typedef struct copyData copyData;

class copyFileIF
{
public:
  virtual int copyInit(copyData*, TCHAR* srcFile, TCHAR* desFile) = 0;
  virtual int copyFirst(copyData*p) = 0;
  virtual int copyNext(copyData*p) = 0;
  virtual int copyFinal(copyData*p) = 0;
};

class fileMng
{
private:
  int truncate(TCHAR* zFile, long long size);
public:
  enum {copyComplete = -1};
  int copyDir(TCHAR* zFile);
  int copyFile(TCHAR* srcFile, TCHAR* desFile);
  int copyFile(copyData*, TCHAR* srcFile, TCHAR* desFile);
  int copyFirst(copyData*p);
  int copyNext(copyData*p);
  int copyCanel(copyData*p);
  int copyClose(copyData*p);
};
//-----------------file buffer-------------------
#define _FILE_MNG_BUFFER_
class fileMng_buffer
  : private fileMng
{
public:
  int copyFile(copyData*, TCHAR* srcFile, TCHAR* desFile);
  int copyFirst(copyData*p);
  int copyNext(copyData*p);
};

#define FILEMNG_2THREADS
class copyFile_2threads
  : public copyFileIF
{
private:
  //copying info
  struct {
    RingMem   mem;        //reader + writer
    long long copySize;
    long long readSize;
    long long writeSize;  //main + writer
    int isReadingComplete;//reader + writer
    CRITICAL_SECTION lock;
    void getWriteSize(long long *pOut) {
      enterLock();
      *pOut = writeSize;
      leaveLock();
    }
    void setWriteSize(long long size) {
      enterLock();
      writeSize = size;
      leaveLock();
    }
    void incWriteSize(int size) {
      enterLock();
      writeSize += size;
      leaveLock();
    }
    void enterLock() {EnterCriticalSection(&lock);}
    void leaveLock() {LeaveCriticalSection(&lock);}
  } mCopyingInfo;
  //general state
  enum {
    readingStarted  = 0x01,
    writingStarted  = 0x02,
    readingComplete = 0x04,
    writingComplete = 0x80,
  };
  int mStateFlags;
  //files info
  struct {
    TCHAR* zName;
    HANDLE hFile;
  } mFileIn, mFileOut;
  //threads info
  HANDLE  mReadThreadHandle;
  DWORD   mReadThreadId;
  int     mReadBlockSize;
  int     mWriteBlockSize;
  HANDLE  mWriteThreadHandle;
  DWORD   mWriteThreadId;
  //thread mng
  CRITICAL_SECTION mThreadLock;
  void threadLockEnter();
  void threadLockLeave();
  HANDLE m_CreateThread(void* pcallback, void* param, DWORD *pId);
  int m_TerminateThread(HANDLE hThread, int timeOut);
public:
  copyFile_2threads();
  ~copyFile_2threads();
  int copyInit(copyData*, TCHAR* srcFile, TCHAR* desFile);
  int copyFirst(copyData*p);
  int copyNext(copyData*p);
  int copyCancel(copyData*p);
  int copyFinal(copyData*p);
  //
  int ReadThreadMain();
  int WriteThreadMain();
};
