//#include <assert.h>
#include <crtdbg.h>
#define assert(x) _ASSERT(x)

#include <string.h>
#include <tchar.h>
#include <Windows.h>

#include "ringmem.h"
#include "tree.h"

//massive file
void test_fileTree2()
{
  enum {
    qSize = 128,  //by page
    sSize = 16,  //by page
    pgSize = 512,
    maxDept = 8,
  };
  TCHAR *zRoot = TEXT("e:\\temp\\");
  char qBuff[qSize * pgSize];
  char sBuff[sSize * pgSize];
  FileTreeParams ftp;
  ftp.mode = ftp.queueModeVmem | ftp.exportMode;
  //+ queue
  ftp.queueBuffer = qBuff;
  ftp.queueBufferSize = qSize * pgSize;
  ftp.vMemSize = 1<<30;
  ftp.pgSize = 512*8;
  ftp.zSwapFile = TEXT("c:\\tmp\\fs.txt");
  //+ stack
  ftp.stackBuffer = sBuff;
  ftp.stackBufferSize = sSize * pgSize;
  ftp.zRoot = zRoot;
  ftp.maxDept = maxDept;
  //+ log
  ftp.zLogFile = TEXT("c:\\tmp\\log.txt");
  FileTree ft(&ftp);

  ft.Build();
  ft.Extract();
}
//import file bin
void test_fileTree3()
{
  enum {
    qSize = 128,  //by page
    sSize = 16,  //by page
    pgSize = 512,
    maxDept = 8,
  };
  TCHAR *zRoot = TEXT("e:\\temp");
  char qBuff[qSize * pgSize];
  char sBuff[sSize * pgSize];
  FileTreeParams ftp;
  ftp.mode = ftp.queueModeVmem | ftp.importMode;
  //+ queue
  ftp.queueBuffer = qBuff;
  ftp.queueBufferSize = qSize * pgSize;
  ftp.vMemSize = 1<<30;
  ftp.pgSize = 512*8;
  ftp.zSwapFile = TEXT("c:\\tmp\\fs.txt");
  //+ stack
  ftp.stackBuffer = sBuff;
  ftp.stackBufferSize = sSize * pgSize;
  ftp.zRoot = zRoot;
  ftp.maxDept = maxDept;
  //+ log
  ftp.zLogFile = TEXT("c:\\tmp\\log.txt");
  FileTree ft(&ftp);

  ft.Extract();
}
int main()
{
  //test_rm();
  //test_rm2();
  test_fileTree2();
  test_fileTree3();
  return 0;
}
