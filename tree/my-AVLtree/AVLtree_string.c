#include "avltree.h"

#include <stdlib.h>

#include <crtdbg.h>
#define assert _ASSERT
#define my_assert   assert

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
};
//+++++
void reset_node_str(NodePtr pNode, char* zKey)
{
    my_assert(pNode != NULL);
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
    int rc = tree_search_key(&AVLtree_str, pRoot, zKey, &pNode, NULL);
    return pNode;
}
int zAVL_search_node(NodePtr pRoot, NodePtr pNode)
{
    assert(pRoot);
    assert(pNode);
    int rc = tree_search_node(&AVLtree_str, pRoot, pNode, NULL);
    return (rc == 0);
}
//+++++
