#include <crtdbg.h>
#define assert(x) _ASSERT(x)

#include "tree.h"
#include "ringmem.h"
#include "qsMem.h"

#include <stdio.h>
#include <Windows.h>
#include <vector>

#define ALIGNBY4(size)        (((size) + 3) & (~3))
#define zEndNode              TEXT(".")
#define zRootNode             TEXT("Root")

#define ALIGNBY8(x)           (((x) + 0x07) & ~0x07)
#define ALIGNEDBY8(x)         (((x) & 0x07) == 0x00)
//[LOG]
#define LOG_Write(...) _ftprintf(fLog, __VA_ARGS__);

//-----------------node--------------------------
#ifdef _NODE_MNG_
//structure
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
struct node :public nodeHdr
{
  enum {
    DATA_COUNT = 256,
  };
  TCHAR data[DATA_COUNT];
};
//methods
int nodeMng::nodeAlignSize(nodeHdr* pHdr)
{
  return ALIGNBY4(pHdr->size + sizeof(nodeHdr));
}

//if success func return size of node
//else func return 0
int nodeMng::nodePop(MemMngIF* pmm, node* pNode)
{
  int n = 0;
  MemMngIF &mm = *pmm;
  nodeHdr hdr;
  //case explorer is slower than exporter
  enum { timeOut = 1, nRetry = 16};
  for (int i=0; (i<nRetry) && (!mm.Count()); Sleep(timeOut));
  assert(mm.Count());
  mm.Peek(0, (char*)&hdr, sizeof(nodeHdr), &n);
  if (n == 0) {
    //do nothing
  } else {
    assert(n == sizeof(nodeHdr));
    n += hdr.size;
    if (pNode) {
      mm.Peek(0, (char*)pNode, n, &n);
    }
    n = ALIGNBY4(n);
    assert(n == nodeAlignSize(&hdr));
    mm.Discard(n, &n);
  }
  return n;
}
//if success func return size of node
//else if error func return 0
int nodeMng::nodePush(MemMngIF* pmm, node* pNode)
{
  MemMngIF &mm = *pmm;
  assert(mm.Remain() != 0);
  //int n = ALIGNBY4(pNode->size + sizeof(nodeHdr));
  int n = nodeAlignSize(pNode);
  mm.Write((char*)pNode, n, &n);
  return n;
}
int nodeMng::nodeFormat(node* pNode, int nodeId, TCHAR *zName, int isDir)
{
  pNode->id = nodeId;

  //set flags
  if (isDir)
    pNode->flags = node::isNode;
  else
    pNode->flags = node::isLeaf;

  //set data
  int n;
  for (n = 0; zName[n] != TEXT('\0'); n++)
  {
    pNode->data[n] = zName[n];
  }
  //data contain '\0' terminate
  pNode->data[n] = 0;
  n++;

  //set data size in byte
  n*=sizeof(TCHAR);
  pNode->size = n;

  //align size
  n = ALIGNBY4(n + sizeof(nodeHdr));
  assert(n == nodeAlignSize(pNode));
  return n;
}
int nodeMng::nodePush(MemMngIF* pStack, int* in)
{
  int n;
  MemMngIF &mm = *pStack;
  mm.Write((char*)in, sizeof(int), &n);
  return n;
}
int nodeMng::nodePop(MemMngIF* pStack, int* out)
{
  int n;
  MemMngIF &mm = *pStack;
  mm.Read((char*)out, sizeof(int), &n);
  return n;
}
#endif //_NODE_MNG_
//-----------------path--------------------------
#ifdef _PATH_MNG_
//PARAMS
//  __in count: size of buff in TCHAR
int pathMng::pathPush(TCHAR* buff, int count, TCHAR* zElement)
{
  int len = _tcslen(buff);
  if (len > 0) {
    buff[len] = TEXT('\\');
    len++;
  }
  len += _stprintf_s(buff + len, count - len, TEXT("%s"), zElement);
  return 0;
}
int pathMng::pathPop(TCHAR* buff)
{
  int len = _tcslen(buff);
  //buff = "C:\\folder"
  //           ^-len
  while(buff[len] != TEXT('\\')) {
    len--;
  }
  assert(len >= 0);
  buff[len] = 0;
  return 0;
}
#endif //_PATH_MNG_
//-----------------file--------------------------
#ifdef _FILE_COPY_
#define ALIGN_BY_PGSIZE(x) (((x)+(copyData::copyDataPgSize-1))&(~(copyData::copyDataPgSize-1)))
struct copyData {
  // + state
  enum {
    copyDataInitialized = 1,
    copyDataCopying,
    copyDataCompleted,
  };
  int flags;
  // + fin
  union {
    struct {
      DWORD size_low;
      DWORD size_high;
    };
    long long size;
  };
  HANDLE  fin;
  TCHAR   zFin[MAX_PATH];
  // + fout
  long long copied;
  HANDLE  fout;
  TCHAR   zFout[MAX_PATH];
  // + copy file handle - encrypt/decrypt
  //fileCopy_IF* hCopier;
  enum {
    copyDataEncryptMode,
    copyDataDecryptMode,
  } cryptMode;
  // + buffer
  enum {
    copyDataPgSize    = 4*1024,
    copyDataBlockSize = 512*1024,
  };
  char buff[copyDataBlockSize];
};
//+++++++methods
//if create directory success or directory already exist,
//  func return 0
//else func return error code
int fileCopy::copyDir(TCHAR* zFile)
{
  int rc = 0;
  rc = CreateDirectory(zFile, 0);
  if (rc) {
    rc = 0;
  } else {
    rc = GetLastError();
    if (rc == ERROR_ALREADY_EXISTS) {
      rc = 0;
    } else {
      assert(0);
      LOG_ERROR(TEXT("copyDir fail zDir %s rc %d"), zFile, rc);
      rc = -1;
    }
  }
  return rc;
}
//if success func return 0
//else return error code
int fileCopy::copyFile(TCHAR* srcFile, TCHAR* desFile)
{
  copyData* pData = new copyData;
  int rc = copyFile(pData, srcFile, desFile);
  delete pData;
  return rc;
}
int fileCopy::copyFile(copyData* pCopyData, TCHAR* srcFile, TCHAR* desFile)
{
  copyData& _copyData = *pCopyData;

  _tcscpy_s(_copyData.zFin, srcFile);
  _tcscpy_s(_copyData.zFout, desFile);
  for(int rc = copyFirst(&_copyData); rc == 0;) {
    //do something
    rc = copyNext(&_copyData);
    if (_copyData.flags == copyData::copyDataCompleted) break;
  }
  return 0;
}
//if success open fin fout, func return 0
//else func return error code
int fileCopy::copyFirst(copyData* pCopyData)
{
  copyData& _copyData = *pCopyData;

  int rc = 2; //2 steps
  do {
    //(1) open fin
    _copyData.fin = CreateFile(
      _copyData.zFin,
      GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      0,
      OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
      0
      );
    if (_copyData.fin == INVALID_HANDLE_VALUE) {
      LOG_ERROR(TEXT("copyFirst open file fail zFin %s err %d"), _copyData.zFin, GetLastError());
      break;
    }
    // + get size of fin
    _copyData.size_low = GetFileSize(_copyData.fin, &(_copyData.size_high));
    rc--;

    //(2) create fout
    _copyData.fout = CreateFile(
      _copyData.zFout,
      GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      0,
      CREATE_NEW,
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
      0
      );
    if (_copyData.fout == INVALID_HANDLE_VALUE) {
      LOG_ERROR(TEXT("copyFirst crt file fail zFout %s err %d"), _copyData.zFout, GetLastError());
      break;
    }
    // + update flags
    _copyData.flags = copyData::copyDataCopying;
    rc--;
  } while (FALSE);

  switch (rc) {
    case 1:   //(2) create fout error  - rollback (1)
      assert(0);
      CloseHandle(_copyData.fin);
      //fall through
    default:  // all done or nothing done
      break;
  }

  return rc;
}
//if success return 0
//NOTE: if copy complete, return -1
int fileCopy::copyNext(copyData* pCopyData)
{
  copyData& _copyData = *pCopyData;
  char* buff = _copyData.buff;

  DWORD nRead;
  DWORD nWrite;
  BOOL  isEndOfFile;
  int   rc;
  int   nDone = 0;

  do {
    //(1) read data to buff
    rc = ReadFile(_copyData.fin, buff, copyData::copyDataBlockSize, &nRead, 0);
    if (!rc) {
      LOG_ERROR(TEXT("copyNext read fail zFin %s err %d"), _copyData.zFin, GetLastError());
      break;
    }
    isEndOfFile = (nRead < copyData::copyDataBlockSize);
    nDone = 1;

    //(2) write data to file
    // + need align by pg size
    if (nRead) {
      nWrite = ALIGN_BY_PGSIZE(nRead);
      rc = WriteFile(_copyData.fout, buff, nWrite, &nWrite, 0);
    }
    if (!rc) {
      LOG_ERROR(TEXT("copyNext write fail zFout %s err %d"), _copyData.zFout, GetLastError());
      break;
    }
    // + update copy data
    _copyData.copied += nRead;
    nDone = 2;

    //(3) if eof close file handle + truncate file
    if (!isEndOfFile) break;
    // + close file handle
    rc = copyClose(&_copyData);
    assert(rc == 0);
    // + truncate file
    rc = truncate(_copyData.zFout, _copyData.size);
    assert(rc == 0);
    // + update flags
    _copyData.flags = copyData::copyDataCompleted;
    nDone = 3;
  } while (FALSE);

  switch (nDone) {
    case 0: //(1) read data fail
    case 1: //(2) write data fail
      assert(0);
      break;
    case 2: //not eof
    case 3: //is eof
      rc = 0;
      break;
  }

  return rc;
}
int fileCopy::copyClose(copyData*pCopyData)
{
  copyData& _copyData = *pCopyData;

  int rc = CloseHandle(_copyData.fin);
  assert(rc);
  rc = CloseHandle(_copyData.fout);
  assert(rc);
  return !rc;
}
int fileCopy::truncate(TCHAR* zFile, long long size)
{
  int rc = 1;

  do {
    // + open file
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
    if (hf == INVALID_HANDLE_VALUE) {
      LOG_ERROR(TEXT("truncate open fail zFile %s err %d"), zFile, GetLastError());
      break;
    }
    rc = 0;
    // + set end of file
    SetFilePointer(hf, li.LowPart, &li.HighPart, FILE_BEGIN);
    SetEndOfFile(hf);
    CloseHandle(hf);
  } while (FALSE);

  return rc;
}
fileCopy::~fileCopy(){}
//-----------------big file
//return 0 if success
//NOTE:
//+ before call initHdr, loadHdr, saveHdr
//  host file |   prev subfile EOF  |
//                                  ^-curPos
//+ after call
//  host file |                     |hdr  |
//                                        ^curPos
int bigFile::initHdr(BOOL isHost) {
  file_info& fi = isHost?mHostFile:mSubFile;
  file_hdr& hdr = *(file_hdr*)mBuff;
  HANDLE& hFile = mHostFile.hFile;
  long long& curPos = mHostFile.curPos;

  do {
    //init hdr
    DWORD nWrite;

    if (getPos() != curPos) break;

    hdr.offset = fi.offset;
    hdr.size = fi.size;
    hdr.zName[0] = 0;
    int rc = ::WriteFile(hFile, &hdr, pgSize, &nWrite, 0);
    if (!rc) break;
    if (nWrite != pgSize) break;
    curPos += nWrite;

    return 0;
  } while (FALSE);
  return -1;
}
int bigFile::loadHdr(BOOL isHost) {
  file_info& fi = isHost?mHostFile:mSubFile;
  file_hdr& hdr = *(file_hdr*)mBuff;
  HANDLE& hFile = mHostFile.hFile;
  long long& curPos = mHostFile.curPos;

  do {
    DWORD nRead;

    LARGE_INTEGER li;
    li.LowPart = GetFileSize(hFile, (DWORD*)&li.HighPart);
    assert(!(li.QuadPart < pgSize));
    assert(getPos() == curPos);

    int rc = ::ReadFile(hFile, &hdr, pgSize, &nRead, 0);
    if (!rc) break;
    if (nRead != pgSize) break;
    curPos += nRead;

    fi.size = hdr.size;
    fi.offset = hdr.offset;
    //_tcscpy_s(fi.zName, hdr.zName);
    assert(_tcscmp(fi.zName, hdr.zName) == 0);

    return 0;
  } while (FALSE);
  return -1;
}
int bigFile::saveHdr(BOOL isHost) {
  file_info& fi = isHost?mHostFile:mSubFile;
  file_hdr& hdr = *(file_hdr*)mBuff;
  HANDLE& hFile = mHostFile.hFile;
  long long& curPos = mHostFile.curPos;

  do {
    DWORD nWrite;

    assert(curPos == (fi.offset - pgSize));
    if (getPos() != curPos) break;

    //write hdr
    hdr.offset = fi.offset;
    hdr.size = fi.size;
    _tcscpy_s(hdr.zName, fi.zName);
    int rc = ::WriteFile(hFile, &hdr, pgSize, &nWrite, 0);
    if (!rc) break;
    if (nWrite != pgSize) break;

    return 0;
  } while (FALSE);
  return -1;
}
//NOTE
// before call allocate, locate
//host  |   prev sub file EOF |
//                            ^-curPos
// after call
//host  |   prev sub file EOF |hdr  |
//                                  ^-curPos
int bigFile::allocateSubFile()
{
  file_info& fi = mSubFile;
  long long& curPos = mHostFile.curPos;

  assert(curPos == getPos());
  fi.curPos = 0;
  fi.offset = curPos + pgSize;
  fi.size = 0;
  int rc = initHdr(FALSE);
  return rc;
}
int bigFile::locateSubFile()
{
  file_info& fi = mSubFile;
  long long& curPos = mHostFile.curPos;

  assert(curPos == getPos());
  fi.curPos = 0;
  int rc = loadHdr(FALSE);
  return rc;
}
//NOTE
// before call commit, discard
//host  |   cur  sub file EOF |
//                            ^-curPos
// after call
//host  |   cur  sub file EOF |
//                            ^-curPos
int bigFile::commitSubFile()
{
  file_info& fi = mSubFile;
  long long& curPos = mHostFile.curPos;

  assert(curPos == getPos());
  do {
    int rc;
    long long oldPos = curPos;

    curPos = fi.offset - pgSize;
    setPos(curPos);
    rc = saveHdr(FALSE);
    if (rc) break;

    curPos = oldPos;
    setPos(curPos);
    assert((fi.offset + align(fi.size)) == curPos);
    mHostFile.size = curPos;

    return 0;
  } while (FALSE);
  return -1;
}
int bigFile::discardSubFile()
{
  return 0;
}
int bigFile::setPos(long long pos)
{
  LARGE_INTEGER li;
  li.QuadPart = pos;
  li.LowPart = SetFilePointer(mHostFile.hFile, li.LowPart, &li.HighPart, FILE_BEGIN);
  return 0;
}
long long bigFile::getPos()
{
  long long pos = 0;
  *(DWORD*)&pos = SetFilePointer(mHostFile.hFile, 0, 1 + (PLONG)&pos, FILE_CURRENT);
  return pos;
}
long long bigFile::align(long long size)
{
  return ((size + pgSize-1) & ~(pgSize-1));
}
BOOL bigFile::aligned(long long size) {
  return (size & (pgSize-1)) == 0;
}

bigFile::bigFile(TCHAR* zName, BOOL isExport)
{
  int nDone = 0;
  int rc;

  mFlags = none;
  do {
    //(1)
    //save mode
    mFlags |= isExport?exportMode:0;
    //save host file name
    _tcscpy_s(mHostFile.zName, zName);
    //open host file
    DWORD crtFlags = isExport?CREATE_NEW:OPEN_EXISTING;
    mHostFile.hFile = ::CreateFile(
      mHostFile.zName,
      GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      0,
      crtFlags,
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
      0
      );
    if (mHostFile.hFile == INVALID_HANDLE_VALUE) break;
    nDone++;

    //(2)
    //init host file
    if (isExport) {
      mHostFile.curPos = 0;
      mHostFile.offset = pgSize;
      mHostFile.size = pgSize;
      rc = initHdr(TRUE);
      if (rc) break;
    } else {
      //load hdr
      mHostFile.curPos = 0;
      rc = loadHdr(TRUE);
      if (rc) break;
    }
    nDone++;

    mFlags |= initialized;
  } while (FALSE);
  assert(nDone==2);
}
bigFile::~bigFile()
{
  assert(!(mFlags & opening));
  if (mFlags & exportMode) {
    mHostFile.curPos = 0;
    setPos(0);
    saveHdr(TRUE);
  }
  CloseHandle(mHostFile.hFile);
}
HANDLE bigFile::CreateFile(TCHAR* zName)
{
  //set subfile name
  _tcscpy_s(mSubFile.zName, zName);

  int nDone = 0;
  int rc;
  do {
    //(1)
    if (mFlags & opening) break;
    nDone++;

    //(2)
    if (mFlags & exportMode) {
      rc = allocateSubFile();
    } else {
      rc = locateSubFile();
    }
    if (rc) break;
    nDone++;

    mFlags |= opening;
  } while (FALSE);

  switch (nDone) {
    case 2:   //all done
      return (HANDLE)1;
    default:  //if error
      return INVALID_HANDLE_VALUE;
  }
}
BOOL bigFile::CloseFile(HANDLE hFile)
{
  int nDone = 0;
  int rc;
  do {
    //(1)
    if (!(mFlags & opening)) break;
    nDone++;

    //(2)
    if (mFlags & exportMode) {
      rc = commitSubFile();
    } else {
      rc = discardSubFile();
    }
    if (rc) break;
    nDone++;

    mFlags &= ~opening;
  } while (FALSE);

  switch (nDone) {
    case 2:   //all done
      return TRUE;
    default:  //if error
      return FALSE;
  }
}
BOOL bigFile::ReadFile(HANDLE, char* buff, int size, int* nRead)
{
  int nDone = 0;
  int rc;
  do {
    //(1)
    if (!(mFlags & opening)) break;
    nDone++;
    assert(!(mFlags & exportMode)); //import mode
    assert(aligned(size));
    //(2)
    if (mSubFile.curPos >= mSubFile.size) break;
    long long remain = mSubFile.size - mSubFile.curPos;
    if (size > remain) size = (int)align(remain);
    rc = ::ReadFile(mHostFile.hFile, buff, size, (DWORD*)nRead, 0);
    if (!rc) break;
    assert(*nRead == size);
    mHostFile.curPos += *nRead;
    mSubFile.curPos += *nRead;
    nDone++;
    //check is eof
    remain = mSubFile.size - mSubFile.curPos;
    if (remain > 0) break;
    *nRead += (int)remain;
  } while (FALSE);

  switch (nDone) {
    case 2: //all done
      return TRUE;
    default:
      return FALSE;
  }
}
BOOL bigFile::WriteFile(HANDLE, char* data, int size, int* nWrite)
{
  int nDone = 0;
  int rc;
  do {
    //(1)
    if (!(mFlags & opening)) break;
    nDone++;
    assert(mFlags & exportMode); //import mode
    assert(aligned(size));
    //(2)
    rc = ::WriteFile(mHostFile.hFile, data, size, (DWORD*)nWrite, 0);
    if (!rc) break;
    mHostFile.curPos+=*nWrite;
    mSubFile.curPos+=*nWrite;
    nDone++;
  } while (FALSE);

  switch (nDone) {
    case 2: //all done
      return TRUE;
    default:
      return FALSE;
  }
}
//seek file from begin
BOOL bigFile::SeekFile(HANDLE, long long pos)
{
  int nDone = 0;
  do {
    //(1)
    if (!(mFlags & opening)) break;
    assert(aligned(pos));
    nDone++;

    //(2)
    //should >= 0
    if (pos < 0) break;
    LARGE_INTEGER li;
    li.QuadPart = pos + mSubFile.offset;
    //not exceed file size
    if (!(li.QuadPart < mHostFile.size)) break;
    li.LowPart = ::SetFilePointer(mHostFile.hFile, li.LowPart, &li.HighPart, FILE_BEGIN);
    mHostFile.curPos = li.QuadPart;
    mSubFile.curPos = pos;
    nDone++;
  } while (FALSE);

  switch (nDone) {
    case 2: //all done
      return TRUE;
    default:
      return FALSE;
  }
}
BOOL bigFile::TruncateFile(HANDLE, long long size)
{
  int nDone = 0;
  do {
    //(1)
    if (!(mFlags & opening)) break;
    nDone++;

    //(2)
    //should >= 0
    if (size < 0) break;
    mSubFile.size = size;

    nDone++;
  } while (FALSE);

  switch (nDone) {
    case 2: //all done
      return TRUE;
    default:
      return FALSE;
  }
}
//-----------------key mng
int encrypt(HCRYPTKEY key, void* buff, DWORD* size) {
  return CryptEncrypt(
    key,
    0, FALSE, 0,
    (BYTE*)buff,
    size,
    *size
    );
}
int decrypt(HCRYPTKEY key, void*buff, DWORD* size) {
  return CryptDecrypt(
    key,
    0, FALSE, 0,
    (BYTE*)buff,
    size
    );
}
char keyStr[] = "TEST123456789987";
HCRYPTKEY createKey(
  HCRYPTPROV hProv,
  unsigned char* keyData,
  unsigned int keyDataSize,
  unsigned char* ivData,
  unsigned int /*ivDataSize*/,
  int encryptMode
  )
{
  BOOL result = FALSE;
  HCRYPTKEY key = 0;
  // create key
  {
    const BLOBHEADER hdr = {
      PLAINTEXTKEYBLOB,
      CUR_BLOB_VERSION,
      0,
      (keyDataSize == 16) ? CALG_AES_128 :
      (keyDataSize == 24) ? CALG_AES_192 : CALG_AES_256,
    };
    const DWORD size = keyDataSize;
    const unsigned int importKeySize = sizeof(hdr) + sizeof(size) + keyDataSize;
    std::vector<unsigned char> importKey(importKeySize);

    memcpy_s(&importKey[0], sizeof(hdr), &hdr, sizeof(hdr));
    memcpy_s(&importKey[sizeof(hdr)], sizeof(size), &size, sizeof(size));
    memcpy_s(&importKey[sizeof(hdr)+sizeof(size)], keyDataSize, keyData, keyDataSize);

    result = ::CryptImportKey(hProv, reinterpret_cast<BYTE*>(&importKey[0]), importKeySize, 0, 0, &key);
  }
  // initial vector
  if(encryptMode == CRYPT_MODE_CBC)
  {
    result = ::CryptSetKeyParam(key, KP_IV, (BYTE*)ivData, 0);
  }
  // encrypt mode
  {
    DWORD mode = encryptMode;
    result = ::CryptSetKeyParam(key, KP_MODE, (BYTE*)&mode, 0);
  }
  return key;
}
//if success func return 0
//else if error func return -1
int initKey(HCRYPTKEY *pKey, HCRYPTPROV* pProv)
{
  int nDone = 0;
  do {
    //(1)
    int rc = ::CryptAcquireContext(pProv, NULL, NULL, PROV_RSA_AES, 0);
    if(!rc) {
      rc = ::CryptAcquireContext(pProv, NULL, NULL, PROV_RSA_AES, CRYPT_NEWKEYSET);
    }
    if(!rc) break;
    nDone++;
    //(2)
    unsigned char keyArr[16];
    for(int i=0; i<16; ++i)
    {
      keyArr[i] = keyStr[i];
    }
    *pKey = createKey(*pProv, keyArr, 16, NULL, 0, CRYPT_MODE_ECB);
    if(*pKey == 0) break;
    nDone++;
  } while (FALSE);
  return (nDone==2)?0:-1;
}
int finalKey(HCRYPTKEY hKey, HCRYPTPROV hProv)
{
  CryptDestroyKey(hKey);
  if(hProv) CryptReleaseContext(hProv, 0);
  return 0;
}
//-----------------copy file db
fileDbCopy::fileDbCopy(int isExport) {
	initKey(&mKey,&mProv);
	mCryptCallback = isExport?encrypt:decrypt;
}
int fileDbCopy::copyFirst(copyData* pCopyData)
{
  copyData& _copyData = *pCopyData;
  //fileDbCopy& copier = *(fileDbCopy*)_copyData.hCopier;
  //assert(this == &copier);
  char* buff = _copyData.buff;

  int nDone = 0;
  do {
    //(1) open file handle
    DWORD nRead;
    DWORD nWrite;
    int rc = fileCopy::copyFirst(&_copyData);
    if (rc != 0) break;
    assert(_copyData.flags == copyData::copyDataCopying);
    nDone++;
    //(2) check header
    if(_copyData.cryptMode == _copyData.copyDataEncryptMode) {
      mValidData = _copyData.size;
      //create hdr
      buff[0] = '0';
      *(long long*)(buff + 8) = 0;
      rc = WriteFile(_copyData.fout, buff, copyData::copyDataPgSize, &nWrite, 0);
      if (!rc) break;
      if (nWrite != copyData::copyDataPgSize) break;
    }
    else {
      assert(_copyData.cryptMode == copyData::copyDataDecryptMode);
      //read header
      rc = ReadFile(_copyData.fin, buff, copyData::copyDataPgSize, &nRead, 0);
      if (!rc) break;
      if (nRead != copyData::copyDataPgSize) break;
      //if encrypted file error
      if (buff[0] != '1') break;
      mValidData = *(long long*)(buff + 8);
      //update copied
      _copyData.copied += nRead;
    }
    nDone++;
  } while (FALSE);
  switch (nDone) {
    case 1: //step 2 fail - rollback 1
      fileCopy::copyClose(pCopyData);
      //fall through
    default:
      break;
  }
  return (nDone==2)?0:-1;
}
int fileDbCopy::copyNext(copyData* pCopyData)
{
  copyData& _copyData = *pCopyData;
  //fileDbCopy& copier = *(fileDbCopy*)_copyData.hCopier;
  //assert(this == &copier);
  char *buff = _copyData.buff;

  DWORD nRead;
  DWORD nWrite;
  BOOL  isEndOfFile;
  int   rc;
  int   nDone = 0;
  do {
    //(1)
    rc = ReadFile(_copyData.fin, buff, copyData::copyDataPgSize, &nRead, 0);
    if (!rc) break;
    isEndOfFile = (nRead < copyData::copyDataPgSize);
    nDone++;

    //(2) encrypt/decrypt
    if (nRead) {
      nWrite = copyData::copyDataPgSize;
      rc = mCryptCallback(mKey, buff, &nWrite);
      if (!rc) break;
      assert(nWrite == copyData::copyDataPgSize);
      rc = WriteFile(_copyData.fout, buff, copyData::copyDataPgSize, &nWrite, 0);
      if (!rc) break;
      if (nWrite != copyData::copyDataPgSize) break;
    }
    // + update copy data
    _copyData.copied += nRead;
    nDone++;

    //(3) file end
    if (!isEndOfFile) break;
    // + update encrypted file header
    if(_copyData.cryptMode == copyData::copyDataEncryptMode) {
      SetFilePointer(_copyData.fout, 0, 0, FILE_BEGIN);
      buff[0] = '1';
      *(long long*)(buff+8) = _copyData.size;
      rc = WriteFile(_copyData.fout, buff, copyData::copyDataPgSize, &nWrite, 0);
      if(!rc) break;
      if(nWrite != copyData::copyDataPgSize) break;
    }
    // + close handle
    rc = copyClose(&_copyData);
    assert(rc == 0);
    // + truncate decrypted file
    if (_copyData.cryptMode == copyData::copyDataDecryptMode) {
      rc = truncate(_copyData.zFout, mValidData);
      assert(rc == 0);
    }
    // + update flags
    _copyData.flags = copyData::copyDataCompleted;
    nDone++;
  } while (FALSE);

  switch (nDone) {
    case 0: //(1) read data fail
    case 1: //(2) write data fail
      assert(0);
      break;
    case 2: //not eof
    case 3: //is eof
      rc = 0;
      break;
  }

  return rc;
}
int fileDbCopy::copyFile(TCHAR* srcFile, TCHAR* desFile, bool encrypt)
{
  copyData* pData = new copyData;
  //pData->hCopier = this;
  if(encrypt) 
	  pData->cryptMode = copyData::copyDataEncryptMode;
  else 
	  pData->cryptMode = copyData::copyDataDecryptMode;
  int rc = copyFileDb(pData, srcFile, desFile);
  delete pData;
  return rc;
}
int fileDbCopy::copyFileDb(copyData* pCopyData, TCHAR* srcFile, TCHAR* desFile)
{
  copyData& _copyData = *pCopyData;

  _tcscpy_s(_copyData.zFin, srcFile);
  _tcscpy_s(_copyData.zFout, desFile);
  for(int rc = copyFirst(&_copyData); rc == 0;) {
    //do something
    rc = copyNext(&_copyData);
    if (_copyData.flags == copyData::copyDataCompleted) break;
  }
  return 0;
}
fileDbCopy::~fileDbCopy()
{
	finalKey(mKey, mProv);
}
#endif //_FILE_MNG_
//-----------------cleaner-----------------------
#ifdef _CLEANER_
//+++++++clean recursive
struct walkData
{
  int Depth;
  walkCallback* callbackObj;
};
int recursiveWalk::cleanFiles(TCHAR* zDesPath, int excludeDirLevel)
{
  class cleaner : public walkCallback
  {
  private:
    int mExcludeDirLevel;  //keep folder at special level
  public:
    cleaner(int excludeDirLevel) {
      mExcludeDirLevel = excludeDirLevel;
    }
    //+++walk callback
    //if (success remove file) func return 0
    int onMetFile(int /*level*/, TCHAR* path, TCHAR* zName) {
      int rc;
      TCHAR zFile[MAX_PATH];
      _stprintf_s(zFile, MAX_PATH, TEXT("%s\\%s"), path, zName);
      rc = DeleteFile(zFile);
      if (rc) {
        rc = 0;
      } else {
        LOG_ERROR(TEXT("cleanFiles fail zFile %s err %d"), zFile, GetLastError());
        rc = -1;
      }
      LOG_DEBUG(TEXT("[remove] File path %s rc %d"), zFile, rc);
      return rc;
    }
    //if (remove folder success) func return 0
    int onMetDir(int level, TCHAR* path, TCHAR* zName) {
      int rc = 0;
      TCHAR zDir[MAX_PATH];
      _stprintf_s(zDir, TEXT("%s\\%s"), path, zName);
      if (level > mExcludeDirLevel) {
        rc = RemoveDirectory(zDir);
        if (rc) {
          rc = 0;
        } else {
          LOG_ERROR(TEXT("cleanFiles fail zDir %s err %d"), zDir, GetLastError());
          rc = -1;
        }
        LOG_DEBUG(TEXT("[remove] remove dir path %s rc %d"), zDir, rc);
      } else {
        //keep folder at level
        LOG_DEBUG(TEXT("[remove] keep dir path %s rc %d"), zDir, rc);
      }
      return rc;
    }
  };

  walkData data;
  cleaner cln(excludeDirLevel);
  data.Depth = 0;
  data.callbackObj = &cln;

  DWORD elapsed = GetTickCount();
  int count = walkPreOrder(&data, zDesPath, 0);
  elapsed = GetTickCount() - elapsed;
  LOG_DEBUG(TEXT("[walk] path %s count %d depth %d elapsed %d"),
    zDesPath, count, 0, elapsed);

  return 0;
}

int recursiveWalk::listFiles(TCHAR* zDesPath, int* pDepth)
{
  class listFileCallback :public walkCallback
  {
  public:
    int onMetFile(int level, TCHAR* path, TCHAR* zName) {
      LOG_DEBUG(TEXT("[ls] %d zFile %s\\%s"), level, path, zName);
      return 0;
    }
    int onMetDir(int level, TCHAR* path, TCHAR* zName) {
      LOG_DEBUG(TEXT("[ls] %d zDir %s\\%s"), level, path, zName);
      return 0;
    }
  };

  walkData data;
  listFileCallback lf;
  data.Depth = 0;
  data.callbackObj = &lf;

  DWORD elapsed = GetTickCount();
  int count = walkPreOrder(&data, zDesPath, 0);
  elapsed = GetTickCount() - elapsed;
  LOG_DEBUG(TEXT("[walk] path %s count %d depth %d elapsed %d"),
    zDesPath, count, data.Depth, elapsed);

  *pDepth = data.Depth;

  return 0;
}

int recursiveWalk::Walk(TCHAR* path)
{
  walkData wd = {0};
  wd.Depth = 0;

  int elapsed = GetTickCount();
  int count = walkPreOrder(&wd, path, 0);
  elapsed = GetTickCount() - elapsed;
  LOG_DEBUG(TEXT("walk path %s count %d depth %d elapsed %d"),
    path, count, wd.Depth, elapsed);

  return 0;
}
int recursiveWalk::walkPreOrder(walkData *pData, TCHAR* path, int curLevel)
{
  walkData &wd = *pData;
  WIN32_FIND_DATA fd;
  HANDLE hFind;
  TCHAR findPath[MAX_PATH];
  TCHAR buff[MAX_PATH];
  int rc;
  int count = 0;

  _stprintf_s(findPath, MAX_PATH, TEXT("%s\\*.*"), path);
  hFind = FindFirstFile(findPath, &fd);
  if (hFind != INVALID_HANDLE_VALUE) {
    for (;;)
    {
      if (_tcscmp(fd.cFileName, TEXT(".")) == 0
        || _tcscmp(fd.cFileName, TEXT("..")) == 0
        || fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
      {
        //do nothing
      }
      else if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        //is directory, recursive find
        count++;
        _stprintf_s(buff, MAX_PATH, TEXT("%s\\%s"), path, fd.cFileName);
        //pathPush(wd.findPath, MAX_PATH, fd.cFileName);
        count += walkPreOrder(pData, buff, curLevel + 1);
        //pathPop(wd.findPath);
        //_stprintf_s(buff, MAX_PATH, TEXT("%s\\%s"), path, fd.cFileName);
        if (wd.callbackObj) wd.callbackObj->onMetDir(curLevel, path, fd.cFileName);
      }
      else
      {
        //is file,
        count++;
        //_stprintf_s(buff, MAX_PATH, TEXT("%s\\%s"), path, fd.cFileName);
        if (wd.callbackObj) wd.callbackObj->onMetFile(curLevel, path, fd.cFileName);
      }

      //find next
      rc = FindNextFile(hFind, &fd);
      if (rc) {
        //continue;
      }
      else
      {
        FindClose(hFind);
        break;
      }
    }
  }

  //update depth
  if(curLevel > wd.Depth) {wd.Depth = curLevel;}

  return count;
}
#endif //_CLEANER_
//-----------------tree--------------------------
#ifdef _TREE_MNG_
int treeMng::getId()
{
  return mQueueItemCount++;
}
//return node id
int treeMng::Init(MemMngIF* pQueue)
{
  mQueueItemCount = 0;
  mQueueHandle = pQueue;
  return 0;
}
int treeMng::Final()
{
  mQueueItemCount = -1;
  mQueueHandle = 0;
  return 0;
}
int treeMng::EnQueue(int parentId, TCHAR* zName, BOOL isDir)
{
  int rc = 0;
  node tmpNode;
  {
    rc = nodeFormat(&tmpNode, getId(), zName, isDir);
    assert(mQueueHandle->Remain() >= rc);
    tmpNode.parentId = parentId;
    rc = nodePush(mQueueHandle, &tmpNode);
    assert(rc == nodeAlignSize(&tmpNode));
    rc = tmpNode.id;
  }
  return rc;
}
//if success func return 0
//else if error func return error code
int treeMng::DeQueue(node* pNode)
{
  int size = nodePop(mQueueHandle, pNode);
  if (size)
    return 0;
  else
    return -1;
}
#endif //_TREE_MNG_
//-----------------extractor---------------------
#ifdef _EXTRACTOR_
//structure
struct ExtractData {
  enum {
    curPathSize = MAX_PATH,   //in TCHAR
  };
  int   topId;
  TCHAR curPath[curPathSize];
  node  nd;

  treeMng*  treeHandle;
  MemMngIF* stackHandle;
};
//methods
int extractor::extractFirst(ExtractData* pParams)
{
  int rc = 0;
  ExtractData &ed = *pParams;
  treeMng &tree = *ed.treeHandle;
  //init extract params
  ed.topId = tree.RootId;
  ed.curPath[0] = 0;
  //init
  rc = tree.DeQueue(&ed.nd);
  return rc;
}
int extractor::extractNext(ExtractData* pParams)
{
  int rc = 0;
  ExtractData &ed = *pParams;
  treeMng &tree = *ed.treeHandle;
  node tempNode;
  //begin loop
  //for(; rc!= -1;)
  {
    if(ed.nd.flags & node::isNode)
    {
      //if (ep.nd.id == 0) {
      //  //root folder
      //  LOG_Write(TEXT("%s\n"), ep.nd.data);
      //}
      //else {
      //  //create folder
      //  LOG_Write(TEXT("%s\\%s\n"), ep.curPath, ep.nd.data);
      //}
      //push folder
      ed.topId = ed.nd.id;
      assert(ed.topId != -1);
      pathPush(ed.curPath, ed.curPathSize, ed.nd.data);
      rc = nodePush(ed.stackHandle, &ed.nd);
      assert(rc == nodeAlignSize(&ed.nd));
    }
    else
    {
      assert(ed.nd.flags & node::isLeaf);
      ////create file
      //LOG_Write(TEXT("%s\\%s\n"), ep.curPath, ep.nd.data);
    }
    rc = tree.DeQueue(&ed.nd);

    //if end node
    if (ed.nd.parentId == tree.RootId) {
      assert(_tcscmp(ed.nd.data, zEndNode) == 0);
      //break;
      rc = -1;
    }
    else {
      assert(rc == 0);
      for(; ed.nd.parentId != ed.topId;) {
        rc = nodePop(ed.stackHandle, &tempNode);
        assert(rc == nodeAlignSize(&tempNode));
        rc = !rc; //if success rc = 0
        pathPop(ed.curPath);
        ed.topId = tempNode.parentId;
        assert(ed.topId != -1);
      }
    }
  }

  return rc;
}
#endif //_EXTRACTOR_
//-----------------traverse context--------------
#ifdef _TRAVERSE_CONTEXT_
struct traversalContext {
  TCHAR findPath[MAX_PATH];
  WIN32_FIND_DATA fd;
  HANDLE hFind;
  int curLevel;
};

int contextMng::contextPush(MemMngIF *pStack,traversalContext* in)
{
  MemMngIF &mm = *pStack;
  int n = 0;
  mm.Write((char*)in, sizeof(traversalContext), &n);
  assert(n == sizeof(traversalContext));
  return n;
}
int contextMng::contextPop(MemMngIF *pStack, traversalContext* out)
{
  MemMngIF &mm = *pStack;
  int n = 0;
  if (mm.Count() > 0) {
    mm.Read((char*)out, sizeof(traversalContext), &n);
    assert(n == sizeof(traversalContext));
  }
  return n;
}
#endif //_TRAVERSE_CONTEXT_
//-----------------traverser---------------------
#ifdef _TRAVERSER_
typedef int (*pCallback)(int , void*);
struct traversalCallback {
  //callback methods
  pCallback onGoUpBegin;
  pCallback onGoUpComplete;
  pCallback onGoDownBegin;
  pCallback onGoDownComplete;
  //params for callback
  void *pParams;
};
//int TraversalCallback::onGoUpBegin(int , void*) {return 0;}
//int TraversalCallback::onGoUpComplete(int , void*) {return 0;}
//int TraversalCallback::onGoDownBegin(int , void*) {return 0;}
//int TraversalCallback::onGoDownComplete(int , void*) {return 0;}

struct traverseData {
  enum {
    initialized = 1,
    traversing,
    completed,

    maxDepth = 16,
#if (REMOVE_DIR_LEVEL)
    error_unknown           = -1,
    error_completed_all     = 1,
    error_completed_one_dir = 2,
#endif
  };
  int flags;
  long long fileSize;
#if (REMOVE_DIR_LEVEL)
  //pCallback rmDir;
  TCHAR lastCompletedPath[MAX_PATH];
  int lastError;
#endif

  MemMngIF* stack;
  traversalContext  context;
  traversalCallback callback;
};
//get first file/folder info
//if success return 0
int traverser::traverseFirst(traverseData* p)
{
  int rc;
  traversalContext &context = p->context;
  pathPush(context.findPath, MAX_PATH, TEXT("*.*"));
  context.hFind = FindFirstFile(context.findPath, &context.fd);
  if (context.hFind != INVALID_HANDLE_VALUE) {
    //return rc = 0
    rc = 0;
  } else {
    assert(0);
    rc = GetLastError();
  }
#if (REMOVE_DIR_LEVEL)
  p->lastError = rc?p->error_unknown:0;
#endif
  return rc;
}
//filter available files
//return 0 if success
int traverser::traverseFilter(traverseData*p)
{
  int rc = 0;
  DWORD attrib = p->context.fd.dwFileAttributes;
  TCHAR* zName = p->context.fd.cFileName;
  if (attrib & (FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_SYSTEM))
    rc = attrib;
  else if (_tcscmp(zName, TEXT("."))==0 || _tcscmp(zName, TEXT(".."))==0)
    rc = -1;
  return rc;
}
//push prev result to queue
// and get next file/folder
//if success return 0
int traverser::traverseNext(traverseData* p)
{
  int rc = 0;
  int level;
  enum {
    goDown = 0x01,
    goUp   = 0x02,
    getSibling = 0x10,
  };
  int flags = 0;
  traversalContext &context = p->context;
  traversalCallback &callback = p->callback;

#if (REMOVE_DIR_LEVEL)
  assert(p->lastError != p->error_unknown);
  if (p->lastError == p->error_completed_one_dir) {
    flags |= getSibling;
  }
  else
#endif
  //enqueue prev result
  {
    if (traverseFilter(p))
    {
      //do nothing
      flags |= getSibling;
    }
    //if folder
    else if (context.fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
#if (AUTO_ENQUEUE)
      //enqueue
      p->lastId = EnQueue(context.curId, context.fd.cFileName, TRUE);
#endif
      flags |= goDown;
    }
    //if file
    else{
#if (AUTO_ENQUEUE)
      //enqueue
      p->lastId = EnQueue(context.curId, context.fd.cFileName, FALSE);
#endif
      flags |= getSibling;
    }
  }

  //go deeper
  if (flags & goDown) {
#if (REMOVE_DIR_LEVEL)
    p->lastCompletedPath[0] = 0;
#endif
    //execute callback
    if (callback.onGoDownBegin) callback.onGoDownBegin(0, (void*)p);

    level = context.curLevel + 1;
    //save parent context before go to child
    rc = contextPush(p->stack, &context);
    assert(rc == sizeof(traversalContext));
    //prepare before go to child
    //+ make findPath
    //  + remove *.*
    //  + append new element
    pathPop(context.findPath);
    pathPush(context.findPath, MAX_PATH, context.fd.cFileName);
    //+ update curContext
    //context.curId = p->lastId;
    context.curLevel = level;
    //find first file/folder in child
    pathPush(context.findPath, MAX_PATH, TEXT("*.*"));
    //memset(&context.fd, 0, sizeof(context.fd));
    context.hFind = FindFirstFile(context.findPath, &context.fd);
    if (context.hFind != INVALID_HANDLE_VALUE) {
      rc = 0;
    }
    else {
      assert(0);
      rc = GetLastError();
    }

    //execute callback
    if (callback.onGoDownComplete) callback.onGoDownComplete(0, (void*)p);
  }

  for(;;)
  {
    //get sibling
    if (flags & getSibling) {
      rc = FindNextFile(context.hFind, &context.fd);
      if (rc) {
        //success, return sibling
        rc = 0;
      }
      else {
        //if no more file
        rc = GetLastError();
        assert(rc == ERROR_NO_MORE_FILES);
        FindClose(context.hFind);
      }
    }

    //if success return got sibling
    //else if (error or no more file) goUp
    if (rc == 0) {
      break;
    } else {
      assert(flags & getSibling);
      flags |= goUp;
    }

    //goUp
    if (flags & goUp) {
      //execute callback
      if (callback.onGoUpBegin) callback.onGoUpBegin(0, (void*)p);

#if (REMOVE_DIR_LEVEL)
      _tcscpy_s(p->lastCompletedPath, MAX_PATH, context.findPath);
#endif

      //pop out context
      //if (is root) stop traversal
      //else get parent's sibling
      //memset(&context, 0, sizeof(context));
      rc = contextPop(p->stack, &context);
#if (REMOVE_DIR_LEVEL)
      if (rc == 0) {
        rc = p->error_completed_all;
      } else {
        rc = p->error_completed_one_dir;
      }
      break;
#endif
      if (rc == 0) {
        rc = -1;
        break;
      }

      //execute callback
      if (callback.onGoUpComplete) callback.onGoUpComplete(0, (void*)p);
    }
  }
#if (REMOVE_DIR_LEVEL)
  p->lastError = rc;
#endif

  return rc;
}
//close all find handle in stack
int traverser::traverseCancel(traverseData* p)
{
  traversalContext &context = p->context;
  for(int rc = 1; rc;) {
    FindClose(context.hFind);
    rc = contextPop(p->stack, &context);
  }
  return 0;
}
#endif //_TRAVERSER_
//-----------------transportArg------------------
#ifdef _TRANSPORT_ARG_
static const TCHAR* zTransportArgsSeps = TEXT("|<>");
struct transportArgs {
  TCHAR zParams[MAX_PATH];
  //int nRead;
  //int count;
  TCHAR* curPos;
  TCHAR* srcpath;
  TCHAR* alias;
  TCHAR* despath;
};
//+++++++methods
transportArgs* argsParser::InitArgs(TCHAR* zParams)
{
  transportArgs *newArgs = new transportArgs;
  if (newArgs) {
    _tcscpy_s(newArgs->zParams, zParams);
  }
  return newArgs;
}
void argsParser::FinalArgs(transportArgs* pArgs)
{
  if (pArgs) delete pArgs;
}
TCHAR* argsParser::nextElement(TCHAR* zPath, TCHAR* seps, TCHAR** curPos)
{
  assert(curPos);
  TCHAR* zTarget;
  int iStart, iEnd;

  if (zPath) {
    zTarget = zPath;
  } else {
    zTarget = *curPos;
  }
  //trim left
  for(iStart = 0; _tcschr(seps, zTarget[iStart]); iStart++);
  //if met terminate
  if (zTarget[iStart] == 0) {return 0;}
  //extract element
  for(iEnd = iStart; zTarget[iEnd]; iEnd++) {
    if (_tcschr(seps, zTarget[iEnd])) break;
  }
  zTarget[iEnd] = 0;
  //go to next element
  *curPos = zTarget + iEnd + 1;

  return zTarget + iStart;
}
//if ok return 0
int argsParser::formatParams(TCHAR* zPath)
{
  int rc = 0;
  int len = _tcslen(zPath);
  for(len--;len > 0;len--) {
    if (zPath[len] == TEXT('\\'))
      zPath[len] = 0;
    else
      break;
  }
  rc = zPath[len - 1] == TEXT('\\');
  return rc;
}
int argsParser::FirstParams(transportArgs* p)
{
  int rc = -1;
  do {
    p->srcpath = _tcstok_s(p->zParams, (TCHAR*)zTransportArgsSeps, &(p->curPos));
    formatParams(p->srcpath);
    if(p->srcpath == 0) break;
    p->alias = _tcstok_s(NULL, (TCHAR*)zTransportArgsSeps, &(p->curPos));
    assert(p->alias);
    formatParams(p->alias);
    p->despath = _tcstok_s(NULL, (TCHAR*)zTransportArgsSeps, &(p->curPos));
    assert(p->despath);
    formatParams(p->despath);
    rc = 0;
  } while (FALSE);
  //p->nRead = 1;
  return rc;
}
int argsParser::NextParams(transportArgs* p)
{
  int rc = -1;
  do {
    //p->nRead++;
    p->srcpath = _tcstok_s(NULL, (TCHAR*)zTransportArgsSeps, &(p->curPos));
    if(p->srcpath == 0) break;
    formatParams(p->srcpath);
    p->alias = _tcstok_s(NULL, (TCHAR*)zTransportArgsSeps, &(p->curPos));
    assert(p->alias);
    formatParams(p->alias);
    p->despath = _tcstok_s(NULL, (TCHAR*)zTransportArgsSeps, &(p->curPos));
    assert(p->srcpath);
    formatParams(p->despath);
    rc = 0;
  } while (FALSE);
  return rc;
}

#endif //_TRANSPORT_ARG_
//-----------------explorer----------------------
#ifdef _EXPLORER_
struct exploreData
{
  int parentId;
  int lastId;
  traverseData TD;
  MemMngIF*   stackHandle;
  treeMng*    treeHandle;
};
int explorer_onGoDownBegin(int, void*p);
int explorer_onGoUpBegin(int, void*p);
//+++++++methods
enum {
  exploreDataSize = ALIGNBY8(sizeof(exploreData)),
  exploreDataStackSize = ALIGNBY8(TREE_MAX_DEPTH * sizeof(int)),
  //traverseDataStackSize = TREE_MAX_DEPTH * sizeof(traversalContext),
};
int explorer::getDataSize(int depth)
{
  int reqSize;
  int traverseDataStackSize = ALIGNBY8(depth * sizeof(traversalContext));
  reqSize = exploreDataSize
    + exploreDataStackSize
    + traverseDataStackSize;
  return reqSize;
}
int explorer::InitData(treeMng* treeHandle, char* buff, int size)
{
  int rc = 2;
  int traverseDataStackSize = size - exploreDataSize - exploreDataStackSize;
  do {
    // + init explore data stack - nodeId stack
    QSMemParams qsp;
    qsp.mBuffer = buff + exploreDataSize;
    qsp.mBufferSize = exploreDataStackSize;
    LOG_DEBUG(TEXT("[expl] init nodeId stack buff %x size %x"), qsp.mBuffer, qsp.mBufferSize);
    qsp.mFlags = qsp.modeStack | qsp.addrAbsolute;
    QSMem *nodeIdStack = new QSMem;
    if (!nodeIdStack) break;
    nodeIdStack->Init(&qsp);
    rc--;

    // + init traverse data stack - context stack
    qsp.mBuffer = buff + exploreDataSize + exploreDataStackSize;
    qsp.mBufferSize = traverseDataStackSize;
    LOG_DEBUG(TEXT("[expl] init context stack buff %x size %x"), qsp.mBuffer, qsp.mBufferSize);
    qsp.mFlags = qsp.modeStack | qsp.addrAbsolute;
    QSMem *contextStack = new QSMem;
    if (!contextStack) {
      delete nodeIdStack;
      break;
    }
    contextStack->Init(&qsp);
    rc--;

    //+ init explore data
    exploreData &ed = *(exploreData*)(buff);
    LOG_DEBUG(TEXT("[expl] init exploredata %x size %x"), buff, exploreDataSize);
    ed.stackHandle = nodeIdStack;
    ed.treeHandle = treeHandle;
    //+ init traverse data
    traverseData &td = ed.TD;
    td.stack = contextStack;
    td.callback.pParams = &ed;
    td.callback.onGoDownBegin = explorer_onGoDownBegin;
    td.callback.onGoDownComplete = 0;
    td.callback.onGoUpBegin   = explorer_onGoUpBegin;
    td.callback.onGoUpComplete = 0;
  } while (FALSE);

  assert(0 <= rc && rc <=2);
  switch(rc) {
    case 1:   //init contextStack fail
      //fall through
    default:
      //if all done or nothing done - do nothing
      break;
  }
  return rc;
}
exploreData* explorer::InitData(treeMng* treeHandle)
{
  int rc;
  int nDone = 0;
  int reqSize;
  char* buff;
  int traverseDataStackSize = ALIGNBY8(TREE_MAX_DEPTH * sizeof(traversalContext));
  //(1) alloc buffer for explore data
  reqSize = exploreDataSize
    + exploreDataStackSize
    + traverseDataStackSize;
  assert(ALIGNEDBY8(reqSize));
  do {
    //+ allocate space
    //buff      ->|explorer data        |
    //stack area->|nodeId stack buffer  |
    //            |context stack buffer |
    buff = new char[reqSize];
    if (!buff) break;
    //memset(buff, 0, reqSize);
    nDone++;
    //+ init data
    rc = InitData(treeHandle, buff, reqSize);
    if (rc) break;
    nDone++;
  } while (FALSE);

  assert(0 <= nDone && nDone <= 2);
  switch(nDone) {
    case 2:
      //all done
      break;
    case 1: //init data fail
      delete buff;
      buff = 0;
      //fall through
    default: //allocate buff fail
      assert(buff == 0);
      break;
  }
  return (exploreData*)buff;
}
void explorer::FinalData(exploreData* p)
{
  exploreData &ed = *p;
  delete (QSMem*)ed.stackHandle;
  delete (QSMem*)ed.TD.stack;
  delete (char*)p;
}
int explorer::exploreBegin(exploreData* in_out, transportArgs* args)
{
  TCHAR* zAlias = args->alias;
  return exploreBegin(in_out, zAlias);
}
int explorer::exploreBegin(exploreData* in_out, TCHAR* zAlias)
{
  int rc = 0;
  exploreData &ed = *in_out;
  treeMng &tree = *(ed.treeHandle);
  //rc = CreateDirectory(tp.despath, NULL);
  //if (!rc) rc = GetLastError();
  //CleanFiles(tp.despath);
  int nodeId = tree.EnQueue(tree.RootId, zAlias, TRUE);
  ed.parentId = nodeId;
  ed.lastId = nodeId;
  return rc;
}
int explorer::exploreFirst(exploreData* in_out, transportArgs* args)
{
  TCHAR *zSrcPath = args->srcpath;
  return exploreFirst(in_out, zSrcPath);
}
int explorer::exploreFirst(exploreData* p, TCHAR* zSrcPath)
{
  int rc = 0;
  //init params
  traverseData &td = p->TD;
  traverser trav;
  td.context.curLevel = 0;
  td.context.findPath[0] = 0;
  //td.stack = mTraversalStack;
  pathPush(td.context.findPath, MAX_PATH, zSrcPath);
  rc = trav.traverseFirst(&td);
  return rc;
}
int explorer::explorePush(exploreData* in_out)
{
  enum {
    needCopy = 0x01,
    needPush = 0x02,
    isDir    = 0x10,
  };
  int flags = 0;
  //TCHAR buff[MAX_PATH];
  exploreData &ed = *in_out;
  traverser &trav = *this;
  treeMng &tree   = *ed.treeHandle;
  traverseData &td= ed.TD;
  //do some thing
  //if "." or ".."
  //if soft link
  //if file
  //if folder
  if (trav.traverseFilter(&td)) {
    //soft link or system file/folder
    //do nothing
  }
  //if folder
  else if (td.context.fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
    //push to queue
    //assert(ed.lastId != -1);
    flags |= needPush | needCopy | isDir;
  }
  //if file
  else {
    //push to queue
    //assert(td.lastId != -1);
    flags |= needPush | needCopy;
    LOG_DEBUG(TEXT("[expl] file %s\\%s size %d"), ed.TD.context.findPath, ed.TD.context.fd.cFileName, ed.TD.context.fd.nFileSizeLow);
  }
  if (flags & needPush) {
      ed.lastId = tree.EnQueue(ed.parentId, td.context.fd.cFileName, flags & isDir);
  }
  //if (flags & needCopy) {
  //  //do something
  //  //LOG_Write(TEXT("%s %s %08x\n"), td.context.findPath, td.context.fd.cFileName, td.context.fd.dwFileAttributes);
  //  buff[0] = 0;
  //  pathPush(buff, MAX_PATH, td.context.findPath);
  //  pathPop(buff);
  //  pathPush(buff, MAX_PATH, td.context.fd.cFileName);
  //  //LOG_Write(TEXT("copy %s %s %s\n"), buff, srcPath, desPath);
  //  ExportOne(buff, srcPath, desPath, flags & isDir);
  //}
  return 0;
}
int explorer::exploreNext(exploreData* in_out)
{
  int rc = 0;
  exploreData &ed = *in_out;
  traverser trav;
  traverseData &td= ed.TD;
#if (REMOVE_DIR_LEVEL)
    for (rc = trav.traverseNext(&td); rc != 0;) {
      if (rc == td.error_completed_all) {
        break;
      }
      else if (rc == td.error_completed_one_dir) {
        //get sibling
      }
      //find next
      rc = trav.traverseNext(&td);
    }
#else
    rc = trav.traverseNext(&td);
#endif
  return rc;
}
int explorer::exploreCancel(exploreData* in_out)
{
  exploreData &ed = *in_out;
  //close find handle
  traverseCancel(&ed.TD);
  return 0;
}
int explorer::exploreEnd(exploreData* in)
{
  exploreData &ed = *in;
  treeMng &tree = *ed.treeHandle;
  tree.EnQueue(tree.RootId, zEndNode, TRUE);
  return 0;
}
int explorer::getFileSize(exploreData* in, long long *pSize)
{
  exploreData &ed = *in;
  LARGE_INTEGER li;
  li.HighPart = ed.TD.context.fd.nFileSizeHigh;
  li.LowPart = ed.TD.context.fd.nFileSizeLow;
  *pSize = li.QuadPart;
  return 0;
}
//+++++++callback
int explorer_onGoUpBegin(int, void*p)
//int explorer::onGoUpBegin(int, void*p)
{
  int rc = 0;
  nodeMng nm;
  traverseData &td = *(traverseData*)p;
  exploreData &ed = *(exploreData*)td.callback.pParams;
  //update parent id
  assert(ed.stackHandle);
  if (td.context.curLevel != 0) {
    assert(ed.stackHandle->Count());
    rc = nm.nodePop(ed.stackHandle, &ed.parentId);
    assert(rc == sizeof(int));
  } else {
    assert(0 == ed.stackHandle->Count());
  }
  return rc;
}
int explorer_onGoDownBegin(int, void*p)
//int explorer::onGoDownBegin(int, void*p)
{
  int rc;
  traverseData &td = *(traverseData*)p;
  exploreData &ed = *(exploreData*)td.callback.pParams;
  //push parent id
  assert(ed.stackHandle && ed.stackHandle->Remain());
  nodeMng nm;
  rc = nm.nodePush(ed.stackHandle, &ed.parentId);
  assert(rc == sizeof(int));
  ed.parentId = ed.lastId;
  return rc;
}
//int explorer::onGoUpEnd(int, void*) { return 0; }
//int explorer::onGoDownEnd(int, void*) { return 0; }
#endif //_EXPLORER_
//-----------------exporter
#ifdef _EXPORTER_
struct XportData
{
  ExtractData ED;
  copyData CD;
};
enum {
  exportDataSize = ALIGNBY8(sizeof(XportData)),
  ExtractDataStackSize = ALIGNBY8(TREE_MAX_DEPTH * sizeof(nodeHdr) + sizeof(node)),
};
int Xporter::getDataSize()
{
  int reqSize = exportDataSize + ExtractDataStackSize;
  reqSize = ALIGN_BY_16(reqSize);
  return reqSize;
}
//if success func return 0
int Xporter::InitData(treeMng* treeHandle, char* buff, int size)
{
  int rc = 1;
  QSMem *nodeStack;
  do {
    //memset(buff, 0, reqSize);
    // + init extract data stack - node stack
    QSMemParams qsp;
    qsp.mBuffer = buff + exportDataSize;
    qsp.mBufferSize = ExtractDataStackSize;
    LOG_DEBUG(TEXT("[xpot] init nodestack %x size %x"), qsp.mBuffer, qsp.mBufferSize);
    qsp.mFlags = qsp.modeStack | qsp.addrAbsolute;
    nodeStack = new QSMem;
    if (!nodeStack) break;
    nodeStack->Init(&qsp);
    rc--;
    //+ init extract data
    XportData &ed = *(XportData*)(buff);
    LOG_DEBUG(TEXT("[xpot] init xpotdata %x size %x"), buff, exportDataSize);
    ed.ED.stackHandle = nodeStack;
    ed.ED.treeHandle = treeHandle;
    //+ init export data
  } while (FALSE);

  if (rc) assert(nodeStack == 0);

  return rc;
}
XportData* Xporter::InitData(treeMng* treeHandle)
{
  int rc;
  int reqSize;
  char* buff;
  //(1) alloc buffer for explore data
  reqSize = exportDataSize + ExtractDataStackSize;
  assert(ALIGNEDBY8(reqSize));
  buff = new char[reqSize];
  if (buff) {
    rc = InitData(treeHandle, buff, reqSize);
    if (rc) { //if error
      delete buff;
      buff = 0;
    }
  }
  return (XportData*)buff;
}
void Xporter::FinalData(XportData* p)
{
  XportData &ed = *p;
  delete (QSMem*)(ed.ED.stackHandle);
  delete (char*)p;
}
int Xporter::extractFirst(XportData* in_out)
{
  int rc;
  XportData &ed = *in_out;
  rc = extractor::extractFirst(&ed.ED);
  return rc;
}
//clean destination directory before import/export
int Xporter::clean(transportArgs* pTransportArgs)
{
  int rc = 0;
  transportArgs &arg = *pTransportArgs;
  int attrib = GetFileAttributes(arg.despath);
  if (attrib == INVALID_FILE_ATTRIBUTES) {
    //create dir
    rc = copyDir(arg.despath);
    assert(rc == 0);
  }
  else {
    //clean all files/folders EXCEPT folders level 1
    cleanFiles(arg.despath, 0);
  }
  return rc;
}
//if create directory success return error_copyComplete
//if create file success return error_copyNext and start copy file content
//else if error return error_copyFail
int Xporter::xportFirst(XportData* in_out, transportArgs* pTransportArgs)
{
  TCHAR zName[MAX_PATH];
  int rc;
  int len;

  XportData &xd = *in_out;
  transportArgs &arg = *pTransportArgs;

  copyData &cd = xd.CD;
  cd.zFin[0] = 0;
  cd.zFout[0] = 0;
  cd.size = 0;
  cd.copied = 0;

  LOG_DEBUG(TEXT("[xportNode] nodeId %d"), xd.ED.nd.id);

  //zFile = <srcDir>\\<path>
  //                ^-len
  len = _tcslen(arg.alias);

  if(xd.ED.nd.flags & node::isNode) {
    if (xd.ED.nd.parentId == -1) {
      //root folder
      LOG_DEBUG(TEXT("[xport] root %s"), xd.ED.nd.data);
      rc = error_copyComplete;
    }
    else {
      _stprintf_s(zName, MAX_PATH, TEXT("%s\\%s"), xd.ED.curPath, xd.ED.nd.data);
      assert(_tcsncmp(zName, arg.alias, len) == 0);
      _stprintf_s(cd.zFout, MAX_PATH, TEXT("%s\\%s"), arg.despath, zName + len + 1);

      rc = copyDir(cd.zFout);
      LOG_DEBUG(TEXT("[xport] dir %s"), cd.zFout);
      LOG_DEBUG(TEXT("[xport] rc %d"), rc);

      if (rc) rc = error_copyFail;
      else rc = error_copyComplete;
    }
  }
  else {
    assert(xd.ED.nd.flags & node::isLeaf);
    _stprintf_s(zName, MAX_PATH, TEXT("%s\\%s"), xd.ED.curPath, xd.ED.nd.data);
    assert(_tcsncmp(zName, arg.alias, len) == 0);
    _stprintf_s(cd.zFout, MAX_PATH, TEXT("%s\\%s"), arg.despath, zName + len + 1);
    _stprintf_s(cd.zFin, MAX_PATH, TEXT("%s\\%s"), arg.srcpath, zName + len + 1);

    //open file
    rc = copyFirst(&cd);
    LOG_DEBUG(TEXT("[xport] file %s"), cd.zFin);
    LOG_DEBUG(TEXT("[xport] file %s"), cd.zFout);
    //reset zFin, zFout - not use anymore

    if (rc) rc = error_copyFail;
    else rc = error_copyNext;
  }

  return rc;
}
int Xporter::xportFirst(transportArgs* pTransportArgs, exploreData* pExploreData, XportData* pXportData)
{
  int rc;
  int len;

  XportData &xd = *pXportData;
  transportArgs &arg = *pTransportArgs;
  exploreData &ed = *pExploreData;

  copyData &cd = xd.CD;
  cd.zFin[0] = 0;
  cd.zFout[0] = 0;
  cd.size = 0;
  cd.copied = 0;

  //zFile = <srcDir>\\<path>
  //                ^-len
  len = _tcslen(arg.srcpath);
  pathMng pathMng;
  pathMng.pathPush(cd.zFin, MAX_PATH, ed.TD.context.findPath);
  pathMng.pathPop(cd.zFin);
  pathMng.pathPush(cd.zFin, MAX_PATH, ed.TD.context.fd.cFileName);
  assert(_tcsncmp(cd.zFin, arg.srcpath, len) == 0);
  _stprintf_s(cd.zFout, MAX_PATH, TEXT("%s\\%s"), arg.despath, cd.zFin + len + 1);

  traverser trav;
  rc = trav.traverseFilter(&ed.TD);
  if (rc == 0) {
    if(ed.TD.context.fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      rc = copyDir(cd.zFout);
      LOG_DEBUG(TEXT("[xport] dir %s"), cd.zFout);
      LOG_DEBUG(TEXT("[xport] rc %d"), rc);

      if (rc) rc = error_copyFail;
      else rc = error_copyComplete;
    }
    else {
      //open file
      rc = copyFirst(&cd);
      LOG_DEBUG(TEXT("[xport] file %s"), cd.zFin);
      LOG_DEBUG(TEXT("[xport] file %s"), cd.zFout);

      if (rc) rc = error_copyFail;
      else rc = error_copyNext;
    }
  } else {
    //skip file/folder
    LOG_DEBUG(TEXT("[xport] skip %s"), cd.zFin);
    rc = error_copyComplete;
  }

  return rc;
}
//if remain data return error_copyNext to continue copy
//if copy file complete return error_copyComplete
//else if error return error_copyFail
int Xporter::xportNext(XportData* in_out)
{
  int rc;
  XportData &xd = *in_out;
  copyData &cd = xd.CD;
  rc = copyNext(&cd);
  LOG_DEBUG(TEXT("[xport] file rc %d size %I64d copied %I64d"), rc, cd.size, cd.copied);
  if (rc) {
    //copy complete
    rc = error_copyComplete;
  } else {
    rc = error_copyNext;
  }
  return rc;
}
//return 0;
int Xporter::xportCancel(XportData* in_out)
{
  XportData &xd = *in_out;
  copyData &cd = xd.CD;
  copyClose(&cd);
  return 0;
}
int Xporter::extractNext(XportData* in_out)
{
  int rc;
  XportData &ed = *in_out;
  rc = extractor::extractNext(&ed.ED);
  return rc;
}
int Xporter::getFileSize(XportData* pData, long long* pSize, long long *pCopied)
{
  copyData &CD = pData->CD;
  *pSize = CD.size;
  *pCopied = CD.copied;
  LOG_DEBUG(TEXT("[getSize] nodeId %d size %I64d copied %I64d"), pData->ED.nd.id, CD.size, CD.copied);
  return 0;
}
#if(1)  //WORKING MODE 1 - 1 thread no file.bin
//+++++++callback
int xport_onGoUpBegin(int, void*p)
{
  traverseData &td = *(traverseData*)p;
  XportData &xd = *(XportData*)td.callback.pParams;
  if (td.context.curLevel != 0) {
    pathMng pathMng;
    pathMng.pathPop(xd.ED.curPath);
  }
  return 0;
}
int xport_onGoDownBegin(int, void*p)
{
  traverseData &td = *(traverseData*)p;
  XportData &xd = *(XportData*)td.callback.pParams;
  pathMng pathMng;
  pathMng.pathPush(xd.ED.curPath, xd.ED.curPathSize, td.context.fd.cFileName);
  return 0;
}
//if convert success func return 0
//else func return error code (=-1)
int Xporter::convertEDtoXD(exploreData* pExploreData, XportData* pXportData)
{
  int rc;
  exploreData &ed = *pExploreData;
  XportData &xd = *pXportData;
  //+filter
  traverser trav;
  rc = trav.traverseFilter(&ed.TD);
  if (!rc) {
    //if file/folder available to transfer
    int isDir = ed.TD.context.fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    ////+init curPath
    //_tcscpy_s(xd.ED.curPath, ed.TD.context.findPath);
    //pathMng pathMng;
    //pathMng.pathPop(xd.ED.curPath);
    //+int node.data
    nodeMng nodeMng;
    nodeMng.nodeFormat(&xd.ED.nd, 0, ed.TD.context.fd.cFileName, isDir);
    xd.ED.nd.parentId = 0;
  }
  return rc;
}
//modify traversal callback
int Xporter::xportBegin(transportArgs* pTransportArgs, exploreData* pExploreData, XportData* pXportData)
{
  exploreData &ed = *(exploreData*)pExploreData;
  XportData &xd = *(XportData*)pXportData;
  //reset traverse callback
  memset(&ed.TD.callback, 0, sizeof(traversalCallback));
  //reset extract data
  xd.ED.topId = 0;
  xd.ED.curPath[0] = 0;
  memset(&xd.ED.nd, 0, sizeof(node));
  return 0;
}
#endif //WORKING MODE 1 - 1 thread no file.bin
#endif //_EXPORTER_
//-----------------log---------------------------
#ifdef _LOG_
log gLog;

int GetModuleDir(TCHAR *buff, int count)
{
  TCHAR szFullName[MAX_PATH];
  int rc = GetModuleFileName(NULL, szFullName, MAX_PATH);
  if (rc == 0) {
    //return rc = 0
  } else {
    TCHAR* slash = _tcsrchr(szFullName, TEXT('\\'));
    slash[0] = 0;
    int len = (slash - szFullName);
    if (len < count) {
      rc = _tcsncpy_s(buff, count, szFullName, len);
      rc = len;
    } else {
      //if not enough space, return rc = 0
    }
  }
  return rc;
}

void log::logInit()
{
  int rc;
  TCHAR zDir[MAX_PATH];
  GetModuleDir(zDir, MAX_PATH);

  TCHAR buff[MAX_PATH];
  enum {nFile = 2,};
  struct {
    TCHAR *zFile;
    FILE **pHandle;
  } params[nFile] = {
    {TEXT("error.log"), &fLogError},
    {TEXT("debug.log"), &fLogDebug},
  };
  for (int i = 0; i < nFile; i++) {
    _stprintf_s(buff, MAX_PATH, TEXT("%s\\%s"), zDir, params[i].zFile);
    rc = _tfopen_s(params[i].pHandle, buff, TEXT("w+"));
    if (rc == 0) {
      //success
    } else {
      assert(0);
      rc = GetLastError();
    }
  }
  InitializeCriticalSection(&lock4AllThreadWriteLock);
  startTime = GetTickCount();
}
//NOTE: logInit() should be called before this call
void log::logFinal()
{
  fclose(fLogError);
  fclose(fLogDebug);
  DeleteCriticalSection(&lock4AllThreadWriteLock);
}

void log::LogWrite(int log, TCHAR* text)
{
  FILE *fout;
  DWORD temp = 0;
  DWORD curTID = GetCurrentThreadId();
  DWORD curTime = GetTickCount();
  int rqLck = 0;
  switch(log) {
    case L4ERROR:
      temp = 1;
      fout = fLogError;
      rqLck = 1;
      break;
    default:
      assert(log == L4DEBUG);
      fout = fLogDebug;
      rqLck = 1;
      break;
  }

  if (rqLck) {
    EnterCriticalSection(&lock4AllThreadWriteLock);
  }

  _ftprintf_s(fout, TEXT("[%08d][%08x][%08x] %s \n"), curTime - startTime, curTID, temp, text);

  fflush(fout);

  if (rqLck) {
    LeaveCriticalSection(&lock4AllThreadWriteLock);
  }
}
#endif //_LOG_
