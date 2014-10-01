//test copy
#include <crtdbg.h>
#define assert(x) _ASSERT(x)
#include <tchar.h>
#include <Windows.h>

#include "fileMng.h"
#include "ringmem.h"

#include <fstream>
using namespace std;

//functions
int truncate(TCHAR* zFile, long long size);

#ifdef FILEMNG_2THREADS
#define USE_LOCATE 1
//+++++++thread
DWORD WINAPI ReadThreadMain_callback(LPVOID lpParam)
{
  copyFile_2threads &copier = *(copyFile_2threads*)lpParam;
  int rc = copier.ReadThreadMain();
  return rc;
}
DWORD WINAPI WriteThreadMain_callback(LPVOID lpParam)
{
  copyFile_2threads &copier = *(copyFile_2threads*)lpParam;
  int rc = copier.WriteThreadMain();
  return rc;
}
int copyFile_2threads::m_TerminateThread(HANDLE hThread, int timeOut)
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
void copyFile_2threads::threadLockEnter()
{
  EnterCriticalSection(&mThreadLock);
}
void copyFile_2threads::threadLockLeave()
{
  LeaveCriticalSection(&mThreadLock);
}
HANDLE copyFile_2threads::m_CreateThread(void* pcallback, void* param, DWORD *pId)
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
  return hThread;
}
//+++++++methods
copyFile_2threads::copyFile_2threads()
{
  InitializeCriticalSection(&mCopyingInfo.lock);
  InitializeCriticalSection(&mThreadLock);
}
copyFile_2threads::~copyFile_2threads()
{
  DeleteCriticalSection(&mCopyingInfo.lock);
  DeleteCriticalSection(&mThreadLock);
}
int copyFile_2threads::copyInit(copyData* p, TCHAR* srcFile, TCHAR* desFile)
{
  LARGE_INTEGER fileSize;
  int rc = 4; //4 steps

  //(0)
  //+ init ringMem,
  mCopyingInfo.mem.Reset(p->buff, p->blockSize);
  //+ init file info
  mFileIn.zName = p->zFin;
  mFileOut.zName = p->zFout;
  mReadBlockSize = p->pgSize;
  mWriteBlockSize = p->pgSize;
  mCopyingInfo.isReadingComplete = 0;
  mCopyingInfo.readSize = 0;
  mCopyingInfo.writeSize = 0;
  //+ init copyData
  _tcscpy_s(p->zFin, srcFile);
  _tcscpy_s(p->zFout, desFile);
  p->flags = 0;
  p->fin = 0;
  p->fout = 0;

  do {
    //(1) open fin
    mFileIn.hFile = CreateFile(
      mFileIn.zName,
      GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      0,
      OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL,
      0
      );
    if (mFileIn.hFile == INVALID_HANDLE_VALUE) break;
    
    rc--;
    //init file size, size copied
    fileSize.LowPart = GetFileSize(mFileIn.hFile, (LPDWORD)&fileSize.HighPart);
    mCopyingInfo.copySize = fileSize.QuadPart;

    //(2) create fout
    mFileOut.hFile = CreateFile(
      mFileOut.zName,
      GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      0,
      CREATE_NEW,
      FILE_ATTRIBUTE_NORMAL,
      0
      );
    if (mFileOut.hFile == INVALID_HANDLE_VALUE) break;
    rc--;

    //(3) create read thread
    mReadThreadHandle = m_CreateThread(
      ReadThreadMain_callback,
      this,
      &mReadThreadId
      );
    if (mReadThreadHandle == INVALID_HANDLE_VALUE) break;
    rc--;

    //(4) create write thread
    mWriteThreadHandle = m_CreateThread(
      WriteThreadMain_callback,
      this,
      &mWriteThreadId
      );
    if (mWriteThreadHandle == INVALID_HANDLE_VALUE) break;
    rc--;

    //update copy data
    p->flags = p->initialized;
    p->size = mCopyingInfo.copySize;
    p->copied = 0;
  } while (FALSE);

  switch(rc) {
    case 1: //(4) create write thread fail  - rollback (3)
      m_TerminateThread(mReadThreadHandle, 0);
      //fall through
    case 2: //(3) create read thread fail   - rollback (2)
      CloseHandle(mFileOut.hFile);
      //fall through
    case 3: //(2) create fout fail          - rollback (1)
      CloseHandle(mFileIn.hFile);
      //fall through
    default://all done or nothing done
      break;
  }

  return rc;
}
int copyFile_2threads::copyFirst(copyData*p)
{
  assert(p->flags == p->initialized);
  mStateFlags = readingStarted | writingStarted;
  ResumeThread(mReadThreadHandle);
  ResumeThread(mWriteThreadHandle);
  p->flags = p->copying;
  return 0;
}
//access copying info
int copyFile_2threads::copyNext(copyData*p)
{
  assert(p->flags == p->copying);
  Sleep(500);
  //get copy status
  mCopyingInfo.getWriteSize(&p->copied);
  if (p->copied == p->size) {
    //completed
    p->flags = p->completed;
    return -1;
  }
  return 0;
}
int copyFile_2threads::copyFinal(copyData*p)
{
  assert(mStateFlags & readingComplete);
  assert(mStateFlags & writingComplete);
  assert(p->flags == p->completed);
  //(4) terminate write thread
  assert(mWriteThreadHandle != INVALID_HANDLE_VALUE);
  m_TerminateThread(mWriteThreadHandle, 0);
  //(3) terminate read thread
  assert(mReadThreadHandle != INVALID_HANDLE_VALUE);
  m_TerminateThread(mReadThreadHandle, 0);
  //(2) close fout
  assert(mFileOut.hFile != INVALID_HANDLE_VALUE);
  CloseHandle(mFileOut.hFile);
  truncate(mFileOut.zName, mCopyingInfo.writeSize);
  //(1) close fin
  assert(mFileIn.hFile != INVALID_HANDLE_VALUE);
  CloseHandle(mFileIn.hFile);
  //truncate fout
  return 0;
}
//+++++++
int copyFile_2threads::ReadThreadMain()
{
  int count;
  char* data;

#if (!USE_LOCATE)
  data = new char[mReadBlockSize];
#endif

  for (;;) {
    threadLockEnter();
    count = mCopyingInfo.mem.Remain();
    threadLockLeave();

    if (count < mReadBlockSize) {
      Sleep(0);
      continue;
    }

#if (USE_LOCATE)
    threadLockEnter();
    mCopyingInfo.mem.Allocate(&data, mReadBlockSize, &count);
    assert(count == mReadBlockSize);
    threadLockLeave();
#endif

    ReadFile(mFileIn.hFile, data, (DWORD)mReadBlockSize, (DWORD*)&count, 0);
    mCopyingInfo.readSize += count;

#if (USE_LOCATE)
    threadLockEnter();
    mCopyingInfo.mem.Commit(count, &count);
    threadLockLeave();
#endif
#if (!USE_LOCATE)
    threadLockEnter();
    mCopyingInfo.mem.Write(data, count, &count);
    mCopyingInfo.readSize += count;
    threadLockLeave();
#endif

    if (count < mReadBlockSize) break;
  }

  threadLockEnter();
  mCopyingInfo.isReadingComplete = 1;
  threadLockLeave();

  mStateFlags &= ~(readingStarted);
  mStateFlags |= readingComplete;

#if (!USE_LOCATE)
  delete data;
#endif

  return 0;
}
int copyFile_2threads::WriteThreadMain()
{
  int count;
  int nWrite;
  int isReadingCompleted;
  char* data;

#if (!USE_LOCATE)
  data = new char[mWriteBlockSize];
#endif

  //start write
  for (;;) {
    threadLockEnter();
    count = mCopyingInfo.mem.Count();
    isReadingCompleted = mCopyingInfo.isReadingComplete;
    threadLockLeave();

    if (count == 0) {
      if (isReadingCompleted) {
        break;
      }
      Sleep(0);
      continue;
    }

#if (USE_LOCATE)
    threadLockEnter();
    mCopyingInfo.mem.Locate(0, &data, mWriteBlockSize, &count);
    threadLockLeave();
#endif
#if (!USE_LOCATE)
    threadLockEnter();
    mCopyingInfo.mem.Read(data, mWriteBlockSize, &count);
    threadLockLeave();
#endif

    WriteFile(mFileOut.hFile, data, (DWORD)mWriteBlockSize, (DWORD*)&nWrite, 0);
    assert(nWrite == mWriteBlockSize);

    mCopyingInfo.incWriteSize(count);

#if (USE_LOCATE)
    threadLockEnter();
    mCopyingInfo.mem.Discard(count, &count);
    threadLockLeave();
#endif
  }
  //update state
  mStateFlags &= ~(writingStarted);
  mStateFlags |= writingComplete;

#if (!USE_LOCATE)
  delete data;
#endif

  return 0;
}
#endif //FILEMNG_2THREADS

int copy_use_stream(const std::wstring& localFileName, const std::wstring& serverFileName)
{
	{
		std::ifstream	currentSrcFile;
		std::ofstream	currentDestFile;
		ULONGLONG		iCompletedSize = 0;
		char*			pBuffer;
		int				iBlockSize = 0;
		ULONGLONG		iLength = 0;
	
		currentSrcFile.open(localFileName.c_str(), std::ios_base::in | std::ios::binary);
		currentDestFile.open(serverFileName.c_str(), std::ios_base::out | std::ios::binary | std::ios::trunc);
		currentSrcFile.seekg (0, std::ios::end);
		iLength = currentSrcFile.tellg();
		currentSrcFile.seekg (0, std::ios::beg);
    const int SYNC_DATA_SIZE = 512*1024;
		pBuffer = new char[SYNC_DATA_SIZE];
		while((iLength > iCompletedSize))
		{
			iBlockSize = (int)min(SYNC_DATA_SIZE, (iLength - iCompletedSize));
			if(iBlockSize > 0)
			{
				currentSrcFile.read(&pBuffer[0], iBlockSize);
				currentDestFile.write(&pBuffer[0], iBlockSize);
				iCompletedSize += iBlockSize;
			}
		}
		currentSrcFile.close();
		currentDestFile.close();
		delete[] pBuffer;

		return 0;
	}
}

int copy_use_crtfile(TCHAR *srcFile, TCHAR* desFile)
{
  enum {bufferSize = 512*1024};
  char buff[bufferSize];
  HANDLE fin, fout;
  union {
    struct {
      DWORD size_low;
      DWORD size_high;
    };
    long long size;
  };
  int rc = 0;
  DWORD n;
  fin = CreateFile(
    srcFile,
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    0,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    0
    );
  if (fin != INVALID_HANDLE_VALUE) {
    //get size of fin
    size_low = GetFileSize(fin, &size_high);
    //create fout
    fout = CreateFile(
      desFile,
      GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      0,
      CREATE_NEW,
      FILE_ATTRIBUTE_NORMAL,
      0
      );
    if (fout != INVALID_HANDLE_VALUE) {
      //start copy
      for(n = bufferSize; n == bufferSize; size -= n) {
        rc = ReadFile(fin, buff, bufferSize, &n, 0);
        assert(rc);
        rc = WriteFile(fout, buff, n, &n, 0);
        assert(rc);
      }
      assert(size == 0);
      CloseHandle(fout);
      CloseHandle(fin);
    }
    else {
      assert(0);
      rc = GetLastError();
      CloseHandle(fin);
    }
  }
  else {
    assert(0);
    rc = GetLastError();
  }
  return 0;
}
int truncate(TCHAR* zFile, long long size)
{
  int rc;
  LARGE_INTEGER li;
  li.QuadPart = size;
  HANDLE hf = CreateFile(
    zFile,
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    0,
    OPEN_ALWAYS,
    FILE_ATTRIBUTE_NORMAL,
    0
    );
  if (hf != INVALID_HANDLE_VALUE) {
    rc = SetFilePointer(hf, li.LowPart, &li.HighPart, FILE_BEGIN);
    if (rc == size) {
      rc = SetEndOfFile(hf);
      assert(rc);
      rc = 0;
    }
    else {
      assert(0);
      rc = GetLastError();
    }
  } else {
    assert(0);
    rc = GetLastError();
  }
  return rc;
}
int copy_use_crtfile_flagNoBuffer(TCHAR *srcFile, TCHAR* desFile)
{
  enum {bufferSize = 512*1024};
  char *buff = new char[bufferSize];
  HANDLE fin, fout;
  union {
    struct {
      DWORD size_low;
      DWORD size_high;
    };
    long long size;
  };
  int rc = 0;
  DWORD nRead, nWrite;
  long long copied;
  fin = CreateFile(
    srcFile,
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    0,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
    0
    );
  if (fin != INVALID_HANDLE_VALUE) {
    //get size of fin
    size_low = GetFileSize(fin, &size_high);
    //create fout
    fout = CreateFile(
      desFile,
      GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      0,
      CREATE_NEW,
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
      0
      );
    if (fout != INVALID_HANDLE_VALUE) {
      //start copy
      for(nRead = bufferSize, copied = 0; nRead == bufferSize; copied += nRead) {
        rc = ReadFile(fin, buff, bufferSize, &nRead, 0);
        assert(rc);
        rc = WriteFile(fout, buff, bufferSize, &nWrite, 0);
        assert(rc);
      }
      assert(size == copied);
      CloseHandle(fout);
      CloseHandle(fin);
      truncate(desFile, size);
    }
    else {
      assert(0);
      rc = GetLastError();
      CloseHandle(fin);
    }
  }
  else {
    assert(0);
    rc = GetLastError();
  }
  delete buff;
  return 0;
}

//copy file test
void copyfile(TCHAR *srcFile, TCHAR* desFile)
{
  TCHAR buff[MAX_PATH];
  buff[0] = 0;
  //size(MB) 619
  //copyData
  //block pgSize    elapsed(ms)
  //1     4096      74,616 
  //4               44,257    <-best
  //8               43,743 
  //4     512       217,154 
  //1     512*1024  24,664
  //1     4*4*1024  40,077
  //1     1024*1024 24,227
  DWORD elapsed = GetTickCount();

#define test_sample_copy      (1)
#define test_copy_no_buffer   (2)
#define test_copy_with_buffer (3)
#define copy_4                (4)
#define copy_5                (5)
#define copy_2threads         (6)

#define test_content          copy_2threads
#if(test_content == test_copy_no_buffer)
  fileMng fm;
  copyData *pCD = new copyData;
  copyData &cd = *pCD;
  fm.copyFile(&cd, srcFile, desFile);
  _stprintf_s(buff, MAX_PATH, TEXT("copy %s size %I64d pgSize %d blockSize %d"),
    srcFile, cd.size, cd.pgSize, cd.blockSize);
  delete pCD;
#endif

#if (test_content == test_sample_copy)
  copy_use_stream(wstring(srcFile), wstring(desFile));
  _stprintf_s(buff, MAX_PATH, TEXT("copy sample %s"), srcFile);
#endif

#if (test_content == test_copy_with_buffer)
  fileMng_buffer fm;
  copyData *pCD = new copyData;
  copyData &cd = *pCD;
  fm.copyFile(&cd, srcFile, desFile);
  _stprintf_s(buff, MAX_PATH, TEXT("copy with buffer %s size %I64d pgSize %d blockSize %d"),
    srcFile, cd.size, cd.pgSize, cd.blockSize);
  delete pCD;
#endif

#if (test_content == copy_4)
  copy_use_crtfile(srcFile, desFile);
  _stprintf_s(buff, MAX_PATH, TEXT("copy use crt file %s"), srcFile);
#endif
#if (test_content == copy_5)
  copy_use_crtfile_flagNoBuffer(srcFile, desFile);
  _stprintf_s(buff, MAX_PATH, TEXT("copy use crt file flags no buffer %s"), srcFile);
#endif
#if (test_content == copy_2threads)
  copyFile_2threads copier;
  copyData *pCD = new copyData;
  int rc;
  copier.copyInit(pCD, srcFile, desFile);
  for(rc = copier.copyFirst(pCD); rc == 0; rc = copier.copyNext(pCD)) {
    Sleep(500);
  }
  _stprintf_s(buff, MAX_PATH, TEXT("copy use 2 threads %s"), srcFile);
  copier.copyFinal(pCD);
  delete pCD;
#endif

  elapsed = GetTickCount() - elapsed;
  _tprintf(TEXT("%s elapsed %d\n"), buff, elapsed);
}

void test_rm()
{
  char buff[3];
  RingMem rm(buff, 2);
  char*data;
  int n;

  rm.Allocate(&data, 1, &n);  //1.0
  assert(n == 1);
  data[0] = '1';
  rm.Commit(1, &n);           //1.1
  assert(n == 1);
  rm.Locate(0, &data, 1, &n); //2.0
  assert(n==1);
  assert(data[0] == '1');
  rm.Discard(1, &n);          //2.1
  assert(n==1);

  rm.Allocate(&data, 1, &n);
  data[0] = '2';
  rm.Commit(1, &n);

  rm.Allocate(&data, 1, &n);  //1.0
  assert(n == 1);
  data[0] = '3';
  rm.Locate(0, &data, 1, &n); //2.0
  assert(n==1);
  assert(data[0] == '2');
  rm.Discard(1, &n);          //2.1
  assert(n==1);
  rm.Commit(1, &n);           //1.2
  assert(n==1);

  rm.Locate(0, &data, 1, &n);
  assert(data[0] == '3');

  rm.Allocate(&data, 1, &n);  //1.0
  assert(n == 1);
  data[0] = '4';
  rm.Locate(0, &data, 1, &n); //2.0
  assert(n==1);
  assert(data[0] == '3');
  rm.Commit(1, &n);           //1.1
  assert(n==1);
  rm.Locate(1, &data, 1, &n);
  assert(data[0] == '4');
  rm.Discard(1, &n);          //2.1
  assert(n==1);

  rm.Locate(0, &data, 1, &n); //2.0
  assert(data[0] == '4');
  rm.Allocate(&data, 1, &n);  //1.0
  assert(n==1);
  data[0] = '5';
  rm.Commit(1, &n);           //1.1
  assert(n == 1);
  rm.Discard(1, &n);          //2.1
  assert(n==1);

  rm.Locate(0, &data, 1, &n); //2.0
  assert(data[0] == '5');
  rm.Allocate(&data, 1, &n);  //1.0
  assert(n==1);
  data[0] = '6';
  rm.Discard(1, &n);          //2.1
  assert(n==1);
  rm.Commit(1, &n);           //1.1
  rm.Locate(0, &data, 1, &n);
  assert(data[0] == '6');

}

int _tmain(int argc, TCHAR* argv[])
{
  int rc = 3;

  do {
    if (argc < 3) break;
    rc--;
    TCHAR* srcFile = argv[1];
    TCHAR* desFile = argv[2];
    DWORD attb = GetFileAttributes(srcFile);
    if (attb == INVALID_FILE_ATTRIBUTES) break;
    rc--;
    if (attb & FILE_ATTRIBUTE_DIRECTORY) break;
    rc--;
    copyfile(srcFile, desFile);
  } while (FALSE);

  if (rc) _tprintf(TEXT("copy <srcfile> <desfile>"));

  return 0;
}
