#include "..\flatRm\ringmem.h"

#include <assert.h>
#include <string.h>

int main()
{
    enum {
        buffSize = 256,
    };
    char buff[buffSize];
    union
    {
        void* handle;
        RingMem *prm;
    };
    handle = buff;
    prm->Reset(buff, buffSize, RingMem::RingMemAddrModeRelative);
    assert(prm->BufferSize() == (buffSize - RingMem::AlignedModuleSize()));

    int count;
    prm->Write("abc", 4, &count);
    assert(count == 4);

    char *p;
    prm->Locate(0, &p, 5, &count);
    assert(count == 4);
    assert(strcmp("abc", p) == 0);
    assert(p == (buff + RingMem::AlignedModuleSize()));

    return 0;
}