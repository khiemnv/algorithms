//#include <assert.h>
#include <crtdbg.h>
#define assert(x) _ASSERT(x)

#include <string.h>
#include <tchar.h>
#include <Windows.h>
#include <stdio.h>

#include "ringmem.h"

#define MODE_TEST     (1)
//test params
#define DES_PATH      TEXT("E:")
//#define ROOT_PATH     TEXT("c:\\tmp1")
#define ROOT_PATH     TEXT("C:\\tmp1")
#define FS_FILE_LOG   TEXT("d:\\tmp\\fs.log")
#define FS_FILE_SWAP  TEXT("d:\\tmp\\fs.bin")
//multi paths
//+ export paths = "<srcpath1>|<alias1>|<despath1>|<srcpath2>|<alias2>|<despath2>|..."
#define MULTI_PATHS_ZEXPORT   TEXT("<X:\\|C|F:\\C\\><Z:\\|D|F:\\D\\><Y:\\|E|F:\\E\\>")
//+ import paths = "<srcpath1>|<alias1>|<despath1>|<srcpath2>|<alias2>|<despath2>|..."
#define MULTI_PATHS_ZIMPPORT  TEXT("<F:\\C\\|C|X:\\><F:\\D\\|D|Y:\\><F:\\C\\|E|Z:\\>")
#define MULTI_PATHS_COUNT     (3)

#if (TEST_FILE_TRAVERSAL)
//-----------------default traversal-------------
typedef struct TraversalParams {
  enum {
    modeImport,
    modeExport,
    modeWalk,
  };
  TCHAR rootPath[MAX_PATH];
  TCHAR swapFIle[MAX_PATH];
  TCHAR logFile[MAX_PATH];
  int mode;
}FTparams;
int Traversal(FTparams *p)
{
  enum {
    qSize = 32,   //by page
    sSize = 1,    //by page
    pgSize = 512*8,
    maxDept = 8,
    qVMemSize = 1<<30,
  };

  TCHAR *zRoot = p->rootPath;
  char qBuff[qSize * pgSize];
  char sBuff[sSize * pgSize];

  FileTraverserParams params;
  params.mode = params.queueModeVmem;
  if (p->mode == p->modeImport)
    params.mode |= params.importMode;
  else if (p->mode == p->modeExport)
    params.mode |= params.exportMode;
  else
    params.mode |= params.importMode;
  //+ queue
  params.queueBuffer = qBuff;
  params.queueBufferSize = qSize * pgSize;
  params.vMemSize = qVMemSize;
  params.pgSize = pgSize;
  params.zSwapFile = p->swapFIle;
  //+ stack
  params.stackBuffer = sBuff;
  params.stackBufferSize = sSize * pgSize;
  params.zRoot = zRoot;
  params.maxDept = maxDept;
  //+ log
  params.zLogFile = p->logFile;
  FileTraverser ft(&params);

  int rc = 0;
  if (p->mode == p->modeImport) {
    rc = ft.Extract();
  } else if (p->mode == p->modeExport) {
    rc = ft.Build();
  } else if (p->mode == p->modeWalk) {
    ft.Walk(p->rootPath);
  }

  return rc;
}
//-----------------test--------------------------

void test_rm2()
{
  //init buffer
  enum {bufferSize = 4};
  char buffer[bufferSize];

  RingMemParams rmp;
  rmp.buffer = buffer;
  rmp.bufferSize = bufferSize;
  rmp.mode = RingMem::RingMemAddrModeAbsolute;
  rmp.type = RingMem::RingMemTypeStack;
  RingMem rm(&rmp);

  int n;
  char buff[4];
  rm.Write("12", 2, &n);
  rm.Write("34", 2, &n);
  rm.Read(buff, 2, &n);
  rm.Read(buff+2, 2, &n);
  rm.Read(buff, 1, &n);
}
void test_rm()
{
  //init buffer
  enum {bufferSize = 8};
  char buffer[bufferSize];

  RingMemParams rmp;
  rmp.buffer = buffer;
  rmp.bufferSize = bufferSize;
  rmp.mode = RingMem::RingMemAddrModeAbsolute;
  rmp.type = RingMem::RingMemTypeStack;
  RingMem rm(&rmp);

  int n;
  int rc;
  char buff2[16];
  rc = rm.Write("123", 3, &n);
  assert(rc == 0 && n == 3);
  rc = rm.Read(buff2, 1, &n);
  assert(strncmp(buff2, "1", 1) == 0);
  rc = rm.Read(buff2, 2, &n);
  assert(strncmp(buff2, "23", 2) == 0);

  char* text = "1234567890";
  rc = rm.Write(text, 10, &n);
  assert(n == 8);
  rc = rm.Read(buff2, 10, &n);
  assert(strncmp(buff2, text, 8) == 0);

  rc = rm.Write(text, 8, &n);
  rc = rm.Write("abc", 3, &n);
  rc = rm.Read(buff2, 3, &n);
  assert(strncmp(buff2, "abc", 3) == 0);
  rc = rm.Read(buff2, 10, &n);
  assert(strncmp(buff2, text, n) == 0);

  rc = rm.Write(text, 8, &n);
  rc = rm.Write("abc", 3, &n);
  rc = rm.Read(buff2, 1, &n);
  assert(strncmp(buff2, "abc", 1) == 0);
  rc = rm.Write("ABC", 3, &n);
  rc = rm.Read(buff2, 8, &n);
  assert(strncmp(buff2, "ABC", 3) == 0);
  assert(strncmp(buff2+3, "bc", 2) == 0);
  assert(strncmp(buff2+5, text, 3) == 0);

  char* text2="abcdefgh";
  rc = rm.Write(text, 7, &n);
  rc = rm.Write(text2, 8, &n);
  rc = rm.Read(buff2, 4, &n);
  assert(strncmp(buff2, text2, n) == 0);
  rc = rm.Write(text, 6, &n);
  rc = rm.Read(buff2, n, &n);
  assert(strncmp(buff2, text, 5) == 0);
}

//export & import file struct
void test_fileTree2()
{
  enum {
    qSize = 128,  //by page
    sSize = 16,  //by page
    pgSize = 512,
    maxDept = 8,
  };
  TCHAR *zRoot = ROOT_PATH;
  char qBuff[qSize * pgSize];
  char sBuff[sSize * pgSize];
  FileTraverserParams ftp;
  ftp.mode = ftp.queueModeVmem | ftp.exportMode;
  //+ queue
  ftp.queueBuffer = qBuff;
  ftp.queueBufferSize = qSize * pgSize;
  ftp.vMemSize = 1<<30;
  ftp.pgSize = 512*8;
  ftp.zSwapFile = FS_FILE_SWAP;
  //+ stack
  ftp.stackBuffer = sBuff;
  ftp.stackBufferSize = sSize * pgSize;
  ftp.zRoot = zRoot;
  ftp.zDesPath = DES_PATH;
  ftp.maxDept = maxDept;
  //+ log
  ftp.zLogFile = FS_FILE_LOG;
  FileTraverser ft(&ftp);

  ft.Build();
  //ft.Extract();
}
//import file bin
void test_fileTree3()
{
#define TEST3_1   0x01    //test import
#define TEST3_2   0x02    //compare import vs walk
#define TEST3     TEST3_1
  enum {
    qSize = 250,  //by page
    sSize = 1,    //by page
    pgSize = 4*1024,
    maxDept = 8,
  };
  TCHAR *zRoot = ROOT_PATH;
  char qBuff[qSize * pgSize];
  char sBuff[sSize * pgSize];
  FileTraverserParams ftp = {0};
  ftp.mode = ftp.queueModeVmem | ftp.importMode;
  //+ queue
  ftp.queueBuffer = qBuff;
  ftp.queueBufferSize = qSize * pgSize;
  ftp.vMemSize = 1<<30;
  ftp.pgSize = pgSize;
  ftp.zSwapFile = FS_FILE_SWAP;
  //+ stack
  ftp.stackBuffer = sBuff;
  ftp.stackBufferSize = sSize * pgSize;
  ftp.zRoot = zRoot;
  ftp.maxDept = maxDept;
  //+ log
  ftp.zLogFile = FS_FILE_LOG;
  FileTraverser ft(&ftp);
#if (TEST3 & TEST3_1)
  ft.Extract();
#endif
#if (TEST3 & TEST3_2)
  ft.Walk(zRoot);
#endif
}

//export file struct
void test_fileTree4()
{
#define TEST4_1 (0x01)  //test traversal multi path
#define TEST4_2 (0x02)  //traversal 1 path
#define TEST4_3 (0x04)  //walk
#define TEST4   (TEST4_1)
  enum {
    qSize = 128,  //by page
    sSize = 16,   //by page
    pgSize = 512,
    maxDept = 8,
  };
  char qBuff[qSize * pgSize];
  char sBuff[sSize * pgSize];
  FileTraverserParams ftp = {0};
  ftp.mode = ftp.queueModeVmem | ftp.exportMode;
  //+ queue
  ftp.queueBuffer = qBuff;
  ftp.queueBufferSize = qSize * pgSize;
  ftp.vMemSize = 1<<30;
  ftp.pgSize = 512*8;
  ftp.zSwapFile = FS_FILE_SWAP;
  //+ stack
  ftp.stackBuffer = sBuff;
  ftp.stackBufferSize = sSize * pgSize;
  //+ root
  ftp.zRoot = ROOT_PATH;
  ftp.zDesPath = DES_PATH;
  ftp.maxDept = maxDept;
  //+ log
  ftp.zLogFile = FS_FILE_LOG;
  FileTraverser ft(&ftp);

#if (TEST4 & TEST4_1) //TRAVERSAL_MULTI_PATHS
  //build multiple drivers
  ft.Build(MULTI_PATHS_ZEXPORT, MULTI_PATHS_COUNT);
  //ft.Extract();
#endif
#if (TEST4 & TEST4_2)
  ft.Build(TEXT("c:\\tmp|c:|"), 1);
#endif
}

//use file mapping to improve performance
void test_import(TraversalParams *p)
{
#define TEST_EXTRACT_ONE    (1)
#define TEST_MULTI_PATHs    (2)
#define TEST5               TEST_MULTI_PATHs
  int rc;
  FileSwapParams fsp;
  fsp.mode = fsp.ModeNoBuffer;
  fsp.zName = p->swapFIle;
  FileSwap fs(&fsp);
  void* mapAddr = 0;
  rc = fs.MapFile(&mapAddr);
  assert(rc == 0);

  FileTraverserParams ftp = {0};
  //+ mode
  ftp.mode = ftp.importMode;
  //+ queue
  ftp.queueBuffer = (char*)mapAddr;
  ftp.queueBufferSize = fs.Size();
  //+ stack
  enum {sBuffSize = 1024};
  char sBuff[sBuffSize];
  ftp.stackBuffer = sBuff;
  ftp.stackBufferSize = sBuffSize;
  ftp.zRoot = ROOT_PATH;
  ftp.zDesPath = ROOT_PATH;
  //+ log
  ftp.zLogFile = p->logFile;
  FileTraverser ft(&ftp);
#if (TEST5 == TEST_EXTRACT_ONE)
  //clean desdir
  ft.CleanFiles(ROOT_PATH);
  //extract data to des path
  ExtractParams ep = {0};
  TCHAR zText[MAX_PATH];
  TCHAR* zAlias = TEXT("c:\\tmp1");
  rc = ft.ExtractFirst(&ep);
  for(;rc!=-1;) {
    if(ep.nd.flags & node::isNode)
    {
      if (ep.nd.id == 0) {
        //root folder
        _stprintf_s(zText, MAX_PATH, TEXT("%s"), ep.nd.data);
        ft.WriteLog(zText);
      }
      else {
        //create folder
        _stprintf_s(zText, MAX_PATH, TEXT("%s\\%s"), ep.curPath, ep.nd.data);
        ft.WriteLog(zText);
        ft.ImportOne(zText, zAlias, TEXT("F:\\C"), TEXT("c:\\tmp1"), TRUE);
      }
    }
    else
    {
      assert(ep.nd.flags & node::isLeaf);
      //create file
      _stprintf_s(zText, MAX_PATH, TEXT("%s\\%s"), ep.curPath, ep.nd.data);
      ft.ImportOne(zText, zAlias , TEXT("F:\\C"), TEXT("c:\\tmp1"), FALSE);
      ft.WriteLog(zText);
    }
    rc = ft.ExtractNext(&ep);
  }
#endif
#if (TEST5 == TEST_MULTI_PATHs)
  ft.Extract(MULTI_PATHS_ZIMPPORT, MULTI_PATHS_COUNT);
#endif
  //ft.Extract();
  fs.UnMapFile();
}
void test_soft_link()
{
  TCHAR *zFile = TEXT("c:\\tmp1\\e_tmp");
  TCHAR buff[MAX_PATH];
  int rc;
  rc = GetVolumeNameForVolumeMountPoint(
    zFile,
    buff,
    MAX_PATH
    );
  if (!rc) {
    rc = GetLastError();
  }
  HANDLE hf = CreateFile(
    zFile,
    GENERIC_READ,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    0,
    OPEN_EXISTING,
    //FILE_FLAG_BACKUP_SEMANTICS,
    FILE_FLAG_OPEN_REPARSE_POINT| FILE_FLAG_BACKUP_SEMANTICS,
    0
    );
  if (hf != INVALID_HANDLE_VALUE) {
    /*rc = GetFinalPathNameByHandle(
      hf,
      buff,
      MAX_PATH,
      0
      );
    if (rc == 0) {
      rc = GetLastError();
    }*/
    REPARSE_GUID_DATA_BUFFER* rgdb;
    DWORD n;
    rc = DeviceIoControl(
      hf,
      FSCTL_GET_REPARSE_POINT,
      NULL,
      0,
      &buff,
      sizeof(buff),
      &n,
      0
      );
    if (!rc) {
      rc = GetLastError();
    }
    rgdb = (REPARSE_GUID_DATA_BUFFER*)buff;
    CloseHandle(hf);
  } else {
    rc = GetLastError();
  }
}
void getFileAttribute(TCHAR* zFile)
{
  int rc;
  HANDLE hf = CreateFile(
    zFile,
    GENERIC_READ,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    0,
    OPEN_EXISTING,
    //FILE_FLAG_BACKUP_SEMANTICS,
    FILE_FLAG_OPEN_REPARSE_POINT| FILE_FLAG_BACKUP_SEMANTICS,
    0
    );
  if (hf != INVALID_HANDLE_VALUE) {
    BY_HANDLE_FILE_INFORMATION finfo;
    rc = GetFileInformationByHandle(hf, &finfo);
    if (!rc) {
      rc = GetLastError();
      printf("get file info error %d\n", rc);
    }
    printf("dwFileAttributes %08x\n", finfo.dwFileAttributes);
    CloseHandle(hf);
  } else {
    rc = GetLastError();
    printf("open file error %d\n", rc);
  }
}
void test_sercurity()
{
  int rc;
  TCHAR* zFile = TEXT("x:\\System Volume Information");
  HANDLE hf = CreateFile(
    zFile,
    READ_CONTROL | ACCESS_SYSTEM_SECURITY,
    0,
    NULL,
    OPEN_EXISTING,
    FILE_FLAG_BACKUP_SEMANTICS,
    0
    );
  if (hf != INVALID_HANDLE_VALUE) {
    CloseHandle(hf);
  } else {
    rc = GetLastError();
    printf("open file error %d\n", rc);
  }
}
#endif //TEST_FILE_TRAVERSAL
//-----------------main--------------------------
int _tmain(int argc, TCHAR* argv[])
{
  //test_sercurity();
  //return 0;
#if (MODE_TEST)
  //test_rm();
  //test_rm2();
//#if (0)
  //test_fileTree2(); //export - use vmem
  //test_fileTree3(); //import - use vmem
  //test_fileTree4(); //export multi paths
//#else
  //{
  //  //import using file mapping
  //  FTparams params = {0};
  //  params.mode = params.modeImport;
  //  _tcsncpy_s(params.rootPath, MAX_PATH, ROOT_PATH, _tcslen(ROOT_PATH));
  //  _tcsncpy_s(params.swapFIle, MAX_PATH, FS_FILE_SWAP, _tcslen(FS_FILE_SWAP));
  //  _tcsncpy_s(params.logFile, MAX_PATH, FS_FILE_LOG, _tcslen(FS_FILE_LOG));
  //  test_import(&params);
  //}
//#endif
  //test_soft_link();
  //getFileAttribute(argv[1]);
#else //MODE_TEST
  FTparams params = {0};
#define FT_SWAP_FILE      TEXT("ft.bin")
#define FT_LOG_FILE       TEXT("ft.log")
  TCHAR *swapFile = FT_SWAP_FILE;
  TCHAR *logFile = FT_LOG_FILE;
  if (2 < argc) {
    if (_tcscmp(TEXT("import"), argv[1]) == 0) {
      params.mode = params.modeImport;
    }
    else if (_tcscmp(TEXT("export"), argv[1]) == 0) {
      params.mode = params.modeExport;
    }
    else if (_tcscmp(TEXT("walk"), argv[1]) == 0) {
      params.mode = params.modeWalk;
    }
    else {
      //default is import
    }

    _tcsncpy_s(params.rootPath, MAX_PATH, argv[2], _tcslen(argv[2]));
    if (3 < argc) {
      swapFile = argv[3];
      if (4 < argc) {
        logFile = argv[4];
      }
    }
    _tcsncpy_s(params.swapFIle, MAX_PATH, swapFile, _tcslen(swapFile));
    _tcsncpy_s(params.logFile, MAX_PATH, logFile, _tcslen(logFile));

    //int rc = Traversal(&params);
    if (params.mode == params.modeExport) {
      Traversal(&params);
    }
    else if (params.mode == params.modeImport) {
      test_import(&params);
    }
    else if (params.mode == params.modeWalk) {
      Traversal(&params);
    }
  }
  else
  {
    _tprintf(TEXT("<import/export/walk> <rootPath> [<swapFile>] [<logFIle>]\n"));
  }
#endif  //MODE_TEST
  return 0;
}