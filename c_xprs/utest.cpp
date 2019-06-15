/*************************************************************************\
*   Unit test of C expression parser and calculator lib(based on BNFlite) *
*   Copyright (c) 2018 by Alexander A. Semjonov.  ALL RIGHTS RESERVED.    *
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


#include <ostream>
#include <iostream>
#include "c_xprs.h"


C_Xprs gramma;

#define TEST_FORCE_ERR(a) test_c_xprs(#a, -1); 
#define TEST_C_XPRS(a) test_c_xprs(#a, (a)); 

static void test_c_xprs(const char *expression, int test_result)
{
    int value;
    if (gramma.Evaluate(expression, value))
        std::cout << "Passed: " << expression << " is " <<  value <<"; Test: "
                << test_result <<  (value == test_result?" OK": "  Error") <<  "\n";
    else if (test_result == -1)
        std::cout << "Passed with expected error, expression: " << expression << "\n";
    else
        std::cout << "Not Passed: problem with expression: " << expression << "\n";
}


int main()
{
	TEST_FORCE_ERR(1 + variable =); 


    TEST_C_XPRS(0XFFFf);

    TEST_C_XPRS(!(0));

    TEST_C_XPRS(2+0x11);
   
    TEST_C_XPRS((2&&1)==!(!2||!1));
    TEST_C_XPRS((3&0xE)==~(~3|~0Xe));


    TEST_C_XPRS(1+(0? 2: 3)-1);
    TEST_C_XPRS(2+0xF);
    TEST_C_XPRS(1+(0? 2: 3)-1);
    TEST_C_XPRS(2+3-0x4+5);
    TEST_C_XPRS(((5+1)*4)+1);

    TEST_C_XPRS( 1 + (0&&3) + (3|4) * (1-1<0?5:6)-4<<8-7 );  

    return 0;
}



