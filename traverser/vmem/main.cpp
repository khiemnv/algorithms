//#include <assert.h>
#include <crtdbg.h>
#define assert(x) _ASSERT(x)

#include <string.h>
#include <tchar.h>
#include <Windows.h>
#include <stdio.h>

#include "transporter.h"
#include "tree.h"

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
//#define MULTI_PATHS_ZEXPORT   TEXT("<Y:\\|E|F:\\E\\><Z:\\|D|F:\\D\\>")
//+ import paths = "<srcpath1>|<alias1>|<despath1>|<srcpath2>|<alias2>|<despath2>|..."
#define MULTI_PATHS_ZIMPPORT  TEXT("<F:\\C\\|C|X:\\><F:\\D\\|D|Z:\\><F:\\E\\|E|Y:\\>")
#define MULTI_PATHS_COUNT     (3)


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
void test_explorer()
{
#define TEST_EXPORT
#define TEST_THREADS
  enum {
    queueBufferSize = 4096*8,
  };
  char queueBuffer[queueBufferSize];
  TransportParams* pParams;
  Transporter transporter;
  pParams = transporter.TransportDataInit(
#ifdef TEST_EXPORT
    TRUE,
#else
    FALSE,            //importMode
#endif
    queueBuffer,
    queueBufferSize,
    FS_FILE_SWAP,
#ifdef TEST_EXPORT
    MULTI_PATHS_ZEXPORT
#else
    MULTI_PATHS_ZIMPPORT
#endif
    );
  transporter.Init(pParams);
#ifdef TEST_THREADS
  transporter.Start();
  for (;;) {
    TransportStatus status;
    transporter.GetStatus(&status);
    _tprintf(TEXT("status %d expl %I64d xpot %I64d\n"), status.stateFlags, status.exploredSize, status.xportedSize);
    if (status.stateFlags & TransportStatus::xportingComplete)
      break;
    Sleep(500);
  }
#else //TEST_THREADS
#ifdef TEST_EXPORT
  transporter.ExploreThreadMain();
  transporter.XportThreadMain();
#else
  transporter.XportThreadMain();
#endif
#endif //TEST_THREADS
  transporter.Final();
  transporter.TransportDataFinal(pParams);
}
void test_copy()
{
  //size(MB) 619
  //copyData
  //block pgSize  elapsed(ms)
  //1     4096    74,616 
  //4             44,257    <-best
  //8             43,743 
  //4     512     217,154 
  TCHAR *zSrcpath= TEXT("E:\\setup\\GRMWDK_EN_7600_1.ISO");
  TCHAR* zDespath[] = {
    TEXT("F:\\GRMWDK_EN_7600_1.ISO_1"),
    TEXT("F:\\GRMWDK_EN_7600_1.ISO_2"),
    //TEXT("F:\\GRMWDK_EN_7600_1.ISO_3")
  };
  fileCopy fm;
  DWORD elapsed;
  for (int i = 0; i < _countof(zDespath); i++) {
    elapsed = GetTickCount();
    fm.copyFile(zSrcpath, zDespath[i]);
    elapsed = GetTickCount() - elapsed;
    _tprintf(TEXT("copy %s elapsed %d\n"), zSrcpath, elapsed);
  }
}
//<export/import> [zParams]
void shell_xpot(int argc, TCHAR* argv[])
{
  int rc;
  int nDone = 0;
  int isExport = 0;
  TCHAR* zParams;
  TCHAR zSwapFile[MAX_PATH];

  enum {
    queueBufferSize = 4096*8,
  };
  char queueBuffer[queueBufferSize];
  TransportParams* pParams;
  Transporter transporter;

  do {
    //(1) get input args
    if (argc < 2) break;
    if (_tcscmp(argv[1], TEXT("export")) == 0) {
      zParams = MULTI_PATHS_ZEXPORT;
      isExport = TRUE;
    }
    else if (_tcscmp(argv[1], TEXT("import")) == 0) {
      zParams = MULTI_PATHS_ZIMPPORT;
      isExport = FALSE;
    } else
      break;
    if (argc == 3) zParams = argv[2];
    nDone = 1;
    //(2) init params
    GetModuleDir(zSwapFile, MAX_PATH);
    pathMng path;
    path.pathPush(zSwapFile, MAX_PATH, TEXT("\\fs.bin"));
    pParams = transporter.TransportDataInit(
      isExport,
      queueBuffer,
      queueBufferSize,
      zSwapFile,
      zParams
      );
    rc = transporter.Init(pParams);
    if (rc) break;
    transporter.Start();
    for (int i = 0;;i++) {
      TransportStatus status;
      transporter.GetStatus(&status);
      _tprintf(TEXT("status %d expl %I64d xpot %I64d\n"), status.stateFlags, status.exploredSize, status.xportedSize);
      if (status.stateFlags & TransportStatus::xportingComplete)
        break;
#if (1)
      Sleep(500);
#else //test interrupt
      Sleep(50);
      if (i==1) {
        transporter.Cancel();
      }
#endif
    }
    transporter.Final();
    nDone = 2;
  } while(FALSE);

  switch (nDone) {
    case 2:
      break;
    case 1:
      _tprintf(TEXT("init fail\n"));
      break;
    default:
      assert(nDone == 0);
      _tprintf(TEXT("<export/import> [zParams]\n"));
      break;
  }
}
//<clean/walk> <dir>
void shell_walk(int argc, TCHAR* argv[])
{
  int nDone = 0;
  recursiveWalk rwalk;
  DWORD attrib;
  enum {
    cmdClean = 1,
    cmdWalk
  };
  int iCmd = 0;
  int depth;

  gLog.logInit();
  do {
    if (argc < 3) break;
    if (_tcscmp(argv[1], TEXT("clean")) == 0)
      iCmd = cmdClean;
    else if (_tcscmp(argv[1], TEXT("walk")) == 0)
      iCmd = cmdWalk;
    else
      break;
    attrib = GetFileAttributes(argv[2]);
    if (attrib == INVALID_FILE_ATTRIBUTES) break;
    nDone = 1;
    if (!(attrib & FILE_ATTRIBUTE_DIRECTORY)) break;
    switch (iCmd) {
      case cmdClean:
        rwalk.cleanFiles(argv[2], -1);
        break;
      case cmdWalk:
        rwalk.listFiles(argv[2], &depth);
        break;
    }
  } while(FALSE);

  gLog.logFinal();
  if (!nDone) _tprintf(TEXT("<clean> <dir>\n"));
}
void test_big_file()
{
  TCHAR* zBigFile = TEXT("D:\\tmp\\traverser\\Debug\\bigfile.txt");
  bigFile *ef = new bigFile(zBigFile, TRUE);
  //write out big file
  TCHAR* z1 = TEXT("out1.txt");
  HANDLE h1 = ef->CreateFile(z1);
  assert(h1 != INVALID_HANDLE_VALUE);
  char buff[4096];
  TCHAR* data1 = TEXT("12345");
  int nWrite;
  memcpy(buff, data1, 12);
  ef->WriteFile(h1, buff, sizeof(buff), &nWrite);
  ef->TruncateFile(h1, 12);
  ef->CloseFile(h1);

  //write out big file
  TCHAR* z2 = TEXT("out1.txt");
  HANDLE h2 = ef->CreateFile(z2);
  assert(h2 != INVALID_HANDLE_VALUE);
  TCHAR* data2 = TEXT("12345");
  memcpy(buff, data2, 12);
  ef->WriteFile(h2, buff, sizeof(buff), &nWrite);
  ef->TruncateFile(h2, 12);
  ef->CloseFile(h2);

  delete ef;

  //read from big file
  ef = new bigFile(zBigFile, FALSE);
  h1 = ef->CreateFile(z1);
  assert(h1 != INVALID_HANDLE_VALUE);
  int nRead;
  ef->ReadFile(h1, (char*)buff, sizeof(buff), &nRead);
  ef->CloseFile(h1);

  h2 = ef->CreateFile(z2);
  assert(h2 != INVALID_HANDLE_VALUE);
  ef->ReadFile(h2, (char*)buff, sizeof(buff), &nRead);
  ef->CloseFile(h2);

  delete ef;
}
//-----------------main--------------------------
int _tmain(int argc, TCHAR* argv[])
{
  //test_explorer();
  //test_copy();
  //shell_xpot(argc, argv);
  //shell_walk(argc, argv);
  test_big_file();
  return 0;
}
