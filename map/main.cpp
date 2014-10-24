#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//--------------debug
#define MY_DEBUG   1
#if(MY_DEBUG)
#define TRACE           printf
#define MY_PRINTF       printf
#define MY_PRINTF_LIST  freelist_print
#define MY_ASSERT       assert
#define MY_ASSERT2(x)   my_assert(x);assert(x);
#else
#define TRACE(...)
#define MY_PRINTF(...)
#define MY_PRINTF_LIST(...)
#define MY_ASSERT(...)
#define MY_ASSERT2(...)
#endif  //MY_DEBUG

struct BlockInfo {
  // Size of the block (in the high bits) and tags for whether the
  // block and its predecessor in memory are in use.  See the SIZE()
  // and TAG macros, below, for more details.
  size_t sizeAndTags;
  // Pointer to the next block in the free list.
  struct BlockInfo* next;
  // Pointer to the previous block in the free list.
  struct BlockInfo* prev;

  int status;
};
typedef struct BlockInfo BlockInfo;

/* Alignment of blocks returned by mm_malloc. */
#define ALIGNMENT 8

#define SIZE(x) ((x) & ~(ALIGNMENT - 1))

#if (1) //hash table
#define element_sibling(e)      (e)->prev
#define element_child(e)        (e)->next
#define element_size(e)         SIZE(e->sizeAndTags)
#define element_key(e)          (e)->status
typedef struct BlockInfo element;
void element_reset(BlockInfo* pBlock)
{
  element_sibling(pBlock) = NULL;
  element_child(pBlock) = NULL;
}
int element_cmp(element* left, element *right) {
  size_t rc = element_size(left) - element_size(right);
  return (int)rc;
}
element* element_crt(int key) {
  element* pElem = new element;
  size_t size = (key % 8) << 8;
  memset(pElem, 0, sizeof(element));
  pElem->sizeAndTags = size;
  element_key(pElem) = key;
  return pElem;
}
void element_del(element* pElem) {
  delete pElem;
}
//if found return block,
//else return NULL
element* table_search(element* pRoot, size_t reqSize) {
  element* pCur;
  for (pCur = pRoot; pCur != NULL; pCur = element_sibling(pCur)) {
    if (element_size(pCur) >= reqSize) {
      return pCur;
    }
  }
  return NULL;
};
//return new pRoot
element* table_insert(element* pRoot, element* pBlock) {
  element* pCur = pRoot;
  if (element_size(pCur) == element_size(pBlock)) {
    //pCur->pBlock->pChild
    element_child(pBlock) = element_child(pCur);
    element_child(pCur) = pBlock;
  }
  else if (element_size(pCur) > element_size(pBlock)) {
    //pBlock
    //pCur
    element_sibling(pBlock) = pCur;
    pCur = pBlock;
  }
  else {
    //pCur
    //pBlock
    MY_ASSERT(element_size(pCur) < element_size(pBlock));
    if (element_sibling(pCur) != NULL) {
      element_sibling(pCur) = table_insert(element_sibling(pCur), pBlock);
    }
    else {
      element_sibling(pCur) = pBlock;
    }
  }
  return pCur;
};
int table_printf(BlockInfo* pRoot)
{
  MY_PRINTF("\n");
  if (pRoot == NULL) return 0;

  BlockInfo* pCur = pRoot;
  int count = 0;
  for (; pCur != NULL; pCur = element_child(pCur)) {
    MY_PRINTF("+%2x(%3lx)   ", element_key(pCur), element_size(pCur));
    count++;
  }
  MY_PRINTF("\n");
  count += table_printf(element_sibling(pRoot));
  return count;
}
element* child_remove(BlockInfo* pFirst, BlockInfo* pBlock) {
  MY_PRINTF("  child remove %p %lx %p %lx\n", pFirst, element_size(pFirst), pBlock, element_size(pBlock));
  BlockInfo* pPrev = NULL;
  BlockInfo* pCur;
  for (pCur = pFirst; pCur != NULL; pCur = element_child(pCur)) {
    if (pCur == pBlock) {
      break;
    }
    pPrev = pCur;
  }
  MY_ASSERT(pCur != NULL);
  if (pPrev != NULL) {
    MY_ASSERT(pBlock != pFirst);
    element_child(pPrev) = element_child(pCur);
    return pFirst;
  }
  MY_ASSERT(pBlock == pFirst);
  return element_child(pCur);
}
element* table_remove(BlockInfo* pRoot, BlockInfo* pBlock) {
  MY_PRINTF("  table remove\n");
  element* pCur = pRoot;
  MY_ASSERT(table_search(pRoot, element_size(pBlock)) != NULL);
  if (pCur == pBlock) {
    //pCur->child
    //pSib
    if (element_child(pCur)) {
      element* pSib = element_sibling(pCur);
      pCur = element_child(pCur);
      element_sibling(pCur) = pSib;
    }
    else {
      pCur = element_sibling(pCur);
    }
  }
  else if (element_size(pCur) == element_size(pBlock)) {
    MY_ASSERT(pCur != pBlock);
    MY_ASSERT(element_child(pCur) != NULL);
    element_child(pCur) = child_remove(element_child(pCur), pBlock);
  }
  else if (element_size(pCur) < element_size(pBlock)) {
    MY_ASSERT(element_sibling(pCur) != NULL);
    element_sibling(pCur) = table_remove(element_sibling(pCur), pBlock);
  }
  else {
    MY_ASSERT(element_size(pCur) > element_size(pBlock));
    MY_ASSERT(0);
  }
  return pCur;
};
#endif

//-----------------------------test code

class queue
{
  typedef struct {
    void* pData;
    int level;
  } queue_element;
  int m_size;
  queue_element* m_data;
  int m_iFirst;
  int m_count;
public:
  queue(int size)
  {
    m_size = size;
    m_data = new queue_element[m_size];
    m_iFirst = 0;
    m_count = 0;
  }
  void push(void* pData, int level = -1)
  {
    if (m_count < m_size) {
      int i = m_iFirst + m_count;
      m_count++;
      m_data[i].pData = pData;
      m_data[i].level = level;
    }
  }
  void* pop(int* level = NULL)
  {
    void* pData = NULL;
    if (m_count > 0) {
      pData = m_data[m_iFirst].pData;
      if (level) *level = m_data[m_iFirst].level;
      m_iFirst++;
      m_count--;
    }
    return pData;
  }
  int isEmpty()
  {
    return m_count == 0;
  }
  ~queue()
  {
    delete(m_data);
  }
};
//-----------------------------
void test_1()
{
  element arr[] = {
    { 0x100 },
    { 0x200 },
    { 0x300 },
    { 0x300 },
    { 0x400 },
  };
  element* pRoot = &arr[0];
  for (int i = 1; i < (sizeof(arr) / sizeof(arr[0])); i++) {
    pRoot = table_insert(pRoot, &arr[i]);
    printf("insert %d\n", i);
    table_printf(pRoot);
  }

  for (int i = 0; i < (sizeof(arr) / sizeof(arr[0])); i++) {
    pRoot = table_remove(pRoot, &arr[i]);
    printf("remove %d\n", i);
    table_printf(pRoot);
  }
}
void test_2()
{
  //generate_keys(30, 2);
  int i;
  enum { nNode = 50 };
  int keys[nNode] = {
    0x38, 0x85, 0x37, 0x55, 0x43,
    0x86, 0x78, 0x23, 0x62, 0x57,
    0x24, 0x02, 0x17, 0x34, 0x96,
    0x22, 0x07, 0x89, 0x91, 0x12,
    0x61, 0x05, 0x36, 0x09, 0x41,
    0x00, 0x31, 0x88, 0x94, 0x04,
    0x18, 0x97, 0x68, 0x35, 0x83,
    0x53, 0x93, 0x64, 0x65, 0x03,
    0x58, 0x60, 0x52, 0x15, 0x30,
    0x06, 0x25, 0x32, 0x39, 0x63
  };
  queue q(nNode);
  element* pNode = NULL;
  element* pRoot = NULL;
  //init
  pRoot = element_crt(keys[0]);
  q.push(pRoot);
  //generate data
  for (i = 1; i < nNode;) {
    //add node
    pNode = element_crt(keys[i]);
    q.push(pNode);
    pRoot = table_insert(pRoot, pNode);
    i++;
  }
  //print
  int count = table_printf(pRoot);
  printf("  count %d\n", count);
  //clean
  for (pNode = (element*)q.pop(); pNode != NULL; pNode = (element*)q.pop()) {
    pRoot = table_remove(pRoot, pNode);
    count = table_printf(pRoot);
    printf("  count %d\n", count);
    element_del(pNode);
  }
}
int main()
{
  //test_1();
  test_2();
  return 0;
}