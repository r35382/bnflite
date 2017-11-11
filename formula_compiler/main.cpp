/****************************************************************************\
*   Starter of byte-code formula compiler (based on BNFLite)                 *
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


int main(int argc, char* argv[])
{
    std::string expression;
    for (int i = 1; i < argc; i++) {
        expression += argv[i];
    }
    if (!expression.size()) {
        std::cout <<  "No expression\n"
        "Use integer/float numbers and functions to make arithmetic expression\n"
        "Available functions are:\n";
        for (size_t i = 1; i < GFunTableSize; i++) {
            const char* types[] = {"?", "Int", "Float", "String"};
            std::cout << "   " << GFunTable[i].name; 
            char dlim = '('; size_t j = 0;
            for (; j < MAX_PARAM_NUM && GFunTable[i].param[j]; j++, dlim = ',') 
                std::cout << dlim << types[GFunTable[i].param[j] & opMaskType];
            std::cout << ");\n";
        }
        return 1;
    }

    std::list<byte_code> bl = bnflite_byte_code(expression);

    std::cout  <<  "Byte-code: ";
    for (std::list<byte_code>::iterator itr = bl.begin(); itr != bl.end(); ++itr)
            std::cout << *itr <<  (itr != (++bl.rbegin()).base()? ",": ";\n");

    union { int val_i;  float val_f; } res[4] = {0};
    int err = EvaluateBC(bl, res);
    if (err || !bl.size())
        std::cout << "running error: "  << err << std::endl;
    else {
        std::cout << "result = ";
        for (size_t i = 0; i < sizeof(res)/sizeof(res[0]); i++) 
            std::cout  << 
                (byte_code::toType(bl.back().type) == opInt? (float)res[i].val_i: res[i].val_f)
                       << (i < sizeof(res)/sizeof(res[0]) - 1? ", ": ";\n");
    }
    return err;
}






