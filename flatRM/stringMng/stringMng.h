#ifndef __STRING_MNG_H__
#define __STRING_MNG_H__

#define ALIGN_BY_4(x) (((x) + (3)) & ~(3))

//string mng
struct StringHdr
{
  int addr;
  int size;
};
struct StringItem :public StringHdr
{
  char data[1];
};
struct StringQueueParams
{
  char* buffer;
  int bufferSize;
  char* zTempFile;
};

class StringQueue
{
private:
  void* m_RMem;
  void* m_vMem;
  void* m_fileSwap;

  enum { itemAlignment = 4 };
  int itemAlign(int size) { return ALIGN_BY_4(size); }
public:
  StringQueue();
  StringQueue(void* pRingMem);
  int Init (StringQueueParams* p);
  int Final();
  ~StringQueue();
  int Push(char* data, int size);
  int Pop(char* buff, int size);
};
#endif  //__STRING_MNG_H__
