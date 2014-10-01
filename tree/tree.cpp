#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//debug
#if(0)
#define TRACE               printf
#define TRAVERSE            tree_traverse_2
#else
#define TRACE(...)
#define TRAVERSE(...)
#endif

//node
#if(1)
#define NODE_RIGHT(node) node->right()
#define NODE_LEFT(node)  node->left()
#define NODE_KEY(node)   node->key()
#define NODE_STAT(node)  node->status()

class tree_key
{
public:
    int m_val;
    void operator=(int val)
    {
        m_val = val;
    }
    void operator=(tree_key& r)
    {
        m_val = r.m_val;
    }
    operator int()
    {
        return m_val;
    }
    int operator-(tree_key& r)
    {
        return cmp(r);
    }
    bool operator>(tree_key& r)
    {
        return cmp(r) > 0;
    }
    bool operator<(tree_key& r)
    {
        return cmp(r) < 0;
    }
    bool operator==(tree_key& r)
    {
        return cmp(r) == 0;
    }
    int operator-(int key)
    {
        return m_val - key;
    }
    bool operator>(int key)
    {
        return m_val > key;
    }
    bool operator<(int key)
    {
        return m_val < key;
    }
    bool operator==(int key)
    {
        return m_val == key;
    }
private:
    int cmp(tree_key& r)
    {
        if (m_val > r.m_val) return 1;
        if (m_val < r.m_val) return -1;
        if (this > &r) return 1;
        if (this < &r) return -1;
        return 0;
    }
};

class node
{
public:
    virtual node*& left() = 0;
    virtual node*& right() = 0;
    virtual tree_key& key() = 0;
    virtual ~node() {}
};

class AVLnode : public node
{
public:
    enum {
        balanced = 0,
        leftHeavy = -1,
        rightHeavy = 1,
    };
    virtual int& status() = 0;
};

class node_int :public node
{
    node* m_pLeft;
    node* m_pRight;
    tree_key m_key;
public:
    node_int(int val)
    {
        m_pLeft = NULL;
        m_pRight = NULL;
        m_key.m_val = val;

        TRACE("  construct node %d\n", val);
    }
    node*& left()
    {
        return m_pLeft;
    }
    node*& right()
    {
        return m_pRight;
    }
    tree_key& key()
    {
        return m_key;
    }
    ~node_int()
    {
        TRACE("  destroy node %d\n", (int)m_key);
    }
};

class AVLnode_int :public AVLnode
{
    tree_key m_key;
    node* m_pLeft;
    node* m_pRight;
    int m_status;
public:
    AVLnode_int(int val)
    {
        m_key = val;
        m_pLeft = NULL;
        m_pRight = NULL;
        m_status = 0;
    }
    tree_key& key() { return m_key; }
    node*& left() { return m_pLeft; }
    node*& right() { return m_pRight; }
    int& status() { return m_status; }
};

#else
#define NODE_RIGHT(node) node->pRight
#define NODE_LEFT(node)  node->pLeft
#define NODE_KEY(node)   node->key
#define NODE_STAT(node)  node->status

class node
{
public:
    node(int val)
    {
        key = val;
        pLeft = 0;
        pRight = 0;
    }
    node* pLeft;
    node* pRight;
    int key;
    virtual ~node() {}
};
typedef class node node_int;
class AVLnode :public node
{
public:
    enum {
        balanced = 0,
        leftHeavy = -1,
        rightHeavy = 1,
    };
    AVLnode(int val) :node(val){ status = 0; }
    int status;
    virtual ~AVLnode(){}
};
typedef class AVLnode AVLnode_int;
#endif


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

//if match return 0
//else return !=0
int tree_search(node* pRoot, int key, node** ppFound = NULL, node** ppParent = NULL)
{
    int rc;
    assert(pRoot != NULL);
    node* pCur = pRoot;
    node* pPrev = NULL;
    do {
        rc = key - NODE_KEY(pCur);
        if (rc == 0) break;
        pPrev = pCur;
        pCur = (rc > 0) ? NODE_RIGHT(pCur) : NODE_LEFT(pCur);
    } while (pCur);
    if (ppFound) {
        *ppFound = pCur;
    }
    if (ppParent) {
        *ppParent = pPrev;
    }
    return rc;
}

int tree_search(node* pRoot, node* pNode, node** ppFound = NULL, node** ppParent = NULL)
{
    int rc;
    assert(pRoot != NULL);
    assert(pNode != NULL);
    node* pCur = pRoot;
    node* pPrev = NULL;
    do {
        rc = NODE_KEY(pNode) - NODE_KEY(pCur);
        if (rc == 0) break;
        pPrev = pCur;
        pCur = (rc > 0) ? NODE_RIGHT(pCur) : NODE_LEFT(pCur);
    } while (pCur);
    if (ppFound) {
        *ppFound = pCur;
    }
    if (ppParent) {
        *ppParent = pPrev;
    }
    return rc;
}

int tree_insert(node* pRoot, node* pNode)
{
    assert(pRoot != NULL);
    assert(NODE_RIGHT(pNode) == NULL);
    assert(NODE_LEFT(pNode) == NULL);
    node* pParent;
    int rc = tree_search(pRoot, pNode, NULL, &pParent);
    assert(rc != 0);
    assert(pParent);
    if (rc > 0) {
        //insert right
        assert(NODE_KEY(pNode) > NODE_KEY(pParent));
        assert(NODE_RIGHT(pParent) == 0);
        NODE_RIGHT(pParent) = pNode;
    }
    else {
        assert(NODE_KEY(pNode) < NODE_KEY(pParent));
        assert(NODE_LEFT(pParent) == 0);
        NODE_LEFT(pParent) = pNode;
    }
    return 0;
}

node* tree_min(node* pRoot)
{
    assert(pRoot != NULL);
    node* pCur = pRoot;
    while (NODE_LEFT(pCur) != NULL) {
        pCur = NODE_LEFT(pCur);
    }
    return pCur;
}
node* tree_max(node* pRoot)
{
    assert(pRoot != NULL);
    node* pCur = pRoot;
    while (NODE_RIGHT(pCur) != NULL) {
        pCur = NODE_RIGHT(pCur);
    }
    return pCur;
}
node* tree_replace(node* pParent, node* pNode, node* pNew)
{
    if (pParent) {
        if (NODE_LEFT(pParent) == pNode) {
            NODE_LEFT(pParent) = pNew;
        }
        else {
            NODE_RIGHT(pParent) = pNew;
        }
        return pParent;
    }
    return pNew;
}

node* tree_remove(node* pRoot, node* pNode)
{
    assert(pRoot != NULL);
    node* pParent;
    node* pCur;
    node* pSuc;
    int rc = tree_search(pRoot, pNode, &pCur, &pParent);
    assert(rc == 0);

    if (NODE_LEFT(pCur) && NODE_RIGHT(pCur)) {
        pSuc = tree_min(NODE_RIGHT(pCur));
        NODE_RIGHT(pSuc) = tree_remove(NODE_RIGHT(pCur), pSuc);
        NODE_LEFT(pSuc) = NODE_LEFT(pCur);
        tree_replace(pParent, pCur, pSuc);
    }
    else if (NODE_LEFT(pCur)) {
        pSuc = tree_replace(pParent, pCur, NODE_LEFT(pCur));
    }
    else if (NODE_RIGHT(pCur)) {
        pSuc = tree_replace(pParent, pCur, NODE_RIGHT(pCur));
    }
    else {
        pSuc = tree_replace(pParent, pCur, NULL);
    }
    return (pRoot == pCur) ? pSuc : pRoot;
}

node* tree_remove(node* pRoot, int key)
{
    assert(pRoot != NULL);
    node* pNode;
    int rc = tree_search(pRoot, key, &pNode);
    assert(rc == 0);

    return tree_remove(pRoot, pNode);
}

int tree_traverse(node* pRoot)
{
    if (pRoot == NULL) return 0;
    tree_traverse(NODE_LEFT(pRoot));
    printf(" %d ", (int)NODE_KEY(pRoot));
    tree_traverse(NODE_RIGHT(pRoot));
    return 1;
}
int tree_traverse_2(node* pRoot)
{
    queue q(100);
    if (pRoot == NULL) return 0;

    int prevLevel = 0;
    q.push(pRoot, 0);
    printf("  level 0:\n  ");
    while (!q.isEmpty()) {
        int level;
        node* pCur = (node*)q.pop(&level);
        if (prevLevel != abs(level)) {
            prevLevel = abs(level);
            printf("\n  level %d:\n  ", prevLevel);
            //printf("\n");
        }
        printf("  %c %d  ", level > 0 ? '\\' : '/', (int)NODE_KEY(pCur));
        if (NODE_LEFT(pCur)) q.push(NODE_LEFT(pCur), 0 - (prevLevel + 1));
        if (NODE_RIGHT(pCur)) q.push(NODE_RIGHT(pCur), prevLevel + 1);
    }
    printf("\n");
    return 1;
}

node* tree_traverse_first()
{
    return 0;
}

class tree
{
    node* m_pRoot;
public:
    tree()
    {
        m_pRoot = NULL;
    }
    //if node not existing in tree, return true
    //else return false
    bool Insert(node* pNode)
    {
        bool rc = false;
        if (m_pRoot == NULL) {
            m_pRoot = pNode;
            rc = true;
        }
        else if (0 != tree_search(m_pRoot, pNode)) {
            tree_insert(m_pRoot, pNode);
            rc = true;
        }
        return rc;
    }
    //if node existing, return true
    //else return false
    bool Remove(int key)
    {
        bool rc = false;
        node* pNode;
        do {
            if (m_pRoot == NULL) break;
            if (tree_search(m_pRoot, key, &pNode) != 0) break;
            assert(pNode != NULL);

            m_pRoot = tree_remove(m_pRoot, pNode);
            rc = true;
        } while (false);
        return rc;
    }
    bool Remove(node* pNode)
    {
        bool rc = false;
        do {
            if (m_pRoot == NULL) break;
            if (0 != tree_search(m_pRoot, pNode)) break;

            m_pRoot = tree_remove(m_pRoot, pNode);
            rc = true;
        } while (false);
        return rc;
    }
    node* Search(int key)
    {
        node* pRes = NULL;
        if (m_pRoot != NULL) {
            tree_search(m_pRoot, key, &pRes);
        }
        return pRes;
    }
    bool Search(node* pNode)
    {
        bool rc = false;
        do {
            if (pNode == NULL) break;
            if (m_pRoot == NULL) break;
            rc = tree_search(m_pRoot, pNode) == 0;
        } while (false);
        return rc;
    }
    //travese
    void SeekSet();
    node* GetFirst();
    node* GetNext();
    node* Min()
    {
        if (m_pRoot != NULL)
            return tree_min(m_pRoot);
        return NULL;
    }
    node* Max()
    {
        if (m_pRoot != NULL)
            return tree_max(m_pRoot);
        return NULL;
    }
};

//-------------test tree api
void test_api_tree_remove()
{
    TRACE("test tree remove\n");

    node* pRoot = new node_int(5);
    queue q(100);
    q.push(pRoot);

    int arr[] = { 3, 6, 0, 4, 8, 9, 2, 1, 7 };
    for (int i = 0; i < 9; i++) {
        node* pNode = new node_int(arr[i]);
        tree_insert(pRoot, pNode);
        q.push(pNode);
    }

    TRAVERSE(pRoot);

    TRACE("\n  rm 2\n");
    pRoot = tree_remove(pRoot, 2);
    TRAVERSE(pRoot);

    TRACE("\n  rm 3, 4\n");
    pRoot = tree_remove(pRoot, 3);
    pRoot = tree_remove(pRoot, 4);
    TRAVERSE(pRoot);

    TRACE("\n  rm 5, 6\n");
    pRoot = tree_remove(pRoot, 5);
    pRoot = tree_remove(pRoot, 6);
    TRAVERSE(pRoot);

    TRACE("\n  rm 7, 8\n");
    pRoot = tree_remove(pRoot, 7);
    pRoot = tree_remove(pRoot, 8);
    TRAVERSE(pRoot);

    TRACE("\n  rm 9, 0\n");
    pRoot = tree_remove(pRoot, 9);
    pRoot = tree_remove(pRoot, 0);
    TRAVERSE(pRoot);

    pRoot = tree_remove(pRoot, 1);
    assert(pRoot == NULL);

    TRACE("\n");

    //free created node
    while (!q.isEmpty()) {
        delete (node_int*)q.pop();
    }
}
void test_api_tree_insert()
{
    TRACE("test tree insert\n");

    node* pRoot = new node_int(5);
    queue q(100);
    q.push(pRoot);

    srand(clock());
    for (int i = 0; i < 10; i++) {
        int val = rand() % 10;
        node* tmp;
        int rc = (tree_search(pRoot, val, &tmp));
        TRACE("  search %d rc %d\n", val, rc);
        if (rc != 0) {
            node* pNode = new node_int(val);
            TRACE("  insert %d\n", (int)NODE_KEY(pNode));
            tree_insert(pRoot, pNode);
            q.push(pNode);
        }
    }

    TRAVERSE(pRoot);

    TRACE("\n");

    //free created node
    while (!q.isEmpty()) {
        delete (node*)q.pop();
    }
}
//-------------test tree class
void test_tree_insert()
{
    tree tree;
    node* node1 = new node_int(1);
    node* node2 = new node_int(2);
    tree.Insert(node1);
    tree.Insert(node2);
    assert(tree.Search(1) == node1);
    assert(tree.Search(2) == node2);
    delete node1;
    delete node2;
}
//(1) test tree_remove
//(1.1) test tree_remove(int key)
void test_tree_remove()
{
    tree tree;
    node* node1 = new node_int(1);
    node* node2 = new node_int(2);
    tree.Insert(node1);
    tree.Insert(node2);
    tree.Remove(1);
    assert(tree.Search(1) == NULL);
    tree.Remove(NODE_KEY(node2));
    assert(tree.Search(2) == NULL);
    delete node1;
    delete node2;
}
//(1.2) test tree_remove(node* pNode)
void test_tree_remove_2()
{
    tree tree;
    node* node1 = new node_int(1);
    node* node2 = new node_int(1);
    tree.Insert(node1);
    tree.Insert(node2);
    tree.Remove(node1);
    assert(tree.Search(1) == node2);
    tree.Remove(node2);
    assert(tree.Search(1) == NULL);
    delete node1;
    delete node2;
}
void test_tree_min()
{
    tree tree;
    node* node1 = new node_int(1);
    node* node2 = new node_int(2);
    tree.Insert(node1);
    tree.Insert(node2);
    assert(tree.Min() == node1);
    delete node1;
    delete node2;
}
void test_tree_max()
{
    tree tree;
    node* node1 = new node_int(1);
    node* node2 = new node_int(2);
    tree.Insert(node1);
    tree.Insert(node2);
    assert(tree.Max() == node2);
    delete node1;
    delete node2;
}
void test_tree_search()
{
    tree tree;
    node* arr[10];
    srand(clock());
    for (int i = 0; i < 10;) {
        int val = rand() % 20;
        if (tree.Search(val) != NULL) continue;
        arr[i] = new node_int(val);
        tree.Insert(arr[i]);
        i++;
    }

    for (int i = 0; i < 10; i++)
        assert(tree.Search(NODE_KEY(arr[i])));

    for (int i = 0; i < 10; i++)
        delete arr[i];
}

void test_type()
{
    size_t i = -1;
    long int li = i;
    assert(i > 0);
    assert(li < 0);
    assert(sizeof(li) == sizeof(i));
}

//-------------AVLTree
//-------------rotate
AVLnode* AVLtree_single_rotate_left(AVLnode* pParent)
{
    TRACE("  single rotate left\n");

    AVLnode* pRightChild = (AVLnode*)NODE_RIGHT(pParent);
    NODE_STAT(pParent) = AVLnode::balanced;
    NODE_STAT(pRightChild) = AVLnode::balanced;
    NODE_RIGHT(pParent) = NODE_LEFT(pRightChild);
    NODE_LEFT(pRightChild) = pParent;
    return pRightChild;
}
AVLnode* AVLtree_single_rotate_right(AVLnode* pParent)
{
    TRACE("  single rotate right\n");
    AVLnode* pLeftChild = (AVLnode*)NODE_LEFT(pParent);
    NODE_STAT(pLeftChild) = AVLnode::balanced;
    NODE_STAT(pParent) = AVLnode::balanced;
    NODE_LEFT(pParent) = NODE_RIGHT(pLeftChild);
    NODE_RIGHT(pLeftChild) = pParent;
    return pLeftChild;
}
//only use when remove node
AVLnode* AVLtree_special_rotate_left(AVLnode* pParent)
{
    TRACE("  special rotate left\n");
    AVLnode* pRightChild = (AVLnode*)NODE_RIGHT(pParent);
    NODE_STAT(pRightChild) = AVLnode::leftHeavy;
    NODE_STAT(pParent) = AVLnode::rightHeavy;
    NODE_RIGHT(pParent) = NODE_LEFT(pRightChild);
    NODE_LEFT(pRightChild) = pParent;
    return pRightChild;
}
//only use when remove node
AVLnode* AVLtree_special_rotate_right(AVLnode* pParent)
{
    TRACE("  special rotate right\n");
    AVLnode* pLeftChild = (AVLnode*)NODE_LEFT(pParent);
    NODE_STAT(pLeftChild) = AVLnode::rightHeavy;
    NODE_STAT(pParent) = AVLnode::leftHeavy;
    NODE_LEFT(pParent) = NODE_RIGHT(pLeftChild);
    NODE_RIGHT(pLeftChild) = pParent;
    return pLeftChild;
}
AVLnode* AVLtree_double_rotate_left(AVLnode* pParent)
{
    TRACE("  double rotate left\n");

    AVLnode* pRightChild = (AVLnode*)NODE_RIGHT(pParent);
    AVLnode* pNewParent = (AVLnode*)NODE_LEFT(pRightChild);

    if (NODE_STAT(pNewParent) == AVLnode::leftHeavy) {
        NODE_STAT(pParent) = AVLnode::balanced;
        NODE_STAT(pRightChild) = AVLnode::rightHeavy;
    }
    else if (NODE_STAT(pNewParent) == AVLnode::balanced) {
        NODE_STAT(pParent) = AVLnode::balanced;
        NODE_STAT(pRightChild) = AVLnode::balanced;
    }
    else {
        NODE_STAT(pParent) = AVLnode::leftHeavy;
        NODE_STAT(pRightChild) = AVLnode::balanced;
    }
    NODE_STAT(pNewParent) = AVLnode::balanced;

    NODE_LEFT(pRightChild) = NODE_RIGHT(pNewParent);
    NODE_RIGHT(pNewParent) = pRightChild;
    NODE_RIGHT(pParent) = NODE_LEFT(pNewParent);
    NODE_LEFT(pNewParent) = pParent;

    return pNewParent;
}
AVLnode* AVLtree_double_rotate_right(AVLnode* pParent)
{
    TRACE("  double rotate right\n");

    AVLnode* pLeftChild = (AVLnode*)NODE_LEFT(pParent);
    AVLnode* pNewParent = (AVLnode*)NODE_RIGHT(pLeftChild);

    if (NODE_STAT(pNewParent) == AVLnode::rightHeavy) {
        NODE_STAT(pParent) = AVLnode::balanced;
        NODE_STAT(pLeftChild) = AVLnode::leftHeavy;
    }
    else if (NODE_STAT(pNewParent) == AVLnode::balanced) {
        NODE_STAT(pParent) = AVLnode::balanced;
        NODE_STAT(pLeftChild) = AVLnode::balanced;
    }
    else {
        NODE_STAT(pParent) = AVLnode::rightHeavy;
        NODE_STAT(pLeftChild) = AVLnode::balanced;
    }
    NODE_STAT(pNewParent) = AVLnode::balanced;

    NODE_RIGHT(pLeftChild) = NODE_LEFT(pNewParent);
    NODE_LEFT(pNewParent) = pLeftChild;
    NODE_LEFT(pParent) = NODE_RIGHT(pNewParent);
    NODE_RIGHT(pNewParent) = pParent;

    return pNewParent;
}
AVLnode* AVLtree_update_left(AVLnode *pParent, int *pHeightChg)
{
    int heightChg = 0;
    AVLnode* pLeftChild = (AVLnode*)NODE_LEFT(pParent);

    if (NODE_STAT(pLeftChild) == AVLnode::leftHeavy) {
        pParent = AVLtree_single_rotate_right(pParent);
    }
    else if (NODE_STAT(pLeftChild) == AVLnode::balanced) {
        //rebalance after remove
        pParent = AVLtree_special_rotate_right(pParent);
        heightChg = 1;
    }
    else {
        assert(NODE_STAT(pLeftChild) == AVLnode::rightHeavy);
        pParent = AVLtree_double_rotate_right(pParent);
    }
    *pHeightChg = heightChg;
    return pParent;
}
AVLnode* AVLtree_update_right(AVLnode *pParent, int *pHeightChg)
{
    int heightChg = 0;
    AVLnode* pRightChild = (AVLnode*)NODE_RIGHT(pParent);

    if (NODE_STAT(pRightChild) == AVLnode::rightHeavy) {
        pParent = AVLtree_single_rotate_left(pParent);
    }
    else if (NODE_STAT(pRightChild) == AVLnode::balanced) {
        //rebalance after remove
        pParent = AVLtree_special_rotate_left(pParent);
        heightChg = 1;
    }
    else {
        assert(NODE_STAT(pRightChild) == AVLnode::leftHeavy);
        pParent = AVLtree_double_rotate_left(pParent);
    }
    *pHeightChg = heightChg;
    return pParent;
}
//-------------alter
AVLnode* AVLtree_insert(AVLnode*, AVLnode*, int*);
AVLnode* AVLtree_insert_left(AVLnode* pRoot, AVLnode* pNode, int* pHeightChg)
{
    int heightChg = 0;
    AVLnode* pLeftChild = (AVLnode*)NODE_LEFT(pRoot);

    //insert left
    //+ if leaf
    if (pLeftChild == NULL) {
        NODE_LEFT(pRoot) = pNode;
        NODE_STAT(pRoot) -= 1;
        heightChg = (NODE_STAT(pRoot) == AVLnode::leftHeavy);
    }
    else {
        //insert to left child
        int childHeightChg;
        NODE_LEFT(pRoot) = AVLtree_insert(pLeftChild, pNode, &childHeightChg);
        NODE_STAT(pRoot) -= childHeightChg;
        //rebalance
        if (NODE_STAT(pRoot) < AVLnode::leftHeavy) {
            pRoot = AVLtree_update_left(pRoot, &heightChg);
        }
        heightChg = childHeightChg && (NODE_STAT(pRoot) == AVLnode::leftHeavy);
    }
    *pHeightChg = heightChg;
    return pRoot;
}
AVLnode* AVLtree_insert_right(AVLnode* pRoot, AVLnode* pNode, int* pHeightChg)
{
    int heightChg = 0;
    AVLnode* pRightChild = (AVLnode*)NODE_RIGHT(pRoot);

    //insert right
    //+ if leaf
    if (NODE_RIGHT(pRoot) == NULL) {
        NODE_RIGHT(pRoot) = pNode;
        NODE_STAT(pRoot) += 1;
        heightChg = (NODE_STAT(pRoot) == AVLnode::rightHeavy);
    }
    else {
        //insert to the right
        int childHeightChg;
        NODE_RIGHT(pRoot) = AVLtree_insert(pRightChild, pNode, &childHeightChg);
        NODE_STAT(pRoot) += childHeightChg;
        //rebalance
        if (NODE_STAT(pRoot) > AVLnode::rightHeavy) {
            pRoot = AVLtree_update_right(pRoot, &heightChg);
        }
        heightChg = childHeightChg && (NODE_STAT(pRoot) == AVLnode::rightHeavy);
    }
    *pHeightChg = heightChg;
    return pRoot;
}
AVLnode* AVLtree_insert(AVLnode* pRoot, AVLnode* pNode, int* pHeightChg = NULL)
{
    assert(pRoot != NULL);
    assert(pNode != NULL);
    assert(NODE_STAT(pNode) == AVLnode::balanced);

    int heightChg = 0;
    //left insert
    if (NODE_KEY(pRoot) > NODE_KEY(pNode)) {
        pRoot = AVLtree_insert_left(pRoot, pNode, &heightChg);
    }
    else {
        pRoot = AVLtree_insert_right(pRoot, pNode, &heightChg);
    }
    if (pHeightChg != NULL) {
        *pHeightChg = heightChg;
    }
    return pRoot;
}
//remove a node
AVLnode* AVLtree_remove(AVLnode*, AVLnode*, int*);
AVLnode* AVLtree_remove_left(AVLnode* pRoot, AVLnode* pNode, int* pHeightChg)
{
    int childHeightChg;
    int heightChg = 0;
    AVLnode* pLeftChild = (AVLnode*)NODE_LEFT(pRoot);

    NODE_LEFT(pRoot) = AVLtree_remove(pLeftChild, pNode, &childHeightChg);
    NODE_STAT(pRoot) += childHeightChg;
    if (NODE_STAT(pRoot) > AVLnode::rightHeavy) {
        pRoot = AVLtree_update_right(pRoot, &heightChg);
    }
    heightChg = childHeightChg && (NODE_STAT(pRoot) == AVLnode::balanced);
    *pHeightChg = heightChg;
    return pRoot;
}
AVLnode* AVLtree_remove_right(AVLnode* pRoot, AVLnode* pNode, int* pHeightChg)
{
    int childHeightChg;
    int heightChg = 0;
    AVLnode* pRightChild = (AVLnode*)NODE_RIGHT(pRoot);

    NODE_RIGHT(pRoot) = AVLtree_remove(pRightChild, pNode, &childHeightChg);
    NODE_STAT(pRoot) -= childHeightChg;
    if (NODE_STAT(pRoot) < AVLnode::leftHeavy) {
        pRoot = AVLtree_update_left(pRoot, &heightChg);
    }
    heightChg = childHeightChg && (NODE_STAT(pRoot) == AVLnode::balanced);
    *pHeightChg = heightChg;
    return pRoot;
}
AVLnode* AVLtree_remove(AVLnode* pRoot, AVLnode* pNode, int* pHeightChg = NULL)
{
    assert(pRoot != NULL);
    assert(pNode != NULL);

    int heightChg = 0;
    int childHeightChg;
    AVLnode* pSuc = NULL;
    AVLnode* pLeftChild = (AVLnode*)NODE_LEFT(pRoot);
    AVLnode* pRightChild = (AVLnode*)NODE_RIGHT(pRoot);

    if (NODE_KEY(pRoot) == NODE_KEY(pNode)) {
        //replace with successor
        if (pLeftChild && pRightChild) {
            pSuc = (AVLnode*)tree_min(pRightChild);
            NODE_RIGHT(pSuc) = AVLtree_remove(pRightChild, pSuc, &childHeightChg);
            NODE_LEFT(pSuc) = pLeftChild;
            NODE_STAT(pSuc) = NODE_STAT(pRoot) - childHeightChg;
            if (NODE_STAT(pSuc) < AVLnode::leftHeavy) {
                //rebalance
                pSuc = AVLtree_update_left(pSuc, &heightChg);
            }
            heightChg = childHeightChg && (NODE_STAT(pSuc) == AVLnode::balanced);
        }
        else {
            pSuc = (pLeftChild != NULL) ? pLeftChild : pRightChild;
            heightChg = 1;
        }
    }
    else if (NODE_KEY(pRoot) > NODE_KEY(pNode)) {
        //left remove
        pRoot = AVLtree_remove_left(pRoot, pNode, &heightChg);
    }
    else {
        assert(NODE_KEY(pRoot) < NODE_KEY(pNode));
        //right remove
        pRoot = AVLtree_remove_right(pRoot, pNode, &heightChg);
    }

    if (pHeightChg != NULL) {
        *pHeightChg = heightChg;
    }

    return (pRoot == pNode) ? pSuc : pRoot;
}
//-------------test avl tree
void generate_keys(int* buff = NULL, int nKeys = 10, int range = 1)
{
    srand(clock());
    queue q(nKeys);
    int key = rand() % (nKeys*range);
    //init
    node* pRoot = new node_int(key);
    q.push(pRoot);
    if (buff) buff[0] = key;
    int i = 1;
    //generate keys
    for (; i < nKeys;) {
        key = rand() % (nKeys*range);
        if (0 == tree_search(pRoot, key)) continue;
        //create new node
        node* pNode = new node_int(key);
        tree_insert(pRoot, pNode);
        if (buff) buff[i] = key;
        q.push(pNode);
        i++;
    }

    //clean
    for (node* pNode = (node*)q.pop(); pNode != NULL; pNode = (node*)q.pop()) {
        printf("%02d, ", (int)NODE_KEY(pNode));
        delete pNode;
    }
    printf("\n");
}
//k0+-k1+-k3
//      +-k4
//  +-k2
int AVLtree_traverse_3(AVLnode* pRoot, int level = 0)
{
    if (pRoot == NULL) return 0;
    printf("+-%2d(%2d)", (int)NODE_KEY(pRoot), NODE_STAT(pRoot));
    AVLtree_traverse_3((AVLnode*)NODE_LEFT(pRoot), level + 1);
    printf("\n");
    for (int i = 0; i < level + 1; i++) printf("        ");
    AVLtree_traverse_3((AVLnode*)NODE_RIGHT(pRoot), level + 1);
    return 0;
}

void test_avl_tree()
{
    //generate_keys(30, 2);
    int i;
    enum { nNode = 50 };
    int keys[nNode] = {
        38, 85, 37, 55, 43,
        86, 78, 23, 62, 57,
        24, 02, 17, 34, 96,
        22, 07, 89, 91, 12,
        61, 05, 36, 9, 41,
        00, 31, 88, 94, 04,
        18, 97, 68, 35, 83,
        53, 93, 64, 65, 3,
        58, 60, 52, 15, 30,
        6, 25, 32, 39, 63
    };
    queue q(nNode);
    AVLnode* pNode = NULL;
    AVLnode* pRoot = NULL;
    //init
    pRoot = new AVLnode_int(keys[0]);
    q.push(pRoot);
    //generate data
    for (i = 1; i < nNode;) {
        //add node
        pNode = new AVLnode_int(keys[i]);
        q.push(pNode);
        pRoot = AVLtree_insert(pRoot, pNode);
        i++;
    }
    //print
    AVLtree_traverse_3(pRoot);
    //clean
    for (AVLnode* pNode = (AVLnode*)q.pop(); pNode != NULL; pNode = (AVLnode*)q.pop())
        delete pNode;
}
void test_avl_tree_2()
{
    enum { nKeys = 50 };
    int buff[nKeys];
    generate_keys(buff, nKeys, 2);
    queue q(nKeys);
    //init
    AVLnode* pRoot = new AVLnode_int(buff[0]);
    q.push(pRoot);
    //generate data
    for (int i = 1; i < nKeys; i++) {
        AVLnode* pNode = new AVLnode_int(buff[i]);
        pRoot = AVLtree_insert(pRoot, pNode);
        q.push(pNode);
    }
    //print tree
    AVLtree_traverse_3(pRoot);
    //clean
    for (AVLnode* pNode = (AVLnode*)q.pop(); pNode != NULL; pNode = (AVLnode*)q.pop())
        delete pNode;
}
void test_avl_tree_3()
{
    enum { nNodes = 10 };
    AVLnode* arr[nNodes];
    //init
    arr[0] = new  AVLnode_int(0);
    AVLnode* pRoot = arr[0];
    //generate data
    for (int i = 1; i < nNodes; i++) {
        arr[i] = new AVLnode_int(i);
        pRoot = AVLtree_insert(pRoot, arr[i]);

        printf("\n");
        AVLtree_traverse_3(pRoot);
    }

    for (int i = 0; i < nNodes; i++) delete arr[i];
}
void test_avl_tree_remove()
{
    enum { nNodes = 10 };
    AVLnode* arr[nNodes];
    //init
    arr[0] = new  AVLnode_int(0);
    AVLnode* pRoot = arr[0];
    //generate data
    for (int i = 1; i < nNodes; i++) {
        arr[i] = new AVLnode_int(i);
        pRoot = AVLtree_insert(pRoot, arr[i]);
    }
    //remove data
    for (int i = 0; i < nNodes; i++) {
        pRoot = AVLtree_remove(pRoot, arr[i]);
        printf("\n");
        AVLtree_traverse_3(pRoot);
    }

    for (int i = 0; i < nNodes; i++) delete arr[i];
}
void test_avl_tree_remove_2()
{
    //generate_keys(30, 2);
    int i;
    enum { nNode = 50 };
    int keys[nNode] = {
        38, 85, 37, 55, 43,
        86, 78, 23, 62, 57,
        24, 02, 17, 34, 96,
        22, 07, 89, 91, 12,
        61, 05, 36, 9, 41,
        00, 31, 88, 94, 04,
        18, 97, 68, 35, 83,
        53, 93, 64, 65, 3,
        58, 60, 52, 15, 30,
        6, 25, 32, 39, 63
    };
    queue q(nNode);
    AVLnode* pNode = NULL;
    AVLnode* pRoot = NULL;
    //init
    pRoot = new AVLnode_int(keys[0]);
    q.push(pRoot);
    //generate data
    for (i = 1; i < nNode;) {
        //add node
        pNode = new AVLnode_int(keys[i]);
        q.push(pNode);
        pRoot = AVLtree_insert(pRoot, pNode);
        i++;
    }
    //print
    printf("\n");
    AVLtree_traverse_3(pRoot);
    //clean
    for (AVLnode* pNode = (AVLnode*)q.pop(); pNode != NULL; pNode = (AVLnode*)q.pop()) {
        pRoot = AVLtree_remove(pRoot, pNode);
        printf("\n");
        AVLtree_traverse_3(pRoot);
        delete pNode;
    }
}
//-------------main

int main()
{
    //test_api_tree_insert();
    //test_api_tree_remove();

    //test_tree_remove();
    //test_tree_remove_2();
    //test_tree_search();
    //test_avl_tree();
    //test_avl_tree_2();
    //test_avl_tree_2();
    //test_avl_tree_3();
    //test_avl_tree_remove();
    test_avl_tree_remove_2();
    return 0;
}
