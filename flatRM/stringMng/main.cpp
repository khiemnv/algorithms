//#include <assert.h>
#include <crtdbg.h>
#define assert(x) _ASSERT(x)

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>

#include "vmem.h"
#include "ringmem.h"
#include "stringMng.h"

//test string queue with buffer
void test_stringMng2()
{
  enum {nPages =  1};
  int rc;
  int sysPgSize = 512;
  int bufferSize = nPages * PageMng::CalcPgSize(sysPgSize);
  char* buffer = (char*)malloc(bufferSize);

  StringQueueParams sqp;
  sqp.buffer = buffer;
  sqp.bufferSize = bufferSize;
  sqp.zTempFile = "c:\\tmp\\test.txt";

  StringQueue sq;
  rc = sq.Init(&sqp);

  char buff[128];
  rc = sq.Push("123", 4);
  assert(rc == 0);
  rc = sq.Pop(buff, 128);
  assert(rc == 0);

  sq.Final();
  free(buffer);
}
//test string queue with ring mem
void test_stringMng1()
{
  //init ring mem
  enum {
    nPages = 2,
    vmSize = 8,   //by page
  };

  int rc;
  int sysPgSize;
  int bufferSize;
  char* buffer;

  //1. init buffer
  //sysPgSize = RingMem::SystemPageSize();
  sysPgSize = 512;
  bufferSize = nPages * PageMng::CalcPgSize(sysPgSize);
  buffer = (char*)malloc(bufferSize);

  //2. init file swap
  char* zFile = "C:\\tmp\\test.txt";
  FileSwapParams fsp;
  fsp.mode = FileSwapParams::ModeNoBuffer;
  fsp.zName = zFile;
  FileSwap fs(&fsp);
  rc = fs.Extend(vmSize * sysPgSize);
  assert(rc == 0);

  //3. init vmem
  VMemParams vmp;
  vmp.buffer = buffer;
  vmp.bufferSize = bufferSize;
  vmp.pgDataSize = sysPgSize;
  vmp.hFile = &fs;
  vmp.vMemSize = vmSize * sysPgSize;
  VMem vm;
  vm.Init(&vmp);

  //4. init ringmem
  RingMemParams rmp;
  rmp.mode = RingMem::RingMemVMemMode;
  rmp.hVMem = &vm;
  RingMem rm(&rmp);

  //init string queue with created rm
  StringQueue sq(&rm);
  rc = sq.Push(0, 0);

  vm.Final();
  free(buffer);
}
int main()
{
  test_stringMng2();
  return 0;
}