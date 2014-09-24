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
#if(0)
#define NODE_RIGHT(node) node->right()
#define NODE_LEFT(node)  node->left()
#define NODE_KEY(node)   node->key()

class node
{
public:
    virtual node*& left() = 0;
    virtual node*& right() = 0;
    virtual int& key() = 0;
    virtual ~node() {}
};
class node_int:public node
{
    node* m_pLeft;
    node* m_pRight;
    int m_key;
public:
    node_int(int val)
    {
        m_pLeft = NULL;
        m_pRight = NULL;
        m_key = val;

        TRACE("  construct node %d\n", val);
    }
    node*& left(){return m_pLeft;}
    node*& right(){return m_pRight;}
    int& key()
    {
        return m_key;
    }
    ~node_int()
    {
        TRACE("  destroy node %d\n", m_key);
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
    typedef struct
    {
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
        if (m_count < m_size)
        {
            int i = m_iFirst + m_count;
            m_count++;
            m_data[i].pData = pData;
            m_data[i].level = level;
        }
    }
    void* pop(int* level = NULL)
    {
        void* pData = NULL;
        if (m_count > 0)
        {
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
    do
    {
//        *out = pCur;
        rc = key - NODE_KEY(pCur);
        if (rc == 0) break;
        pPrev = pCur;
        pCur = (rc > 0)?NODE_RIGHT(pCur):NODE_LEFT(pCur);
    }
    while (pCur);
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

int tree_insert(node* pRoot, node* pNode)
{
    assert(pRoot != NULL);
    assert(NODE_RIGHT(pNode) == NULL);
    assert(NODE_LEFT(pNode) == NULL);
    node* pParent;
    int rc = tree_search(pRoot, NODE_KEY(pNode), NULL, &pParent);
    assert(rc != 0);
    assert(pParent);
    if (rc > 0)
    {
        //insert right
        assert(NODE_KEY(pNode) > NODE_KEY(pParent));
        assert(NODE_RIGHT(pParent) == 0);
        NODE_RIGHT(pParent) = pNode;
    }
    else
    {
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
    while (NODE_LEFT(pCur) != NULL)
    {
        pCur = NODE_LEFT(pCur);
    }
    return pCur;
}
node* tree_max(node* pRoot)
{
    assert(pRoot != NULL);
    node* pCur = pRoot;
    while (NODE_RIGHT(pCur) != NULL)
    {
        pCur = NODE_RIGHT(pCur);
    }
    return pCur;
}
node* tree_replace(node* pParent, node* pNode, node* pNew)
{
    if (pParent)
    {
        if (NODE_LEFT(pParent) == pNode)
        {
            NODE_LEFT(pParent) = pNew;
        }
        else
        {
            NODE_RIGHT(pParent) = pNew;
        }
        return pParent;
    }
    return pNew;
}

node* tree_remove(node* pRoot, int key)
{
    assert(pRoot != NULL);
    node* pParent;
    node* pCur;
    node* pSuc;
    int rc = tree_search(pRoot, key, &pCur, &pParent);
    assert(rc == 0);
    assert(NODE_KEY(pCur) == key);

    if (NODE_LEFT(pCur) && NODE_RIGHT(pCur))
    {
        pSuc = tree_min(NODE_RIGHT(pCur));
        NODE_RIGHT(pSuc) = tree_remove(NODE_RIGHT(pCur), NODE_KEY(pSuc));
        NODE_LEFT(pSuc) = NODE_LEFT(pCur);
        tree_replace(pParent, pCur, pSuc);
    }
    else if (NODE_LEFT(pCur))
    {
        pSuc = tree_replace(pParent, pCur, NODE_LEFT(pCur));
    }
    else if (NODE_RIGHT(pCur))
    {
        pSuc = tree_replace(pParent, pCur, NODE_RIGHT(pCur));
    }
    else
    {
        pSuc = tree_replace(pParent, pCur, NULL);
    }
    return (pRoot == pCur)? pSuc:pRoot;
}
int tree_traverse(node* pRoot)
{
    if (pRoot == NULL) return 0;
    tree_traverse(NODE_LEFT(pRoot));
    printf(" %d ", NODE_KEY(pRoot));
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
    while (!q.isEmpty())
    {
        int level;
        node* pCur = (node*)q.pop(&level);
        if (prevLevel != abs(level))
        {
            prevLevel = abs(level);
            printf("\n  level %d:\n  ", prevLevel);
            //printf("\n");
        }
        printf("  %c %d  ", level>0?'\\':'/', NODE_KEY(pCur));
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
    //if node not existing in tree, return true
    //else return false
    bool Insert(node* pNode)
    {
        bool rc = false;
        if (m_pRoot == NULL)
        {
            m_pRoot = pNode;
            rc = true;
        }
        else if (0 != tree_search(m_pRoot, NODE_KEY(pNode)))
        {
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
        do
        {
            if (m_pRoot == NULL) break;
            if (0 != tree_search(m_pRoot, key)) break;

            m_pRoot = tree_remove(m_pRoot, key);
            rc = true;
        }
        while (false);
        return rc;
    }
//    bool Remove(node* pNode)
//    {
//        return Remove(NODE_KEY(pNode));
//    }
    //if found retun node*
    //else return NULL
    node* Search(int key)
    {
        node* pRes = NULL;
        if (m_pRoot != NULL)
        {
            tree_search(m_pRoot, key, &pRes);
        }
        return pRes;
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
    for (int i = 0; i < 9; i++)
    {
        node* pNode = new node_int(arr[i]);
        tree_insert(pRoot, pNode);
        q.push(pNode);
    }

    TRAVERSE(pRoot);

//    node* tmp = (node*)q.pop();
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
    while(!q.isEmpty())
    {
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
    for (int i = 0; i < 10; i++)
    {
        int val = rand()%10;
        node* tmp;
        int rc = (tree_search(pRoot, val, &tmp));
        TRACE("  search %d rc %d\n", val, rc);
        if (rc != 0)
        {
            node* pNode = new node_int(val);
            TRACE("  insert %d\n", NODE_KEY(pNode));
            tree_insert(pRoot, pNode);
            q.push(pNode);
        }
    }

    TRAVERSE(pRoot);

    TRACE("\n");

    //free created node
    while(!q.isEmpty())
    {
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
int main()
{
//    test_api_tree_insert();
//    test_api_tree_remove();
//    test_tree_search();
    size_t i = -1;
    long int li = i;
    assert(i > 0);
    assert(li < 0);
    assert(sizeof(li) == sizeof(i));
    return 0;
}
