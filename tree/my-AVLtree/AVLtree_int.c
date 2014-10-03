#include "avltree.h"

#include <stdlib.h>

#include <crtdbg.h>
#define assert _ASSERT
#define my_assert   assert

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
    int rc = (key1 > key2) ? 1 :
        (key1 < key2) ? -1 : CMP_RES_EQU;
    return rc;
}
static int node_int_cmp_key(KeyPtr l, KeyPtr r) {
    return *(int*)l - *(int*)r;
}
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
};
//+++++
void reset_node_int(NodePtr pNode, int key)
{
    my_assert(pNode != NULL);
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
    int rc = tree_search_key(&AVLtree_int, pRoot, &key, &pNode, NULL);
    return pNode;
}
int iAVL_search_node(NodePtr pRoot, NodePtr pNode)
{
    assert(pRoot);
    assert(pNode);
    int rc = tree_search_node(&AVLtree_int, pRoot, pNode, NULL);
    return (rc == 0);
}
//+++++
