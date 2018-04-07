/*************************************************************************\
*   Simple arithmetic calculator (based on BNFlite)                       *
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

#include "bnflite.h"
#include <ostream>
#include <iostream>

using namespace bnf;


typedef Interface<double> Calc;

/* pass result without brackets */
Calc DoBracket(std::vector<Calc>& res)
{
    return *res[0].text == '('? res[1] : res[0];
}

/* pass number result */
Calc DoNumber(std::vector<Calc>& res)
{
    char* lst;
    double value = strtod(res[0].text, &lst); // formaly it is not correct for not 0-terminated strings
    Calc ret(value, res);
    if (lst - res[0].text - ret.length != 0) {
        std::cout << "number parse error:"; std::cout.write(ret.text, ret.length);
    }
    return ret;
}

/* pass rersult of unary operation ( just only '-' ) */
Calc DoUnary(std::vector<Calc>& res)
{
    if (*res[0].text == '-') return Calc(-res[1].data, res);
    else if (*res[0].text == '+' ) return Calc(res[1].data, res);
    else return Calc(res[0].data, res);
}


/* pass result of binary operation (shared for several rules) */
Calc DoBinary(std::vector<Calc>& res)
{
    double value = res[0].data;
    for (unsigned int i = 1; i <  res.size(); i += 2) {
        switch(*res[i].text) {
            case '+': value += res[i + 1].data; break;
            case '-': value -= res[i + 1].data; break;
            case '*': value *= res[i + 1].data; break;
            case '/': value /= res[i + 1].data; break;
            case '%': value = (int)value % (int)res[i + 1].data; break;
            default:  std::cout << "syntax error:"; std::cout.write(res[i].text, res[i].length);
        }
    }
    return Calc(value, res);
}



int main(int argc, char* argv[])
{
    std::string clc;
    for (int i = 1; i < argc; i++) {
        clc += argv[i];
    }
/* Example of ABNF notation of the number (from RFC 4627)
        number = [ minus ] int [ frac ] [ exp ]
        decimal-point = %x2E       ; .
        digit1-9 = %x31-39         ; 1-9
        e = %x65 / %x45            ; e E
        exp = e [ minus / plus ] 1*DIGIT
        frac = decimal-point 1*DIGIT
        int = zero / ( digit1-9 *DIGIT )
        minus = %x2D               ; -
        plus = %x2B                ; +
        zero = %x30                ; 0
*/
    Token digit1_9('1', '9');
    Token DIGIT("0123456789");
    Lexem I_DIGIT = 1*DIGIT; // Series(1, DIGIT);
    Lexem frac = "." + I_DIGIT;
    Lexem int_ = "0" | digit1_9  + *DIGIT; //Series(0, DIGIT);
#if __cplusplus > 199711L
    Lexem exp = "Ee" + !"+-"_T + I_DIGIT ;
#else
    Lexem exp = "Ee" + !Token("+-") + I_DIGIT ;
#endif
    Rule number = !Token("-") + int_ + !frac + !exp;
    Bind(number, DoNumber);

    Rule Expression;
    Bind(Expression, Calc::ByPass);

    Rule PrimaryExpression = "(" + Expression + ")" | number;
#if __cplusplus > 199711L
    PrimaryExpression[
         *[](std::vector<Calc>& res) { return *res[0].text == '('? res[1] : res[0]; }
    ];
#else
     Bind(PrimaryExpression, DoBracket);
#endif

    Rule UnaryExpression = !Token("+-") + PrimaryExpression;
    Bind(UnaryExpression, DoUnary);

    Rule MulExpression = UnaryExpression + *("*%/" + UnaryExpression);
    Bind(MulExpression, DoBinary);

    Rule AddExpression = MulExpression + *("+-" + MulExpression );
    Bind(AddExpression, DoBinary);

    Expression = AddExpression; // Rule recursion created!

    if (!clc.size()) {
        clc = "2*3*4*5";
        std::cout << "Run \"" << argv[0] << " " << clc << "\"\n";
    }
    const char* tail = 0;
    Calc result;

    int tst = Analyze(Expression, clc.c_str(), &tail, result);
 	if (tst > 0)
		std::cout <<"Result of " << clc << " = " << result.data << std::endl;
	else
        std::cout << "Parsing errors detected, status = " << std::hex << tst << std::endl
         << "stopped at: " << tail << std::endl;

    Expression = Null();  // disjoin Rule recursion to safe Rules removal
    return 0;
}
