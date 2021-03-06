
#include "avltree.h"
#include <crtdbg.h>
#define assert _ASSERT
//#include <assert.h>
#include <stdlib.h>

int main(void)
{
    enum {nNode = 101};
    NodePtr arr[nNode];
    int i = 0;
    //init
    arr[0] = crt_node_int(0);
    NodePtr pRoot = arr[0];
    //crt data
    for (i = 1; i < nNode; i++) {
        arr[i] = crt_node_int(i);
        pRoot = iAVL_insert(pRoot, arr[i]);
    }

    for (i = 0; i < nNode; i++) {
        NodePtr pFound = iAVL_search_key(pRoot, i);
        assert(pFound != NULL);
        int iFound = iAVL_search_node(pRoot, arr[i]);
        assert(iFound);
    }

    iAVLtraverse(pRoot);

    for (i = 0; i < nNode; i++) {
        pRoot = iAVL_remove(pRoot, arr[i]);
        NodePtr pFound = iAVL_ge_key(pRoot, i);
        del_node_int(arr[i]);
    }

    //---------test node key string
    //init
    arr[0] = crt_node_str("0");
    char buff[16];
    pRoot = arr[0];
    //crt data
    for (i = 1; i < nNode; i++) {
        _itoa_s(i, buff, sizeof(buff)/sizeof(buff[0]), 10);
        arr[i] = crt_node_str(buff);
        pRoot = zAVL_insert(pRoot, arr[i]);
    }

    for (i = 0; i < nNode; i++) {
        _itoa_s(i, buff, 4, 10);
        NodePtr pFound = zAVL_search_key(pRoot, buff);
        assert(pFound != NULL);
        int iFound = zAVL_search_node(pRoot, arr[i]);
        assert(iFound);
    }

    zAVLtraverse(pRoot);

    for (i = 0; i < nNode; i++) {
        del_node_str(arr[i]);
    }

    return 0;
}