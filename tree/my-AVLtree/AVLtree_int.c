#include "avltree.h"

#include <stdlib.h>

#if(DEBUG_MODE)
#include <crtdbg.h>
#define assert _ASSERT
#define MY_ASSERT   assert
#include <stdio.h>
#else
#define assert(...)
#define MY_ASSERT(...)
#endif

//-------------AVLtree_int
//+++++implement node int
#if(DEBUG_NODE_INT)
struct node_int {
    node_int* pLeft;
    node_int* pRight;
    int iKey;
    int status;
};
#else
typedef struct node_int {
    NodePtr pLeft;
    NodePtr pRight;
    int iKey;
    int status;
} node_int;
#endif
//node.left
static void node_int_set_left_child(NodePtr pParent, NodePtr pNode) {
    ((node_int*)pParent)->pLeft = pNode;
}
static NodePtr node_int_get_left_child(NodePtr pParent) {
    return ((node_int*)pParent)->pLeft;
}
//node.right
static void node_int_set_right_child(NodePtr pParent, NodePtr pNode) {
    ((node_int*)pParent)->pRight = pNode;
}
NodePtr node_int_get_right_child(NodePtr pParent) {
    return ((node_int*)pParent)->pRight;
}
//node.status
static void node_int_set_status(NodePtr pNode, int status) {
    ((node_int*)pNode)->status = status;
}
static int node_int_get_status(NodePtr pNode) {
    return ((node_int*)pNode)->status;
}
//node.key
static KeyPtr node_int_get_key(NodePtr pNode) {
    return &((node_int*)pNode)->iKey;
}
//node compare
static int node_int_cmp_node(NodePtr l, NodePtr r) {
    int key1 = ((node_int*)l)->iKey;
    int key2 = ((node_int*)r)->iKey;
    int rc = key1 - key2;
    return rc;
}
static int node_int_cmp_key(KeyPtr l, KeyPtr r) {
    return *(int*)l - *(int*)r;
}
//++debug
#if (DEBUG_MODE)
static int node_int_print_node(NodePtr pNode, char* buff, int buffSize) {
    assert(buffSize > 8);
    enum {size = 16};
    union {
        char z[size];
        long long LL[1];
    }key;
    sprintf_s(key.z, size, "(%+d)%+-4d", ((node_int*)pNode)->status, ((node_int*)pNode)->iKey);
    *((long long*)buff) = key.LL[0];
    buff[8] = 0;    //truncate
    return 8;
}
#endif
//+++++init AVLtree obj
static AVLtree AVLtree_int = {
    &node_int_set_left_child,
    &node_int_get_left_child,
    &node_int_set_right_child,
    &node_int_get_right_child,

    &node_int_set_status,
    &node_int_get_status,
    &node_int_get_key,

    &node_int_cmp_node,
    &node_int_cmp_key,
#if(DEBUG_MODE)
    &node_int_print_node,
#endif
};
//+++++
void reset_node_int(NodePtr pNode, int key)
{
    MY_ASSERT(pNode != NULL);
    ((node_int*)pNode)->iKey = key;
    ((node_int*)pNode)->pLeft = NULL;
    ((node_int*)pNode)->pRight = NULL;
    ((node_int*)pNode)->status = NODE_STAT_BALANCED;
}
NodePtr crt_node_int(int key) {
    node_int* pNode = (node_int*)malloc(sizeof(node_int));
    if (pNode) {
        reset_node_int(pNode, key);
    }
    return pNode;
}
void del_node_int(NodePtr pNode)
{
    free(pNode);
}
//+++++
NodePtr iAVL_insert(NodePtr pRoot, NodePtr pNew)
{
    if (pRoot == NULL) return pRoot;
    return AVLtree_insert(&AVLtree_int, pRoot, pNew, NULL);
}
NodePtr iAVL_remove(NodePtr pRoot, NodePtr pNode)
{
    assert(pRoot != NULL);
    assert(pNode != NULL);
    return AVLtree_remove(&AVLtree_int, pRoot, pNode, NULL);
}
NodePtr iAVL_search_key(NodePtr pRoot, int key)
{
    assert(pRoot);
    NodePtr pNode;
    tree_search_key(&AVLtree_int, pRoot, &key, &pNode, NULL);
    return pNode;
}
int iAVL_search_node(NodePtr pRoot, NodePtr pNode)
{
    assert(pRoot);
    assert(pNode);
    int rc = tree_search_node(&AVLtree_int, pRoot, pNode, NULL);
    return (rc == 0);
}
#if(DEBUG_MODE)
void iAVLtraverse(NodePtr pRoot)
{
    int size = AVLtree_traverse(&AVLtree_int, pRoot, 0);
    printf("\ntree size %d\n", size);
}
#endif
//+++++
