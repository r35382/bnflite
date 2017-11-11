/*************************************************************************\
*   Parser part of  bite-code formula compiler (based on BNFlite)         *
*   Copyright (c) 2017 by Alexander A. Semjonov.  ALL RIGHTS RESERVED.    *
*                                                                         *
*   This code is free software: you can redistribute it and/or modify it  *
*   under the terms of the GNU Lesser General Public License as published *
*   by the Free Software Foundation, either version 3 of the License,     *
*   or (at your option) any later version.                                *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
\*************************************************************************/

#include "stdio.h"
#include "string.h"

#include "bnflite.h"
#include "byte_code.h"

using namespace bnf;

static bool printErr(const char* lexem, size_t len)
{
    printf("strings are not supported yet: %.*s;\n", len, lexem);
    return false;
}

typedef Interface< std::list<byte_code> > Gen;


Gen DoBracket(std::vector<Gen>& res)
{
    return *res[0].text == '('? res[1] : res[0]; /* pass result without brackets */
}

Gen DoString(std::vector<Gen>& res)
{
    return res[1];
}


Gen DoNumber(std::vector<Gen>& res)
{   /* use strtol/strtod to get value of parsed number */
    char* lst;
	int j = res.size() - 1;
    int ivalue = strtol(res[0].text, &lst, 10); 
    if (lst - res[j].text - res[j].length == 0) {
		return Gen(std::list<byte_code>(1, byte_code(opInt, ivalue)), res);
	}
    float fvalue = (float)strtod(res[0].text, &lst); 
    if (lst - res[j].text - res[j].length == 0) {
		return Gen(std::list<byte_code>(1, byte_code(opFloat, fvalue)), res);
	}
    std::cout << "number parse error:"; std::cout.write(res[0].text, res[0].length);
    return  Gen(std::list<byte_code>(1, byte_code(opError, 0)), res);

}

Gen DoUnary(std::vector<Gen>& res)
{   /* pass result of unary operation ( just only '-' ) */
    if (*res[0].text == '-') {
        return Gen(GenUnaryOp('-', res[1].data), res);
    }
    return res[0];
}

Gen DoBinary(std::vector<Gen>& res)
{   /* pass result of binary operation (shared for several rules) */
    std::list<byte_code> left = res[0].data;
    for (unsigned int i = 1; i <  ((res.size() - 1) | 1); i += 2) {
        left = GenBinaryOp(left, *res[i].text, res[i + 1].data);
    }
    return Gen(left, res);
}

Gen DoFunction(std::vector<Gen>& res)
{
    std::vector< std::list<byte_code> > args;

    for (unsigned int i = 1; i <  res.size(); i++) {
        if( *res[i].text == '(' ||  *res[i].text == ',' ||  *res[i].text == ')' ) {
            continue;
        }
        args.push_back(res[i].data);
    }
    return Gen(GenCallOp(std::string(res[0].text, res[0].length), args), res);
}


std::list<byte_code> bnflite_byte_code(std::string expr)
{
    Token digit1_9('1', '9');
    Token DIGIT("0123456789");
    Lexem i_digit = 1*DIGIT;
    Lexem frac_ = "." + i_digit;
    Lexem int_ = "0" | digit1_9  + *DIGIT;
    Lexem exp_ = "Ee" + !Token("+-") + i_digit;
    Lexem number_ = !Token("-") + int_ + !frac_ + !exp_;
    Rule number = number_;

    Token az_("_"); az_.Add('A', 'Z'); az_.Add('a', 'z');
    Token az01_(az_); az01_.Add('0', '9');

    Token all(1,255); all.Remove("\"");

    Lexem identifier = az_  + *(az01_);
    Lexem quotedstring = "\"" + *all + "\"";

    Rule expression;
    Rule unary;

    Rule function = identifier + "(" + !(expression + *("," + expression)) +  ")";

    Rule elementary = AcceptFirst()
            | "(" + expression + ")"
            | function
            | number
            | quotedstring + printErr
            | unary;

    unary = Token("-") + elementary;

    Rule primary = elementary + *("*%/" + elementary);

    /* Rule */ expression = primary + *("+-" + primary);

    Bind(number, DoNumber);
    Bind(elementary, DoBracket);
    Bind(unary, DoUnary);
    Bind(primary, DoBinary);
    Bind(expression, DoBinary);
    Bind(function, DoFunction);

    const char* tail = 0;
    Gen result;

    int tst = Analyze(expression, expr.c_str(), &tail, result);
    if (tst > 0)
        std::cout << result.data.size() << " byte-codes in: " << expr << std::endl;
    else
        std::cout << "Parsing errors detected, status = " << std::hex << tst << std::endl
         << "stopped at: " << tail << std::endl;

    expression = Null();  // disjoin Rule recursion to safe Rules removal
    unary = Null();
    return result.data;
}
