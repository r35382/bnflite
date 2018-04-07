/*************************************************************************\
*   Parser of simple command line parameters (based on BNFlite)           *
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
#include "stdlib.h"
#include "bnflite.h"

using namespace bnf;


static bool printFilter(const char* lexem, size_t len)
{
    printf("Filter : %.*s;\n", len, lexem);
	return true;
}

int main(int argc, char* argv[])
{
/*
    BNF description of the ffmpeg filtergraph syntax 
    from https://ffmpeg.org/ffmpeg-filters.html (simplified for this example)
        NAME             ::= sequence of alphanumeric characters and '_'
        LINKLABEL        ::= "[" NAME "]"
        LINKLABELS       ::= LINKLABEL [LINKLABELS]
        FILTER_ARGUMENTS ::= sequence of chars (possibly quoted)
        FILTER           ::= [LINKLABELS] NAME ["=" FILTER_ARGUMENTS] [LINKLABELS]
        FILTERCHAIN      ::= FILTER [,FILTERCHAIN]
*/
	Token Alphanumeric('_');    // start declare one element of "sequence of alphanumeric characters"
	Alphanumeric.Add('0', '9'); // appended numeric part
	Alphanumeric.Add('a', 'z'); // appended alphabetic lowercase part
	Alphanumeric.Add('A', 'Z'); // appended alphabetic capital part
    Lexem NAME = Series(1, Alphanumeric); // declare "sequence of alphanumeric characters"
    Lexem LINKLABEL = "[" + NAME + "]";
    Lexem LINKLABELS1  =  Iterate(1, LINKLABEL); // declare as described
    Lexem LINKLABELS0  =  Iterate(0, LINKLABEL); // declare as needed to use
    Token  SequenceOfChars(' ' + 1, 0x7F - 1); // declare one element of "sequence of chars"
    SequenceOfChars.Remove("=,");  // exclude used(reserved) chars
    Lexem FILTER_ARGUMENTS = Series(1, SequenceOfChars); // declare "sequence of chars"
    Lexem FILTER = LINKLABELS0 + NAME + Iterate(0, "=" + FILTER_ARGUMENTS) + LINKLABELS0;
    Rule Filter = FILTER + printFilter;  // form found filter
    Rule FILTERCHAIN = Filter + Repeat(0, "," + Filter); // declare several filters

    const char* test; int stat;
    const char* pstr = 0;
    test = "[0]amerge=0=5, c1";
	stat = Analyze(FILTERCHAIN, test, &pstr); // Start parsing 
 	if (stat > 0)
		printf("Passed\n");
	else
        printf("Failed, stopped at=%.40s\n status = 0x%0X,  flg = %s%s%s%s%s%s%s%s\n",
            pstr?pstr:"", stat,
            stat&eOk?"eOk":"Not",
            stat&eRest?", eRest":"",
            stat&eOver?", eOver":"",
            stat&eEof?", eEof":"",
            stat&eBadRule?", eBadRule":"",
            stat&eBadLexem?", eBadLexem":"",
            stat&eSyntax?", eSyntax":"",
            stat&eError?", eError":"" );
	return 0;
}

