/****************************************************************************\
*   Embedded functions for byte-code formula compiler (based on BNFLite)     *
*   Copyright (c) 2017  Alexander A. Semjonov <alexander.as0@mail.ru>        *
*                                                                            *
*   Permission to use, copy, modify, and distribute this software for any    *
*   purpose with or without fee is hereby granted, provided that the above   *
*   copyright notice and this permission notice appear in all copies.        *
*                                                                            *
*   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES *
*   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         *
*   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  *
*   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   *
*   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    *
*   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  *
*   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           *
\****************************************************************************/
#include "byte_code.h"
#include "math.h"


static int GetX()
{
    static int cnt;
    return cnt++;
}

static int Series(int a)
{
    static int cnt;
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
    { opInt, "GetX",  { opNop, opNop, opNop },  (void*)GetX, 0, 0 },
    { opInt, "Series",  { opInt, opNop, opNop },  (void*)Series, 0, 0 },
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



