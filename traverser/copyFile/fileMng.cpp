#include <crtdbg.h>
#define assert(x) _ASSERT(x)
#include <tchar.h>
#include <Windows.h>

#include "fileMng.h"

//-----------------file--------------------------
#ifdef _FILE_MNG_
//+++++++methods
//if create directory success or directory already exist,
//  func return 0
//else func return error code
int fileMng::copyDir(TCHAR* zFile)
{
  int rc = 0;
  rc = CreateDirectory(zFile, 0);
  if (rc) {
    rc = 0;
  } else {
    rc = GetLastError();
    if (rc == ERROR_ALREADY_EXISTS) {
#if (AUTO_CLEAR)
      //clear all file in folder
#endif
      rc = 0;
    }
  }
  return rc;
}
//if success func return 0
//else return error code
int fileMng::copyFile(TCHAR* srcFile, TCHAR* desFile)
{
  copyData cd = {0};
  return copyFile(&cd, srcFile, desFile);
}
int fileMng::copyFile(copyData *pData, TCHAR* srcFile, TCHAR* desFile)
{
  copyData &cd = *pData;
  _tcscpy_s(cd.zFin, srcFile);
  _tcscpy_s(cd.zFout, desFile);
  for(int rc = copyFirst(&cd); rc == 0;) {
    //do something
    rc = copyNext(&cd);
  }
  return 0;
}
//if success open fin fout, func return 0
//else func return error code
int fileMng::copyFirst(copyData*p)
{
  int rc = 0;
  p->flags = p->initialized;
  p->fin = CreateFile(
    p->zFin,
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    0,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
    0
    );
  if (p->fin != INVALID_HANDLE_VALUE) {
    //get size of fin
    p->size_low = GetFileSize(p->fin, &(p->size_high));
    //create fout
    p->fout = CreateFile(
      p->zFout,
      GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      0,
      CREATE_NEW,
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
      0
      );
    if (p->fout == INVALID_HANDLE_VALUE) {
      assert(0);
      rc = GetLastError();
      CloseHandle(p->fin);
    }
  }
  else {
    assert(0);
    rc = GetLastError();
  }
  return rc;
}
//if success return 0
//NOTE: if copy complete, return -1
int fileMng::copyNext(copyData*p)
{
  DWORD count = 0;
  DWORD n;
  enum {endOfFile = 0x01};
  int flags = 0;
  int rc;
  for(;count < copyData::blockSize;) {
    rc = ReadFile(p->fin, p->buff + count, copyData::pgSize, &n, 0);
    assert(rc);
    count += n;
    if (n < copyData::pgSize) {
      flags |= endOfFile;
      break;
    }
  }
  if (count) {
    n = (count + copyData::blockSize - 1) & (~(copyData::blockSize - 1));
    rc = WriteFile(p->fout, p->buff, n, &n, 0);
    if (rc) {
      p->copied += count;
      rc = 0;
    } else {
      rc = GetLastError();
    }
  }
  if (flags & endOfFile) {
    rc = SetFileValidData(p->fin, p->size);
    if(!rc) {
      rc = GetLastError();
    }
    rc = copyClose(p);
    assert(rc == 0);
    //truncate file
    rc = truncate(p->zFout, p->size);
    assert(rc == 0);
    //copy complete
    rc = copyComplete;
  }
  return rc;
}
int fileMng::copyClose(copyData*p)
{
  int rc = CloseHandle(p->fin);
  assert(rc);
  rc = CloseHandle(p->fout);
  assert(rc);
  return !rc;
}
int fileMng::truncate(TCHAR* zFile, long long size)
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
#endif //_FILE_MNG_
//-----------------file buffer-------------------
#ifdef _FILE_MNG_BUFFER_
int fileMng_buffer::copyFile(copyData *pData, TCHAR* srcFile, TCHAR* desFile)
{
  copyData &cd = *pData;
  _tcscpy_s(cd.zFin, srcFile);
  _tcscpy_s(cd.zFout, desFile);
  for(int rc = copyFirst(&cd); rc == 0;) {
    //do something
    rc = copyNext(&cd);
  }
  return 0;
}
//if success open fin fout, func return 0
//else func return error code
int fileMng_buffer::copyFirst(copyData*p)
{
  int rc = 0;
  p->flags = p->initialized;
  p->fin = CreateFile(
    p->zFin,
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    0,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    0
    );
  if (p->fin != INVALID_HANDLE_VALUE) {
    //get size of fin
    p->size_low = GetFileSize(p->fin, &(p->size_high));
    //create fout
    p->fout = CreateFile(
      p->zFout,
      GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      0,
      CREATE_NEW,
      FILE_ATTRIBUTE_NORMAL,
      0
      );
    if (p->fout == INVALID_HANDLE_VALUE) {
      assert(0);
      rc = GetLastError();
      CloseHandle(p->fin);
    }
  }
  else {
    assert(0);
    rc = GetLastError();
  }
  return rc;
}
//if success return 0
//NOTE: if copy complete, return -1
int fileMng_buffer::copyNext(copyData*p)
{
  DWORD count = 0;
  DWORD n;
  enum {endOfFile = 0x01};
  int flags = 0;
  int rc;
  for(;count < copyData::blockSize;) {
    rc = ReadFile(p->fin, p->buff + count, copyData::pgSize, &n, 0);
    assert(rc);
    count += n;
    if (n < copyData::pgSize) {
      flags |= endOfFile;
      break;
    }
  }
  if (count) {
    n = count;
    rc = WriteFile(p->fout, p->buff, n, &n, 0);
    if (rc) {
      p->copied += count;
      rc = 0;
    } else {
      rc = GetLastError();
    }
  }
  if (flags & endOfFile) {
    rc = copyClose(p);
    assert(rc == 0);
    //copy complete
    rc = copyComplete;
  }
  return rc;
}
#endif _FILE_MNG_BUFFER_
