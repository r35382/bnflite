/****************************************************************************\
*   Byte-code interpreter(SSE2) of formula compiler (based on BNFLite)       *
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


#include <immintrin.h>
#include <emmintrin.h>

int EvaluateBC(std::list<byte_code> bc, void* res)
{
    __m128   X[16];
    int i = -1;

	for (const auto &code : bc) {
        switch (code.type) {
        default:
            return -2;
        case OP2(opInt, opError):
        case OP2(opFloat, opError):
            return -1;

        case OP1(opInt):
            X[++i] = _mm_castsi128_ps(_mm_set1_epi32(code.val_i));
            break;
       case OP1(opFloat):
            X[++i] = _mm_set_ps1(code.val_f);
            break;
        case  OP2(opInt, opNeg):
            X[i] = _mm_castsi128_ps(_mm_sub_epi32(_mm_set1_epi32(0), _mm_castps_si128(X[i])));
            break;
        case  OP2(opFloat, opNeg):
            X[i] = _mm_mul_ps(X[i], _mm_set_ps1(-1L));
            break;

        case OP3(opInt, opInt, opAdd):
            X[i - 1] = _mm_castsi128_ps(_mm_sub_epi32( _mm_castps_si128(X[i - 1]),
                                    _mm_sub_epi32(_mm_set1_epi32(0), _mm_castps_si128(X[i]))));
            i -= 1; break;
        case OP3(opFloat, opFloat, opAdd):
            X[i - 1] = _mm_add_ps(X[i - 1], X[i]);
            i -= 1; break;
        case OP3(opInt, opInt, opSub):
            X[i - 1] = _mm_castsi128_ps(_mm_sub_epi32(_mm_castps_si128(X[i - 1]), _mm_castps_si128(X[i])));
            i -= 1; break;
        case OP3(opFloat, opFloat, opSub):
            X[i - 1] = _mm_sub_ps(X[i - 1], X[i]);
            i -= 1;  break;
        case OP3(opInt, opInt, opMul):
            X[i - 1] = _mm_castsi128_ps(_mm_cvtps_epi32(_mm_mul_ps(
                _mm_cvtepi32_ps(_mm_castps_si128(X[i - 1])),_mm_cvtepi32_ps(_mm_castps_si128(X[i])))));
            i -= 1; break;
        case OP3(opFloat, opFloat, opMul):
            X[i - 1] = _mm_mul_ps(X[i - 1], X[i]);
            i -= 1; break;
        case OP3(opInt, opInt, opDiv):
            X[i - 1] = _mm_castsi128_ps(_mm_cvtps_epi32(_mm_div_ps(
                _mm_cvtepi32_ps(_mm_castps_si128(X[i - 1])),_mm_cvtepi32_ps(_mm_castps_si128(X[i])))));
            i -= 1; break;
        case OP3(opFloat, opFloat, opDiv):
            X[i - 1] = _mm_div_ps(X[i - 1], X[i]);
            i -= 1; break;

        case OP2(opInt, opToFloat):
            X[i] = _mm_cvtepi32_ps(_mm_castps_si128(X[i]));
            break;
        case OP2(opFloat, opToInt):
            X[i] = _mm_castsi128_ps(_mm_cvtps_epi32(X[i]));
            break;

        case OP2(opInt, opCall):
        case OP2(opFloat, opCall):
        case OP2(opStr, opCall):
            // __m128  res;
            int j = code.val_i;
            void (*f)() = (void(*)())GFunTable[j].fun;
            i = i - GFunTable[j].num + 1;
            int (&iX)[][4] =(int(&)[][4])(X[i]);
            float (&fX)[][4] =(float(&)[][4])(X[i]);

            for (int l = 0; l < 4; l++) {
                switch (GFunTable[j].call_idx) {
                case PRM(opInt, 0, 0, 0):
                    iX[0][l] = ((int(*)())(f))();
                    break;
                case PRM(opInt, opInt, 0, 0):
                    iX[0][l] = ((int(*)(int))(f))(iX[0][l]);
                    break;
                case PRM(opFloat, opFloat, 0, 0):
                    fX[0][l] = ((float(*)(float))(f))(fX[0][l]);
                    break;
                case PRM(opInt, opInt, opInt, 0):
                    iX[0][l] = ((int(*)(int, int))(f))(iX[0][l], iX[1][l]);
                    break;
                case PRM(opFloat, opFloat, opInt, 0):
                    fX[0][l] = ((float(*)(float, int))(f))(fX[0][l], iX[1][l]);
                    break;
                case PRM(opFloat, opInt, opFloat, 0):
                    fX[0][l] = ((float(*)(int, float))(f))(iX[0][l],fX[1][l]);
                    break;
                case PRM(opFloat, opFloat, opFloat, 0):
                    fX[0][l] = ((float(*)(float, float))(f))(fX[0][l], fX[1][l]);
                    break;
                default:
                    return  -3;
                }
            }
            break;
        }
    }
    *(__m128*)res  =  X[i];
    return i;
}


