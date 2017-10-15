#include "byte_code.h"


std::list<byte_code> GenUnaryOp(char op, std::list<byte_code> unr)
{
    if (op == '-')
        unr.push_back(byte_code(OP2(byte_code::toType(unr.back().type), opNeg)));
    return unr;
}

std::list<byte_code> GenBinaryOp(std::list<byte_code> left, char op, std::list<byte_code> right)
{
    static struct
    {
        int issue;
        OpCode bin;
        OpCode left;
        OpCode right;
    } bin_op[] = {
    {CP2(opInt, '+', opInt), OP3(opInt, opInt, opAdd), opNop, opNop},
    {CP2(opInt, '-', opInt), OP3(opInt, opInt, opSub), opNop, opNop},
    {CP2(opInt, '*', opInt), OP3(opInt, opInt, opMul), opNop, opNop},
    {CP2(opInt, '/', opInt), OP3(opInt, opInt, opDiv), opNop, opNop},
    {CP2(opInt, '+', opFloat), OP3(opFloat, opFloat, opAdd), OP2(opInt, opToFloat), opNop},
    {CP2(opInt, '-', opFloat), OP3(opFloat, opFloat, opSub), OP2(opInt, opToFloat), opNop},
    {CP2(opInt, '*', opFloat), OP3(opFloat, opFloat, opMul), OP2(opInt, opToFloat), opNop},
    {CP2(opInt, '/', opFloat), OP3(opFloat, opFloat, opDiv), OP2(opInt, opToFloat), opNop},
    {CP2(opFloat, '+', opInt), OP3(opFloat, opFloat, opAdd), opNop, OP2(opInt, opToFloat)},
    {CP2(opFloat, '-', opInt), OP3(opFloat, opFloat, opSub), opNop, OP2(opInt, opToFloat)},
    {CP2(opFloat, '*', opInt), OP3(opFloat, opFloat, opMul), opNop, OP2(opInt, opToFloat)},
    {CP2(opFloat, '/', opInt), OP3(opFloat, opFloat, opDiv), opNop, OP2(opInt, opToFloat)},
    {CP2(opFloat, '+', opFloat), OP3(opFloat, opFloat, opAdd), opNop, opNop},
    {CP2(opFloat, '-', opFloat), OP3(opFloat, opFloat, opSub), opNop, opNop},
    {CP2(opFloat, '*', opFloat), OP3(opFloat, opFloat, opMul), opNop, opNop},
    {CP2(opFloat, '/', opFloat), OP3(opFloat, opFloat, opDiv), opNop, opNop}
    };

    int issue = CP2(byte_code::toType(left.back().type), op, byte_code::toType(right.back().type));

    for (unsigned int i = 0; i < sizeof(bin_op)/sizeof(bin_op[0]); i++) {
        if (issue == bin_op[i].issue) {
            if (bin_op[i].left)
                left.push_back(byte_code(bin_op[i].left));
            if (bin_op[i].right)
                right.push_back(byte_code(bin_op[i].right));
            right.push_back(byte_code(bin_op[i].bin));
            break;
        }
    }
    left.splice(left.end(), right);
    return left;
}


std::list<byte_code> GenCallOp(std::string name, std::vector<std::list<byte_code> > args)
{
    unsigned int i, j;
    std::list<byte_code> all;

    for (i = 0; i < GFunTableSize; i++) {
        if (name != GFunTable[i].name)
            continue;
        if (args.size() > MAX_PARAM_NUM)
            continue;
        unsigned int lim = args.size() < MAX_PARAM_NUM ? args.size(): MAX_PARAM_NUM;
        for (j = 0; j < lim; j++) {
            if (GFunTable[i].param[j] != byte_code::toType(args[j].back().type))
                break;
        }
        if (j < lim || (j < MAX_PARAM_NUM && GFunTable[i].param[j] != 0))
            continue;

        for (j = 0; j < lim; j++) {
            all.splice(all.end(), args[j]);
        }
        all.push_back(byte_code(OP2(GFunTable[i].ret & opMaskType, opCall), (signed)i));
        GFunTable[i].num = j;
        GFunTable[i].call_idx = PRM(GFunTable[i].ret,
            GFunTable[i].param[0], GFunTable[i].param[1], GFunTable[i].param[2]);
        return all;
    }
    all.push_back(byte_code(OP2(opInt, opError), (signed)i));
    return all;
}

std::ostream& operator<<(std::ostream& out, const byte_code& bc)
{
    switch (bc.type) {
    case opInt:  out << "Int(" << bc.val_i << ")"; break;
    case opFloat:  out <<  "Float(" << bc.val_f << ")"; break;
    case opStr:  out <<  "Str(" << bc.val_s << ")"; break;
    default:
        switch (bc.type >> 2) {
            case opError: out << "opError<"; break;
            case opNeg: out << "opNeg<"; break;
            case opCall: out << "opCall<"; break;
            case opToInt: out << "opToInt<"; break;
            case opToFloat: out << "opToFloat<"; break;
            case opToStr: out << "opToStr<"; break;
            default:
                switch (bc.type >> 4) {
                    case opAdd: out << "opAdd<"; break;
                    case opSub: out << "opSub<"; break;
                    case opMul: out << "opMul<"; break;
                    case opDiv: out << "opDiv<"; break;
                    default: out << "Error<"; break;
                }
                out << byte_code::pType((bc.type >>2) & opMaskType) << ',';
        }
        out << byte_code::pType(bc.type & opMaskType) << '>';
    }
    return out;
}
