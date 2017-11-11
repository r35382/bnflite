/****************************************************************************\
*   Heqader of byte-code formula compiler (based on BNFLite)                 *
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
#ifndef _BYTE_CODE_H
#define _BYTE_CODE_H

#include <string>
#include <vector>
#include <list>
#include <functional>
#include <algorithm>
#include <iostream>

enum OpCode
{
    opFatal = -1, opNop = 0,
    opInt = 1,  opFloat = 2,  opStr = 3,  opMaskType = 0x03,
    opError = 1, opNeg = 2, opPos = 3, opCall = 4,
    opToInt = 5,  opToFloat = 6, opToStr = 7,
    opAdd = 2,  opSub = 3,  opMul = 4,  opDiv = 5,
};

#define OP3(scd, fst, op)  (OpCode) ( ((op) << 4) | ((fst) << 2) | ((scd) << 0) )
#define OP2(fst, op) (OpCode)( ((op) << 2) | ((fst) << 0)  )
#define OP1(op) (OpCode)(op)

#define CP2(l, op, r) ( (((l) & 0xF) << 16) | (((r) & 0xF) << 12) | ((op) & 0x7F) )

#define MAX_PARAM_NUM 3
#define PRM(r, p1, p2, p3) ( (((p3) & 3) << 6) | (((p2) & 3) << 4) | (((p1) & 3) << 2) | ((r) & 3) )

struct byte_code
{
    OpCode type;
    union {
        int val_i;
        float val_f;
        const char* val_s;
    };

    byte_code(): type(opNop), val_i(0) {};
    byte_code(OpCode t, int val = 0) : type(t), val_i(val) {};
    byte_code(OpCode t, float val) : type(t), val_f(val) {};
    byte_code(OpCode t, const char* val) : type(t), val_s(val) {};

    friend std::ostream& operator<<(std::ostream& out, const byte_code& bc);
    static char pType(int a)
        { return a > 1? a > 2?'S':'F' : a < 1?'?':'I'; }
    static int toType(int type)
    {   switch (type) {
        case OP3(opInt, opFloat, opAdd):
        case OP3(opInt, opFloat, opSub):
        case OP3(opInt, opFloat, opMul):
        case OP3(opInt, opFloat, opDiv):
            return opFloat;
        default:
            return type & opMaskType; }
    }
};

typedef std::list<byte_code> script;
extern std::list<byte_code> GenCallOp(std::string name, std::vector<std::list<byte_code> > args);
extern std::list<byte_code> GenUnaryOp(char op, std::list<byte_code> unr);
extern std::list<byte_code> GenBinaryOp(std::list<byte_code> left, char op, std::list<byte_code> right);

struct FuncTable
{
    OpCode ret;
    const char* name;
    OpCode param[MAX_PARAM_NUM];
    //void (*fun)();
    void *fun;
    int num;
    int call_idx;
};

extern struct FuncTable GFunTable[];
extern size_t GFunTableSize;

std::list<byte_code> spirit_byte_code(std::string expr);
std::list<byte_code> bnflite_byte_code(std::string expr);
int EvaluateBC(std::list<byte_code> bc, void* res);

#endif //_BYTE_CODE_H
