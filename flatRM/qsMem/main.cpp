#include <crtdbg.h>
#define assert(x) _ASSERT(x)
#include <string.h>
#include <stdlib.h>

#include "qsMem.h"

void test_queue()
{
  enum {bufferSize = 8};
  char buffer[bufferSize];
  QSMemParams qp = { 0 };
  qp.mFlags = qp.modeQueue | qp.addrAbsolute;
  qp.mBuffer = buffer;
  qp.mBufferSize = bufferSize;
  QSMem qsMem;
  qsMem.Init(&qp);

  char* text = "12345678";
  char buff[8];
  int n;

  qsMem.Write(text, 2, &n);
  assert(n == 2);

  qsMem.Read(buff, 1, &n);
  assert(n == 1);
  assert(strncmp(text, buff, n) == 0);

  qsMem.Read(buff, 2, &n);
  assert(n == 1);
  assert(strncmp(text + 1, buff, n) == 0);

  qsMem.Write(text, 8, &n);
  assert(n == 6);
  assert(qsMem.Remain() == 0);

  qsMem.Read(buff, 8, &n);
  assert(n == 6);
  assert(qsMem.Count() == 0);
  assert(strncmp(text, buff, n) == 0);

  qsMem.Write(text, 8, &n);
  assert(n == 0);
  qsMem.Read(buff, 8, &n);
  assert(n == 0);
}
void test_stack()
{
  enum { bufferSize = 5 };
  char buffer[bufferSize];
  QSMemParams qp = { 0 };
  qp.mFlags = qp.modeStack | qp.addrAbsolute;
  qp.mBuffer = buffer;
  qp.mBufferSize = bufferSize;
  QSMem qsMem;
  qsMem.Init(&qp);

  char* text = "12345678";
  char buff[8];
  int n;

  qsMem.Write(text, 2, &n);
  assert(n == 2);
  assert(qsMem.Remain() == 3);
  assert(qsMem.Count() == 2);

  qsMem.Write(text + 2, 6, &n);
  assert(n == 3);
  assert(qsMem.Remain() == 0);

  qsMem.Read(buff, 8, &n);
  assert(n == 5);
  assert(strncmp(buff, "34512", 5) == 0);
  assert(qsMem.Count() == 0);
  assert(qsMem.Remain() == 5);
}
void test_addr_relative()
{
  enum { bufferSize = 128 };
  char buffer[bufferSize];
  union {
    QSMemParams *qp;
    QSMem *qsm;
  };
  memset(buffer, 0, sizeof(QSMemParams));
  qp = (QSMemParams*)buffer;
  qp->mFlags = qp->modeQueue | qp->addrRelative;
  qp->mBufferSize = bufferSize;
  qsm->Init(qp);

  QSMem &qsMem = *qsm;
  char* text = "12345678";
  char buff[8];
  int n;

  qsMem.Write(text, 2, &n);
  assert(n == 2);

  qsMem.Read(buff, 1, &n);
  assert(n == 1);
  assert(strncmp(text, buff, n) == 0);

  qsMem.Read(buff, 2, &n);
  assert(n == 1);
  assert(strncmp(text + 1, buff, n) == 0);

  qsMem.Write(text, 8, &n);
  assert(n == 8);

  qsMem.Read(buff, 8, &n);
  assert(n == 8);
  assert(qsMem.Count() == 0);
  assert(strncmp(text, buff, n) == 0);
}
int main()
{
  //test_queue();
  //test_stack();
  test_addr_relative();
  return 0;
}
