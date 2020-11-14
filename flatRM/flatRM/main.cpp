#include "ringmem.h"
#include "vmem.h"

//#include <assert.h>
#include <crtdbg.h>
#define assert(x) _ASSERT(x)
#include <string.h>
#include <stdlib.h>
#include <tchar.h>
#include <Windows.h>

int test_rm()
{
  char buff[16];
  char* data2write = "ABCDEFGHIJ";
  char readdata[16];
  int count;
  //init ring mem
  RingMem mymem(buff, 5);
  //test normal case
  //	ringmem(buff,5)
  //buffer  |1  2  3  4  5 |1  2  3  4  5 |
  //write   |A  B  C  D |E  F  G  H |I  J |
  //read    |1  2  3 |4  5  6 |7  8  9 |10|

  mymem.Write("ABCD", 4, &count);
  assert(count == 4);
  assert(mymem.Remain() == 1);
  mymem.Read(readdata, 3, &count);
  assert(count == 3);
  assert(mymem.Remain() == 4);

  mymem.Write("EFGH", 4, &count);
  assert(count == 4);
  mymem.Read(readdata + 3, 3, &count);
  assert(count == 3);

  mymem.Write("IJ", 2, &count);
  assert(count == 2);
  mymem.Read(readdata + 6, 3, &count);
  assert(count == 3);

  mymem.Read(readdata + 9, 3, &count);
  assert(count == 1);

  assert(strncmp("ABCDEFGHIJ", readdata, 10) == 0);

  //test over write
  //	ringmem(buff,5)
  //buffer  |1  2  3  4  5 |1  2  3  4  5 |
  //write   |A  B  C  D |E  F  G  H |
  //read             |1  2  3  4  5 |
  mymem.Write("ABCD", 4, &count);
  assert(count == 4);
  mymem.Write("EFGH", 4, &count);
  assert(count == 4);

  mymem.Read(readdata, 8, &count);
  assert(count == 5);

  assert(mymem.IsEmpty());
  assert(strncmp("DEFGH", readdata, 5) == 0);

  //test write full
  mymem.Write("ABCDEFGH", 8, &count);
  assert(count == 5);
  mymem.Read(readdata, 8, &count);
  assert(count == 5);
  assert(strncmp("ABCDE", readdata, 5) == 0);
  return 0;
}
int test_rm2()
{
  char buff[16];
  int count;
  RingMem rm(buff, 6);
  RingMem *pOld;

  //test func IsEmpty, IsFull, Remain, Empty,
  assert(rm.IsEmpty());
  assert(!rm.IsFull());
  assert(rm.Remain() == 6);

  rm.Write("123", 3, &count);

  assert(!rm.IsEmpty());
  assert(!rm.IsFull());
  assert(rm.Remain() == 3);

  rm.Empty();

  assert(rm.IsEmpty());
  assert(!rm.IsFull());
  assert(rm.Remain() == 6);

  rm.Write("123456", 6, &count);

  assert(!rm.IsEmpty());
  assert(rm.IsFull());
  assert(rm.Remain() == 0);

  //change ring mem buff and size
  pOld = &rm;
  rm = RingMem(buff, 7);

  assert(pOld == &rm);
  assert(rm.IsEmpty());
  assert(!rm.IsFull());
  assert(rm.Remain() == 7);

  return 0;
}
//test ringmem resize
void test_rm3()
{
  char buff[16];
  char outbuff[16];
  RingMem *prm, *pold;
  int count;

  //init with no space
  prm = new RingMem(0, 0);
  pold = prm;

  //5 bytes space
  (*prm) = RingMem(buff, 5);
  prm->Write("123456", 6, &count);
  pold->Read(outbuff, 16, &count);
  assert(count == 5);
  assert(strncmp(outbuff, buff, 5) == 0);

  //6 bytes space
  (*prm) = RingMem(buff, 6);
  prm->Write("123456", 6, &count);
  pold->Read(outbuff, 16, &count);
  assert(count == 6);
  assert(strncmp(outbuff, buff, 6) == 0);

  delete prm;
}
//(4) test Allocate & Locate methods
void test_rm4_1();
void test_rm4_2();
void test_rm4()
{
  //test relative addr mode
  test_rm4_2();
  //test absolute addr mode
  test_rm4_1();
}
void test_rm4_1()
{
  char buff[16];
  RingMem rm(buff, 5);
  //test Allocate
  PCHAR data;
  char buff2[16];
  int count;

  rm.Allocate(&data, 3, &count);
  assert(count == 3);
  rm.Commit(0, &count);
  assert(count == 0);

  rm.Allocate(&data, 3, &count);
  assert(count == 3);
  data[0] = 'A';
  data[1] = 'B';
  data[2] = 'C';
  assert(strncmp(buff, "ABC", 3) == 0);
  rm.Commit(3, &count);
  assert(count == 3);

  //commit(0)
  rm.Allocate(&data, 3, &count);
  assert(count == 2);
  rm.Commit(0, &count);
  assert(count == 0);

  rm.Allocate(&data, 2, &count);
  assert(count == 2);
  rm.Commit(0, &count);
  assert(count == 0);

  rm.Allocate(&data, 3, &count);
  assert(count == 2);
  data[0] = 'D';
  data[1] = 'E';
  assert(strncmp(buff, "ABCDE", 5) == 0);
  rm.Commit(3, &count);
  assert(count == 2);

  rm.Allocate(&data, 3, &count);
  assert(count == 3);
  data[0] = 'F';
  data[1] = 'G';
  data[2] = 'H';
  assert(strncmp(buff, "FGHDE", 3) == 0);
  rm.Commit(3, &count);
  assert(count == 3);
  //locate
  rm.Locate(0, &data, 2, &count);
  assert(count == 2);
  assert(strncmp(data, "DE", 2) == 0);

  rm.Locate(1, &data, 3, &count);
  assert(count == 1);
  assert(strncmp(data, "E", 1) == 0);

  rm.Locate(2, &data, 3, &count);
  assert(count == 3);
  assert(strncmp(data, "FGH", 3) == 0);
  //peek
  rm.Peek(0, buff2, 2, &count);
  assert(count == 2);
  assert(strncmp(buff2, "DE", 2) == 0);

  rm.Peek(1, buff2, 3, &count);
  assert(count == 3);
  assert(strncmp(buff2, "EFG", 3) == 0);

  rm.Peek(2, buff2, 3, &count);
  assert(count == 3);
  assert(strncmp(buff2, "FGH", 3) == 0);

  rm.Peek(3, buff2, 3, &count);
  assert(count == 2);
  assert(strncmp(buff2, "GH", 2) == 0);

  //discard
  rm.Discard(2, &count);
  assert(count == 2);
  rm.Locate(0, &data, 3, &count);
  assert(count == 3);
  assert(strncmp(data, "FGH", 3) == 0);
}
void test_rm4_2()
{
  char tempbuff[128];
  //test Allocate
  PCHAR data;
  char buff2[16];
  int  count;
  char *buff;
  union
  {
    void    *handle;
    RingMem *prm;
  };
  handle = tempbuff;
  int moduleSize = RingMem::AlignedModuleSize();
  prm->Reset(tempbuff, moduleSize + 5, 1);

  RingMem &rm = *prm;
  assert(rm.BufferSize() == 5);

  buff = tempbuff + moduleSize;

  rm.Allocate(&data, 3, &count);
  assert(count == 3);
  rm.Commit(0, &count);
  assert(count == 0);

  rm.Allocate(&data, 3, &count);
  assert(count == 3);
  data[0] = 'A';
  data[1] = 'B';
  data[2] = 'C';
  assert(strncmp(buff, "ABC", 3) == 0);
  rm.Commit(3, &count);
  assert(count == 3);

  //commit(0)
  rm.Allocate(&data, 3, &count);
  assert(count == 2);
  rm.Commit(0, &count);
  assert(count == 0);

  rm.Allocate(&data, 2, &count);
  assert(count == 2);
  rm.Commit(0, &count);
  assert(count == 0);

  rm.Allocate(&data, 3, &count);
  assert(count == 2);
  data[0] = 'D';
  data[1] = 'E';
  assert(strncmp(buff, "ABCDE", 5) == 0);
  rm.Commit(3, &count);
  assert(count == 2);

  rm.Allocate(&data, 3, &count);
  assert(count == 3);
  data[0] = 'F';
  data[1] = 'G';
  data[2] = 'H';
  assert(strncmp(buff, "FGHDE", 3) == 0);
  rm.Commit(3, &count);
  assert(count == 3);
  //locate
  rm.Locate(0, &data, 2, &count);
  assert(count == 2);
  assert(strncmp(data, "DE", 2) == 0);

  rm.Locate(1, &data, 3, &count);
  assert(count == 1);
  assert(strncmp(data, "E", 1) == 0);

  rm.Locate(2, &data, 3, &count);
  assert(count == 3);
  assert(strncmp(data, "FGH", 3) == 0);
  //peek
  rm.Peek(0, buff2, 2, &count);
  assert(count == 2);
  assert(strncmp(buff2, "DE", 2) == 0);

  rm.Peek(1, buff2, 3, &count);
  assert(count == 3);
  assert(strncmp(buff2, "EFG", 3) == 0);

  rm.Peek(2, buff2, 3, &count);
  assert(count == 3);
  assert(strncmp(buff2, "FGH", 3) == 0);

  rm.Peek(3, buff2, 3, &count);
  assert(count == 2);
  assert(strncmp(buff2, "GH", 2) == 0);

  //discard
  rm.Discard(2, &count);
  assert(count == 2);
  rm.Locate(0, &data, 3, &count);
  assert(count == 3);
  assert(strncmp(data, "FGH", 3) == 0);
}
//flat ring mem
void test_rm5()
{
  //mode relative addr
  enum { buffSize = 256, };
  char buff[buffSize];
  union {
    void    *handle;
    RingMem *prm;
  };
  handle = buff;
  prm->Reset(buff, buffSize, 1);

  char outBuff[16];
  int  count;
  prm->Write("abc", 4, &count);
  assert(count == 4);
  prm->Read(outBuff, 4, &count);
  assert(strncmp("abc", outBuff, 3) == 0);

  //mode absolute
  RingMem rm(0, 0);
  rm.Reset(buff, buffSize);
  rm.Write("abc", 4, &count);
  assert(count == 4);
  rm.Read(outBuff, 4, &count);
  assert(strncmp("abc", outBuff, 3) == 0);
}
//flat ring mem
void test_rm6()
{
  //mode relative addr
  enum { buffSize = 256, };
  char buff[buffSize];
  union {
    void    *handle;
    RingMem *prm;
  };
  handle = buff;
  prm->Reset(buff, buffSize, RingMem::RingMemAddrModeRelative);

  char outBuff[16];
  int  count;
  prm->Write("abc", 4, &count);
  assert(count == 4);
  prm->Read(outBuff, 4, &count);
  assert(strncmp("abc", outBuff, 3) == 0);

  //mode absolute
  RingMem rm(0, 0);
  char buff2[buffSize];
  prm->Write("def", 4, &count);
  assert(count == 4);

  //missing asign: mode miss match
  rm = *prm;
  assert(rm.IsEmpty());

  rm.Reset(buff2, buffSize);
  rm.Write("abc", 4, &count);
  assert(count == 4);

  //missing asign: mode relative - absolute
  *prm = rm;

  prm->Read(outBuff, sizeof(outBuff), &count);
  assert(strncmp(outBuff, "def", 3) == 0);
  rm.Read(outBuff, 4, &count);
  assert(strncmp("abc", outBuff, 3) == 0);

  //mode absolute asign
  rm.Write("12345", 5, &count);
  RingMem rm2(0, 0);
  //NOTE: RingMem rm2 = rm; ->not call operator=() ->error: lock handle not duplicated
  // so disable lock function
  rm2 = rm;
  //+ rm2 has all data and state of rm
  rm2.Read(outBuff, 4, &count);
  assert(count == 4);
  assert(strncmp(outBuff, "1234", 4) == 0);
  assert(rm2.Count() == 1);
  //+ rm is out of date, so after asign should not use rm anymore
  assert(rm.Count() == 5);
  rm.Peek(0, outBuff, 1, &count);
  assert(outBuff[0] == '1');
  rm2.Peek(0, outBuff, 1, &count);
  assert(outBuff[0] == '5');

  //mode relative asign
  prm->Reset(buff, buffSize, 1);
  prm->Write("12345", 5, &count);
  RingMem *prm2 = prm;
  prm2->Read(outBuff, 1, &count);
  prm2->Peek(0, outBuff, 1, &count);
  assert(outBuff[0] == '2');
  prm->Peek(0, outBuff, 1, &count);
  assert(outBuff[0] == '2');
}
//rm as stack
void test_stackrm2()
{
  //init buffer
  enum { bufferSize = 4 };
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
  rm.Read(buff + 2, 2, &n);
  rm.Read(buff, 1, &n);
}
void test_stackrm()
{
  //init buffer
  enum { bufferSize = 8 };
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
  assert(strncmp(buff2 + 3, "bc", 2) == 0);
  assert(strncmp(buff2 + 5, text, 3) == 0);

  char* text2 = "abcdefgh";
  rc = rm.Write(text, 7, &n);
  rc = rm.Write(text2, 8, &n);
  rc = rm.Read(buff2, 4, &n);
  assert(strncmp(buff2, text2, n) == 0);
  rc = rm.Write(text, 6, &n);
  rc = rm.Read(buff2, n, &n);
  assert(strncmp(buff2, text, 5) == 0);
}
//rm with vmem
void test_rmvm()
{
  enum {
    nPages = 2,
    vmSize = 3,     //by page
    sysPgSize = 2,  //smal page for test purpose
  };

  int rc;
  //int sysPgSize;
  int bufferSize;
  char* buffer;

  //1. init buffer
  bufferSize = nPages * PageMng::CalcPgSize(sysPgSize);
  buffer = (char*)malloc(bufferSize);

  //2. init file swap
  TCHAR* zFile = TEXT("C:\\tmp\\test.txt");
  FileSwapParams fsp;
  fsp.mode = FileSwapParams::ModeNormal;
  fsp.zName = zFile;
  FileSwap fs(&fsp);
  fs.Truncate(0);

  //3. init vmem
  VMemParams vmp;
  vmp.buffer = buffer;
  vmp.bufferSize = bufferSize;
  vmp.pgDataSize = sysPgSize;
  vmp.hFile = &fs;
  vmp.vMemSize = vmSize * sysPgSize;
  VMem vm;
  rc = vm.Init(&vmp);

  //init ring mem
  RingMemParams rmp;
  rmp.hVMem = &vm;
  rmp.type = RingMem::RingMemTypeQueue;
  rmp.mode = RingMem::RingMemVMemMode;
  RingMem rm(&rmp);

  //rmsize = 6
  {
    char* text = "1234567890";
    char buff[10];
    int n;

    //+ test write/read
    rm.Write(text, 1, &n);
    rm.Write(text + 1, 4, &n);
    rm.Write(text + 5, 3, &n);
    rm.Peek(0, buff, 10, &n);
    assert(n == 6);
    assert(strncmp(buff, "345678", n) == 0);
    rm.Write(text + 8, 2, &n);
    rm.Peek(0, buff, 10, &n);
    assert(n == 6);
    assert(strncmp(buff, "567890", n) == 0);

    //+ test write/read over size
    rm.Write(text, 7, &n);
    assert(n == 6);
    rm.Read(buff, 10, &n);
    assert(strncmp(buff, text, n) == 0);

    //+ test locate
    rm.Write(text, 5, &n);
    assert(n == 5);
    char* data;
    rm.Locate(0, &data, 4, &n);
    assert(n == 2);
    assert(strncmp(data, text, n) == 0);

    rm.Locate(2, &data, 4, &n);
    assert(n == 2);
    assert(strncmp(data, text + 2, n) == 0);

    rm.Locate(3, &data, 4, &n);
    assert(n == 1);
    assert(strncmp(data, text + 3, n) == 0);

    rm.Locate(4, &data, 4, &n);
    assert(n == 1);
    assert(strncmp(data, text + 4, n) == 0);

    rc = rm.Locate(5, &data, 4, &n);
    assert(n == 0);

    rm.Commit(4);
  }

  vm.Final();
  free(buffer);
}
int main()
{
  test_rm();
  test_rm2();
  test_rm3();
  test_rm4();
  test_rm5();
  test_rm6();
  test_stackrm();
  test_stackrm2();
  test_rmvm();
}