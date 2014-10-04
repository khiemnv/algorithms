#include "avltree.h"

#include <stdlib.h>

#if(DEBUG_MODE)
#include <crtdbg.h>
#define assert      _ASSERT
#define MY_ASSERT   assert
#include <stdio.h>
#define MY_PRINTF   printf
#else
#define assert(...)
#define MY_ASSERT(...)
#define MY_PRINTF(...)
#endif

#include <string.h>

//-------------AVLtree_string
//+++++implement node string
#if(DEBUG_NODE_STR)
//+++++implement node string
struct node_str {
    node_str* pLeft;
    node_str* pRight;
    char* zKey;
    int status;
};
#else
typedef struct node_str {
    NodePtr pLeft;
    NodePtr pRight;
    char* zKey;
    int status;
} node_str;
#endif
//node.left
static void node_str_set_left_child(NodePtr pParent, NodePtr pNode) {
    ((node_str*)pParent)->pLeft = pNode;
}
static NodePtr node_str_get_left_child(NodePtr pParent) {
    return ((node_str*)pParent)->pLeft;
}
//node.right
static void node_str_set_right_child(NodePtr pParent, NodePtr pNode) {
    ((node_str*)pParent)->pRight = pNode;
}
NodePtr node_str_get_right_child(NodePtr pParent) {
    return ((node_str*)pParent)->pRight;
}
//node.status
static void node_str_set_status(NodePtr pNode, int status) {
    ((node_str*)pNode)->status = status;
}
static int node_str_get_status(NodePtr pNode) {
    return ((node_str*)pNode)->status;
}
//node.key
static KeyPtr node_str_get_key(NodePtr pNode) {
    return ((node_str*)pNode)->zKey;
}
//node compare
static int node_str_cmp_node(NodePtr l, NodePtr r) {
    char* key1 = ((node_str*)l)->zKey;
    char* key2 = ((node_str*)r)->zKey;
    int rc = strcmp(key1, key2);
    return rc;
}
static int node_str_cmp_key(KeyPtr l, KeyPtr r) {
    return strcmp((char*)l, (char*)r);
}
#if(DEBUG_MODE)
#if(0)
static int node_str_print_node(NodePtr pNode, char* buff, int buffSize)
{
    //assert(buffSize > 12);
    //(+x)'zKey'..
    enum { size = 16, nStat = 4, nChars = 6 };
    union {
        char zArr[16 + 1];
        int iArr[1];
    } stat = { "(-1)(+0)(+1)(NG)" };
    char* zKey = ((node_str*)pNode)->zKey;
    int len = 4;
    int i = (((node_str*)pNode)->status + 1) & 0x03;
    *((int*)buff) = stat.iArr[i];
    for (i = 0, buff[len++] = '\''; zKey[i] && (i < nChars); i++)
        buff[len++] = zKey[i];
    //assert(len < (nStat + 2 + nChars));
    for (buff[len++] = '\''; len < (nStat + 2 + nChars);)
        buff[len++] = ' ';
    buff[len] = 0;
    return len;
}
#else
static int node_str_print_node(NodePtr pNode, char* buff, int buffSize)
{
    enum { size = 16, nChars = 6, nStat = 4 };
    int i;
    union {
        char zCode[size + 1];
        int iCode[1];
    } stat = { "(-1)(+0)(+1)(NG)" };
    char* zKey = ((node_str*)pNode)->zKey;

    //check buffer size
    if (buffSize < (nStat + 2 + nChars + 1)) return 0;

    //++status
    i = (((node_str*)pNode)->status + 1) & 0x03;
    *((int*)buff) = stat.iCode[i];
    buff += 4;
    buff[0] = '\'';
    buff++;

#if(0)
    //++append zKey
    i = strlen(zKey);
    i = i < nChars ? i : nChars;
    //memcpy(buff, zKey, i);
    for (int j = 0; j < i; j++) { buff[j] = zKey[j]; }
    buff[i] = '\'';
    buff++;

    //++padding space
    for (; i < nChars; i++) { buff[i] = ' '; }
    buff[i] = 0;
#else
    int c;
    int flags = 0;
    //flags == 0    ->c = c
    //flags == 1    ->c = ' '
    //c == 0        ->flags = 1; c = '\''
    for (i = 0; i < (nChars + 1); i++) {
        c = zKey[i];
        c = (flags == 0) ? c : ' ';
        flags = (c == 0) ? 1 : flags;
        c = (c == 0) ? '\'' : c;
        buff[i] = (char)c;
    }
    buff[i] = 0;
#endif
    return (nStat + 2 + nChars);
}
#endif
#endif  //DEBUG_MODE
//+++++init AVLtree obj
static AVLtree AVLtree_str = {
    &node_str_set_left_child,
    &node_str_get_left_child,
    &node_str_set_right_child,
    &node_str_get_right_child,

    &node_str_set_status,
    &node_str_get_status,
    &node_str_get_key,

    &node_str_cmp_node,
    &node_str_cmp_key,
#if(DEBUG_MODE)
    &node_str_print_node,
#endif
};
//+++++
void reset_node_str(NodePtr pNode, char* zKey)
{
    MY_ASSERT(pNode != NULL);
    int reqSize = strlen(zKey) + 1;
    ((node_str*)pNode)->zKey = (char*)malloc(reqSize);
    strcpy_s(((node_str*)pNode)->zKey, reqSize, zKey);
    ((node_str*)pNode)->pLeft = NULL;
    ((node_str*)pNode)->pRight = NULL;
    ((node_str*)pNode)->status = NODE_STAT_BALANCED;
}
NodePtr crt_node_str(char* zKey) {
    node_str* pNode = (node_str*)malloc(sizeof(node_str));
    if (pNode) {
        reset_node_str(pNode, zKey);
    }
    return pNode;
}
void del_node_str(NodePtr* pNode) {
    assert(pNode);
    assert(((node_str*)pNode)->zKey);
    free(((node_str*)pNode)->zKey);
    free((node_str*)pNode);
}
//+++++
NodePtr zAVL_insert(NodePtr pRoot, NodePtr pNew)
{
    if (pRoot == NULL) return pRoot;
    return AVLtree_insert(&AVLtree_str, pRoot, pNew, NULL);
}
NodePtr zAVL_remove(NodePtr pRoot, NodePtr pNode)
{
    assert(pRoot != NULL);
    assert(pNode != NULL);
    return AVLtree_remove(&AVLtree_str, pRoot, pNode, NULL);
}
NodePtr zAVL_search_key(NodePtr pRoot, char* zKey)
{
    assert(pRoot);
    NodePtr pNode;
    tree_search_key(&AVLtree_str, pRoot, zKey, &pNode, NULL);
    return pNode;
}
int zAVL_search_node(NodePtr pRoot, NodePtr pNode)
{
    assert(pRoot);
    assert(pNode);
    int rc = tree_search_node(&AVLtree_str, pRoot, pNode, NULL);
    return (rc == 0);
}
#if(DEBUG_MODE)
void zAVLtraverse(NodePtr pRoot)
{
    int size = AVLtree_traverse(&AVLtree_str, pRoot, 0);
    printf("\ntree size %d\n", size);
}
#endif
//+++++
