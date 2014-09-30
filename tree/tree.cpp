#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//debug
#if(1)
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
    operator int ()
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
class node_int:public node
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
#else
#define NODE_RIGHT(node) node->pRight
#define NODE_LEFT(node)  node->pLeft
#define NODE_KEY(node)   node->key

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
        pCur = (rc > 0)?NODE_RIGHT(pCur):NODE_LEFT(pCur);
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
        pCur = (rc > 0)?NODE_RIGHT(pCur):NODE_LEFT(pCur);
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
    } else {
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
        } else {
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
    } else if (NODE_LEFT(pCur)) {
        pSuc = tree_replace(pParent, pCur, NODE_LEFT(pCur));
    } else if (NODE_RIGHT(pCur)) {
        pSuc = tree_replace(pParent, pCur, NODE_RIGHT(pCur));
    } else {
        pSuc = tree_replace(pParent, pCur, NULL);
    }
    return (pRoot == pCur)? pSuc:pRoot;
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
        printf("  %c %d  ", level>0?'\\':'/', (int)NODE_KEY(pCur));
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
        } else if (0 != tree_search(m_pRoot, pNode)) {
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
        } while(false);
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

    int arr[] = {3, 6, 0, 4, 8, 9, 2, 1, 7};
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
    while(!q.isEmpty()) {
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
        int val = rand()%10;
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
    while(!q.isEmpty()) {
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
class AVLnode: public node
{
public:
    enum {
        balanced  = 0,
        leftHeavy = -1,
        rightHeavy = 1,
    };
    AVLnode() {
        m_pLeft = NULL;
        m_pRight = NULL;
        m_status = NULL;
    }
    node*& right() {return m_pRight;}
    node*& left() {return m_pLeft;}
    int& status()
    {
        return m_status;
    }
private:
    node* m_pLeft;
    node* m_pRight;
    int m_status;
};
//-------------rotate
AVLnode* AVLtree_single_rotate_left(AVLnode* pParent)
{
    AVLnode* pRightChild = (AVLnode*)pParent->right();
    pParent->status() = AVLnode::balanced;
    pRightChild->status() = AVLnode::balanced;
    pParent->right() = pRightChild->left();
    pRightChild->left() = pParent;
    return pRightChild;
}
AVLnode* AVLtree_single_rotate_right(AVLnode* pParent)
{
    AVLnode* pLeftChild = (AVLnode*)pParent->left();
    pLeftChild->status() = AVLnode::balanced;
    pParent->status() = AVLnode::balanced;
    pParent->left() = pLeftChild->right();
    pLeftChild->right() = pParent;
    return pLeftChild;
}
AVLnode* AVLtree_double_rotate_left(AVLnode* pParent)
{
    AVLnode* pRightChild = (AVLnode*)pParent->right();
    AVLnode* pNewParent = (AVLnode*)pRightChild->left();

    if (pNewParent->status() == AVLnode::leftHeavy) {
        pParent->status() = AVLnode::balanced;
        pRightChild->status() = AVLnode::rightHeavy;
    } else if (pNewParent->status() == AVLnode::balanced) {
        pParent->status() = AVLnode::balanced;
        pRightChild->status() = AVLnode::balanced;
    } else {
        pParent->status() = AVLnode::leftHeavy;
        pRightChild->status() = AVLnode::balanced;
    }
    pNewParent->status() = AVLnode::balanced;

    pRightChild->left() = pNewParent->right();
    pNewParent->right() = pRightChild;
    pParent->right() = pNewParent->left();
    pNewParent->left() = pParent;

    return pNewParent;
}
AVLnode* AVLtree_double_rotate_right(AVLnode* pParent)
{
    AVLnode* pLeftChild = (AVLnode*)pParent->left();
    AVLnode* pNewParent = (AVLnode*)pLeftChild->right();

    if (pNewParent->status() == AVLnode::rightHeavy) {
        pParent->status() = AVLnode::balanced;
        pLeftChild->status() = AVLnode::leftHeavy;
    } else if (pNewParent->status() == AVLnode::balanced) {
        pParent->status() = AVLnode::balanced;
        pLeftChild->status() = AVLnode::balanced;
    } else {
        pParent->status() = AVLnode::rightHeavy;
        pLeftChild->status() = AVLnode::balanced;
    }
    pNewParent->status() = AVLnode::balanced;

    pLeftChild->right() = pNewParent->left();
    pNewParent->left() = pLeftChild;
    pParent->left() = pNewParent->right();
    pNewParent->right() = pParent;

    return pNewParent;
}
AVLnode* AVLtree_update_left(AVLnode *pParent, int *pHeightChg)
{
    int heightChg = 0;
    AVLnode* pLeftChild = (AVLnode*)pParent->left();
    if (pLeftChild->status() == AVLnode::leftHeavy) {
        pParent = AVLtree_single_rotate_left(pParent);
    }
    else {
        assert(pLeftChild->status() == AVLnode::rightHeavy);
        pParent = AVLtree_double_rotate_right(pParent);
    }
    *pHeightChg = heightChg;
    return pParent;
}
AVLnode* AVLtree_update_right(AVLnode *pParent, int *pHeightChg)
{
    int heightChg = 0;
    AVLnode* pRightChild = (AVLnode*)pParent->right();
    if (pRightChild->status() == AVLnode::rightHeavy) {
        pParent = AVLtree_single_rotate_right(pParent);
    }
    else {
        assert(pRightChild->status() == AVLnode::leftHeavy);
        pParent = AVLtree_double_rotate_left(pParent);
    }
    *pHeightChg = heightChg;
    return pParent;
}
//-------------alter
AVLnode* AVLtree_insert(AVLnode* pRoot, AVLnode* pNode, int* pHeightChg = NULL);
AVLnode* AVLtree_insert_left(AVLnode* pRoot, AVLnode* pNode, int* pHeightChg)
{
    int heightChg = 0;
    //insert left
    //+ if leaf
    if (pRoot->left() == NULL) {
        pRoot->left() = pNode;
        pRoot->status() -= 1;
        heightChg = (pRoot->status() == AVLnode::leftHeavy);
    } else {
        //insert to left child
        int childHeightChg;
        pRoot->left() = AVLtree_insert((AVLnode*)pRoot->left(), pNode, &childHeightChg);
        pRoot->status() -= childHeightChg;
        //rebalance
        if (pRoot->status() < AVLnode::leftHeavy) {
            pRoot = AVLtree_update_left(pRoot, &heightChg);
        }
        heightChg = (pRoot->status() == AVLnode::leftHeavy);
    }
    *pHeightChg = heightChg;
    return pRoot;
}
AVLnode* AVLtree_insert_right(AVLnode* pRoot, AVLnode* pNode, int* pHeightChg)
{
    int heightChg;
    //insert right
    //+ if leaf
    if (pRoot->right() == NULL) {
        pRoot->right() = pNode;
        pRoot->status() += 1;
        heightChg = (pRoot->status() == AVLnode::rightHeavy);
    } else {
        //insert to the right
        int childHeightChg;
        pRoot->right() = AVLtree_insert((AVLnode*)pRoot->right(), pNode, &childHeightChg);
        pRoot->status() += childHeightChg;
        //rebalance
        if (pRoot->status() > AVLnode::rightHeavy) {
            pRoot = AVLtree_update_right(pRoot, &heightChg);
        }
        heightChg = (pRoot->status() == AVLnode::rightHeavy);
    }
    *pHeightChg = heightChg;
    return pRoot;
}
AVLnode* AVLtree_insert(AVLnode* pRoot, AVLnode* pNode, int* pHeightChg)
{
    assert(pRoot != NULL);
    assert(pNode != NULL);
    assert(pNode->status() == AVLnode::balanced);

    int heightChg = 0;
    //left insert
    if (pRoot->key() > pNode->key()) {
        pRoot = AVLtree_insert_left(pRoot, pNode, &heightChg);
    } else {
        pRoot = AVLtree_insert_right(pRoot, pNode, &heightChg);
    }
    if (pHeightChg != NULL) {
        *pHeightChg = heightChg;
    }
    return pRoot;
}
int AVLtree_remove(AVLnode* pRoot, AVLnode* pNode)
{
    return 0;
}
//-------------test avl tree
class AVLnode_int :public AVLnode
{
    tree_key m_key;
public:
    AVLnode_int(int val) {m_key = val;}
    tree_key& key() {return m_key;}
};
void test_avl_tree()
{
    int i;
    enum {nNode = 20};
    queue q(nNode);
    AVLnode* pNode = NULL;
    AVLnode* pRoot = NULL;
    //init
    srand(clock());
    int key = rand() % (nNode*2);
    pRoot = new AVLnode_int(key);
    q.push(pRoot);
    //generate data
    for(i = 0; i < nNode;) {
        key = rand() % (nNode*2);
        if (0 == tree_search(pRoot, key)) continue;
        //add node
        pNode = new AVLnode_int(key);
        q.push(pNode);
        pRoot = AVLtree_insert(pRoot, pNode);
        i++;
    }

    tree_traverse_2(pRoot);

    //clean
    do {
        pNode = (AVLnode*)q.pop();
        if (pNode == NULL) break;
        delete pNode;
    }while(1);
}

//-------------main

int main()
{
    //test_api_tree_insert();
    //test_api_tree_remove();

    //test_tree_remove();
    //test_tree_remove_2();
    //test_tree_search();
    test_avl_tree();
    return 0;
}
