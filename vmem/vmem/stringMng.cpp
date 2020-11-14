#include <crtdbg.h>
#define assert(x) _ASSERT(x)

#include "stringMng.h"
#include "vmem.h"
#include "ringmem.h"

//-----------------StringQueue-------------------
StringQueue::StringQueue()
{
  m_RMem = 0;
  m_vMem = 0;
  m_fileSwap = 0;
}
StringQueue::StringQueue(void* pRingMem)
{
  m_RMem = 0;
  m_vMem = 0;
  m_RMem = pRingMem;
}
int StringQueue::Init(StringQueueParams* p)
{
  int rc;

  //init ring mem
  int nPages;
  int vmSize; //by pages

  int sysPgSize;
  int bufferSize = p->bufferSize;
  char* buffer = p->buffer;

  //1. init buffer
  //sysPgSize = RingMem::SystemPageSize();
  sysPgSize = 512;
  nPages = bufferSize / PageMng::CalcPgSize(sysPgSize);
  bufferSize = nPages * PageMng::CalcPgSize(sysPgSize);
  assert(nPages > 0);

  //2. init file swap
  char* zFile = p->zTempFile;
  FileSwapParams fsp;
  fsp.mode = FileSwapParams::ModeNoBuffer;
  fsp.zName = zFile;
  FileSwap* pfs = new FileSwap(&fsp);
  rc = pfs->Extend(1 << 30);
  assert(rc);
  m_fileSwap = pfs;

  //3. init vmem
  vmSize = (1 << 30) / sysPgSize;
  VMemParams vmp;
  vmp.buffer = buffer;
  vmp.bufferSize = bufferSize;
  vmp.pgDataSize = sysPgSize;
  vmp.hFile = pfs;
  vmp.vMemSize = vmSize * sysPgSize;
  VMem *pvm = new VMem();
  rc = pvm->Init(&vmp);
  assert(rc == 0);
  m_vMem = pvm;

  //4. init ringmem
  RingMemParams rmp;
  rmp.mode = RingMem::RingMemVMemMode;
  rmp.hVMem = pvm;
  RingMem *prm = new RingMem(&rmp);
  m_RMem = prm;

  return rc;
}
StringQueue::~StringQueue()
{
  ;
}
int StringQueue::Final()
{
  int rc = 0;
  assert(m_RMem && m_vMem && m_fileSwap);
  if (m_vMem)
  {
    VMem* pvm = (VMem*)m_vMem;
    rc = pvm->Final();
    delete pvm;
  }

  if (m_fileSwap)
  {
    FileSwap *pfs = (FileSwap*)m_fileSwap;
    delete pfs;
  }

  if (m_RMem)
  {
    RingMem *prm = (RingMem*)m_RMem;
    delete prm;
  }
  return rc;
}

int StringQueue::Push(char* data, int size)
{
  int rc;
  RingMem *prm = (RingMem*)m_RMem;
  int n;
  int itemSize;
  StringHdr hdr;

  hdr.addr = -1;
  hdr.size = size;

  itemSize = itemAlign(sizeof(hdr)+size);
  if (itemSize > prm->Remain())
    rc = -1;
  else
  {
    rc = prm->Write((char*)&hdr, sizeof(hdr), &n);
    assert(n == sizeof(hdr));
    itemSize -= n;
    rc = prm->Write(data, size, &n);
    assert(n == size);
    itemSize -= n;
    assert(itemSize < itemAlignment);
    if (itemSize)
    {
      rc = prm->Write("0000", itemSize, &n);
      assert(n == itemSize);
    }
  }

  return rc;
}
int StringQueue::Pop(char* buff, int size)
{
  int rc;
  RingMem *prm = (RingMem*)m_RMem;
  int n;
  int itemSize;
  StringHdr hdr;

  rc = prm->Peek(0, (char*)&hdr, sizeof(hdr), &n);
  assert(n == sizeof(hdr));

  itemSize = itemAlign(sizeof(hdr)+hdr.size);
  assert(itemSize <= prm->Count());
  if (size > hdr.size)
  {
    rc = prm->Discard(sizeof(hdr), &n);
    assert(n == sizeof(hdr));
    itemSize -= n;
    rc = prm->Read(buff, hdr.size, &n);
    assert(n == hdr.size);
    itemSize -= n;
    assert(itemSize < itemAlignment);
    if (itemSize)
    {
      rc = prm->Discard(itemSize, &n);
      assert(n == itemSize);
    }
  }

  return rc;
}
