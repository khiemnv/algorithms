
#include "avltree.h"
#include <crtdbg.h>
#define assert _ASSERT
//#include <assert.h>
#include <stdlib.h>

int main()
{
    enum {nNode = 10};
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

    for (i = 0; i < nNode; i++) {
        free(arr[i]);
    }

    //---------test node key string
    //init
    arr[0] = crt_node_str("0");
    char buff[4];
    pRoot = arr[0];
    //crt data
    for (i = 1; i < nNode; i++) {
        _itoa_s(i, buff, 4, 10);
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

    for (i = 0; i < nNode; i++) {
        del_node_str(arr[i]);
    }

    return 0;
}