
#if(1)  //AVLtree
//-------------mode
#define DEBUG_MODE      1
#define DEBUG_NODE_INT  0
#define DEBUG_NODE_STR  0
//+++++AVL object
#define NODE_STAT_BALANCED      0
#define NODE_STAT_LEFT_HEAVY    -1
#define NODE_STAT_RIGHT_HEAVY   1

#define CMP_RES_EQU     0

#if(DEBUG_NODE_INT)  //debug AVLtree_int
typedef struct node_int node_int;
typedef struct node_int* NodePtr;
typedef int* KeyPtr;   //key type is: int or string
#else
typedef void* NodePtr;  //
typedef void* KeyPtr;   //key type is: int or string
#endif
//+++++node
//node.left
typedef void(*set_left_child_callback)(NodePtr pParent, NodePtr pNode);
typedef NodePtr(*get_left_child_callback)(NodePtr pParent);
//node.right
typedef void(*set_right_child_callback)(NodePtr pParent, NodePtr pNode);
typedef NodePtr(*get_right_child_callback)(NodePtr pParent);
//node.status
typedef void(*set_status_callback)(NodePtr pNode, int status);
typedef int(*get_status_callback)(NodePtr pNode);
//node.key
typedef KeyPtr(*get_key_callback)(NodePtr pNode);
//node compare
typedef int(*cmp_node_callback)(NodePtr l, NodePtr r);
typedef int(*cmp_key_callback)(KeyPtr l, KeyPtr r);
//debug
#if(DEBUG_MODE)
typedef int(*print_node_callback)(NodePtr pNode, char* buff, int buffSize);
#endif
//+++++key
typedef struct avl_tree
{
    set_left_child_callback  _set_left;
    get_left_child_callback  _get_left;
    set_right_child_callback _set_right;
    get_right_child_callback _get_right;

    set_status_callback _set_status;
    get_status_callback _get_status;
    get_key_callback    _get_key;

    cmp_node_callback   _cmp_node;
    cmp_key_callback    _cmp_key;
#if(DEBUG_MODE)
    print_node_callback _print_node;
#endif
} AVLtree;

//+++++tree methods
int tree_traverse(AVLtree* pTree, NodePtr);
int tree_size(AVLtree* pTree, NodePtr pCur);
int tree_search_key(AVLtree* pTree, NodePtr pRoot, KeyPtr pKey, NodePtr* ppFound, NodePtr* ppParent);
int tree_search_node(AVLtree* pTree, NodePtr pRoot, NodePtr pNode, NodePtr* ppParent);
void tree_insert(AVLtree* pTree, NodePtr pRoot, NodePtr pNode);
NodePtr tree_min(AVLtree* pTree, NodePtr pRoot);
NodePtr tree_replace(AVLtree* pTree, NodePtr pParent, NodePtr pNode, NodePtr pNew);
NodePtr tree_remove(AVLtree* pTree, NodePtr pRoot, NodePtr pNode);

//++++++avltree methods
NodePtr AVLtree_single_rotate_left(AVLtree* pTree, NodePtr pParent);
NodePtr AVLtree_single_rotate_right(AVLtree* pTree, NodePtr pParent);
NodePtr AVLtree_special_rotate_left(AVLtree* pTree, NodePtr pParent);
NodePtr AVLtree_special_rotate_right(AVLtree* pTree, NodePtr pParent);
NodePtr AVLtree_double_rotate_left(AVLtree* pTree, NodePtr pParent);
NodePtr AVLtree_double_rotate_right(AVLtree* pTree, NodePtr pParent);
NodePtr AVLtree_update_left(AVLtree* pTree, NodePtr pParent, int *pHeightChg);
NodePtr AVLtree_update_right(AVLtree* pTree, NodePtr pParent, int *pHeightChg);
NodePtr AVLtree_insert(AVLtree* pTree, NodePtr pRoot, NodePtr pNode, int* pHeightChg);
NodePtr AVLtree_insert_left(AVLtree* pTree, NodePtr pRoot, NodePtr pNode, int* pHeightChg);
NodePtr AVLtree_insert_right(AVLtree* pTree, NodePtr pRoot, NodePtr pNode, int* pHeightChg);
NodePtr AVLtree_remove_left(AVLtree* pTree, NodePtr pRoot, NodePtr pNode, int* pHeightChg);
NodePtr AVLtree_remove_right(AVLtree* pTree, NodePtr pRoot, NodePtr pNode, int* pHeightChg);
NodePtr AVLtree_remove(AVLtree* pTree, NodePtr pRoot, NodePtr pNode, int* pHeightChg);
//++traverse
#if(DEBUG_MODE)
int AVLtree_traverse(AVLtree* pTree, NodePtr pRoot, int level);
#else
#define AVLtree_traverse(...)    0
#endif
//+++++avl tree key int
NodePtr crt_node_int(int key);
void del_node_int(NodePtr pNode);
NodePtr iAVL_insert(NodePtr pRoot, NodePtr pNew);
NodePtr iAVL_remove(NodePtr pRoot, NodePtr pNode);
NodePtr iAVL_search_key(NodePtr pRoot, int key);
int iAVL_search_node(NodePtr pRoot, NodePtr pNode);
//++traverse
#if(DEBUG_MODE)
void iAVLtraverse(NodePtr pRoot);
#else
#define iAVLtraverse(...)
#endif
//+++++avl tree key string
NodePtr crt_node_str(char* zKey);
void del_node_str(NodePtr pNode);
NodePtr zAVL_insert(NodePtr pRoot, NodePtr pNew);
NodePtr zAVL_remove(NodePtr pRoot, NodePtr pNode);
NodePtr zAVL_search_key(NodePtr pRoot, char* zKey);
int zAVL_search_node(NodePtr pRoot, NodePtr pNode);
//++traverse
#if(DEBUG_MODE)
void zAVLtraverse(NodePtr pRoot);
#else
#define zAVLtraverse(...)
#endif
#endif  //AVLtree
