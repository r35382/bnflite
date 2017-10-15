#include "byte_code.h"
#include "math.h"

static int cnt;

static int Tst0()
{
    return cnt++;
}

static int Tst(int a)
{
    return a + cnt++ % 4;
}

static int Pow(int a, int b)
{
    int res = 1;
    while (b-- > 0)
        res *= a;
    return res;
}

template <typename T, typename V, typename W>
T TPow(V v, W w) { return (T)powf((float)v, (float)w); }


struct FuncTable GFunTable[] = {
    { opNop, "Error", { opNop, opNop },  0, 0, 0 },
    { opInt, "TST",  { opNop, opNop, opNop },  (void*)Tst0, 0, 0 },
    { opInt, "TST",  { opInt, opNop, opNop },  (void*)Tst, 0, 0 },
    { opInt, "POW", { opInt, opInt, opNop },  (void*)Pow, 0, 0 }, // static example
    { opFloat, "POW",   { opFloat, opInt, opNop },  (void*)(void(*)())(&TPow <float, float, int>), 0, 0  }, // template example
    { opFloat, "POW",   { opInt, opFloat, opNop },  (void*)(void(*)())(&TPow <float, int, float>), 0, 0  }, // template example
#if 1
    { opFloat, "POW",   { opFloat, opFloat, opNop },  (void*)(void(*)())(&TPow <float, float, float>), 0, 0  }, // template example
#else
    { opFloat, "POW",  { opFloat, opFloat, opNop },   (void*)&[](float a, float b)->float {return powf(a, b);}  }, // lambda example
#endif
};

size_t GFunTableSize = sizeof(GFunTable) / sizeof(GFunTable[0]);



