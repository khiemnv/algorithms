#include "avltree.h"

#if (DEBUG_MODE)
#include <crtdbg.h>
#define assert      _ASSERT
#define MY_ASSERT   assert

#include <stdio.h>
#define TRACE       printf
#define MY_PRINTF   printf
#else
#ifndef NULL
#define NULL    (void*)0
#endif

#define assert(...)
#define MY_ASSERT(...)
#define TRACE(...)
#define MY_PRINTF(...)
#endif

//++++++node & tree
#define NODE_KEY(pNode)     (pTree->_get_key(pNode))
#define NODE_LEFT(pNode)    (pTree->_get_left(pNode))
#define NODE_RIGHT(pNode)   (pTree->_get_right(pNode))
#define NODE_STAT(pNode)    (pTree->_get_status(pNode))
#define NODE_CMP            pTree->_cmp_node
#define KEY_CMP             pTree->_cmp_key
#define SET_NODE_LEFT(p, c)     pTree->_set_left(p, c)
#define SET_NODE_RIGHT(p, c)    pTree->_set_right(p, c)
#define SET_NODE_STAT(p, s)     pTree->_set_status(p, s)
//++debug
#if(DEBUG_MODE)
#define NODE_PRINT(pNode,b,s)   pTree->_print_node(pNode,b,s)
#endif

#define RIGHT_HEAVY 1
#define LEFT_HEAVY  -1
#define BALANCED    0
//--------------AVLtree.cpp
#if(1)  //AVLtree
int tree_size(AVLtree* pTree, NodePtr pCur)
{
    int count = 0;
    if (pCur != NULL) {
        count++;
        if (NODE_LEFT(pCur)) count += tree_size(pTree, NODE_LEFT(pCur));
        if (NODE_RIGHT(pCur)) count += tree_size(pTree, NODE_RIGHT(pCur));
    }
    return count;
}

int tree_search_key(AVLtree* pTree, NodePtr pRoot, KeyPtr pKey, NodePtr* ppFound, NodePtr* ppParent)
{
    int i = 0;
    int rc;
    assert(pRoot != NULL);
    NodePtr pCur = pRoot;
    NodePtr pPrev = NULL;
    do
    {
        i++;
        rc = KEY_CMP(pKey, NODE_KEY(pCur));
        if (rc == CMP_RES_EQU) break;
        pPrev = pCur;
        pCur = (rc > CMP_RES_EQU) ? NODE_RIGHT(pCur) : NODE_LEFT(pCur);
    } while (pCur);
    if (ppFound)
    {
        *ppFound = pCur;
    }
    if (ppParent)
    {
        *ppParent = pPrev;
    }
    return rc;
}

int tree_search_node(AVLtree* pTree, NodePtr pRoot, NodePtr pNode, NodePtr* ppParent)
{
    int i = 0;
    int rc;
    assert(pRoot != NULL);
    assert(pNode != NULL);
    NodePtr pCur = pRoot;
    NodePtr pPrev = NULL;
    do
    {
        i++;
        rc = NODE_CMP(pNode, pCur);
        if (rc == CMP_RES_EQU) break;
        pPrev = pCur;
        pCur = (rc > CMP_RES_EQU) ? NODE_RIGHT(pCur) : NODE_LEFT(pCur);
    } while (pCur);
    if (ppParent)
    {
        *ppParent = pPrev;
    }
    return rc;
}

void tree_insert(AVLtree* pTree, NodePtr pRoot, NodePtr pNode)
{
    MY_PRINTF("  insert root %p node %p\n", pRoot, pNode);
    assert(pRoot != NULL);
    assert(NODE_RIGHT(pNode) == NULL);
    assert(NODE_LEFT(pNode) == NULL);
    NodePtr pParent;
    int rc = tree_search_node(pTree, pRoot, pNode, &pParent);
    assert(rc != CMP_RES_EQU);
    assert(pParent != NULL);
    if (rc > CMP_RES_EQU)
    {
        //insert right
        assert(NODE_CMP(pNode, pParent) > CMP_RES_EQU);
        assert(NODE_RIGHT(pParent) == NULL);
        SET_NODE_RIGHT(pParent, pNode);
    }
    else
    {
        assert(NODE_CMP(pNode, pParent) < CMP_RES_EQU);
        assert(NODE_LEFT(pParent) == NULL);
        SET_NODE_LEFT(pParent, pNode);
    }
}
NodePtr tree_min(AVLtree* pTree, NodePtr pRoot)
{
    assert(pRoot != NULL);
    NodePtr pCur = pRoot;
    while (NODE_LEFT(pCur) != NULL)
    {
        pCur = NODE_LEFT(pCur);
    }
    return pCur;
}
NodePtr tree_replace(AVLtree* pTree, NodePtr pParent, NodePtr pNode, NodePtr pNew)
{
    if (pParent)
    {
        if (NODE_LEFT(pParent) == pNode)
        {
            SET_NODE_LEFT(pParent, pNew);
        }
        else
        {
            SET_NODE_RIGHT(pParent, pNew);
        }
        return pParent;
    }
    return pNew;
}
NodePtr tree_remove(AVLtree* pTree, NodePtr pRoot, NodePtr pNode)
{
    assert(pRoot != NULL);
    NodePtr pParent;
    NodePtr pCur = pNode;
    NodePtr pSuc;
    int rc = tree_search_node(pTree, pRoot, pNode, &pParent);
    MY_ASSERT(pCur != NULL);
    assert(rc == CMP_RES_EQU);

    if (NODE_LEFT(pCur) && NODE_RIGHT(pCur))
    {
        pSuc = tree_min(pTree, NODE_RIGHT(pCur));
        SET_NODE_RIGHT(pSuc, tree_remove(pTree, NODE_RIGHT(pCur), pSuc));
        SET_NODE_LEFT(pSuc, NODE_LEFT(pCur));
        tree_replace(pTree, pParent, pCur, pSuc);
    }
    else if (NODE_LEFT(pCur))
    {
        pSuc = tree_replace(pTree, pParent, pCur, NODE_LEFT(pCur));
    }
    else if (NODE_RIGHT(pCur))
    {
        pSuc = tree_replace(pTree, pParent, pCur, NODE_RIGHT(pCur));
    }
    else
    {
        pSuc = tree_replace(pTree, pParent, pCur, NULL);
    }
    return (pRoot == pCur) ? pSuc : pRoot;
}

//-------------AVLTree
//-------------rotate
NodePtr AVLtree_single_rotate_left(AVLtree* pTree, NodePtr pParent)
{
    TRACE("  single rotate left\n");

    NodePtr pRightChild = NODE_RIGHT(pParent);
    SET_NODE_STAT(pParent, BALANCED);
    SET_NODE_STAT(pRightChild, BALANCED);
    SET_NODE_RIGHT(pParent, NODE_LEFT(pRightChild));
    SET_NODE_LEFT(pRightChild, pParent);
    return pRightChild;
}
NodePtr AVLtree_single_rotate_right(AVLtree* pTree, NodePtr pParent)
{
    TRACE("  single rotate right\n");
    NodePtr pLeftChild = NODE_LEFT(pParent);
    SET_NODE_STAT(pLeftChild, BALANCED);
    SET_NODE_STAT(pParent, BALANCED);
    SET_NODE_LEFT(pParent, NODE_RIGHT(pLeftChild));
    SET_NODE_RIGHT(pLeftChild, pParent);
    return pLeftChild;
}
//only use when remove node
NodePtr AVLtree_special_rotate_left(AVLtree* pTree, NodePtr pParent)
{
    TRACE("  special rotate left\n");
    NodePtr pRightChild = NODE_RIGHT(pParent);
    SET_NODE_STAT(pRightChild, LEFT_HEAVY);
    SET_NODE_STAT(pParent, RIGHT_HEAVY);
    SET_NODE_RIGHT(pParent, NODE_LEFT(pRightChild));
    SET_NODE_LEFT(pRightChild, pParent);
    return pRightChild;
}
//only use when remove node
NodePtr AVLtree_special_rotate_right(AVLtree* pTree, NodePtr pParent)
{
    TRACE("  special rotate right\n");
    NodePtr pLeftChild = NODE_LEFT(pParent);
    SET_NODE_STAT(pLeftChild, RIGHT_HEAVY);
    SET_NODE_STAT(pParent, LEFT_HEAVY);
    SET_NODE_LEFT(pParent, NODE_RIGHT(pLeftChild));
    SET_NODE_RIGHT(pLeftChild, pParent);
    return pLeftChild;
}
NodePtr AVLtree_double_rotate_left(AVLtree* pTree, NodePtr pParent)
{
    TRACE("  double rotate left\n");

    NodePtr pRightChild = NODE_RIGHT(pParent);
    NodePtr pNewParent = NODE_LEFT(pRightChild);

    if (NODE_STAT(pNewParent) == LEFT_HEAVY) {
        SET_NODE_STAT(pParent, BALANCED);
        SET_NODE_STAT(pRightChild, RIGHT_HEAVY);
    }
    else if (NODE_STAT(pNewParent) == BALANCED) {
        SET_NODE_STAT(pParent, BALANCED);
        SET_NODE_STAT(pRightChild, BALANCED);
    }
    else {
        SET_NODE_STAT(pParent, LEFT_HEAVY);
        SET_NODE_STAT(pRightChild, BALANCED);
    }
    SET_NODE_STAT(pNewParent, BALANCED);

    SET_NODE_LEFT(pRightChild, NODE_RIGHT(pNewParent));
    SET_NODE_RIGHT(pNewParent, pRightChild);
    SET_NODE_RIGHT(pParent, NODE_LEFT(pNewParent));
    SET_NODE_LEFT(pNewParent, pParent);

    return pNewParent;
}
NodePtr AVLtree_double_rotate_right(AVLtree* pTree, NodePtr pParent)
{
    TRACE("  double rotate right\n");

    NodePtr pLeftChild = NODE_LEFT(pParent);
    NodePtr pNewParent = NODE_RIGHT(pLeftChild);

    if (NODE_STAT(pNewParent) == RIGHT_HEAVY) {
        SET_NODE_STAT(pParent, BALANCED);
        SET_NODE_STAT(pLeftChild, LEFT_HEAVY);
    }
    else if (NODE_STAT(pNewParent) == BALANCED) {
        SET_NODE_STAT(pParent, BALANCED);
        SET_NODE_STAT(pLeftChild, BALANCED);
    }
    else {
        SET_NODE_STAT(pParent, RIGHT_HEAVY);
        SET_NODE_STAT(pLeftChild, BALANCED);
    }
    SET_NODE_STAT(pNewParent, BALANCED);

    SET_NODE_RIGHT(pLeftChild, NODE_LEFT(pNewParent));
    SET_NODE_LEFT(pNewParent, pLeftChild);
    SET_NODE_LEFT(pParent, NODE_RIGHT(pNewParent));
    SET_NODE_RIGHT(pNewParent, pParent);

    return pNewParent;
}
NodePtr AVLtree_update_left(AVLtree* pTree, NodePtr pParent, int *pHeightChg)
{
    int heightChg = 0;
    NodePtr pLeftChild = NODE_LEFT(pParent);

    if (NODE_STAT(pLeftChild) == LEFT_HEAVY) {
        pParent = AVLtree_single_rotate_right(pTree, pParent);
    }
    else if (NODE_STAT(pLeftChild) == BALANCED) {
        //rebalance after remove
        pParent = AVLtree_special_rotate_right(pTree, pParent);
        heightChg = 1;
    }
    else {
        assert(NODE_STAT(pLeftChild) == RIGHT_HEAVY);
        pParent = AVLtree_double_rotate_right(pTree, pParent);
    }
    *pHeightChg = heightChg;
    return pParent;
}
NodePtr AVLtree_update_right(AVLtree* pTree, NodePtr pParent, int *pHeightChg)
{
    int heightChg = 0;
    NodePtr pRightChild = NODE_RIGHT(pParent);

    if (NODE_STAT(pRightChild) == RIGHT_HEAVY) {
        pParent = AVLtree_single_rotate_left(pTree, pParent);
    }
    else if (NODE_STAT(pRightChild) == BALANCED) {
        //rebalance after remove
        pParent = AVLtree_special_rotate_left(pTree, pParent);
        heightChg = 1;
    }
    else {
        assert(NODE_STAT(pRightChild) == LEFT_HEAVY);
        pParent = AVLtree_double_rotate_left(pTree, pParent);
    }
    *pHeightChg = heightChg;
    return pParent;
}
//-------------alter
NodePtr AVLtree_insert_left(AVLtree* pTree, NodePtr pRoot, NodePtr pNode, int* pHeightChg)
{
    int heightChg = 0;
    NodePtr pLeftChild = NODE_LEFT(pRoot);

    //insert left
    //+ if leaf
    if (pLeftChild == NULL) {
        SET_NODE_LEFT(pRoot, pNode);
        SET_NODE_STAT(pRoot, NODE_STAT(pRoot) - 1);
        heightChg = (NODE_STAT(pRoot) == LEFT_HEAVY);
    }
    else {
        //insert to left child
        int childHeightChg;
        SET_NODE_LEFT(pRoot, AVLtree_insert(pTree, pLeftChild, pNode, &childHeightChg));
        SET_NODE_STAT(pRoot, NODE_STAT(pRoot) - childHeightChg);
        //rebalance
        if (NODE_STAT(pRoot) < LEFT_HEAVY) {
            pRoot = AVLtree_update_left(pTree, pRoot, &heightChg);
        }
        heightChg = childHeightChg && (NODE_STAT(pRoot) == LEFT_HEAVY);
    }
    *pHeightChg = heightChg;
    return pRoot;
}
NodePtr AVLtree_insert_right(AVLtree* pTree, NodePtr pRoot, NodePtr pNode, int* pHeightChg)
{
    int heightChg = 0;
    NodePtr pRightChild = NODE_RIGHT(pRoot);

    //insert right
    //+ if leaf
    if (NODE_RIGHT(pRoot) == NULL) {
        SET_NODE_RIGHT(pRoot, pNode);
        SET_NODE_STAT(pRoot, NODE_STAT(pRoot) + 1);
        heightChg = (NODE_STAT(pRoot) == RIGHT_HEAVY);
    }
    else {
        //insert to the right
        int childHeightChg;
        SET_NODE_RIGHT(pRoot, AVLtree_insert(pTree, pRightChild, pNode, &childHeightChg));
        SET_NODE_STAT(pRoot, NODE_STAT(pRoot) + childHeightChg);
        //rebalance
        if (NODE_STAT(pRoot) > RIGHT_HEAVY) {
            pRoot = AVLtree_update_right(pTree, pRoot, &heightChg);
        }
        heightChg = childHeightChg && (NODE_STAT(pRoot) == RIGHT_HEAVY);
    }
    *pHeightChg = heightChg;
    return pRoot;
}
NodePtr AVLtree_insert(AVLtree* pTree, NodePtr pRoot, NodePtr pNode, int* pHeightChg)
{
    assert(pRoot != NULL);
    assert(pNode != NULL);
    assert(NODE_STAT(pNode) == BALANCED);

    int heightChg = 0;
    //left insert
    //if (NODE_KEY(pRoot) > NODE_KEY(pNode)) {
    if (NODE_CMP(pRoot, pNode) > CMP_RES_EQU) {
        pRoot = AVLtree_insert_left(pTree, pRoot, pNode, &heightChg);
    }
    else {
        pRoot = AVLtree_insert_right(pTree, pRoot, pNode, &heightChg);
    }
    if (pHeightChg != 0) {
        *pHeightChg = heightChg;
    }
    return pRoot;
}
//remove a node
NodePtr AVLtree_remove_left(AVLtree* pTree, NodePtr pRoot, NodePtr pNode, int* pHeightChg)
{
    int childHeightChg;
    int heightChg = 0;
    NodePtr pLeftChild = NODE_LEFT(pRoot);

    SET_NODE_LEFT(pRoot, AVLtree_remove(pTree, pLeftChild, pNode, &childHeightChg));
    SET_NODE_STAT(pRoot, NODE_STAT(pRoot) + childHeightChg);
    if (NODE_STAT(pRoot) > RIGHT_HEAVY) {
        pRoot = AVLtree_update_right(pTree, pRoot, &heightChg);
    }
    heightChg = childHeightChg && (NODE_STAT(pRoot) == BALANCED);
    *pHeightChg = heightChg;
    return pRoot;
}
NodePtr AVLtree_remove_right(AVLtree* pTree, NodePtr pRoot, NodePtr pNode, int* pHeightChg)
{
    int childHeightChg;
    int heightChg = 0;
    NodePtr pRightChild = NODE_RIGHT(pRoot);

    SET_NODE_RIGHT(pRoot, AVLtree_remove(pTree, pRightChild, pNode, &childHeightChg));
    SET_NODE_STAT(pRoot, NODE_STAT(pRoot) - childHeightChg);
    if (NODE_STAT(pRoot) < LEFT_HEAVY) {
        pRoot = AVLtree_update_left(pTree, pRoot, &heightChg);
    }
    heightChg = childHeightChg && (NODE_STAT(pRoot) == BALANCED);
    *pHeightChg = heightChg;
    return pRoot;
}
NodePtr AVLtree_remove(AVLtree* pTree, NodePtr pRoot, NodePtr pNode, int* pHeightChg)
{
    assert(pRoot != NULL);
    assert(pNode != NULL);

    int heightChg = 0;
    int childHeightChg;
    NodePtr pSuc = NULL;
    NodePtr pLeftChild = NODE_LEFT(pRoot);
    NodePtr pRightChild = NODE_RIGHT(pRoot);

    //if (NODE_KEY(pRoot) == NODE_KEY(pNode)) {
    if (NODE_CMP(pRoot, pNode) == CMP_RES_EQU) {
        //replace with successor
        if (pLeftChild && pRightChild) {
            pSuc = tree_min(pTree, pRightChild);
            SET_NODE_RIGHT(pSuc, AVLtree_remove(pTree, pRightChild, pSuc, &childHeightChg));
            SET_NODE_LEFT(pSuc, pLeftChild);
            SET_NODE_STAT(pSuc, NODE_STAT(pRoot) - childHeightChg);
            if (NODE_STAT(pSuc) < LEFT_HEAVY) {
                //rebalance
                pSuc = AVLtree_update_left(pTree, pSuc, &heightChg);
            }
            heightChg = childHeightChg && (NODE_STAT(pSuc) == BALANCED);
        }
        else {
            pSuc = (pLeftChild != NULL) ? pLeftChild : pRightChild;
            heightChg = 1;
        }
    }
    //else if (NODE_KEY(pRoot) > NODE_KEY(pNode)) {
    else if (NODE_CMP(pRoot, pNode) > CMP_RES_EQU) {
        //left remove
        pRoot = AVLtree_remove_left(pTree, pRoot, pNode, &heightChg);
    }
    else {
        assert(NODE_CMP(pRoot, pNode) < CMP_RES_EQU);
        //right remove
        pRoot = AVLtree_remove_right(pTree, pRoot, pNode, &heightChg);
    }

    if (pHeightChg != 0) {
        *pHeightChg = heightChg;
    }

    return (pRoot == pNode) ? pSuc : pRoot;
}

#if(DEBUG_MODE)
//k0+-k1+-k3
//      +-k4
//  +-k2
int AVLtree_traverse(AVLtree* pTree, NodePtr pRoot, int level)
{
    int i;
    int count;
    char buff[16];
    if (pRoot == NULL) return 0;
    //printf("+-%2d(%2d)", (int)NODE_KEY(pRoot), NODE_STAT(pRoot));
    i = NODE_PRINT(pRoot, buff, 16);
    printf("+-%s", buff);
    count = 1 + AVLtree_traverse(pTree, NODE_LEFT(pRoot), level + 1);
    if (NODE_RIGHT(pRoot) != NULL) {
        printf("\n");
        for (i--; i >= 0; i--){ buff[i] = ' '; }
        for (i = 0; i <= level; i++) printf("| %s", buff);
        count += AVLtree_traverse(pTree, NODE_RIGHT(pRoot), level + 1);
    }
    return count;
}
#endif  //DEBUG_MODE
#endif  //AVLtree
