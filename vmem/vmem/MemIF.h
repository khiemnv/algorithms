#ifndef __MEM_IF__
#define __MEM_IF__

//mem manager
class MemMngIF
{
public:
  virtual int Peek(int index, char *outBuff, int nByte, int *pRead) = 0;
  virtual int Discard(int nByte, int *pDiscard) = 0;
  virtual int Read(char *outBuff, int nByte, int *pRead) = 0;
  virtual int Write(char *data, int nByte, int *pWrite) = 0;
  virtual int Remain() = 0;
  virtual int Count() = 0;
  virtual int Allocate(char** pHandle, int size, int *allocatedSize) = 0;
  virtual int Commit(int nByte, int *commited) = 0;
};
class StackIF
  :public MemMngIF
{
};
class QueueIF
  :public MemMngIF
{
};
#endif //__MEM_IF__
