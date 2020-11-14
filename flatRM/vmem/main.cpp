#include "vmem.h"

#include <crtdbg.h>
#define assert(x) _ASSERT(x)

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <Windows.h>

//test vmem with file swap
void test_vm()
{
  enum {
    BUFF_SIZE = 64,   //one page
    //BUFF_SIZE = 128,  //3 pages
    PAGE_SIZE = 8,
    VMEM_SIZE = 128,
  };

  //1. init buffer
  char buff[BUFF_SIZE];
  //2. init file swap
  TCHAR* zFile = TEXT("C:\\tmp\\test.txt");
  FileSwap fs(zFile);

  //3. init vmem
  VMemParams vmp;
  vmp.buffer = buff;
  vmp.bufferSize = BUFF_SIZE;
  vmp.pgDataSize = PAGE_SIZE;
  vmp.hFile = &fs;
  vmp.vMemSize = VMEM_SIZE;
  VMem vm;
  vm.Init(&vmp);

  //test read write
  char buff2[128];
  int n;
  memset(buff2, '0', 128);
  //need fill up swap file before random access
  vm.Write(0, buff2, 128, &n);

  vm.Write(0, "123", 4, &n);
  vm.Read(1, buff2 + 1, 2, &n);
  vm.Read(0, buff2, 1, &n);
  vm.Read(3, buff2 + 3, 1, &n);
  vm.Write(16, "456", 3, &n);
  vm.Read(16, buff2, 3, &n);
  vm.Write(14, "12345", 5, &n);
  vm.Read(15, buff2, 3, &n);
  vm.Write(30, "123456789012", 12, &n);
  vm.Read(30, buff2, 12, &n);
  vm.Write(60, "123456789012", 12, &n);
  vm.Read(60, buff2, 12, &n);

  vm.Write(126, "12", 2, &n);
  vm.Write(120, "123456", 6, &n);
  vm.Read(120, buff2, 8, &n);

  //test locate commit with one page
  char* data;
  vm.Locate(30, &data, &n);
  vm.Write(40, "12345678", 8, &n);
  vm.Write(48, "12345678", 8, &n);
  vm.Write(56, "12345678", 8, &n);
  vm.Commit(30);

  vm.Final();
}

//test memif & vmem without file swap
void test_MemIF()
{
  char buff2[256];
  int n;
  int rc;

  MemIF *p = CreateMem();

  rc = p->Write(0, "123", 4, &n);
  p->Read(1, buff2 + 1, 2, &n);
  p->Read(0, buff2, 1, &n);
  p->Read(3, buff2 + 3, 1, &n);
  p->Write(16, "456", 3, &n);

  DestroyMem(p);
}

//test vmem without file swap
void test_VMem()
{
  VMem c;
  int n;
  char buff[256];
  char buff2[256];
  c.Init(buff, sizeof(buff));

  c.Write(0, "123", 4, &n);
  c.Read(1, buff2 + 1, 2, &n);
  c.Read(0, buff2, 1, &n);
  c.Read(3, buff2 + 3, 1, &n);
  c.Write(16, "456", 3, &n);
  c.Read(16, buff2, 3, &n);
  c.Write(14, "12345", 5, &n);
  c.Read(15, buff2, 3, &n);
  c.Write(30, "123456789012", 12, &n);
  c.Read(30, buff2, 12, &n);
  c.Write(60, "123456789012", 12, &n);
  c.Read(60, buff2, 12, &n);

  c.Final();
}
//test extend/truncate - file nobuffer
void test_FileSwap1()
{
  FileSwapParams fsp;
  fsp.mode = FileSwapParams::ModeNoBuffer;
  fsp.zName = TEXT("c:\\tmp\\test.txt");
  FileSwap fs(&fsp);

  int rc;
  int size;
  int n;
  int addr = 0;
  char* buff;

  //test extend size != n*512
  rc = fs.Truncate(0);
  assert(fs.Size() == 0);
  size = 100;
  rc = fs.Extend(size);
  assert(fs.Size() == size);
  size = 1024;
  rc = fs.Extend(size);
  assert(fs.Size() == size);

  //test truncate size != n*512
  size = 200;
  rc = fs.Truncate(size);
  assert(rc != 0);
  assert(fs.Size() != size);

  //test truncate size = n*512
  size = 512;
  rc = fs.Truncate(size);
  assert(fs.Size() == size);

  //test extend size = n*512
  rc = fs.Truncate(0);
  assert(fs.Size() == 0);
  size = (1 << 30);
  rc = fs.Extend(size);
  assert(size == fs.Size());

  //+ test read write in larger addr
  //+ begin addr
  size = 512;
  buff = (char*)malloc(size);
  memset(buff, '0', size);
  rc = fs.Write(0, buff, size, &n);
  assert(n = size);
  rc = fs.Read(0, buff, size, &n);
  assert(n == size);
  //+ mid addr
  srand((unsigned)time(NULL));
  int range_min = 1;
  int range_max = ((1 << 30) / 512) - 1;
  int u = (double)rand() / (RAND_MAX + 1) * (range_max - range_min)
    + range_min;
  addr = u * 512;
  rc = fs.Write(addr, buff, size, &n);
  assert(n == 512);
  rc = fs.Read(addr, buff, size, &n);
  assert(n == 512);
  //+ end addr
  addr = (1 << 30) - size;
  rc = fs.Write(addr, buff, size, &n);
  assert(n == 512);
  rc = fs.Read(addr, buff, size, &n);
  assert(n == 512);

  free(buff);
}
//test read/write file nobuffer & compare with normal mode
void test_FileSwap2()
{
  int rc;
  int n;
  char buff[512];
  TCHAR* zFile = TEXT("c:\\tmp\\100.txt");
  FileSwapParams fsp;

  //crt file for test
  //in normal mode
  //+ test read/write size != n*512
  fsp.mode = FileSwapParams::ModeNormal;
  fsp.zName = zFile;
  FileSwap *pfs = new FileSwap(&fsp);
  rc = pfs->Truncate(0);
  memset(buff, '0', 100);
  rc = pfs->Write(0, buff, 100, &n);
  assert(n == 100);
  rc = pfs->Read(0, buff, 100, &n);
  assert(n == 100);
  delete pfs;

  //int no buffer mode
  //+ test read/write file size != n*512, size = n*512
  fsp.mode = FileSwapParams::ModeNoBuffer;
  fsp.zName = zFile;
  FileSwap *pfs2 = new FileSwap(&fsp);
  memset(buff, '1', 512);
  rc = pfs2->Read(0, buff, 100, &n);
  assert(n == 0);
  rc = pfs2->Read(0, buff, 512, &n);
  assert(n == 100);
  rc = pfs2->Write(0, buff, 200, &n);
  assert(n == 0);
  memset(buff, '1', 512);
  rc = pfs2->Write(0, buff, 512, &n);
  assert(n == 512);
  delete pfs2;
}

//test vmem with file swap no buffer
void test_vm_nobuffer()
{
  enum {
    nPages = 2,
    vmSize = 8,   //by page
  };

  int rc;
  int sysPgSize;
  int bufferSize;
  char* buffer;

  //1. init buffer
  sysPgSize = 512;
  bufferSize = nPages * PageMng::CalcPgSize(sysPgSize);
  buffer = (char*)malloc(bufferSize);

  //2. init file swap
  TCHAR* zFile = TEXT("C:\\tmp\\test.txt");
  FileSwapParams fsp;
  fsp.mode = FileSwapParams::ModeNoBuffer;
  fsp.zName = zFile;
  FileSwap fs(&fsp);

  //3. init vmem
  VMemParams vmp;
  vmp.buffer = buffer;
  vmp.bufferSize = bufferSize;
  vmp.pgDataSize = sysPgSize;
  vmp.hFile = &fs;
  vmp.vMemSize = vmSize * sysPgSize;
  VMem vm;
  vm.Init(&vmp);

  //test read write
  char buff2[128];
  int n;
  memset(buff2, '0', 128);
  //need fill up swap file before random access
  for (int i = 0;;)
  {
    rc = vm.Write(i, buff2, 128, &n);
    if (rc != 0)
    {
      assert(i == vm.Size());
      break;
    }
    i += n;
  }

  vm.Write(0, "123", 4, &n);
  vm.Read(1, buff2 + 1, 2, &n);
  vm.Read(0, buff2, 1, &n);
  vm.Read(3, buff2 + 3, 1, &n);
  vm.Write(16, "456", 3, &n);
  vm.Read(16, buff2, 3, &n);
  vm.Write(14, "12345", 5, &n);
  vm.Read(15, buff2, 3, &n);
  vm.Write(30, "123456789012", 12, &n);
  vm.Read(30, buff2, 12, &n);
  vm.Write(60, "123456789012", 12, &n);
  vm.Read(60, buff2, 12, &n);

  vm.Write(126, "12", 2, &n);
  vm.Write(120, "123456", 6, &n);
  vm.Read(120, buff2, 8, &n);

  //test locate commit with one page
  char* data;
  vm.Locate(30, &data, &n);
  vm.Write(sysPgSize + 40, "12345678", 8, &n);
  vm.Write(sysPgSize + 48, "12345678", 8, &n);
  vm.Write(56, "12345678", 8, &n);
  vm.Commit(30);

  vm.Final();
  free(buffer);
}
int main()
{
  test_vm();
  test_MemIF();
  test_VMem();
  test_FileSwap1();
  test_FileSwap2();
  test_vm_nobuffer();
  return 0;
}